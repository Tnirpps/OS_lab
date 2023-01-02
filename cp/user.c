#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

#define COLOR_WHITE_BOLD_UNDERLINED  "\e[1;04m"
#define COLOR_GREEN  "\033[1;32m"
#define COLOR_ORANGE  "\033[1;33m"
#define COLOR_MAGENTA  "\033[1;35m"
#define COLOR_OFF   "\e[m"

#define check_ok(VALUE, OK_VAL, MSG) if (VALUE != OK_VAL) { printf("%s", MSG); return 1; }
#define check_wrong(VALUE, WRONG_VAL, MSG) if (VALUE == WRONG_VAL) { printf("%s", MSG); return 1; }

const char* COLORS_PRINT[6] = {COLOR_OFF, "", COLOR_WHITE_BOLD_UNDERLINED, COLOR_ORANGE, COLOR_MAGENTA, COLOR_GREEN};


const char* SHARED_SERVER_NAME = "shared_SeRvEr";
const char* SHARED_MUTEX_NAME  = "shared_mutex";
const char* SHARED_COND_NAME   = "shared_cond";
const int MAX_U_COUNT = 100;


const int MSG_LEN_LIMIT = 128;
const int NICK_MAX_LEN = 8;
const char SERVER = 1;
const char EXE = 2;
const int SHARED_MEMORY_SIZE = 1 + 2 * NICK_MAX_LEN + 1 + MSG_LEN_LIMIT;

typedef struct {
    char* me;
    char* shared;
    pthread_mutex_t* mutex;
    pthread_cond_t* cond;
} Token;


void print(const char* msg, unsigned color) {
    if (color >= 6) {
        printf("%s", msg);
    } else {
        printf("%s", COLORS_PRINT[color]);
        printf("%s", msg);
        printf("%s", COLORS_PRINT[0]);
    }
}

int is_for_me(const char* shared, const char* me) {
    for (int i = 0; i < NICK_MAX_LEN; ++i) {
        if (shared[i] != me[i]) {
            return 0;
        }
    }
    return 1;
}

void* pooling(void* arg) {
    Token* token = ((Token*) arg);
    printf("start pooling\n");
    while (1) {
        pthread_mutex_lock(token->mutex);
        while (is_for_me(token->shared + 1, token->me) != 1) {
            pthread_cond_wait(token->cond, token->mutex);
        }
        printf("[%s] %s", token->shared + NICK_MAX_LEN + 1, token->shared + 2 * NICK_MAX_LEN + 2);
        memset(token->shared, 0, SHARED_MEMORY_SIZE);
        token->shared[2 * NICK_MAX_LEN + 1] = 99; // make it empty
        pthread_cond_signal(token->cond);
        pthread_mutex_unlock(token->mutex);
    }
    return NULL;
}

void start_pooling(const Token* token) {
    pthread_t th;
    if (pthread_create(&th, NULL, &pooling, (void*)token) != 0) {
        printf("cannot create thread\n");
        return;
    }
}

int parse(char* dest, char* buff) {
    if (buff[0] != '[') return -1;
    int i = 1;
    for (; buff[i] != ']'; ++i) {
        if (i > NICK_MAX_LEN) return -1;
        dest[i - 1] = buff[i];
    }
    for (int j = i; j <= NICK_MAX_LEN; ++j) {
        dest[j - 1] = 0;
    }
    return (i + 2);
}


int main(int argc, char** argv) {
    /* Shared file */
    int fd = shm_open(SHARED_SERVER_NAME, O_RDWR, S_IRWXU);
    check_wrong(fd, -1, "Error opening shared file in child process!\n")
    char* sharedFile = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    check_wrong(sharedFile, MAP_FAILED, "Error mapping shared file in child process!\n")
    /* Shared mutex */
    int fdMutex = shm_open(SHARED_MUTEX_NAME, O_RDWR, S_IRWXU);
    check_wrong(fdMutex, -1, "Error opening shared mutex file in child process!\n")
    pthread_mutex_t* mutex = mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, fdMutex, 0);
    check_wrong(mutex, MAP_FAILED, "Error mapping shared mutex file in child process!\n")
    /* Shared cond */
    int fdCond = shm_open(SHARED_COND_NAME, O_RDWR, S_IRWXU);
    check_wrong(fdCond, -1, "Error opening shared cond file in child process!\n")
    pthread_cond_t* condition = mmap(NULL, sizeof(pthread_cond_t), PROT_READ | PROT_WRITE, MAP_SHARED, fdCond, 0);
    check_wrong(condition, MAP_FAILED, "Error mapping shared cond file in child process!\n")

    // have to auth
    int auth = 0;
    char me[NICK_MAX_LEN];
    memset(me, 0, NICK_MAX_LEN);
    while (auth == 0) {
        printf("Enter User name (no longer than %d chars): ", NICK_MAX_LEN - 1);
        fgets(me, NICK_MAX_LEN, stdin);
        // if line longer than NICK_MAX_LEN
        if (me[NICK_MAX_LEN - 2] != '\0') {
            me[NICK_MAX_LEN - 2] = '\n';
            while (getc(stdin) != '\n');
        }
        // remove \n
        for (int i = NICK_MAX_LEN - 1; i >= 0; --i) {
            if (me[i] == '\n') {
                me[i] = '\0';
                break;
            }
        }
        // send query
        check_ok(pthread_mutex_lock(mutex), 0, "Error locking mutex in child!\n")
        printf("wait for empty....\n");
        while (sharedFile[2 * NICK_MAX_LEN + 1] != 99 || sharedFile[0] == EXE) {
            check_ok(pthread_cond_wait(condition, mutex), 0, "Error waiting cond in child!\n")
        }
        printf("sent\n");
        memset(sharedFile + 1, 0, NICK_MAX_LEN); // can be removed
        memcpy(sharedFile + NICK_MAX_LEN + 1, me, NICK_MAX_LEN);
        sharedFile[2 * NICK_MAX_LEN + 1] = 0;
        sharedFile[0] = SERVER;
        for (int i = 0; i < 10; ++i) {
            check_ok(pthread_cond_signal(condition), 0, "Error sending signal in child!\n")
        }
        check_ok(pthread_mutex_unlock(mutex), 0, "Error unlocking mutex in child!\n")

        check_ok(pthread_mutex_lock(mutex), 0, "Error locking mutex in child!\n")
        printf("wait for anwer....\n");
        while (is_for_me(sharedFile + 1, me) != 1) {
            check_ok(pthread_cond_wait(condition, mutex), 0, "Error waiting cond in child!\n")
        }
        auth = sharedFile[2 * NICK_MAX_LEN + 1];
        sharedFile[2 * NICK_MAX_LEN + 1] = 99; // make it empty
        memset(sharedFile, 0, 2 * NICK_MAX_LEN + 1);
        for (int i = 0; i < 10; ++i) {
            check_ok(pthread_cond_signal(condition), 0, "Error sending signal in child!\n")
        }
        check_ok(pthread_mutex_unlock(mutex), 0, "Error unlocking mutex in child!\n")
        if (auth == 1) {
            print("SUCCESS!\n", 5);
            break;
        } else {
            /*
             * http://uc.org.ru/node/200
             */
            printf("\033[1;4;31m");
            printf("FAIL=(\n");
            printf("%s", COLOR_OFF);
        }
    }
    Token token = {me, sharedFile, mutex, condition};
    start_pooling(&token);
    print("Now you can send msg like this:\n   [Vasya01] Hello, how are you??\n   or [!] to exit\n", 1);

    // sending msg
    char buff [NICK_MAX_LEN + 2 + MSG_LEN_LIMIT];
    char dest[NICK_MAX_LEN];
    char poster[NICK_MAX_LEN];
    // buff = "[NICK] msg" so + 2
    while (fgets(buff, NICK_MAX_LEN + 2 + MSG_LEN_LIMIT, stdin)) {
        if (buff[1] == '!') {
            break;
        }
        int start_msg = parse(dest, buff);
        if (start_msg == -1) {
            print("cannot parse your query\n", 3);
            continue;
        }
        check_ok(pthread_mutex_lock(mutex), 0, "Error locking mutex in child!\n")
        printf("wait for empty....\n");
        while (sharedFile[2 * NICK_MAX_LEN + 1] != 99 || sharedFile[0] == EXE) {
            check_ok(pthread_cond_wait(condition, mutex), 0, "Error waiting cond in child!\n")
        }
        printf("sent\n");
        memcpy(sharedFile + 1, dest, NICK_MAX_LEN);
        memcpy(sharedFile + NICK_MAX_LEN + 1, me, NICK_MAX_LEN);
        memcpy(sharedFile + 2 * NICK_MAX_LEN + 2, buff + start_msg, MSG_LEN_LIMIT);
        sharedFile[2 * NICK_MAX_LEN + 1] = 1;
        sharedFile[0] = SERVER;
        for (int i = 0; i < 10; ++i) {
            check_ok(pthread_cond_signal(condition), 0, "Error sending signal in child!\n")
        }
        check_ok(pthread_mutex_unlock(mutex), 0, "Error unlocking mutex in child!\n")
        memset(buff, 0, NICK_MAX_LEN + 2 + MSG_LEN_LIMIT);
    }
}
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

#define check_ok(VALUE, OK_VAL, MSG) if (VALUE != OK_VAL) { printf("%s", MSG); return 1; }
#define check_wrong(VALUE, WRONG_VAL, MSG) if (VALUE == WRONG_VAL) { printf("%s", MSG); return 1; }

/* Ubuntu has 255 symbol filename limit */
const unsigned int FILENAME_LIMIT = 255;
const unsigned int SHARED_MEMORY_SIZE = 1;
const int child_count = 2;

const char* SHARED_FILE_NAME[] = {"1_shared_file", "2_shared_file"};
const char* SHARED_MUTEX_NAME[] = {"1_shared_mutex","2_shared_mutex"};
const char* SHARED_COND_NAME[] = {"1_shared_cond", "2_shared_cond"};


int write_to_process(char* sharedFile, pthread_mutex_t* mutex, pthread_cond_t* condition,  const char c) {
    check_ok(pthread_mutex_lock(mutex), 0, "Error locking mutex in parent!\n")
    while (sharedFile[0] != 0) {
        check_ok(pthread_cond_wait(condition, mutex), 0, "Error waiting cond in child!\n")
    }
    sharedFile[0] = c;
    check_ok(pthread_cond_signal(condition), 0, "Error sending signal in child!\n")
    check_ok(pthread_mutex_unlock(mutex), 0, "Error unlocking mutex in child!\n")
    return 0;
}


int main() {
    assert(child_count <3);
    char file[FILENAME_LIMIT + 1];
    int output[child_count];
    for (int i = 0; i < child_count; ++i) {
        memset(file, 0, FILENAME_LIMIT + 1);
        if (fgets(file, (int) FILENAME_LIMIT, stdin) == NULL) {
            printf("Error reading file name!\n");
            return 1;
        }
        file[strcspn(file, "\n")] = '\0';
        output[i] = open(file, O_CREAT | O_RDWR, 0777);
        check_wrong(output[i], -1, "Error opening output file!\n")
    }


    int fd[child_count];
    int fdMutex[child_count];
    int fdCond[child_count];
    pthread_mutex_t* mutex[child_count];
    pthread_cond_t* condition[child_count];

    pthread_mutexattr_t mutex_attribute;
    check_ok(pthread_mutexattr_init(&mutex_attribute), 0, "Error initializing mutex attribute!\n")
    check_ok(pthread_mutexattr_setpshared(&mutex_attribute, PTHREAD_PROCESS_SHARED), 0, "Error sharing mutex attribute!\n")

    pthread_condattr_t condition_attribute;
    check_ok(pthread_condattr_init(&condition_attribute), 0, "Error initializing cond attribute!\n")
    check_ok(pthread_condattr_setpshared(&condition_attribute, PTHREAD_PROCESS_SHARED), 0, "Error sharing cond attribute!\n")

    for (int i = 0; i < child_count; ++i) {
        /* Shared file */
        fd[i] = shm_open(SHARED_FILE_NAME[i], O_RDWR | O_CREAT, S_IRWXU);
        check_wrong(fd[i], -1, "Error creating shared file!\n")
        check_ok(ftruncate(fd[i], SHARED_MEMORY_SIZE), 0, "Error truncating shared file!\n")
        /* Shared mutex */
        fdMutex[i] = shm_open(SHARED_MUTEX_NAME[i], O_RDWR | O_CREAT, S_IRWXU);
        check_ok(ftruncate(fdMutex[i], sizeof(pthread_mutex_t)), 0, "Error creating shared mutex file!\n")

        mutex[i] = (pthread_mutex_t*) mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, fdMutex[i], 0);
        check_wrong(mutex[i], MAP_FAILED, "Error mapping shared mutex!\n")
        check_ok(pthread_mutex_init(mutex[i], &mutex_attribute), 0, "Error initializing mutex!\n")

        /* Shared cond */
        fdCond[i] = shm_open(SHARED_COND_NAME[i], O_RDWR | O_CREAT, S_IRWXU);
        check_ok(ftruncate(fdCond[i], sizeof(pthread_cond_t)), 0, "Error creating shared cond file!\n")

        condition[i] = (pthread_cond_t*) mmap(NULL, sizeof(pthread_cond_t), PROT_READ | PROT_WRITE, MAP_SHARED, fdCond[i], 0);
        check_ok(pthread_cond_init(condition[i], &condition_attribute), 0, "Error initializing cond!\n")

    }
    check_ok(pthread_mutexattr_destroy(&mutex_attribute), 0, "Error destoying mutex attribute!\n")

    check_ok(pthread_condattr_destroy(&condition_attribute), 0, "Error destoying cond attribute!\n")


    /* Creating child process */
    int id_1 = fork();
    check_wrong(id_1, -1, "Error creating the first process!\n")
    if (id_1 == 0) {
        check_wrong(dup2(output[0], STDOUT_FILENO), -1, "Dup2 error\n")
        char *Child1_argv[] = {"child_1", (char*) SHARED_FILE_NAME[0], (char*) SHARED_MUTEX_NAME[0], (char*) SHARED_COND_NAME[0], NULL};
        check_wrong(execv("child", Child1_argv), -1, "Error executing child process!\n")
    } else {
        int id_2 = fork();
        check_wrong(id_2, -1, "Error creating the second process!\n")
        if (id_2 == 0) {
            check_wrong(dup2(output[1], STDOUT_FILENO), -1, "Dup2 error\n")
            char *Child2_argv[] = {"child_2", (char*) SHARED_FILE_NAME[1], (char*) SHARED_MUTEX_NAME[1], (char*) SHARED_COND_NAME[1], NULL};
            check_wrong(execv("child", Child2_argv), -1, "Error executing child process!\n")
        } else {
            char* sharedFile[child_count];
            for (int i = 0; i < child_count; ++i) {
                sharedFile[i] = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd[i], 0);
                check_wrong(sharedFile[i], MAP_FAILED, "Error creating shared file!")
                sharedFile[i][0] = 0;
            }
            char c;
            int proc_index = 0;
            while (scanf("%c", &c) > 0) {
                if (proc_index != 0 && proc_index != 1) {
                    perror("Code error, it's a bug\n");
                    return -1;
                }
                check_ok(write_to_process(sharedFile[proc_index], mutex[proc_index], condition[proc_index], c), 0, "")
                if (c == '\n') proc_index ^= 1;
            }
            for (int i = 0; i < child_count; ++i) {
                check_ok(pthread_mutex_lock(mutex[i]), 0, "Error locking mutex in child!\n")
                while (sharedFile[i][0] != 0) {
                    check_ok(pthread_cond_wait(condition[i], mutex[i]), 0, "Error waiting cond in child!\n")
                }
                sharedFile[i][0] = -1;
                check_ok(pthread_cond_signal(condition[i]), 0, "Error sending signal in child!\n")
                check_ok(pthread_mutex_unlock(mutex[i]), 0, "Error unlocking mutex in child!\n")
                check_wrong(munmap(sharedFile[i], SHARED_MEMORY_SIZE), -1, "Error unmapping fd1!")
            }
        }

    }
    for (int i = 0; i < child_count; ++i) {
        check_ok(munmap(mutex[i], sizeof(pthread_mutex_t)), 0, "Error unmapping mutex!\n")
        check_ok(munmap(condition[i], sizeof(pthread_cond_t)), 0, "Error unmapping cond!\n")

        check_wrong(shm_unlink(SHARED_FILE_NAME[i]), -1, "Error unlinking shared file!\n")
        check_wrong(shm_unlink(SHARED_MUTEX_NAME[i]), -1, "Error unlinking shared mutex file!\n")
        check_wrong(shm_unlink(SHARED_COND_NAME[i]), -1, "Error unlinking shared cond file!\n")

        check_ok(close(output[i]), 0, "Error closing input file!\n")
    }

    return 0;
}

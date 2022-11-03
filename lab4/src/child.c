#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>


#define check_ok(VALUE, OK_VAL, MSG) if (VALUE != OK_VAL) { printf("%s", MSG); return 1; }
#define check_wrong(VALUE, WRONG_VAL, MSG) if (VALUE == WRONG_VAL) { printf("%s", MSG); return 1; }


int is_vowel(const char c) {
    return (int) (
            c == 'a' ||
            c == 'e' ||
            c == 'u' ||
            c == 'o' ||
            c == 'i');
}

int main(int argc, char** argv) {
    if (argc < 4) {
        perror("Not enough arguments in child process\n");
        return 1;
    }
    /* Shared file */
    int fd = shm_open(argv[1], O_RDWR, S_IRWXU);
    check_wrong(fd, -1, "Error opening shared file in child process!\n")
    struct stat statbuf;
    check_wrong(fstat(fd, &statbuf), -1, "Error getting shared file size in child!\n")
    char* sharedFile = mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    check_wrong(sharedFile, MAP_FAILED, "Error mapping shared file in child process!\n")
    /* Shared mutex */
    int fdMutex = shm_open(argv[2], O_RDWR, S_IRWXU);
    check_wrong(fdMutex, -1, "Error opening shared mutex file in child process!\n")
    pthread_mutex_t* mutex = mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, fdMutex, 0);
    check_wrong(mutex, MAP_FAILED, "Error mapping shared mutex file in child process!\n")
    /* Shared cond */
    int fdCond = shm_open(argv[3], O_RDWR, S_IRWXU);
    check_wrong(fdCond, -1, "Error opening shared cond file in child process!\n")
    pthread_cond_t* condition = mmap(NULL, sizeof(pthread_cond_t), PROT_READ | PROT_WRITE, MAP_SHARED, fdCond, 0);
    check_wrong(condition, MAP_FAILED, "Error mapping shared cond file in child process!\n")
    while (1) {
        check_ok(pthread_mutex_lock(mutex), 0, "Error locking mutex in child!\n")
        while (sharedFile[0] == 0) {
            check_ok(pthread_cond_wait(condition, mutex), 0, "Error waiting cond in child!\n")
        }
        if (sharedFile[0] == -1) {
            check_ok(pthread_mutex_unlock(mutex), 0, "Error unlocking mutex in child!\n")
            break;
        }
        if (!is_vowel(sharedFile[0])) {
            printf("%c", sharedFile[0]);
        }
        sharedFile[0] = 0;
        check_ok(pthread_cond_signal(condition), 0, "Error sending signal in child!\n")
        check_ok(pthread_mutex_unlock(mutex), 0, "Error unlocking mutex in child!\n")
    }
    check_ok(pthread_mutex_destroy(mutex), 0, "Error destroying mutex!\n")
    check_ok(pthread_cond_destroy(condition), 0, "Error destroying cond!\n")

    check_wrong(munmap(mutex, sizeof(pthread_mutex_t)), -1, "Error unmapping shared mutex file in child!")
    check_wrong(munmap(condition, sizeof(pthread_cond_t)), -1, "Error unmapping shared cond file in child!")
    check_wrong(munmap(sharedFile, statbuf.st_size), -1, "Error unmapping shared file in child!")
    return 0;
}

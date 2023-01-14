#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define WRITE_END 1
#define READ_END 0

int send_error_and_stop(const char* massage, int code) {
    fprintf(stderr, "%s\n", massage);
    exit(code);
}

int main(int argc, const char *argv[]) {
    // check that we have all necessary arguments
    if (argc < 4) {
        send_error_and_stop("arguments missing", 1);
    }
    // fd to file, read and write from argv[1], argv[2] and argv[3]
    int fd;
    int fd1[2];
    // transform strings to int
    fd = atoi(argv[1]);
    fd1[0] = atoi(argv[2]);
    fd1[1] = atoi(argv[3]);

    if (dup2(fd, STDOUT_FILENO) == -1) {
        send_error_and_stop("cannot do dup2", 2);
    }

    int  len;  // ans to parent process (0 = stop)
    char* str;
    int error_code = -1;
    while(1) {

        if (read(fd1[READ_END], &len, sizeof (int)) == -1) {
            send_error_and_stop("cannot read from file", 3);
        }
        if (len < 2) break;
        str = malloc(sizeof(char) * len);
        if (str == NULL) {
            send_error_and_stop("cannot allocate memory", 4);
        }
        if (read(fd1[READ_END], str, sizeof (char) * len) == -1) {
            send_error_and_stop("cannot read from file", 5);
        }

        // string: .....[last][\0] index of last = len - 2
        if (str[len - 2] == '.' || str[len - 2] == ';') {
            if (printf("%s\n", str) == -1) {
                send_error_and_stop("cannot write to file", 6);
            }

            if (write(fd1[WRITE_END], &len, sizeof(int)) == -1) {
                send_error_and_stop("cannot write to fd", 7);
            }

        } else {
            if (write(fd1[WRITE_END], &error_code, sizeof(int)) == -1) {
                send_error_and_stop("cannot write to fd", 8);
            }
        }
        free(str);
    }
    if (close(fd1[READ_END]) == -1 || close(fd1[WRITE_END]) == -1 || close(fd) == -1) {
        send_error_and_stop("cannot close fd", 9);
    }
    return 0;
}


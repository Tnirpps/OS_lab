#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>

#define WRITE_END 1
#define READ_END 0

int send_error_and_stop(const char* massage, int code) {
    fprintf(stderr, "%s\n", massage);
    exit(code);
}

int read_line(char** ptr) {
    if (ptr == NULL) {
        return -1;
    }
    int capacity = 10;
    (*ptr) = malloc(sizeof (char) * capacity);
    // cannot malloc memory
    if ((*ptr) == NULL) {
        return -1;
    }
    char* new_ptr; // tmp pointer to expand string
    int c;
    int i = 0;     //  string size
    while ((c = getchar()) != EOF && c!='\n') {
        (*ptr)[i++] = (char) c;
        // expend string
        if (i >= capacity / 2) {
            new_ptr = realloc((*ptr), sizeof (char) * capacity * 2);
            // cannot malloc new memory
            if (new_ptr == NULL) {
                return -1;
            }
            (*ptr) = new_ptr;
            capacity *= 2;
        }

    }
    (*ptr)[i++] = '\0';
    return i;
}

int main(int argc, const char* argv[]) {
    char* FileName;
    if (read_line(&FileName) == -1) {
        send_error_and_stop("cannot read file name to open", 1);
    }
    int fd1[2];
    int fd2[2];
    int p1 = pipe(fd1);
    int p2 = pipe(fd2);
    if (p1 == -1 || p2 == -1) {
        send_error_and_stop("pipe error", 1);
    }
    int fd;

    // 0777 means that everyone has rights to write/read and so on
    fd = open(FileName, O_WRONLY | O_CREAT, 0777);

    // clear memory
    free(FileName);
    FileName = NULL;

    if (fd == -1) {
        send_error_and_stop("cannot open file", 2);
    }
    int pid = fork();
    if (pid == -1) {
        send_error_and_stop("fork() error", 3);
    } else if (pid == 0) {
// =================  Child  ================= //
        // close fd that we won't use
        if (close(fd1[WRITE_END]) == -1 || close(fd2[READ_END]) == -1) {
            send_error_and_stop("cannot close fd", 4);
        }

        // is strings to store (int) fd so 10 characters would be enough
        char fd_file [10];
        char fd_read [10];
        char fd_write [10];

        // transform int to str
        sprintf(fd_file, "%d", fd);
        sprintf(fd_read, "%d", fd1[READ_END]);
        sprintf(fd_write, "%d", fd2[WRITE_END]);

        // args for child program
        char* Child_args[] = {"child", fd_file, fd_read, fd_write, NULL};

        if (execv("child", Child_args) == -1) {
            send_error_and_stop("cannot call exec child", 5);
        }
    } else {
// =================  parent ================= //
        // close fd that we won't use
        if (close(fd1[READ_END]) == -1 || close(fd2[WRITE_END]) == -1) {
            send_error_and_stop("cannot close fd", 6);
        }

        int len;             // length of input line
        int child_answer;    // user input
        char* line;
        while (1) {
            len = read_line(&line);
            /*
             * в условии варианта не указан
             * критерий завершения програмы
             * поэтому програма остановится,
             * когда получит на вход строку
             * из трёх символов:
             * STP
             */
            if (len == 4 
                && line[0] == 'S'
                && line[1] == 'T'
                && line[2] == 'P') {
                len = 0;
            }
            if (write(fd1[WRITE_END], &len, sizeof(int)) == -1) {
                send_error_and_stop("cannot write to fd", 7);
            }
            if (len < 2) {
                free(line);
                break;
            }
            if (write(fd1[WRITE_END], line, sizeof(char) * len) == -1) {
                send_error_and_stop("cannot write to fd", 7);
            }
            if (read(fd2[READ_END], &child_answer, sizeof(int)) == -1) {
                send_error_and_stop("cannot read from fd", 8);
            }
            if (child_answer == -1) {
                printf("%s has no \';\' or \'.\' in the end\n", line);
            }
            free(line);
        }

        // close all fd because we stop to write/read at all
        if (close(fd1[WRITE_END]) == -1 || close(fd2[READ_END]) == -1 || close(fd) == -1) {
            send_error_and_stop("cannot close fd", 9);
        }

        // wait for child process end
        waitpid(pid, NULL, 0);
    }
    return 0;
}


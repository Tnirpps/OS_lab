#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define WRITE_END 1
#define READ_END 0

int main(int argc, const char *argv[]) {
    char *Child1_argv[] = {"child_1", NULL};
    char *Child2_argv[] = {"child_2", NULL};
    int fd1[2];
    int fd2[2];
    int p1 = pipe(fd1);
    int p2 = pipe(fd2);
    if (p1 == -1 || p2 == -1) {
        fprintf(stderr, "%s", "Pipe() error\n");
        return 1;
    }
    int id1 = fork();
    if (id1 == -1) {
        fprintf(stderr, "%s", "Fork() error\n");
        return 1;
    } else if (id1 == 0) {
// =================  Child 1 ================= //
        if (dup2(fd1[READ_END], STDIN_FILENO) == -1) {
            fprintf(stderr, "%s", "dup2 error\n");
            return 1;
        }
        if (close(fd1[WRITE_END]) == -1 ) {
            fprintf(stderr, "%s", "Cannot close fd\n");
            return 1;
        }
        if (execv("child_1", Child1_argv) == -1) {
            fprintf(stderr, "%s", "Cannot call exec child_1\n");
            return 1;
        }
    } else {
        int id2 = fork();
        if (id2 == -1) {
            fprintf(stderr, "%s", "Fork() error\n");
            return 1;
        } else if (id2 == 0) {
// =================  Child 2 ================= //
            if (dup2(fd2[READ_END], STDIN_FILENO) == -1) {
                fprintf(stderr, "%s", "dup2 error\n");
                return 1;
            }
            if (close(fd2[WRITE_END]) == -1) {
                fprintf(stderr, "%s", "Cannot close fd\n");
                return 1;
            }
            if (execv("child_1", Child2_argv) == -1) {
                fprintf(stderr, "%s", "Cannot call exec child_2\n");
                return 1;
            }
        } else {
// =================  parent ================= //
            if (close(fd1[READ_END]) == -1 || close(fd2[READ_END]) == -1) {
                fprintf(stderr, "%s", "Cannot close fd\n");
                return 3;
            }
            char c;
            // write filename to child 1
            while ((c = getchar()) != EOF) {
                if (write(fd1[WRITE_END], &c, sizeof (char)) == -1) {
                    fprintf(stderr, "%s", "Cannot write to fd1\n");
                    return 3;
                } // check error
                if (c == '\n') break;
            }
            // write filename to child 2
            while ((c = getchar()) != EOF) {
                if (write(fd2[WRITE_END], &c, sizeof (char)) == -1) {
                    fprintf(stderr, "%s", "Cannot write to fd1\n");
                    return 3;
                } // check error
                if (c == '\n') break;
            }

            int str_ind = 0; // even or odd line number flag

            while ((c = getchar()) != EOF) {
                switch (str_ind) {
                    case 0:
                        if (write(fd1[WRITE_END], &c, sizeof (char)) == -1) {
                            fprintf(stderr, "%s", "Cannot write to fd1\n");
                            return 3;
                        } // check error
                        if (c == '\n') {
                            str_ind ^= 1;
                        }
                        break;
                    case 1:
                        if (write(fd2[WRITE_END], &c, sizeof (char)) == -1) {
                            fprintf(stderr, "%s", "Cannot write to fd1\n");
                            return 3;
                        } // check error
                        if (c == '\n') {
                            str_ind ^= 1;
                        }
                        break;
                    default:
                        fprintf(stderr, "%s", "I don't know what has happened\n");
                        return 1;
                }
            }
            // we need to define the end of lines to make read easier
            c = '\0';
            if (write(fd1[WRITE_END], &c, sizeof (char)) == -1) {
                fprintf(stderr, "%s", "Cannot write to fd1\n");
                return 3;
            }
            if (write(fd2[WRITE_END], &c, sizeof (char)) == -1) {
                fprintf(stderr, "%s", "Cannot write to fd1\n");
                return 3;
            }

            if (close(fd1[WRITE_END]) == -1) {
                fprintf(stderr, "%s", "Cannot close fd\n");
                return 1;
            }
            if (close(fd2[WRITE_END]) == -1) {
                fprintf(stderr, "%s", "Cannot close fd\n");
                return 1;
            }

            // wait for all child process ends
            waitpid(-1, NULL, 0);
        }
    }
    return 0;
}

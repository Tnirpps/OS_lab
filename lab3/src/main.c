#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct TThreadToken {
    double* R;
    double* step;
    double start;
    int Cpoints;
} ThreadToken;

void exit_with_msg(const char* msg, int return_code);
void* integral(void* arg);
int min(int a, int b);

int main(int argc, const char** argv) {
    int Total_points = 10000000;
    double R, Total_Aria_Size;
    int CountThreads;
    if (argc < 2) {
        exit_with_msg("missing arguments", -1);
    }
    for (int i = 0; argv[1][i] > 0; ++i) {
        if (argv[1][i] >= '0' && argv[1][i] <= '9') {
            CountThreads = CountThreads * 10 + argv[1][i] - '0';
        }
    }
    printf("enter Radius: ");
    scanf("%lf", &R);
    Total_Aria_Size = R * 2;
    pthread_t* th = malloc(sizeof (pthread_t) * CountThreads);
    ThreadToken* token = malloc(sizeof(ThreadToken) * CountThreads);
    double start = -R;
    double step = (Total_Aria_Size / (double ) CountThreads);
    int Cpoints = (Total_points + CountThreads - 1)/CountThreads;
    for (int i = 0; i < CountThreads; ++i) {
        token[i].start = start;
        token[i].step = &step;
        token[i].R = &R;
        token[i].Cpoints = min(Cpoints, Total_points - i*Cpoints);
        start += step;
    }

    for (int i = 0; i < CountThreads; ++i) {
        if (pthread_create(&th[i], NULL, &integral, &token[i]) != 0) {
            exit_with_msg("cannot create thread", 2);
        }
    }
    Cpoints = 0;
    for (int i = 0; i < CountThreads; ++i) {
        if (pthread_join(th[i], NULL) != 0) {
            exit_with_msg("cannot join threads", 3);
        }
        Cpoints += token[i].Cpoints;
    }
    printf("Exact answer is        : %.20lf\n", acos(-1)*R*R);
    printf("Answer is approximately: %.20lf\n",
           Total_Aria_Size*Total_Aria_Size*((double) Cpoints / (Total_points)));
    free(token);
    free(th);
    return 0;
}

int min(int a, int b) {
    if (a < b) return a;
    return b;
}

void exit_with_msg(const char* msg, int return_code) {
    printf("%s\n", msg);
    exit(return_code);
}

int in_circle(double x, double y, double R) {
    return (x*x + y*y <= R*R);
}

void* integral(void* arg) {
    ThreadToken token = *((ThreadToken*) arg);
    double x, y, R;
    R = *token.R;
    int attempts = token.Cpoints;
    token.Cpoints = 0;
    for (int i = 0; i < attempts; ++i) {
        x = token.start + ((double )rand()/(double )(RAND_MAX)) * (*token.step);
        y = (((double )rand()/(double )(RAND_MAX)) - 0.5) * 2*R;
        if (in_circle(x,y, R)) {
            token.Cpoints++;
        }
    }
    ((ThreadToken*) arg)->Cpoints = token.Cpoints;
    return arg;
}






/*

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct TThreadToken {
    int istart;
    int istop;
    int m;
    int k;
    int* A;
    int* B;
    int* res;
} ThreadToken;

int min(int n, int i);
void exit_with_msg(const char* msg, int return_code);
int get_index(int i, int j, int m);
void* mlt(void* arg);

int main() {
    int n, m, k, CountThreads;
    scanf("%d", &CountThreads);
    //printf("Enter the matrix dimensions (3 numbers): ");
    scanf("%d %d %d", &n, &m, &k);

    pthread_t* th = malloc(sizeof (pthread_t) * CountThreads);
    ThreadToken* token = malloc(sizeof(ThreadToken) * CountThreads);
    int* A = malloc(sizeof(int) * n * m);
    int* B = malloc(sizeof(int) * m * k);
    int* res = malloc(sizeof(int) * n * k);

    if (A == NULL || B == NULL || res == NULL || th == NULL) {
        exit_with_msg("cannot malloc memory\n", 1);
    }
    int work = (n + CountThreads - 1) / CountThreads;   // division with rounding to top
    // check err
    for (int i = 0; i < CountThreads; ++i) {
        token[i].istart = i*work;
        token[i].istop = min(n, (i + 1) * work);
        token[i].m = m;
        token[i].k = k;
        token[i].A = A;
        token[i].B = B;
        token[i].res = res;
    }

    //printf("Enter the A(%dх%d) matrix: ", n, m);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            scanf("%d", &A[get_index(i, j, m)]);
        }
    }
    //printf("Enter the B(%dх%d) matrix: ", m, k);
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < k; ++j) {
            scanf("%d", &B[get_index(i, j, k)]);
        }
    }

    for (int i = 0; i < CountThreads; ++i) {
        if (pthread_create(&th[i], NULL, &mlt, &token[i]) != 0) {
            exit_with_msg("cannot create thread", 2);
        }
    }
    for (int i = 0; i < CountThreads; ++i) {
        if (pthread_join(th[i], NULL) != 0) {
            exit_with_msg("cannot join threads", 3);
        }
    }
    printf("%d %d\n", n, k);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < k; ++j) {
            printf("%d ", res[get_index(i, j, k)]);
        }
        printf("\n");
    }
    //printf("End\n");
    free(th);
    free(token);
    free(A);
    free(B);
    free(res);
    return 0;
}

int min(int a, int b) {
    if (a < b) return a;
    return b;
}

void exit_with_msg(const char* msg, int return_code) {
    printf("%s\n", msg);
    exit(return_code);
}

int get_index(int i, int j, int m) {
    return (i * m + j);
}

void* mlt(void* arg) {
    ThreadToken token = *((ThreadToken*) arg);
    for (int i = token.istart; i < token.istop; ++i) {
        for (int j = 0; j < token.k; ++j) {
            token.res[get_index(i, j, token.k)] = 0;
            for (int k = 0; k < token.m; ++k) {
                token.res[get_index(i, j, token.k)] +=
                        token.A[get_index(i, k, token.m)] * token.B[get_index(k,j,token.k)];
            }
        }
    }

    return arg;
}
*/
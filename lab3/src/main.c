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
    if (th == NULL || token == NULL) {
        exit_with_msg("can't allocate memory", 1);
    }
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

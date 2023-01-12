#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/*
Есть К массивов одинаковой длины. Необходимо сложить эти массивы.
Необходимо предусмотреть стратегию, адаптирующуюся под количество
массивов и их длину (по количеству операций)
*/

pthread_mutex_t mutex;

typedef struct {
    int** arr;
    int* res;
    int start;
    int steps;
    int K;
    int N;
} ThreadToken;

int min(int a, int b) {
    if (a < b) return a;
    return b;
}

void exit_with_msg(const char* msg, int return_code) {
    printf("%s\n", msg);
    exit(return_code);
}

void* vertical_sum_arrays(void* arg) {
    ThreadToken token = *((ThreadToken*) arg);
    // printf("(%d;%d)\n", token.start, token.start + token.steps);
    for (int i = token.start; i < token.start + token.steps; ++i) {
        int c = 0;
        for (int j = 0; j < token.K; ++j) {
            c += token.arr[j][i];
        }
        token.res[i] = c;
    }
    return arg;
}

void* horizontal_sum_arrays(void* arg) {
    ThreadToken token = *((ThreadToken*) arg);
    // printf("(%d;%d)\n", token.start, token.start + token.steps);
    for (int i = 0; i < token.N; ++i) {
        int c = 0;
        for (int j = token.start; j < token.start + token.steps; ++j) {
            c += token.arr[j][i];
        }
        pthread_mutex_lock(&mutex);
        token.res[i] += c;
        pthread_mutex_unlock(&mutex);
    }
    return arg;
}

int main(int argc, const char** argv) {
    int CountThreads = 0;
    if (argc < 2) {
        exit_with_msg("missing arguments", -1);
    }
    // str to int
    for (int i = 0; argv[1][i] > 0; ++i) {
        if (argv[1][i] >= '0' && argv[1][i] <= '9') {
            CountThreads = CountThreads * 10 + argv[1][i] - '0';
        }
    }
    int N, K;
    printf("length of arrays N: ");
    if (scanf("%d", &N) == EOF) {
        exit_with_msg("data cannot be read", 1);
    }
    printf("number of arrays K: ");
    if (scanf("%d", &K) == EOF) {
        exit_with_msg("data cannot be read", 1);
    }
    int** all = malloc(sizeof(int*) * K);
    if (all == NULL) {
        exit_with_msg("cannot allocate memory", -5);
    }
    for (int i = 0; i < K; ++i) {
        all[i] = malloc(sizeof(int) * N);
        if (all[i] == NULL) {
            for (int j = 0; j < i; ++j) {
                free(all[j]);
                all[j] = NULL;
            }
            free(all);
            all = NULL;
            exit_with_msg("cannot allocate memory", 1);
        }
        for (int j = 0; j < N; ++j) {
            if (scanf("%d", &all[i][j]) == EOF) {
                exit_with_msg("data cannot be read", 1);
            }
        }
    }

    void* (*function)(void*);
    int end;
    // some kind of ratio of N and K to choose sum way
    if (N > K * 2) {
        function = &vertical_sum_arrays;
        end = N;
        printf("vertical\n");
    } else {
        function = &horizontal_sum_arrays;
        end = K;
        printf("horizontal\n");
        // only in horizontal mode we need to use mutex
        if (pthread_mutex_init(&mutex, NULL) != 0) {
            exit_with_msg("cannot init mutex", 1);
        }
    }
    // if we have only 5 arrays and 10 threads?
    CountThreads = min(CountThreads, end);

    // create arrays of threads and tokens
    pthread_t* th = malloc(sizeof (pthread_t) * CountThreads);
    ThreadToken* token = malloc(sizeof(ThreadToken) * CountThreads);

    // result will be placed here
    int* result = malloc(sizeof(int) * N);

    if (th == NULL || token == NULL || result == NULL) {
        exit_with_msg("cannot allocate memory", 1);
    }
    // init result with 0 value
    for (int i = 0; i < N; ++i) {
        result[i] = 0;
    }
    // Start and End indexes for each thread
    int start = 0;
    int steps = (end + CountThreads - 1) / CountThreads;
    // fill token data for each thread
    for (int i = 0; i < CountThreads; ++i) {
        token[i].arr = all;
        token[i].res = result;
        token[i].start = start;
        token[i].K = K;
        token[i].N = N;
        token[i].steps = min(end - start, steps);
        start += steps;
    }

    // start threads
    for (int i = 0; i < CountThreads; ++i) {
        if (pthread_create(&th[i], NULL, function, &token[i]) != 0) {
            exit_with_msg("cannot create thread", 2);
        }
    }
    // join threads (wait for all of them to end calculations)
    for (int i = 0; i < CountThreads; ++i) {
        if (pthread_join(th[i], NULL) != 0) {
            exit_with_msg("cannot join threads", 3);
        }
    }
    for (int i = 0; i < N; ++i) {
        printf("%d ", result[i]);
    }
    printf("\n");
    for (int i = 0; i < K; ++i) {
        free(all[i]);
        all[i] = NULL;
    }
    if (end == K) {
        // if we really have init it
        pthread_mutex_destroy(&mutex);
    }
    free(all);
    free(token);
    free(th);
    free(result);
    return 0;
}

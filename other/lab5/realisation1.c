#include "realisation.h"

void swap_int(int* x, int* y) {
    int tmp = *x;
    *x = *y;
    *y = tmp;
}

int GCD(int x, int y) {
    while (y > 0) {
        if (x >= y) {
            x = x % y;
        }
        swap_int(&x, &y);
    }
    return x;
}

int isPrime(int n) {
    for(int i = 2; i < n; ++i) {
        if (n % i == 0)
            return 0;
    }
    return 1;
}

int PrimeCount(int A, int B) {
    int res = 0;
    // {all negative, 0, 1} aren't prime
    if (A < 2) A = 2;
    for (int i = A; i <= B; ++i) {
        res += isPrime(i);
        /* if <i> is prime res+=1
         * otherwise res+=0
         */
    }
    return res;
}
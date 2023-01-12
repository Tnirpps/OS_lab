#include "realisation.h"

void swap_int(int* x, int* y) {
    int tmp = *x;
    *x = *y;
    *y = tmp;
}

int GCD(int x, int y) {
    if (x > y) {
        swap_int(&x, &y);
    }
    for (int i = x; i > 1; --i) {
        if (x % i == 0 && y % i == 0) {
            return i;
        }
    }
    return 1;
}

int PrimeCount(int A, int B) {
    // {all negative, 0, 1} aren't prime
    if (A < 2) A = 2;
    int res = 0;
    // init sieve
    char sieve [SQRT_OF_MAXINT];
    for (int i = 0; i < SQRT_OF_MAXINT; ++i) sieve[i] = 1;

    // sieve of Eratosthenes algorithm
    long long primes[SQRT_OF_MAXINT];   // we needn't primes bigger than sqrt(2^32)
                                        // but we don't know how many they are,
                                        // so we create sqrt(2^32) array
                                        // that 100% will be enough
    int primes_count = 0;
    for(long long i = 2; i < SQRT_OF_MAXINT; ++i) {
        if (sieve[i] == 0) continue;
        primes[primes_count++] = i;
        for(long long j = i * i; j < SQRT_OF_MAXINT; j += i)
            sieve[j] = 0;
    }
    // iterate from A to B and check divisors among primes
    int flag;
    for (int i = A; i <= B; ++i) {
        flag = 1;
        // iterate all primes to sqrt(i)
        for (int j = 0; j < primes_count && primes[j]*primes[j] <= i; ++j) {
            if (i % primes[j] == 0) {
                // if it has divisor => not prime
                flag = 0;
                break;
            }
        }
        // if flag still equals 1 => <i> is prime => res+=1
        // otherwise res+=0
        res += flag;
    }
    return res;
}
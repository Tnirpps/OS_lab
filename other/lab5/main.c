#include "realisation.h"
#include <stdio.h>

#define check(VALUE, OKVAL, MSG) if (VALUE != OKVAL) { printf("%s", MSG); return 1; }

int main(int argc, const char** argv) {
    int q;
    printf("Enter query: 1) get GCD(x,y)  2) count primes between A, B\n");
    while (scanf("%d", &q) > 0) {
        if (q == 1) {
            int x, y;
            printf("enter integer x, y: ");
            check(scanf("%d%d", &x, &y), 2, "Error reading integer!\n")
            printf("GCD(%d, %d) = %d\n", x, y, GCD(x, y));
        } else if (q == 2) {
            int A, B;
            printf("enter integer A, B: ");
            check(scanf("%d %d", &A, &B), 2, "Error reading floats!\n")
            printf("There are %d primes between %d and %d\n", PrimeCount(A, B), A, B);
        } else {
            printf("End.\n");
            return 0;
        }
    }
}
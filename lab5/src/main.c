#include "../headers/realisation.h"
#include <stdio.h>

#define check(VALUE, OKVAL, MSG) if (VALUE != OKVAL) { printf("%s", MSG); return 1; }

int main(int argc, const char** argv) {
    int q;
    while (scanf("%d", &q) > 0) {
        if (q == 1) {
            int x, y;
            check(scanf("%d%d", &x, &y), 2, "Error reading integer!\n");
            printf("GCD(%d, %d) = %d\n", x, y, GCD(x, y));
        } else if (q == 2) {
            float A, B;
            check(scanf("%f %f", &A, &B), 2, "Error reading floats!\n");
            printf("Area is: %f\n", Square(A, B));
        } else {
            printf("End.\n");
            return 0;
        }
    }
}

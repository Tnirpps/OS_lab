#include "../headers/realisation.h"

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

float Square(float A, float B) {
    return A/2.f * B;
}

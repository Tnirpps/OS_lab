#include "../headers/realisation.h"

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

float Square(float A, float B) {
    return A * B;
}

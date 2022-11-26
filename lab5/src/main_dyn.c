#include <dlfcn.h>
#include <stdio.h>

#define check(VALUE, OKVAL, MSG) if (VALUE != OKVAL) { printf("%s", MSG); return 1; }
#define check_wrong(VALUE, WRONG_VAL, MSG) if (VALUE == WRONG_VAL) { printf("%s", MSG); return 1; }

// it is very important to make prefix "lib" and set extension ".so"
const char* DYN_LIB_1 = "./libDyn1.so";
const char* DYN_LIB_2 = "./libDyn2.so";

const char* GCD_NAME = "GCD";
const char* SQUARE_NAME = "Square";

int main(int argc, const char** argv) {
    int dynLibNum = 1;
    void* handle = dlopen(DYN_LIB_1, RTLD_LAZY);
    check_wrong(handle, NULL, "Error opening dynamic library!\n")
    int (*GCD)(int, int);
    float (*Square)(float, float);
    *(void**) (&GCD) = dlsym(handle, GCD_NAME);
    *(void**) (&Square) = dlsym(handle, SQUARE_NAME);
    char* error = dlerror();
    check(error, NULL, error)
    int q;
    int x, y;
    float A, B;
    while (scanf("%d", &q) > 0) {
        switch (q) {
            case 0:
                check(dlclose(handle), 0, "Error closing dynamic library!\n")
                if (dynLibNum) {
                    handle = dlopen(DYN_LIB_2, RTLD_LAZY);
                } else {
                    handle = dlopen(DYN_LIB_1, RTLD_LAZY);
                }
                check_wrong(handle, NULL, "Error opening dynamic library!\n")
                *(void**) (&GCD) = dlsym(handle, GCD_NAME);
                *(void**) (&Square) = dlsym(handle, SQUARE_NAME);
                error = dlerror();
                check(error, NULL, error)
                /* switch between 0 and 1 */
                dynLibNum = dynLibNum ^ 1;
                break;
            case 1:
                check(scanf("%d%d", &x, &y), 2, "Error reading integer!\n");
                printf("GCD(%d, %d) = %d\n", x, y, GCD(x, y));
                break;
            case 2:
                check(scanf("%f %f", &A, &B), 2, "Error reading floats!\n");
                printf("Area is: %f\n", Square(A, B));
                break;
            default:
                printf("End.\n");
                check(dlclose(handle), 0, "Error closing dynamic library!\n")
                return 0;
        }
    }
}
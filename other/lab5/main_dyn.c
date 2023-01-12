#include <dlfcn.h>
#include <stdio.h>

#define check(VALUE, OKVAL, MSG) if (VALUE != OKVAL) { printf("%s", MSG); return 1; }
#define check_wrong(VALUE, WRONG_VAL, MSG) if (VALUE == WRONG_VAL) { printf("%s", MSG); return 1; }

// it is very important to make prefix "lib" and set extension ".so"
const char* DYN_LIB_1 = "./libDyn1.so";
const char* DYN_LIB_2 = "./libDyn2.so";

const char* GCD_NAME = "GCD";
const char* PRIME_COUNTER_NAME = "PrimeCount";

int main(int argc, const char** argv) {
    int dynLibNum = 1;
    void* handle = dlopen(DYN_LIB_1, RTLD_LAZY);
    check_wrong(handle, NULL, "Error opening dynamic library!\n")
    int (*GCD)(int, int);
    int (*PrimeCount)(int, int);
    GCD = dlsym(handle, GCD_NAME);
    PrimeCount = dlsym(handle, PRIME_COUNTER_NAME);
    char* error = dlerror();
    check(error, NULL, error)
    int q;
    int x, y;
    int A, B;
    printf("Enter query: 0) change realisation  1) get GCD(x,y)  2) count primes between A, B\n");
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
                GCD = dlsym(handle, GCD_NAME);
                PrimeCount = dlsym(handle, PRIME_COUNTER_NAME);
                error = dlerror();
                check(error, NULL, error)
                /* switch between 0 and 1 */
                dynLibNum = dynLibNum ^ 1;
                break;
            case 1:
                printf("enter integer x, y: ");
                check(scanf("%d%d", &x, &y), 2, "Error reading integer!\n")
                printf("GCD(%d, %d) = %d\n", x, y, GCD(x, y));
                break;
            case 2:
                printf("enter integer A, B: ");
                check(scanf("%d %d", &A, &B), 2, "Error reading floats!\n")
                printf("There are %d primes between %d and %d\n", PrimeCount(A, B), A, B);
                break;
            default:
                printf("End.\n");
                check(dlclose(handle), 0, "Error closing dynamic library!\n")
                return 0;
        }
    }
}

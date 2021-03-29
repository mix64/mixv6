#include "lib/ulib.h"

int main(int argc, char *argv[])
{
    volatile unsigned int f2;
    for (int i = 0; i < 0x100000; i++) {
        volatile unsigned int f0 = 0, f1 = 1;
        while(f1 <0x80000000) {
            f2 = f1 + f0;
            f0 = f1;
            f1 = f2;
        }
    }
    printf("ans:%x\n", f2);
    exit();
}

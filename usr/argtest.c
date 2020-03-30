#include "lib/ulib.h"

int main(int argc, char *argv[])
{   
    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    exit();
    return 0;
}

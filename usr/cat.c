#include "lib/ulib.h"

char buf[1024];

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: cat <filename>\n");
        exit();
    }
    int fd = open(argv[1]);
    if (fd == -1) {
        printf("no such file: %s\n", argv[1]);
        exit();
    }
    read(fd, buf, 1024);
    printf("%s\n", buf);
    close(fd);
    exit();
}

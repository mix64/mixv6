#include "lib/ulib.h"

#define FNAMESIZ 28
#define NDIRENT 16
struct directory {
    struct dir {
        int ino;
        char fname[FNAMESIZ];
    } dir[NDIRENT];
} directory;

int main(void)
{
    int fd = open("/");
    int n  = read(fd, &directory, sizeof(directory));
    if (n > 0) {
        for (int i = 0; i < NDIRENT; i++) {
            if (directory.dir[i].ino) {
                printf("%d - %s\n", directory.dir[i].ino, directory.dir[i].fname);
            }
        }
    }
    close(fd);
    exit();
}

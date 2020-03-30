#include "arg.h"
#include <fs/file.h>
#include <fs/inode.h>
#include <kernel/proc.h>

int sys_open(void)
{
    char *path = argptr(0);
    struct inode *ip;
    struct file *f;
    int fd;

    if ((ip = namei(path)) == 0) {
        return -1;
    }

    if ((f = falloc()) == 0 || (fd = fdalloc(f)) < 0) {
        if (f) {
            ffree(f);
        }
        return -1;
    }

    f->inode  = ip;
    f->offset = 0;
    return fd;
}
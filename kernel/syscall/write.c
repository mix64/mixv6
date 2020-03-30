#include "arg.h"
#include <fs/file.h>
#include <fs/inode.h>
#include <kernel/lock.h>

int sys_write(void)
{
    struct file *f = argfd(0);
    char *src      = argptr(1);
    int size       = argint(2), r;

    if (f == 0 || src == 0 || size < 0) {
        return -1;
    }

    acquire(&f->inode->lock);
    if ((r = writei(f->inode, src, f->offset, size)) > 0) {
        f->offset += r;
    }
    release(&f->inode->lock);

    return r;
}
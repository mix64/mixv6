#include "arg.h"
#include <fs/file.h>
#include <fs/inode.h>
#include <kernel/lock.h>

int sys_read(void)
{
    struct file *f = argfd(0);
    char *dst      = argptr(1);
    int size       = argint(2), r;

    if (f == 0 || dst == 0 || size < 0) {
        return -1;
    }
    acquire(&f->inode->lock);
    if ((r = readi(f->inode, dst, f->offset, size)) > 0) {
        f->offset += r;
    }
    release(&f->inode->lock);

    return r;
}
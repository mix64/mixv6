#include <dev/list.h>
#include <fs/block.h>
#include <fs/file.h>
#include <fs/inode.h>
#include <fs/super.h>
#include <kernel/init.h>
#include <kernel/lock.h>
#include <lib/panic.h>
#include <lib/string.h>
#include <macro.h>

struct file file[64];

void fsinit()
{
    struct inode *ip;

    // stdin
    ip        = ialloc();
    ip->devno = DEV_CONSOLE;
    ip->type  = T_DEV;
    file[STDIN].refcnt  = 1;
    file[STDIN].inode   = ip;
    // stdout
    ip        = ialloc();
    ip->devno = DEV_CONSOLE;
    ip->type  = T_DEV;
    file[STDOUT].refcnt = 1;
    file[STDOUT].inode  = ip;

    memmove((void *)SUPERADDR, &sysfs, sizeof(sysfs));
}

struct file *fget(int index)
{
    pushcli();
    file[index].refcnt += 1;
    popcli();
    return &file[index];
}

struct file *falloc()
{
    pushcli();
    for (int i = 0; i < NELEM(file); i++) {
        if (file[i].refcnt == 0) {
            file[i].refcnt = 1;
            popcli();
            return &file[i];
        }
    }
    popcli();
    return 0;
}

void ffree(struct file *f)
{
    if (f == 0)
        panic("ffree(1)");

    pushcli();
    if (f->refcnt < 1)
        panic("ffree(2)");
    --f->refcnt;
    popcli();
}

struct file *fdup(struct file *f)
{
    pushcli();
    if (f == 0 || f->refcnt < 1)
        panic("fdup");
    f->refcnt++;
    popcli();
    return f;
}

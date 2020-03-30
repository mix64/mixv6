#include "arg.h"
#include <fs/file.h>
#include <kernel/proc.h>

int sys_close(void)
{
    int fd = argint(0);
    if (fd < 2 || fd >= NOFILE) {
        return -1;
    }
    ffree(user.procp->ofile[fd]);
    user.procp->ofile[fd] = 0;
    return 0;
}
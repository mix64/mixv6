#include "arg.h"
#include <kernel/proc.h>
#include <mm/map.h>
#include <x86/trapframe.h>

int argint(int argc)
{
    return *(int *)((user.procp->tf->esp) + 4 * (argc + 1));
}

char *argptr(int argc)
{
    int n = argint(argc);
    return n > KERNEND ? (char *)n : 0;
}

struct file *argfd(int argc)
{
    int fd = argint(argc);
    if (fd < 0 || fd >= NOFILE) {
        return 0;
    }
    return user.procp->ofile[fd];
}

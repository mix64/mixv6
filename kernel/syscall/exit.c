#include <kernel/lock.h>
#include <kernel/proc.h>
#include <lib/panic.h>

int sys_exit(void)
{
    pushcli();

    user.procp->stat = ZOMB;

    if (user.procp->pproc)
        wakeup(user.procp->pproc);

    popcli();

    sched();

    panic("zombie exit");
    return 0;
}
#include <kernel/proc.h>

int sys_wait(void)
{
    sleep(user.procp);
    collect_zombies(); // kill zombies child
    return 0;
}
#include <dev/console.h>
#include <kernel/proc.h>
#include <kernel/syscall.h>
#include <macro.h>
#include <types.h>

extern int sys_exit(void);
extern int sys_fork(void);
extern int sys_read(void);
extern int sys_write(void);
extern int sys_open(void);
extern int sys_close(void);
extern int sys_wait(void);
extern int sys_creat(void);
extern int sys_link(void);
extern int sys_unlink(void);
extern int sys_execv(void);
extern int sys_kill(void);
extern int sys_reboot(void);

static int (*syscalls[])(void) = {
    [SYS_exit]   = sys_exit,
    [SYS_fork]   = sys_fork,
    [SYS_read]   = sys_read,
    [SYS_write]  = sys_write,
    [SYS_open]   = sys_open,
    [SYS_close]  = sys_close,
    [SYS_wait]   = sys_wait,
    [SYS_creat]  = sys_creat,
    [SYS_link]   = sys_link,
    [SYS_unlink] = sys_unlink,
    [SYS_execv]  = sys_execv,
    [SYS_kill]   = sys_kill,
    [SYS_reboot] = sys_reboot};

int syscall(uint32_t num)
{
    if (num > 0 && num < NELEM(syscalls) && syscalls[num]) {
        return syscalls[num]();
    }
    else {
        printf("pid %d: unknown sys call %d\n", user.procp->pid, num);
        return -1;
    }
}

#include <kernel/lock.h>
#include <kernel/proc.h>
#include <lib/panic.h>
#include <types.h>
#include <x86/asm.h>
#include <x86/regs.h>

void acquire(struct lock *lock)
{
    while (lock->locked) {
        sleep(lock);
    }
    lock->locked = 1;
    lock->procp  = user.procp;
}

void release(struct lock *lock)
{
    lock->locked = 0;
    lock->procp  = 0;
    wakeup(lock);
}

int holding(struct lock *lock)
{
    return lock->locked && (lock->procp == user.procp);
}

void pushcli()
{
    int eflags = leflags();

    cli();
    if (user.ncli == 0) {
        user.intena = (eflags & FL_IF);
    }
    user.ncli++;
}

void popcli()
{
    user.ncli--;
    if (leflags() & FL_IF)
        panic("popcli - interruptible");
    if (user.ncli < 0)
        panic("popcli");
    if (user.ncli == 0 && user.intena) {
        user.intena = 0;
        sti();
    }
}
#pragma once

#include <types.h>

/* Long-term locks for processes */
struct lock {
    uint32_t locked;    /* Is the lock held? */
    struct proc *procp; /* Process holding lock */
};

void acquire(struct lock *);
void release(struct lock *);
int holding(struct lock *);
void pushcli();
void popcli();
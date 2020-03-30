#pragma once

#include <types.h>

#define NPROC 64
#define NOFILE 32

struct user {
    struct proc *procp; /* running process */
    uint32_t nextpid;   /* generic for unique process id's */
    uint16_t ncli;      /* Depth of pushcli nesting. */
    uint16_t intena;    /* Were interrupts enabled before pushcli? */
    uint32_t sched;
} user;

enum procstat { UNUSED,
                SET,
                RUN,
                SLEEP,
                ZOMB,
                STOP };

struct proc {
    enum procstat stat;         /* process state */
    uint32_t pri;               /* priority, negative is high */
    uint32_t pid;               /* unique process id */
    struct proc *pproc;         /* parent process */
    uint32_t uid;               /* user id */
    uint32_t gid;               /* group id */
    uint32_t cpu;               /* cpu usage for scheduling */
    uint32_t size;              /* size of process */
    void *wchan;                /* waiting channel */
    uint32_t *pgdir;            /* page directory */
    char *kstack;               /* kernel stack for this process (used by tf, context?) */
    struct trapframe *tf;       /* trap frame */
    struct context *context;    /* context swtch */
    struct inode *cwd;          /* current directory */
    struct file *ofile[NOFILE]; /* open files */
};

struct proc *palloc();
void pfree(struct proc *);
int fdalloc(struct file *);
void sched();
void sleep(void *wchan);
void wakeup(void *wchan);
void procdump();
void collect_zombies();
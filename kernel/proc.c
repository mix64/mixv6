#include <dev/console.h>
#include <fs/file.h>
#include <fs/inode.h>
#include <kernel/init.h>
#include <kernel/lock.h>
#include <kernel/proc.h>
#include <lib/panic.h>
#include <lib/string.h>
#include <mm/map.h>
#include <mm/vmm.h>
#include <types.h>
#include <x86/asm.h>
#include <x86/regs.h>
#include <x86/segment.h>
#include <x86/trapframe.h>

struct context {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebx;
    uint32_t ebp;
    uint32_t eip;
};

static struct proc proc[NPROC];

void swtch(struct context **_old, struct context *_new);
extern void trapret();

char initcode[] =
    {
        // 40000000 <start>:
        0x6a, 0x00,                              // push $0x0
        0x68, 0x19, 0x00, 0x00, 0x40,            // push $0x40000019 # ($init)
        0x6a, 0x00,                              // push $0x0
        0xb8, 0x0B, 0x00, 0x00, 0x00,            // mov $0x0B, %eax # (SYS_execv)
        0xcd, 0x40,                              // int $0x40
                                                 // 40000010 <exit>:
        0xb8, 0x01, 0x00, 0x00, 0x00,            // mov $0x01, %eax
        0xcd, 0x40,                              // int $0x40
        0xeb, 0xf7,                              // jmp exit
                                                 // 40000019 <init>:
        0x73, 0x68, 0x65, 0x6c, 0x6c, 0x00, 0x00 // "shell"
};

/* init struct user */
void uinit()
{
    struct proc *p, *dummy;

    // proc[0]じゃなくても局所変数でいける可能性が？
    dummy       = &proc[0];
    dummy->stat = ZOMB;
    if ((user.procp = p = palloc()) == 0) {
        panic("uinit: palloc");
    }
    if ((p->pgdir = setupkvm()) == 0) {
        panic("uinit: setupkvm");
    }
    inituvm(p->pgdir, initcode, sizeof(initcode));
    p->size    = sizeof(initcode);
    p->tf->esp = KERNEND + PAGESIZE;
    p->tf->eip = KERNEND; /* beginning of initcode.S */
    p->cwd     = getrootdir();
    p->stat    = RUN;
    switchuvm(p);
    scr0(CR0_PG);
    swtch(&dummy->context, p->context);
}

/*
    ・Get a unused entry from process table.
    ・Allocate the kernel stack (procp->kstack).
    ・Set a process status from (UNUSED) to (SET).
    ・Create a trapframe on the kernel stack.
    * You have to set pgdir by using setupkvm() or copyuvm()
*/
struct proc *palloc()
{
    struct proc *p = 0;
    char *sp;

    pushcli();

    for (int i = 0; i < NPROC; i++) {
        if (proc[i].stat == UNUSED) {
            p = &proc[i];
            memset(p, 0, sizeof(struct proc));
            break;
        }
    }

    if (p == 0 || (p->kstack = kalloc()) == 0) { /* process table overflow OR kalloc error */
        popcli();
        return 0;
    }

    p->stat = SET; /* don't choose this proc when sched() */
    p->pid  = user.nextpid++;
    popcli();

    memset(p->kstack, 0, PAGESIZE);

    sp = p->kstack + STACKSIZE;
    sp -= sizeof(struct trapframe);
    p->tf = (struct trapframe *)sp;
    sp -= sizeof(struct context);
    p->context = (struct context *)sp;

    p->context->eip  = (uint32_t)trapret;
    p->tf->eflags    = FL_IF;
    p->tf->cs        = SEG_UCODE | DPL_USER;
    p->tf->ds        = SEG_UDATA | DPL_USER;
    p->tf->es        = p->tf->ds;
    p->tf->ss        = p->tf->ds;
    p->tf->eflags    = FL_IF;
    p->ofile[STDIN]  = fget(STDIN);
    p->ofile[STDOUT] = fget(STDOUT);

    return p;
}

void pfree(struct proc *p)
{
    pushcli();

    for (int fd = 0; fd < NOFILE; fd++) {
        if (p->ofile[fd]) {
            ffree(p->ofile[fd]);
            p->ofile[fd] = 0;
        }
    }

    if (p->cwd) {
        irelse(p->cwd);
        p->cwd = 0;
    }
    if (p->pgdir) {
        freevm(p->pgdir);
        p->pgdir = 0;
    }
    if (p->kstack) {
        kfree(p->kstack);
        p->kstack = 0;
    }
    p->stat = UNUSED;

    popcli();
}

int fdalloc(struct file *f)
{
    for (int fd = 0; fd < NOFILE; fd++) {
        if (user.procp->ofile[fd] == 0) {
            user.procp->ofile[fd] = f;
            return fd;
        }
    }
    return -1;
}

/* 
* swtched proc start from here.
* newproc retrun to trapret, other return to trap() that call sched().
* 
* sched process stack 
* 
* -------------------------------> %esp
* context(edi, esi, ebx, ebp)
* --------------------------------
* retrun from swtch() -> context(eip)
* --------------------------------
* return from sched()
* --------------------------------
* return from trap() -> trapret
* --------------------------------
* trapframe
* -------------------------------- proc.kstack
* 
* 
* new process stack
* 
* -------------------------------> %esp
* context(edi, esi, ebx, ebp)
* --------------------------------
* return from swtch()  -> context(eip) -> trapret
* --------------------------------
* handmade trapframe
* -------------------------------- proc.kstack
* 
*/
void sched()
{
    struct proc *newproc = 0, *oldproc;

    pushcli();

    for (int i = 0; i < NPROC; i++) {
        if (proc[(user.sched + i) % NPROC].stat == RUN) {
            newproc    = &proc[(user.sched + i) % NPROC];
            user.sched = i;
        }
    }

    /* only one running process */
    if (newproc == user.procp) {
        popcli();
        return;
    }

    if (newproc == 0) {
        if (user.procp->stat != ZOMB) {
            popcli();
            return;
        }
        else { // don't return zombie proc
            for (int i = 0; i < NPROC; i++) {
                if (proc[(user.sched + i) % NPROC].stat == SLEEP) {
                    newproc    = &proc[(user.sched + i) % NPROC];
                    user.sched = i;
                }
            }
            if (newproc == 0) {
                panic("no proc.");
            }
        }
    } /* no proc for swtch */

    oldproc    = user.procp;
    user.procp = newproc;

    switchuvm(newproc);
    popcli();

    swtch(&oldproc->context, newproc->context);
}

void sleep(void *wchan)
{
    pushcli();
    user.procp->wchan = wchan;
    user.procp->stat  = SLEEP;
    popcli();
    while (user.procp->wchan) {
        sched();
    }
}

void wakeup(void *wchan)
{
    struct proc *p;
    pushcli();
    for (p = proc; p < &proc[NPROC]; p++) {
        if (p->wchan == wchan) {
            p->stat  = RUN;
            p->wchan = 0;
        }
    }
    popcli();
}

void ps()
{
    pushcli();
    for (int i = 0; i < NPROC; i++) {
        if (proc[i].stat != UNUSED) {
            printf("proc[%d]: PID %d, Status: %d, wchan:%x \n", i, proc[i].pid, proc[i].stat, proc[i].wchan);
        }
    }
    popcli();
}

void procdump()
{
    printf("pid:%d\n", user.procp->pid);
    printf("stat:%d\n", user.procp->stat);
    printf("pgdir:%x\n", user.procp->pgdir);
    printf("wchan:%x\n", user.procp->wchan);
    printf("ncli:%d\n", user.ncli);
    printf("intena:%d\n", user.intena);
    printf("eflags:%x\n", leflags());
}

void collect_zombies()
{
    pushcli();
    for (int i = 0; i < NPROC; i++) {
        if (proc[i].pproc == user.procp && proc[i].stat == ZOMB) {
            pfree(&proc[i]);
        }
    }
    popcli();
}
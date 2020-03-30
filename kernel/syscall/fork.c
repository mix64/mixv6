#include "arg.h"
#include <fs/file.h>
#include <fs/inode.h>
#include <kernel/proc.h>
#include <mm/vmm.h>
#include <x86/trapframe.h>

int sys_fork(void)
{
    struct proc *np;
    struct proc *curproc = user.procp;

    if ((np = palloc()) == 0)
        goto bad;
    if ((np->pgdir = copyuvm(curproc->pgdir, curproc->size)) == 0)
        goto bad;

    /*
        Not necessary to set a context.
        Context is registers that stores the running state of the kernel.
        When a child process is executed with sched(), 
            it returns directly from trapret() to userspace.
    */
    np->size  = curproc->size;
    np->pproc = curproc;
    *np->tf   = *curproc->tf;

    // fork returns 0 in the child.
    np->tf->eax = 0;

    // copy files and cwd
    for (int i = 0; i < NOFILE; i++) {
        if (curproc->ofile[i]) {
            np->ofile[i] = fdup(curproc->ofile[i]);
        }
    }
    np->cwd = idup(curproc->cwd);

    np->stat = RUN;

    // fork returns child pid in the parent.
    return np->pid;

bad:
    if (np)
        pfree(np);
    return -1;
}
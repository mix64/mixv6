#include "arg.h"
#include <elf.h>
#include <fs/inode.h>
#include <kernel/lock.h>
#include <kernel/proc.h>
#include <lib/string.h>
#include <mm/map.h>
#include <mm/vmm.h>
#include <types.h>
#include <x86/trapframe.h>

int sys_execv(void)
{
    char *path  = argptr(0);
    char **argv = (char **)argptr(1);
    struct elfhdr elf;
    struct proghdr ph;
    struct inode *ip;
    uint32_t va, sp, argc = 0;
    uint32_t *pgdir = 0, *oldpgdir;
    char *_argv[16] = {0};
    uint32_t head[3]; // head of user stack


    if ((ip = namei(path)) == 0) {
        return -1;
    }

    acquire(&ip->lock);

    /* Check ELF header */
    if (readi(ip, (char *)&elf, 0, sizeof(elf)) != sizeof(elf))
        goto bad;
    if (elf.magic != ELF_MAGIC)
        goto bad;

    /* switch new pgdir */
    if ((pgdir = setupkvm()) == 0)
        goto bad;

    /* Load program into memory. */
    va = KERNEND;
    for (int i = 0, off = elf.phoff; i < elf.phnum; i++, off += sizeof(ph)) {
        if (readi(ip, (char *)&ph, off, sizeof(ph)) != sizeof(ph))
            goto bad;
        if (ph.type != ELF_PROG_LOAD)
            continue;
        if (ph.memsz < ph.filesz)
            goto bad;
        if (ph.vaddr + ph.memsz < ph.vaddr)
            goto bad;
        if ((va = allocuvm(pgdir, va, ph.vaddr + ph.memsz)) == 0)
            goto bad;
        if (ph.vaddr % PAGESIZE != 0)
            goto bad;
        if (loaduvm(pgdir, (char *)ph.vaddr, ip, ph.off, ph.filesz) < 0)
            goto bad;
        copyout(pgdir, ph.vaddr + ph.filesz, 0, ph.memsz - ph.filesz);
    }
    release(&ip->lock);

    /* setup user stack */
    if ((va = ustack(pgdir, va)) == 0)
        goto bad;

/*
*** User Stack ***

sp->| 0xffff  |  fake return address
    |  argc   |  int argc
    |  argv   |--- char *argv[]
    |  '\0'   |  |
  --| argv[0] | <-
  | | argv[1] | ---
  | |  '\0'   |   |
  ->| "bar\0" |   |
    | "foo\0" | <--
va->***********
*/

    sp = va; // stack pointer 

    /* append arguments */
    if (argv != 0) {
        for (argc = 0; argv[argc]; argc++) {
            sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
            copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1);
            _argv[argc] = (char *)sp;
        }
    }

    sp -= sizeof(_argv);
    copyout(pgdir, sp, (void *)_argv, sizeof(_argv));
    
    head[0] = 0xFFFFFFFF; // fake return address
    head[1] = argc;
    head[2] = sp;

    sp -= sizeof(head);
    copyout(pgdir, sp, (void *)head, sizeof(head));

    oldpgdir = user.procp->pgdir;
    user.procp->pgdir = pgdir;
    switchuvm(user.procp);
    freevm(oldpgdir);

    user.procp->size    = va - KERNEND;
    user.procp->tf->esp = sp;
    user.procp->tf->eip = elf.entry;

    return 0;

bad:
    if (pgdir)
        freevm(pgdir);
    if (holding(&ip->lock))
        release(&ip->lock);
    return -1;
}
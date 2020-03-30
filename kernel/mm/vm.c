#include "vm.h"
#include <dev/console.h>
#include <fs/inode.h>
#include <kernel/init.h>
#include <kernel/lock.h>
#include <kernel/proc.h>
#include <lib/panic.h>
#include <lib/string.h>
#include <macro.h>
#include <mm/map.h>
#include <mm/vmm.h>
#include <types.h>
#include <x86/asm.h>
#include <x86/segment.h>

static struct segdesc gdt[NSEGS];
struct taskstate ts;
static uint16_t gdtr[3];

int mappages(uint32_t *pgdir, uint32_t vaddr, uint32_t size, uint32_t paddr, uint32_t flags);
pte_t *walkpgdir(uint32_t *pgdir, uint32_t vaddr, int isalloc);

void seginit()
{
    gdt[SEG_KCODE >> 3] = SEG(STA_X | STA_R, 0, 0xffffffff, 0);
    gdt[SEG_KDATA >> 3] = SEG(STA_W, 0, 0xffffffff, 0);
    gdt[SEG_UCODE >> 3] = SEG(STA_X | STA_R, 0, 0xffffffff, DPL_USER);
    gdt[SEG_UDATA >> 3] = SEG(STA_W, 0, 0xffffffff, DPL_USER);
    gdtr[0]             = sizeof(gdt) - 1;
    gdtr[1]             = (uint32_t)gdt;
    gdtr[2]             = (uint32_t)gdt >> 16;
    asm volatile("lgdt (%0)" ::"r"(gdtr));
}

static struct kmap {
    uint32_t start;
    uint32_t end;
    uint32_t flags;
} kmap[3];

void kmapinit(struct kmap *k, uint32_t start, uint32_t end, uint32_t flags)
{
    k->start = start;
    k->end   = end;
    k->flags = flags;
}

void kvminit()
{
    kmapinit(&kmap[0], 0, EXTMEM, PTE_W);                     /* I/O space */
    kmapinit(&kmap[1], EXTMEM, (uint32_t)rodata_end, 0);      /* kern text+rodata */
    kmapinit(&kmap[2], (uint32_t)rodata_end, MEMSIZE, PTE_W); /* kern data+memory */
}

uint32_t *setupkvm()
{
    uint32_t *pgdir;

    if ((pgdir = kalloc()) == 0) {
        return 0;
    }
    memset(pgdir, 0, PAGESIZE);

    for (int i = 0; i < NELEM(kmap); i++) {
        /* kernel space is static, so paddr is not used kalloc() */
        if (mappages(pgdir, kmap[i].start, kmap[i].end - kmap[i].start, kmap[i].start, kmap[i].flags) < 0) {
            freevm(pgdir);
            return 0;
        }
    }
    return pgdir;
}

/* initcode space */
void inituvm(uint32_t *pgdir, void *start, uint32_t size)
{
    char *pa = kalloc();
    memset(pa, 0, PAGESIZE);
    mappages(pgdir, KERNEND, PAGESIZE, (uint32_t)pa, PTE_W | PTE_U);
    memmove(start, pa, size);
}

int loaduvm(uint32_t *pgdir, char *va, struct inode *ip, uint32_t offset, uint32_t size)
{
    uint32_t pa, n;
    pte_t *pte;

    if ((uint32_t)va % PAGESIZE != 0) {
        panic("loaduvm: addr must be page aligned");
    }

    for (int i = 0; i < size; i += PAGESIZE) {
        if ((pte = walkpgdir(pgdir, (uint32_t)va + i, 0)) == 0) {
            panic("loaduvm: pde should exist");
        }
        if ((pa = PTE_ADDR(*pte)) == 0) {
            panic("loaduvm: pte should exist");
        }
        n = (size - i < PAGESIZE) ? size - i : PAGESIZE;
        if (readi(ip, (char *)pa, offset + i, n) != n) {
            return -1;
        }
    }
    return 0;
}

/*
    Copy current proc's pgdir to new(fork) proc.
    1st argument is current proc's pgdir.
    2nd argument is the size of current proc,
        it shows that [KERNEND, KERNEND + size] is the area that should be copied.
*/
uint32_t *copyuvm(uint32_t *pgdir, uint32_t size)
{
    uint32_t *new;
    pte_t *pte;
    uint32_t pa, flags;
    void *mem = 0;

    if ((new = setupkvm()) == 0) {
        return 0;
    }

    for (uint32_t va = KERNEND; va < KERNEND + size; va += PAGESIZE) {
        if ((pte = walkpgdir(pgdir, va, 0)) == 0) {
            panic("copyuvm: pte should exist");
        }
        if (!(*pte & PTE_P)) {
            panic("page not present");
        }

        pa    = PTE_ADDR(*pte);
        flags = PTE_FLAGS(*pte);
        if ((mem = kalloc()) == 0)
            goto bad;
        memmove((void *)pa, mem, PAGESIZE);
        if (mappages(new, va, PAGESIZE, (uint32_t)mem, flags) < 0)
            goto bad; /* "mem" needs to be free because we got error while getting the PTE to set the address. */
    }

    return new;

bad:
    if (mem)
        kfree(mem);
    if (new)
        freevm(new);
    return 0;
}

/*
    old = old vaddr
    new = new vaddr
*/
int allocuvm(uint32_t *pgdir, uint32_t old, uint32_t new)
{
    char *pa;
    uint32_t va;

    if (new >= USEREND)
        return 0;
    if (new < old)
        return old;

    va = PGROUNDUP(old);
    for (; va < new; va += PAGESIZE) {
        pa = kalloc();
        if (pa == 0) {
            printf("allocuvm out of memory\n");
            deallocuvm(pgdir, new, old);
            return 0;
        }
        memset(pa, 0, PAGESIZE);
        if (mappages(pgdir, va, PAGESIZE, (uint32_t)pa, PTE_W | PTE_U) < 0) {
            printf("allocuvm out of memory (2)\n");
            deallocuvm(pgdir, new, old);
            kfree(pa);
            return 0;
        }
    }
    return new;
}

int deallocuvm(uint32_t *pgdir, uint32_t old, uint32_t new)
{
    pte_t *pte;
    uint32_t va, pa;

    if (new >= old) {
        return old;
    }

    va = PGROUNDUP(new);
    for (; va < old; va += PAGESIZE) {
        pte = walkpgdir(pgdir, va, 0);
        // pte = 0 -> no page directory
        // *pte = 0 -> no page table
        if (pte == 0) {
            /* no page directory, go to the next directory. */
            va = PGADDR(PDX(va) + 1, 0, 0) - PAGESIZE;
        }
        else if (*pte & PTE_P) { /* pgdir is exist and pgtable have P flags */
            pa = PTE_ADDR(*pte);
            if (pa == 0) {
                panic("deallocuvm");
            }
            kfree((char *)pa);
            *pte = 0;
        }
    }
    return new;
}


/*
    Copy data from [src, +len] to another pgdir's [va, +len].
    If zero specified in [src], we fill [va, +len] zeros with memset().
*/
void copyout(uint32_t *pgdir, uint32_t va, char *src, uint32_t len)
{

    pte_t *pte;
    uint32_t n, offset, pa;

    while (len > 0) {
        
        pte = walkpgdir(pgdir, va, 0);
        if (pte == 0 || (*pte & PTE_P) == 0)
            panic("copyout");

        offset = (va - PGROUNDDOWN(va));
        n = PAGESIZE - offset;
        if (n > len) {
            n = len;
        }
        pa = PTE_ADDR(*pte);
        if (src) {
            memmove(src, (void *)(pa + offset), n);
            src += n;
        }
        else {
            memset((void *)(pa + offset), 0 ,n);
        }
        len -= n;
        va += n;
    }
}

// Allocate two pages at the next page boundary.
// Make the first inaccessible.  Use the second as a user stack.
int ustack(uint32_t *pgdir, uint32_t start)
{
    uint32_t va = PGROUNDUP(start);
    if (allocuvm(pgdir, va, va + 2 * PAGESIZE) == 0)
        return 0;
    pte_t *pte = walkpgdir(pgdir, va, 0);
    *pte &= ~PTE_U;
    return va + 2 * PAGESIZE;
}

void freevm(uint32_t *pgdir)
{
    if (pgdir == 0) {
        panic("freevm: no pgdir");
    }
    deallocuvm(pgdir, USEREND, KERNEND);
    for (int i = 256; i < NPDENT; i++) {
        if (pgdir[i] & PTE_P) {
            kfree((char *)PTE_ADDR(pgdir[i]));
        }
    }
    kfree((char *)pgdir);
}

/* trap from userspace (cross ring)(ring 11 -> ring 00), 
 * %esp will be set address that Task Register's esp0. 
 */
void switchuvm(struct proc *p)
{
    if (p == 0)
        panic("switchuvm: no process");
    if (p->kstack == 0)
        panic("switchuvm: no kstack");
    if (p->pgdir == 0)
        panic("switchuvm: no pgdir");

    pushcli();
    gdt[SEG_TSS >> 3]   = SEG16(STS_T32A, &ts, sizeof(ts) - 1, 0);
    gdt[SEG_TSS >> 3].s = 0;
    ts.ss0              = SEG_KDATA;
    ts.esp0             = (uint32_t)p->kstack + PAGESIZE;
    // setting IOPL=0 in eflags *and* iomb beyond the tss segment limit
    // forbids I/O instructions (e.g., inb and outb) from user space
    ts.iomb = (uint16_t)0xFFFF;
    asm volatile("ltr %0" ::"r"((uint16_t)SEG_TSS));
    scr3(p->pgdir); // switch to process's address space
    popcli();
}

pte_t *walkpgdir(uint32_t *pgdir, uint32_t va, int isalloc)
{
    if (pgdir == 0) {
        panic("walkpgdir");
    }

    pde_t *pde = &pgdir[PDX(va)];
    pte_t *pte;

    if (*pde & PTE_P) {
        pte = (pte_t *)PTE_ADDR(*pde);
    }
    else {
        if (!isalloc || (pte = (pte_t *)kalloc()) == 0) {
            return 0;
        }
        memset(pte, 0, PAGESIZE);
        *pde = (uint32_t)pte | PTE_P | PTE_W | PTE_U;
    }
    return &pte[PTX(va)];
}

int mappages(uint32_t *pgdir, uint32_t va, uint32_t size, uint32_t pa, uint32_t flags)
{
    uint32_t a, last;
    pte_t *pte;

    if (pgdir == 0) {
        panic("mappages");
    }

    a    = PGROUNDDOWN(va);
    last = PGROUNDDOWN(va + size - 1);

    for (; a <= last; a += PAGESIZE, pa += PAGESIZE) {
        if ((pte = walkpgdir(pgdir, a, 1)) == 0) {
            return -1;
        }
        if (*pte & PTE_P) {
            panic("mappages");
        }
        *pte = pa | flags | PTE_P;
    }

    return 0;
}
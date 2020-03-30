#include <dev/console.h>
#include <kernel/init.h>
#include <kernel/lock.h>
#include <lib/panic.h>
#include <lib/string.h>
#include <mm/map.h>
#include <mm/vmm.h>

struct run {
    struct run *next;
};

struct run *coremap;

void kinit()
{
    printf("Kernel Size is %p\n", kernel_end);
    printf("Memory Size is %x\n", MEMSIZE);
    char *p = (char *)PGROUNDUP((uint32_t)kernel_end);
    for (; p < (char *)MEMSIZE; p += PAGESIZE) {
        kfree(p);
    }
}

/* allocate 4KB physical memory */
void *kalloc()
{
    struct run *r;

    pushcli();
    r = coremap;
    if (r) {
        coremap = r->next;
    }
    popcli();
    return (void *)r; /* return 0 if can't allocated */
}

void kfree(char *addr)
{
    struct run *r;

    if ((uint32_t)addr % PAGESIZE || addr < kernel_end || (uint32_t)addr >= MEMSIZE) {
        panic("kfree");
    }
    /* remove data in page for next use */
    memset(addr, 0, PAGESIZE);
    pushcli();
    r       = (struct run *)addr;
    r->next = coremap;
    coremap = r;
    popcli();
}
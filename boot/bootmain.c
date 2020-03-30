/*
 * Boot loader.
 *
 * Part of the boot block, along with bootasm.S, which calls bootmain().
 * bootasm.S has put the processor into protected 32-bit mode.
 * bootmain() loads an ELF kernel image from the disk starting at
 * sector 1 and then jumps to the kernel entry routine.
 */

#include <elf.h>
#include <fs/super.h>
#include <mm/map.h>
#include <types.h>

void readseg(uint8_t *addr, uint8_t count, uint32_t offset);
void memclr(uint32_t *addr, uint32_t count);

void bootmain(void)
{
    struct elfhdr *elf;
    struct proghdr *ph, *eph;
    void (*entry)(void);
    uint8_t *pa;
    uint32_t offset, count;
    elf                 = (struct elfhdr *)0x8000; /* scratch space */
    struct sysfs *sysfs = (struct sysfs *)SUPERADDR;

    readseg((uint8_t *)sysfs, 1, SUPERBLOCK);

    /* Read the first 8 sectors (4KB) of the kernel. */
    readseg((uint8_t *)elf, 8, sysfs->kernel);

    /* Is this an ELF executable? */
    if (elf->magic != ELF_MAGIC)
        return; /* let bootasm.S handle error */

    /* Load each program segment (ignores ph flags). */
    ph  = (struct proghdr *)((uint8_t *)elf + elf->phoff);
    eph = ph + elf->phnum;
    for (; ph < eph; ph++) {
        pa     = (uint8_t *)ph->paddr - (ph->off % SECTSIZE); /* Round down to sector boundary. */
        count  = (ph->filesz + SECTSIZE - 1) / SECTSIZE;      /* how many read sectors */
        offset = (ph->off / SECTSIZE) + sysfs->kernel;        /* Translate from bytes to sectors. */
        readseg(pa, count, offset);
        if (ph->memsz > ph->filesz)
            memclr((uint32_t *)ph->paddr + ph->filesz, ph->memsz - ph->filesz);
    }

    /* Call the entry point from the ELF header. Does not return! */
    entry = (void (*)(void))(elf->entry);
    entry();
}

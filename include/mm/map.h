#pragma once

extern char kernel_end[]; /* first address after kernel loaded from ELF file */
extern char rodata_end[]; /* address of finish read only data */

/* memory map */
#define CGAADDR 0xB8000    /* CGA memory */
#define EXTMEM 0x100000    /* [0,1M] is I/O space that need writable */
#define KERNEND 0x40000000 /* kernel space is [1M, 1G] */
#define USEREND 0xFFC00000 /* user space end */

#define MEMSIZE 0x20000000            /* memory size is 512MB */
#define PAGESIZE 0x1000               /* page size is 4KB */
#define STACKSIZE 0x1000              /* kstack size */
#define SECTSIZE 512                  /* sector size is 512Byte */
#define DISKSIZE 0x400000             /* 4MB, max size is 128GB */
#define NSECTOR (DISKSIZE / SECTSIZE) /* how many sectors in disk */

#define PGROUNDUP(sz) (((sz) + PAGESIZE - 1) & ~(PAGESIZE - 1)) /* round up to page size */
#define PGROUNDDOWN(a) (((a)) & ~(PAGESIZE - 1))                /* round down to page size */

#define USER(x) (x - KERNEND)
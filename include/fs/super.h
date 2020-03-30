#pragma once

#include <types.h>

struct sysfs {
    uint32_t nfree;   /* number of in core free blocks (1-127) */
    uint32_t ninode;  /* number of in core I nodes (1-127) */
    uint32_t rootdir; /* inode number of root directory */
    uint32_t kernel;
    uint32_t freeblock;  /* block of free list, last entry is pointer to next block. */
    uint32_t inodeblock; /* block of inode list, last entry is pointer to next block. */
} sysfs;

#define SUPERBLOCK 1
#define SUPERADDR 0x10000
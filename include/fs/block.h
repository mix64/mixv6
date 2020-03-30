#pragma once

#include <kernel/lock.h>
#include <mm/map.h>
#include <types.h>

struct block {
    uint32_t blockno;
    uint16_t flags;
    uint16_t refcnt;  /* refarence count */
    struct lock lock; /* is locked */
    uint8_t data[SECTSIZE];
};

#define B_VALID 0x1 // buffer is valid
#define B_DIRTY 0x2 // buffer needs to be written to disk

struct block *bget(uint32_t blockno);
struct block *bread(uint32_t blockno);
void bwrite(struct block *);
void brelse(struct block *);
#include <fs/block.h>
#include <kernel/lock.h>
#include <kernel/proc.h>
#include <lib/panic.h>
#include <x86/asm.h>

#define IDE_DATA 0x1F0
#define IDE_SECTCNT 0x1F2
#define IDE_ADDR1 0x1F3
#define IDE_ADDR2 0x1F4
#define IDE_ADDR3 0x1F5
#define IDE_ADDR4 0x1F6
#define IDE_COMMAND 0x1F7

#define IDE_RDCMD 0x20
#define IDE_WRCMD 0x30

struct lock idelock;

void idewait()
{
    while (1) {
        if ((inb(IDE_COMMAND) & 0xC0) == 0x40)
            break;
    }
}

void sendreq(struct block *b)
{
    if (b == 0)
        panic("idestart");
    if (b->blockno >= NSECTOR)
        panic("incorrect blockno");

    uint32_t blockno = (b->blockno &= 0x0FFFFFFF);
    idewait();
    outb(IDE_SECTCNT, 1);
    outb(IDE_ADDR1, blockno & 0xFF);
    outb(IDE_ADDR2, (blockno >> 8) & 0xFF);
    outb(IDE_ADDR3, (blockno >> 16) & 0xFF);
    outb(IDE_ADDR4, (blockno >> 24) | 0xE0);
    if (b->flags & B_DIRTY) {
        outb(IDE_COMMAND, IDE_WRCMD);
        outsl(IDE_DATA, b->data, SECTSIZE / 4);
    }
    else {
        outb(IDE_COMMAND, IDE_RDCMD);
    }
}

void iderw(struct block *b)
{
    if (!holding(&b->lock))
        panic("iderw: block not locked");
    if ((b->flags & (B_VALID | B_DIRTY)) == B_VALID)
        panic("iderw: nothing to do");

    acquire(&idelock);

    sendreq(b);
    while ((b->flags & (B_VALID | B_DIRTY)) != B_VALID) {
        sleep(b);
    }

    release(&idelock);
}

void ideintr()
{
    struct block *b = (struct block *)idelock.procp->wchan;

    /* if wait read data */
    if ((b->flags & B_DIRTY) == 0) {
        insl(IDE_DATA, b->data, SECTSIZE / 4);
    }

    /* Wake process waiting for this block. */
    b->flags |= B_VALID;
    b->flags &= ~B_DIRTY;
    wakeup(b);
}
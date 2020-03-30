#include <fs/block.h>
#include <kernel/lock.h>
#include <lib/panic.h>
#include <macro.h>
#include <types.h>

struct block blocks[256];

extern void iderw(struct block *b);

struct block *bget(uint32_t blockno)
{
    pushcli();
    /* find cache */
    for (int i = 0; i < NELEM(blocks); i++) {
        if (blocks[i].blockno == blockno) {
            blocks[i].refcnt++;
            popcli();
            return &blocks[i];
        }
    }
    /* no cache */
    for (int i = 0; i < NELEM(blocks); i++) {
        if (blocks[i].refcnt == 0 && (blocks[i].flags & B_DIRTY) == 0) {
            blocks[i].blockno = blockno;
            blocks[i].flags   = 0;
            blocks[i].refcnt  = 1;
            popcli();
            return &blocks[i];
        }
    }

    panic("bget: no buffers");
    return 0;
}

struct block *bread(uint32_t blockno)
{
    struct block *b = bget(blockno);
    if (b->refcnt > 1) {
        return b;
    }
    acquire(&b->lock);
    if ((b->flags & B_VALID) == 0) {
        iderw(b);
    }
    release(&b->lock);
    return b;
}

void bwrite(struct block *b)
{
    acquire(&b->lock);
    if (!holding(&b->lock)) {
        panic("bwrite");
    }
    b->flags |= B_DIRTY;
    iderw(b);
    release(&b->lock);
}

void brelse(struct block *b)
{
    b->refcnt--;
}
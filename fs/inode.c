#include <dev/devsw.h>
#include <fs/block.h>
#include <fs/directory.h>
#include <fs/inode.h>
#include <fs/super.h>
#include <kernel/lock.h>
#include <lib/panic.h>
#include <lib/string.h>
#include <macro.h>
#include <types.h>

struct inode inode[64];

struct inode *getrootdir()
{
    struct inode *ip = iget(sysfs.rootdir);
    if (ip == 0)
        panic("no rootdir");
    return ip;
}

struct inode *ialloc()
{
    struct inode *ip = 0;

    pushcli();
    for (int i = 0; i < NELEM(inode); i++) {
        if (!inode[i].valid) {
            ip         = &inode[i];
            ip->valid  = 1;
            ip->refcnt = 1;
            break;
        }
    }
    popcli();
    return ip;
}

struct inode *iget(uint32_t inum)
{
    struct inode *ip = 0;
    /* find from inode table */
    pushcli();
    for (int i = 0; i < NELEM(inode); i++) {
        if (inode[i].valid && inode[i].num == inum) {
            inode[i].refcnt++;
            popcli();
            return &inode[i];
        }
    }

    /* can't find */
    for (int i = 0; i < NELEM(inode); i++) {
        if (!inode[i].valid) {
            ip         = &inode[i];
            ip->valid  = 1;
            ip->num    = inum;
            ip->refcnt = 1;
            ip->devno  = 0;
            break;
        }
    }
    popcli();
    if (ip == 0) {
        return 0;
    }
    struct block *b = bread(inum);
    memmove(b->data, &ip->type, SECTSIZE);
    brelse(b);

    return ip;
}

void irelse(struct inode *ip)
{
    if (ip == 0)
        panic("irelse");

    pushcli();
    if (ip->refcnt > 1) {
        ip->refcnt--;
        popcli();
        return;
    }
    ip->valid = 0;
    popcli();
}

struct inode *idup(struct inode *ip)
{
    if (ip == 0)
        panic("idup");

    pushcli();
    ip->refcnt++;
    popcli();
    return ip;
}

/* caller must get lock for read/write to inode */
int readi(struct inode *ip, char *dst, uint32_t offset, uint32_t size)
{
    uint32_t m;
    struct block *bp;
    if (!holding(&ip->lock)) {
        return -1;
    }
    /* if inode is device */
    if (ip->type == T_DEV) {
        if (ip->devno < 0 || ip->devno >= NDEV || !devsw[ip->devno].read) {
            return -1;
        }
        return devsw[ip->devno].read(ip, dst, size);
    }
    /* check offset */
    if (offset > ip->size || offset + size < offset || offset / SECTSIZE >= 124) {
        return -1;
    }
    if (offset + size > ip->size) {
        size = ip->size - offset;
    }

    for (int cnt = 0; cnt < size; cnt += m, offset += m, dst += m) {
        bp = bread(ip->addr[offset / SECTSIZE]);
        m  = min(size - cnt, SECTSIZE - offset % SECTSIZE);
        memmove(bp->data + offset % SECTSIZE, dst, m);
        brelse(bp);
    }

    return size;
}

int writei(struct inode *ip, char *src, uint32_t offset, uint32_t size)
{
    uint32_t m;
    struct block *bp;

    if (!holding(&ip->lock)) {
        return -1;
    }

    /* if inode is device */
    if (ip->type == T_DEV) {
        if (ip->devno < 0 || ip->devno >= NDEV || !devsw[ip->devno].write) {
            return -1;
        }
        return devsw[ip->devno].write(ip, src, size);
    }

    /* check offset */
    if (offset > ip->size || offset + size < offset || offset / SECTSIZE >= 124 || offset + size > 124 * SECTSIZE) {
        return -1;
    }

    for (int cnt = 0; cnt < size; cnt += m, offset += m, src += m) {
        m = min(size - cnt, SECTSIZE - offset % SECTSIZE);
        if (m == SECTSIZE) {
            bp = bget(ip->addr[offset / SECTSIZE]);
        }
        else {
            bp = bread(ip->addr[offset / SECTSIZE]);
        }
        memmove(src, bp->data + offset % SECTSIZE, m);
        brelse(bp);
    }
    return size;
}

struct inode *dirlookup(struct directory *d, char *fname)
{
    for (int i = 0; i < NDIRENT; i++) {
        if (strcmp(fname, d->dir[i].fname, FNAMESIZ)) {
            return iget(d->dir[i].ino);
        }
    }
    return 0;
}

/* TODO: only file name */
struct inode *namei(char *path)
{
    struct inode *ip;
    struct block *b;

    if (path == 0) {
        return 0;
    }

    // TODO: if first char is '/', return rootdir now.
    ip = getrootdir();
    if (*path == '/') {
        return ip;
    }
    b = bread(ip->addr[0]);
    irelse(ip);

    ip = dirlookup((struct directory *)b->data, path);
    brelse(b);
    return ip;
}
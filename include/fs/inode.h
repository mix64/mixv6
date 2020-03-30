#pragma once

#include <kernel/lock.h>
#include <types.h>

struct inode {       /* sizeof(struct inode) = 512Byte */
    uint32_t num;    /* i number, 1-to-1 with device address */
    uint16_t refcnt; /* reference count */
    uint16_t valid;
    struct lock lock;
    uint32_t devno;

    /* copy from dinode */
    uint16_t type;
    uint16_t uid;       /* user id */
    uint16_t gid;       /* group id */
    uint16_t mode;      /* permission */
    uint32_t nlink;     /* directory entries */
    uint32_t size;      /* file size */
    uint32_t addr[124]; /* device addresses constituting file.*/
};

#define T_DIR 1  /* Directory */
#define T_FILE 2 /* File */
#define T_DEV 3  /* Device */

struct dinode {
    uint16_t type;
    uint16_t uid;       /* user id */
    uint16_t gid;       /* group id */
    uint16_t mode;      /* permission */
    uint32_t nlink;     /* directory entries */
    uint32_t size;      /* file size */
    uint32_t addr[124]; /* device addresses constituting file.*/
};

struct inode *ialloc();
struct inode *iget(uint32_t inum);
void irelse(struct inode *ip);
struct inode *idup(struct inode *ip);
int readi(struct inode *ip, char *dst, uint32_t offset, uint32_t size);
int writei(struct inode *ip, char *src, uint32_t offset, uint32_t size);
struct inode *namei(char *path);
struct inode *getrootdir();

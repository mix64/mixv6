#pragma once

#include <types.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2

struct file {
    uint32_t refcnt;     /* reference count */
    struct inode *inode; /* pointer to inode structure */
    uint32_t offset;     /* read/write character pointer */
};

struct file *fget(int index);
struct file *falloc();
void ffree(struct file *f);
struct file *fdup(struct file *f);
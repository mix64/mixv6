#pragma once

#include <types.h>

#define FNAMESIZ 28
#define NDIRENT 16

// sizeof(struct directory) == 512 == SECTSIZE
struct directory {
    struct dir {
        uint32_t ino;
        char fname[FNAMESIZ];
    } dir[NDIRENT];
};

struct inode *dirlookup(struct directory *d, char *fname);
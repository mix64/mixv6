#pragma once

struct inode;

#define NDEV 10

struct devsw {
    int (*read)(struct inode *, char *, int);
    int (*write)(struct inode *, char *, int);
} devsw[NDEV];
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct sysfs {
    int nfree;   /* number of in core free blocks (1-127) */
    int ninode;  /* number of in core I nodes (1-127) */
    int rootdir; /* inode number of root directory */
    int kernel;
    int freeblock;  /* block of free list, last entry is pointer to next block. */
    int inodeblock; /* block of inode list, last entry is pointer to next block. */
};

#define T_DIR 1  /* Directory */
#define T_FILE 2 /* File */
#define T_DEV 3  /* Device */
struct dinode {
    short type;
    short uid;       /* user id */
    short gid;       /* group id */
    short mode;      /* permission */
    int nlink;     /* directory entries */
    int size;      /* file size */
    int addr[124]; /* device addresses constituting file.*/
};

#define FNAMESIZ 28
#define NDIRENT 16
struct directory {
    struct dir {
        int ino;
        char fname[FNAMESIZ];
    } dir[NDIRENT];
};


#define DISKSIZE (4*1024*1024)

/*
0: bootblock
1: superblock
2-2048: inode block
2048-8192: data block
*/

char used_block[8192];

int ialloc();
int balloc(int blocksize);

void main(int argc, char *argv[])
{
    char *bin = malloc(DISKSIZE);
    memset(bin, 0, DISKSIZE);
    struct sysfs *sysfs = (struct sysfs *)&bin[512];

    // add boot block
    FILE *fp = fopen("build/bootblock", "rb");
    if (fp == NULL) {
        printf("no bootblock\n");
        return;
    }
    fread(bin, 512, 1, fp);
    used_block[0] = 1;
    fclose(fp);
    
    // add kernel
    struct stat stbuf;
    int fd = open("build/kernel", O_RDONLY);
    if (fd == -1) {
        printf("no kernel\n");
        return;
    }
    fp = fdopen(fd, "rb");
    if (fp == NULL) {
        printf("no kernel\n");
        return;
    }
    fstat(fd, &stbuf);
    int blocksize = (stbuf.st_size + 511) / 512;
    sysfs->kernel = balloc(blocksize);
    fread(&bin[sysfs->kernel*512], 512, blocksize, fp);
    fclose(fp);

    // make rootdir
    sysfs->rootdir = ialloc();
    struct dinode *rootdir_inode = (struct dinode *)&bin[sysfs->rootdir*512];
    rootdir_inode->type = T_DIR;
    rootdir_inode->uid = 0;
    rootdir_inode->gid = 0;
    rootdir_inode->mode = 0x755;
    rootdir_inode->nlink = 1;
    rootdir_inode->size = 512;
    rootdir_inode->addr[0] = balloc(512);
    struct directory *rootdir = (struct directory *)&bin[rootdir_inode->addr[0]*512];
    
    // add files (no more 16 files)
    for (int i = 1; i < argc; i++) {
        // make inode
        int ino = ialloc();
        struct dinode *inode = (struct dinode *)&bin[ino*512];
        // set inode to rootdir
        // remove "build/_" from filename
        rootdir->dir[i-1].ino = ino;
        strncpy(rootdir->dir[i-1].fname, &argv[i][7], FNAMESIZ);
        // alloc data block 
        fd = open(argv[i], O_RDONLY);
        if (fd == -1) {
            printf("no file: %s\n", argv[i]);
            return;
        }
        fp = fdopen(fd, "rb");
        if (fp == NULL) {
            printf("no file: %s\n", argv[i]);
            return;
        }
        fstat(fd, &stbuf);
        int blocksize = (stbuf.st_size + 511) / 512;
        if (blocksize > 124) {
            printf("file size too large: %s\n", argv[i]);
            return;
        }
        int blockno = balloc(blocksize);
        printf("add files: %s, ino: %d, blockno: %d\n", &argv[i][7], ino, blockno);
        // setup inode
        inode->type = T_FILE;
        inode->uid = 0;
        inode->gid = 0;
        inode->mode = 0x755;
        inode->nlink = 1;
        inode->size = stbuf.st_size;
        for (int j = 0; j < blocksize; j++) {
            inode->addr[j] = blockno + j;
        }
        fread(&bin[blockno*512], 512, blocksize, fp);
        fclose(fp);
    }

    // make free inode list
    int freeino = ialloc();
    int cnt = 0;
    sysfs->inodeblock = freeino;
    int *block = (int *)&bin[freeino*512];
    for (int i = freeino; i < 2048; i++) {
        block[cnt] = i;
        if (cnt == 127) {
            cnt = 0;
            block = (int *)&bin[i*512];
        }
        else {
            cnt++;
        }
    }

    // make free datablock list
    int freeblock = balloc(512);
    cnt = 0;
    sysfs->freeblock = freeblock;
    block = (int *)&bin[freeblock*512];
    for (int i = freeblock; i < 8192; i++) {
        block[cnt] = i;
        if (cnt == 127) {
            cnt = 0;
            block = (int *)&bin[i*512];
        }
        else {
            cnt++;
        }
    }

    // set superfs
    sysfs->ninode = 127;
    sysfs->nfree = 127;

    // write out
    fp = fopen("mixv6.img", "w");
    fwrite(bin, 512, 8192, fp);
    fclose(fp);
    return;
}

int ialloc()
{
    for (int i = 2; i < 2048; i++) {
        if (used_block[i] == 0) {
            used_block[i] = 1;
            return i;
        }
    }
    return -1;
}

int balloc(int blocksize)
{
    for (int i = 2048; i < 8192; i++) {
        if (used_block[i] == 0) {
            for (int j = 0; j < blocksize; j++) {
                used_block[i+j] = 1;
            }
            return i;
        }
    }
    return -1;
}
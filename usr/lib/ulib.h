
// system calls

int exit();
int fork();
int read(int fd, void *dst, int size);
int write(int fd, void *src, int size);
int open(char *path);
int close(int fd);
int wait();
int creat(char *path);
int link(char *old, char *new);
int unlink(char *path);
int execv(char *path, char *argv[]);
int kill(int pid);
int reboot(int magic);

// uprf.c
void printf(char *fmt, ...);

// ustring.c
void memset(char *addr, char data, int count);
void memmove(char *src, char *dst, int count);
int strcmp(const char *p, const char *q, int limit);

#define stdin 0
#define stdout 1
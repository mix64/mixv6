#include "usys.h"

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret


SYSCALL(exit)
SYSCALL(fork)
SYSCALL(read)
SYSCALL(write)
SYSCALL(open)
SYSCALL(close)
SYSCALL(wait)
SYSCALL(creat)
SYSCALL(link)
SYSCALL(unlink)
SYSCALL(execv)
SYSCALL(kill)
SYSCALL(reboot)

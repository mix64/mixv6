#pragma once

// System call numbers
#define SYS_exit 0x01
#define SYS_fork 0x02
#define SYS_read 0x03
#define SYS_write 0x04
#define SYS_open 0x05
#define SYS_close 0x06
#define SYS_wait 0x07
#define SYS_creat 0x08
#define SYS_link 0x09
#define SYS_unlink 0x0A
#define SYS_execv 0x0B
#define SYS_kill 0x25
#define SYS_reboot 0x58

#define T_SYSCALL 64 // system call
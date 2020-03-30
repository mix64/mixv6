#pragma once

/*
 * assembler macros to create x86 segments
 */

/* The 0xC0 means the limit is in 4096-byte units */
/* and (for executable segments) 32-bit mode. */
#define SEG_ASM(type, base, lim, dpl)                           \
    .word(((lim) >> 12) & 0xffff), ((base)&0xffff);             \
    .byte(((base) >> 16) & 0xff), (0x90 | (dpl << 5) | (type)), \
        (0xC0 | (((lim) >> 28) & 0xf)), (((base) >> 24) & 0xff)

/* segmentation */
#define SEG_KCODE 0x08 /* kernel code */
#define SEG_KDATA 0x10 /* kernel data+stack */
#define SEG_UCODE 0x18 /* user code */
#define SEG_UDATA 0x20 /* user data+stack */
#define SEG_TSS 0x28   /* 32bit TSS */

#define DPL_USER 0x3 /* User DPL */

#define STA_X 0x8 /* Executable segment */
#define STA_W 0x2 /* Writeable (non-executable segments) */
#define STA_R 0x2 /* Readable (executable segments) */

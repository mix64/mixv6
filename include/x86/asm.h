/* Routines to let C code use special x86 instructions. */

#pragma once

#include "types.h"

static inline void cli() { asm volatile("cli"); }

static inline void hlt() { asm volatile("hlt"); }

static inline uint8_t inb(uint16_t port)
{
    uint8_t data;
    asm volatile("in %1,%0"
                 : "=a"(data)
                 : "d"(port));
    return data;
}

static inline void insl(uint16_t port, void *addr, uint32_t cnt)
{
    asm volatile("cld; rep insl"
                 : "=D"(addr), "=c"(cnt)
                 : "d"(port), "0"(addr), "1"(cnt)
                 : "memory", "cc");
}

static inline uint32_t lcr2()
{
    uint32_t val;
    asm volatile("movl %%cr2,%0"
                 : "=r"(val));
    return val;
}

static inline uint32_t leflags(void)
{
    uint32_t eflags;
    asm volatile("pushfl; popl %0"
                 : "=r"(eflags));
    return eflags;
}

static inline void movsb(const void *src, void *dst, uint32_t cnt)
{
    asm volatile("cld; rep movsb"
                 : "=D"(dst), "=S"(src), "=c"(cnt)
                 : "0"(dst), "1"(src), "2"(cnt)
                 : "memory", "cc");
}

static inline void outb(uint16_t port, uint8_t data)
{
    asm volatile("outb %0,%1"
                 :
                 : "a"(data), "d"(port));
}

static inline void outw(uint16_t port, uint16_t data)
{
    asm volatile("outw %0,%1"
                 :
                 : "a"(data), "d"(port));
}

static inline void outsl(uint16_t port, const void *addr, uint32_t cnt)
{
    asm volatile("cld; rep outsl"
                 : "=S"(addr), "=c"(cnt)
                 : "d"(port), "0"(addr), "1"(cnt)
                 : "cc");
}

static inline void scr0(uint32_t flags)
{
    uint32_t val;
    asm volatile("movl %%cr0,%0"
                 : "=r"(val));
    asm volatile("mov %0, %%cr0" ::"r"(val | flags));
}

static inline void scr3(void *addr)
{
    asm volatile("mov %0, %%cr3" ::"r"(addr));
}

static inline void sti() { asm volatile("sti"); }

static inline void stosb(void *addr, int data, uint32_t cnt)
{
    asm volatile("cld; rep stosb"
                 : "=D"(addr), "=c"(cnt)
                 : "0"(addr), "1"(cnt), "a"(data)
                 : "memory", "cc");
}

static inline void breakpoint()
{
    outw(0x8a00, 0x8a00);
    outw(0x8a00, 0x8ae0);
}
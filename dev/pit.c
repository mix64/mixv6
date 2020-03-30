#include <kernel/init.h>
#include <x86/asm.h>

/* Programmable Interval Timer (PIT) ports */
#define PIT_CNT0 0x40
#define PIT_CTRL 0x43
// #define FREQ_MAGIC 0x2e9c // 100Hz
#define FREQ_MAGIC 0xe90c // 20Hz

void pitinit()
{
    outb(PIT_CTRL, 0x34); /* set intterupt */
    outb(PIT_CNT0, FREQ_MAGIC & 0xFF);
    outb(PIT_CNT0, FREQ_MAGIC >> 8);
}

#include <kernel/init.h>
#include <x86/asm.h>

/* Programmable Interrupt Controller (PIC) ports */
#define PIC0_CMD 0x20 /* cmd port */
#define PIC1_CMD 0xA0
#define PIC0_DATA 0x21 /* data port */
#define PIC1_DATA 0xA1
/* PIC Initialization Control Words */
#define PIC_ICW1 0x11        /* 0x1: read ICW4, 0x10: reserve, must 1*/
#define PIC0_ICW2 0x20       /* int vec[0x20=32] */
#define PIC1_ICW2 0x28       /* int vec[0x28=40] */
#define PIC0_ICW3 0x04       /* IR2 connect slave PIC */
#define PIC1_ICW3 0x02       /* connect master PIC IR2 */
#define PIC_ICW4 0x03        /* set in x86 mode */
#define PIC_EOI 0x20         /* end of interrupt */
#define PIC0_MASK 0b11110000 /* IRQ MASK without IR0(timer),IR1(kbd),IR2(slave) */
#define PIC1_MASK 0b10111111 /* MASK without IDE(14) */
#define PIC_MASKALL 0xFF

void picinit()
{
    outb(PIC0_DATA, PIC_MASKALL);
    outb(PIC1_DATA, PIC_MASKALL);
    outb(PIC0_CMD, PIC_ICW1);
    outb(PIC1_CMD, PIC_ICW1);
    outb(PIC0_DATA, PIC0_ICW2);
    outb(PIC1_DATA, PIC1_ICW2);
    outb(PIC0_DATA, PIC0_ICW3);
    outb(PIC1_DATA, PIC1_ICW3);
    outb(PIC0_DATA, PIC_ICW4);
    outb(PIC1_DATA, PIC_ICW4);
    outb(PIC0_DATA, PIC0_MASK);
    outb(PIC1_DATA, PIC1_MASK);
}

void piceoi(uint32_t no)
{
    if (no >= 8) {
        outb(PIC1_CMD, PIC_EOI);
    }
    outb(PIC0_CMD, PIC_EOI);
}
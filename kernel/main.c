#include <kernel/init.h>
#include <lib/panic.h>
#include <x86/asm.h>

int main(void)
{
    cli();
    consoleinit(); /* Console (CGA) */
    seginit();     /* Segment Descriptor Table */

    kinit();   /* physical memory space */
    kvminit(); /* kernel virtual memory space */

    idtinit(); /* Interrupt Descriptor Table */
    picinit(); /* Programmable Interrupt Controller */
    pitinit(); /* Programmable Interval Timer */

    fsinit();
    sti();
    uinit(); /* init process */

    panic("fail init process.");
}

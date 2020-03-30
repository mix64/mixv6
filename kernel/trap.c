#include "trap.h"
#include <dev/console.h>
#include <kernel/proc.h>
#include <kernel/syscall.h>
#include <lib/panic.h>
#include <lib/string.h>
#include <x86/asm.h>
#include <x86/segment.h>
#include <x86/trapframe.h>

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
static uint16_t idtr[3];
extern uint32_t vectors[]; // in vectors.S: array of 256 entry pointers
static uint32_t tick;

extern void piceoi(uint32_t no);
extern void kbdintr();
extern void ideintr();

void dump(struct trapframe *tf);
void unknowntrap(uint32_t trapno);

void idtinit()
{
    for (int i = 0; i < 256; i++)
        SETGATE(idt[i], 0, SEG_KCODE, vectors[i], 0);
    SETGATE(idt[T_SYSCALL], 1, SEG_KCODE, vectors[T_SYSCALL], DPL_USER);

    idtr[0] = sizeof(idt) - 1;
    idtr[1] = (uint32_t)idt;
    idtr[2] = (uint32_t)idt >> 16;
    asm volatile("lidt (%0)" ::"r"(idtr));
}

void trap(struct trapframe *tf)
{
    if (tf->trapno == T_SYSCALL) {
        memmove(tf, user.procp->tf, sizeof(struct trapframe));
        tf->eax = syscall(tf->eax);
        return;
    }

    switch (tf->trapno) {
    case T_IRQ0 + IRQ_TIMER:
        tick++;
        user.procp->cpu++;
        if (tick % 100 == 0) {
            sched();
        }
        piceoi(IRQ_TIMER);
        break;

    case T_IRQ0 + IRQ_KBD:
        kbdintr();
        piceoi(IRQ_KBD);
        break;

    case T_IRQ0 + IRQ_IDE:
        ideintr();
        piceoi(IRQ_IDE);
        break;

    default:
        unknowntrap(tf->trapno);
        dump(tf);
        panic("trap");
        break;
    }
}

void dump(struct trapframe *tf)
{
    printf("-----process dump-----\n");
    procdump();
    printf("-----trapframe dump-----\n");
    printf("EIP: %x\n", tf->eip);
    printf("ESP: %x\n", tf->esp);
    printf("EBP: %x\n", tf->ebp);
    printf("CR2: %x\n", lcr2());
    printf("-----------\n");
}

void unknowntrap(uint32_t trapno)
{
    switch (trapno) {
    /* See Intel SDM Vol.3A chap.6 */
    case T_DIVIDE:
        printf("Divide Error.\n");
        break;
    case T_DEBUG:
        printf("Debug Exception.\n");
        break;
    case T_NMI:
        printf("Non-maskable external interrupt.\n");
        break;
    case T_BRKPT:
        printf("Break Point. (INT 3)\n");
        break;
    case T_OFLOW:
        printf("Overflow.\n");
        break;
    case T_BOUND:
        printf("BOUND Range Exceeded.\n");
        break;
    case T_ILLOP:
        printf("Invalid Opcode (Undefined Opcode).\n");
        break;
    case T_DEVICE:
        printf("Device Not Available (No Math Coprocessor).\n");
        break;
    case T_DBLFLT:
        printf("Double Fault.\n");
        break;
    case T_TSS:
        printf("Invalid TSS (Task Switch Segment).\n");
        break;
    case T_SEGNP:
        printf("Segment Not Present.\n");
        break;
    case T_STACK:
        printf("Stack-Segment Fault.\n");
        break;
    case T_GPFLT:
        printf("General Protection.\n");
        break;
    case T_PGFLT:
        printf("Page Fault.\n");
        break;
    case T_FPERR:
        printf("x87 FPU Floating-Point Error (Math Fault).\n");
        break;
    case T_ALIGN:
        printf("Alignment Check.\n");
        break;
    case T_MCHK:
        printf("Machine Check.\n");
        break;
    case T_SIMDERR:
        printf("SIDM Floating-Point Exception.\n");
        break;
    case T_VIRT:
        printf("Virtualization Exception.\n");
        break;
    default:
        printf("unknown trap: No.%d\n", trapno);
        break;
    }
}
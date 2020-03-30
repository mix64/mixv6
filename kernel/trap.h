#pragma once

#include <types.h>

// Gate descriptors for interrupts and traps
struct gatedesc {
    uint32_t off_15_0 : 16;  // low 16 bits of offset in segment
    uint32_t cs : 16;        // code segment selector
    uint32_t args : 5;       // # args, 0 for interrupt/trap gates
    uint32_t rsv1 : 3;       // reserved(should be zero I guess)
    uint32_t type : 4;       // type(STS_{IG32,TG32})
    uint32_t s : 1;          // must be 0 (system)
    uint32_t dpl : 2;        // descriptor(meaning new) privilege level
    uint32_t p : 1;          // Present
    uint32_t off_31_16 : 16; // high bits of offset in segment
};

#define STS_IG32 0xE // 32-bit Interrupt Gate
#define STS_TG32 0xF // 32-bit Trap Gate

// Set up a normal interrupt/trap gate descriptor.
// - istrap: 1 for a trap (= exception) gate, 0 for an interrupt gate.
//   interrupt gate clears FL_IF, trap gate leaves FL_IF alone
// - sel: Code segment selector for interrupt/trap handler
// - off: Offset in code segment for interrupt/trap handler
// - dpl: Descriptor Privilege Level -
//        the privilege level required for software to invoke
//        this interrupt/trap gate explicitly using an int instruction.
#define SETGATE(gate, istrap, sel, off, d)                 \
    {                                                      \
        (gate).off_15_0  = (uint32_t)(off)&0xffff;         \
        (gate).cs        = (sel);                          \
        (gate).args      = 0;                              \
        (gate).rsv1      = 0;                              \
        (gate).type      = (istrap) ? STS_TG32 : STS_IG32; \
        (gate).s         = 0;                              \
        (gate).dpl       = (d);                            \
        (gate).p         = 1;                              \
        (gate).off_31_16 = (uint32_t)(off) >> 16;          \
    }

// x86 trap and interrupt constants.
// Processor-defined:
#define T_DIVIDE 0 // divide error
#define T_DEBUG 1  // debug exception
#define T_NMI 2    // non-maskable interrupt
#define T_BRKPT 3  // breakpoint
#define T_OFLOW 4  // overflow
#define T_BOUND 5  // bounds check
#define T_ILLOP 6  // illegal opcode
#define T_DEVICE 7 // device not available
#define T_DBLFLT 8 // double fault
// #define T_COPROC      9      // reserved (not used since 486)
#define T_TSS 10   // invalid task switch segment
#define T_SEGNP 11 // segment not present
#define T_STACK 12 // stack exception
#define T_GPFLT 13 // general protection fault
#define T_PGFLT 14 // page fault
// #define T_RES        15      // reserved
#define T_FPERR 16   // floating point error
#define T_ALIGN 17   // aligment check
#define T_MCHK 18    // machine check
#define T_SIMDERR 19 // SIMD floating point error
#define T_VIRT 20    // virtualization exception

// These are arbitrarily chosen, but with care not to overlap
// processor defined exceptions or interrupt vectors.
#define T_DEFAULT 500 // catchall

#define T_IRQ0 32 // IRQ 0 corresponds to int T_IRQ

#define IRQ_TIMER 0
#define IRQ_KBD 1
#define IRQ_COM1 4
#define IRQ_IDE 14
#define IRQ_ERROR 19
#define IRQ_SPURIOUS 31

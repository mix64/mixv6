#pragma once

#include <types.h>

/* parameter for paging */
#define NPDENT 1024 /* number of page directory entry */
#define NPTENT 1024 /* number of page table entry */

/* Page table/directory entry flags. */
#define PTE_P 0x001  /* Present */
#define PTE_W 0x002  /* Writeable */
#define PTE_U 0x004  /* User */
#define PTE_PS 0x080 /* Page Size */

#define NSEGS 6
#define STS_T32A 0x9 // Available 32-bit TSS

#define SEG(type, base, lim, dpl)                                     \
    (struct segdesc)                                                  \
    {                                                                 \
        ((lim) >> 12) & 0xffff, (uint32_t)(base)&0xffff,              \
            ((uint32_t)(base) >> 16) & 0xff, type, 1, dpl, 1,         \
            (uint32_t)(lim) >> 28, 0, 0, 1, 1, (uint32_t)(base) >> 24 \
    }

#define SEG16(type, base, lim, dpl)                                   \
    (struct segdesc)                                                  \
    {                                                                 \
        (lim) & 0xffff, (uint32_t)(base)&0xffff,                      \
            ((uint32_t)(base) >> 16) & 0xff, type, 1, dpl, 1,         \
            (uint32_t)(lim) >> 16, 0, 0, 1, 0, (uint32_t)(base) >> 24 \
    }

#define PDX(va) (((uint32_t)(va) >> 22) & 0x3FF)                  /* page directory index */
#define PTX(va) (((uint32_t)(va) >> 12) & 0x3FF)                  /* page table index */
#define PGADDR(d, t, o) ((uint32_t)((d) << 22 | (t) << 12 | (o))) /* construct virtual address from indexes and offset */

/* Address in page table or page directory entry */
#define PTE_ADDR(pte) ((uint32_t)(pte) & ~0xFFF)
#define PTE_FLAGS(pte) ((uint32_t)(pte)&0xFFF)

struct segdesc {
    uint32_t lim_15_0 : 16;  // Low bits of segment limit
    uint32_t base_15_0 : 16; // Low bits of segment base address
    uint32_t base_23_16 : 8; // Middle bits of segment base address
    uint32_t type : 4;       // Segment type (see STS_ constants)
    uint32_t s : 1;          // 0 = system, 1 = application
    uint32_t dpl : 2;        // Descriptor Privilege Level
    uint32_t p : 1;          // Present
    uint32_t lim_19_16 : 4;  // High bits of segment limit
    uint32_t avl : 1;        // Unused (available for software use)
    uint32_t rsv1 : 1;       // Reserved
    uint32_t db : 1;         // 0 = 16-bit segment, 1 = 32-bit segment
    uint32_t g : 1;          // Granularity: limit scaled by 4K when set
    uint32_t base_31_24 : 8; // High bits of segment base address
};

// Task state segment format
struct taskstate {
    uint32_t link; // Old ts selector
    uint32_t esp0; // Stack pointers and segment selectors
    uint16_t ss0;  //   after an increase in privilege level
    uint16_t padding1;
    uint32_t *esp1;
    uint16_t ss1;
    uint16_t padding2;
    uint32_t *esp2;
    uint16_t ss2;
    uint16_t padding3;
    void *cr3;     // Page directory base
    uint32_t *eip; // Saved state from last task switch
    uint32_t eflags;
    uint32_t eax; // More saved state (registers)
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t *esp;
    uint32_t *ebp;
    uint32_t esi;
    uint32_t edi;
    uint16_t es; // Even more saved state (segment selectors)
    uint16_t padding4;
    uint16_t cs;
    uint16_t padding5;
    uint16_t ss;
    uint16_t padding6;
    uint16_t ds;
    uint16_t padding7;
    uint16_t fs;
    uint16_t padding8;
    uint16_t gs;
    uint16_t padding9;
    uint16_t ldt;
    uint16_t padding10;
    uint16_t t;    // Trap on task switch
    uint16_t iomb; // I/O map base address
};

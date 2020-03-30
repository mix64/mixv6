#pragma once

/* Control register */
#define CR0_PE 0x00000001  /* Protection Enable */
#define CR0_WP 0x00010000  /* Write Protect */
#define CR0_PG 0x80000000  /* Paging Enable */
#define CR4_PSE 0x00000010 /* Page size extension */

/* Eflags register */
#define FL_IF 0x00000200 /* Interrupt Enable */

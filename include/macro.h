#pragma once

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define SECTROUNDUP(sz) (((sz) + SECTSIZE - 1) & ~(SECTSIZE - 1)) /* round up to sector size */
#define SIZE2SECTN(sz) (((sz) + SECTSIZE - 1) / SECTSIZE)         /* round up to sector size */

/* number of elements in fixed-size array */
#define NELEM(x) (sizeof(x) / sizeof((x)[0]))

#include "ulib.h"

void memset(char *addr, char data, int count)
{
    if (count < 0) {
        return;
    }
    while (count--) {
        *addr++ = data;
    }
}

void memmove(char *src, char *dst, int count)
{
    if (count < 0) {
        return;
    }
    while (count--) {
        *dst++ = *src++;
    }
}

int strcmp(const char *p, const char *q, int limit)
{
    while (*p && limit > 0) {
        if (*p != *q)
            return 0;
        p++, q++, limit--;
    }
    return 1;
}

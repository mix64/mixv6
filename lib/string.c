#include <lib/string.h>
#include <types.h>
#include <x86/asm.h>

void memset(void *addr, int data, int count)
{
    stosb(addr, data, count);
}

void memmove(void *src, void *dst, uint32_t count)
{
    movsb(src, dst, count);
}

int strcmp(const char *p, const char *q, uint32_t limit)
{
    while (*p && limit > 0) {
        if (*p != *q)
            return 0;
        p++, q++, limit--;
    }
    if (limit > 0 && *q != 0) {
        return 0;
    }
    return 1;
}

int strlen(const char *s)
{
    int cnt = 0;
    while (s[cnt++]);
    return cnt;
}
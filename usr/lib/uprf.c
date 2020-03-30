#include "types.h"
#include "ulib.h"

#define BUFSIZE 512
struct input {
    char buf[BUFSIZE];
    uint32_t index;
} input;

void put(int c);
void printn(uint32_t n, uint32_t base);
void bflush();

void printf(char *fmt, ...)
{
    uint32_t *argv;
    char *s;
    int c;

    input.index = 0;
    argv        = (uint32_t *)(&fmt + 1);

    while ((c = *fmt++) != 0) {
        if (c != '%') {
            put(c);
            continue;
        }
        switch (c = *fmt++) {
        case 'd':
            printn(*argv++, 10);
            break;
        case 'x':
        case 'p':
            put('0');
            put('x');
            printn(*argv++, 16);
            break;
        case 'o':
            put('0');
            put('o');
            printn(*argv++, 8);
            break;
        case 'c':
            put(*argv++);
            break;
        case 's':
            if ((s = (char *)*argv++) == 0) {
                s = "(null)";
            }
            while ((c = *s++)) {
                put(c);
            }
            break;
        default:
            printf("Unknown format %c", c);
            return;
        }
    }
    bflush();
}

void printn(uint32_t n, uint32_t base)
{
    static char digits[] = "0123456789ABCDEF";
    uint32_t m;
    if ((m = n / base)) {
        printn(m, base);
    }
    put(digits[(n % base)]);
}

void put(int c)
{
    input.buf[input.index] = c;
    input.index++;
    if (input.index == BUFSIZE) {
        bflush();
        input.index = 0;
    }
}

void bflush()
{
    write(1, input.buf, input.index);
}
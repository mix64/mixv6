#include <dev/console.h>
#include <dev/devsw.h>
#include <dev/list.h>
#include <fs/inode.h>
#include <kernel/init.h>
#include <kernel/proc.h>
#include <lib/panic.h>
#include <lib/string.h>
#include <macro.h>
#include <mm/map.h>
#include <types.h>
#include <x86/asm.h>

#define SCREEN_X 80 /* console width */
#define SCREEN_Y 25 /* console height */
#define INPUT_BUF 128

static int cursor;
static uint16_t *screen = (uint16_t *)CGAADDR;

struct input {
    char buf[INPUT_BUF];
    uint32_t index;
} input;

void printn(uint32_t n, uint32_t base);

void printf(char *fmt, ...)
{
    uint32_t *argv;
    char *s;
    int c;

    argv = (uint32_t *)(&fmt + 1);

    while ((c = *fmt++) != 0) {
        if (c != '%') {
            putchar(c);
            continue;
        }
        switch (c = *fmt++) {
        case 'd':
            printn(*argv++, 10);
            break;
        case 'x':
        case 'p':
            printf("0x");
            printn(*argv++, 16);
            break;
        case 'o':
            printf("0o");
            printn(*argv++, 8);
            break;
        case 'c':
            putchar(*argv++);
            break;
        case 's':
            if ((s = (char *)*argv++) == 0) {
                printf("(null)");
            }
            else {
                while ((c = *s++)) {
                    putchar(c);
                }
            }
            break;
        default:
            printf("Unknown format %c", c);
            return;
        }
    }
}

void printn(uint32_t n, uint32_t base)
{
    static char digits[] = "0123456789ABCDEF";
    uint32_t m;
    if ((m = n / base)) {
        printn(m, base);
    }
    putchar(digits[(n % base)]);
}

void putchar(int c)
{
    switch (c) {
    case '\n':
        cursor += SCREEN_X - cursor % SCREEN_X;
        break;

    case '\b':
        if (cursor > 0) {
            cursor--;
        }
        screen[cursor] = 0;
        break;

    default:
        screen[cursor++] = 0x0A00 | (0xFF & c);
        break;
    }
    if (cursor < 0 || cursor > SCREEN_Y * SCREEN_X)
        panic("cursor under/overflow");

    if ((cursor / SCREEN_X) >= (SCREEN_Y - 1)) { // Scroll up.
        memmove(screen + SCREEN_X, screen, sizeof(screen[0]) * (SCREEN_Y - 2) * SCREEN_X);
        cursor -= SCREEN_X;
        memset(screen + cursor, 0, sizeof(screen[0]) * ((SCREEN_Y - 1) * (SCREEN_X)-cursor));
    }

    /* print cursor */
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((cursor >> 8) & 0xFF));
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(cursor & 0xFF));
    screen[cursor] = ' ' | 0x0A00;
}

void panic(char *str)
{
    cli();
    printf("panic: %s\n", str);
    while (1)
        ;
}

void consoleintr(int (*getc)(void))
{
    int c = getc();
    if (input.index >= INPUT_BUF - 1 || c == 0) {
        return;
    }

    switch (c) {
    case '\n':
        wakeup(&input);
        break;

    case '\b':
        input.index--;
        break;

    default:
        input.buf[input.index] = c;
        input.index++;
        break;
    }
    putchar(c);
}

int consoleread(struct inode *ip, char *dst, int size)
{
    sleep(&input);

    pushcli();
    int n = min(size, input.index);

    for (int i = 0; i < n; i++) {
        *dst++ = input.buf[i];
    }
    *dst        = '\0';
    input.index = 0;
    popcli();

    return n;
}

int consolewrite(struct inode *ip, char *src, int size)
{
    pushcli();
    for (int i = 0; i < size; i++) {
        putchar(src[i]);
    }
    popcli();

    return size;
}

void consoleinit()
{
    stosb((void *)CGAADDR, 0, PAGESIZE);
    cursor = 0;

    devsw[DEV_CONSOLE].read  = consoleread;
    devsw[DEV_CONSOLE].write = consolewrite;
}

#include "printf.h"
#include "misc.h"
#include "serial.h"
#include "gprintf/gprintf.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// Defined in main.c
extern void (*p_syscall)(unsigned int a, unsigned int b, const void *p, const void *q, const void *r);

struct printf_config_struct printf_config;

#define UNUSED __attribute__((unused))

static int serial_gprintf_callback(UNUSED void *dummy, int c)
{
    if (c == '\n') {
        serial_putc('\r');
    }
    serial_putc((unsigned char) c);
    return 1;
}

int printf(const char *fmt, ...)
{
    // We're supposed to return the number of chars printed, but we might not
    // have that information (if we only use the p_syscall method), so in that
    // case we just return 0 (indicating success).
    int chars_printed = 0;

    if (printf_config.screen_output) {
        // This invokes the built-in printf function exported by NETXFER on the Evo T30.
        // The first two arguments mean "printf", since p_syscall also provides
        // other functions.  NOTE: If you are porting this code to another
        // platform, you will probably need to disable this call.
        (*p_syscall)(1, 1, fmt, (&fmt)+1, NULL);
    }

    if (printf_config.serial_output) {
        // Output to the serial port.
        chars_printed = general_printf(&serial_gprintf_callback, NULL, fmt, (const int *)(&fmt + 1));
    }

    return chars_printed;
}

// GCC's optimizer replaces printf("foo\n") with puts("foo"), so we implement
// it here.  Without this, we get "undefined reference to 'puts'" errors when
// linking.
int puts(const char *s)
{
    return printf("%s\n", s);
}

// Ditto for printf("x") -> putchar('x')
int putchar(int c)
{
    return printf("%c", c);
}

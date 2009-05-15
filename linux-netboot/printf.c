#include "printf.h"
#include "misc.h"

#include <stddef.h>
#include <stdint.h>

// Defined in main.c
extern void (*p_syscall)(unsigned int a, unsigned int b, const void *p, const void *q, const void *r);

int printf(const char *fmt, ...)
{
    // This invokes the built-in printf function exported by NETXFER on the Evo T30.
    // The first two arguments mean "printf", since p_syscall also provides
    // other functions.  NOTE: If you are porting this code to another
    // platform, you will probably need to disable this call.
    (*p_syscall)(1, 1, fmt, (&fmt)+1, NULL);

    // We're supposed to return the number of chars printed, but we don't
    // have that information, so we just return 0 (indicating success).
    return 0;
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

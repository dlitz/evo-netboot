#include <stdint.h>
#include "mynetboot.h"

/* Invoke the loader's printf-like function */
static void l_print(const char *fmt, void *arg)
{
    (*p_syscall)(1, 1, fmt, arg, (void*)0);
}

void init_c(void)
{
    int x = 1;
    int *y = &x;
    l_print("init_c starting\r\n", (void*)0);
    l_print("stack is in the ballpark of 0x%x\r\n", &y);
}

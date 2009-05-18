#include "memory.h"

void bzero(void *s, unsigned int n)
{
    unsigned char *p = (unsigned char *)s;
    while (n > 0) {
        *p = '\0';
        p++;
        n--;
    }
}

void *memcpy(void *dest, const void *src, unsigned int n)
{
    char *d = dest;
    const char *s = src;
    while (n-- > 0) {
        *d++ = *s++;
    }
    return dest;
}


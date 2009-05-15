#ifndef PORTIO_H
#define PORTIO_H

#include <stdint.h>

static inline void outb(uint8_t value, uint16_t port)
{
    __asm__ volatile (
        "outb %0, %1\n"
        : /* no output */
        : "a"(value), "d"(port)
        );
}

static inline void outw(uint16_t value, uint16_t port)
{
    __asm__ volatile (
        "outl %0, %1\n"
        : /* no output */
        : "a"(value), "d"(port)
        );
}

static inline void outl(uint32_t value, uint16_t port)
{
    __asm__ volatile (
        "outl %0, %1\n"
        : /* no output */
        : "a"(value), "d"(port)
        );
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t retval;
    __asm__ volatile (
        "inb %1, %0\n"
        : "=a"(retval)
        : "d"(port)
        );
    return retval;
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t retval;
    __asm__ volatile (
        "inw %1, %0\n"
        : "=a"(retval)
        : "d"(port)
        );
    return retval;
}

static inline uint32_t inl(uint16_t port)
{
    uint32_t retval;
    __asm__ volatile (
        "inl %1, %0\n"
        : "=a"(retval)
        : "d"(port)
        );
    return retval;
}

#endif /* PORTIO_H */

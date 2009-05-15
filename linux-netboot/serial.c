#include "serial.h"
#include "portio.h"

inline void serial_putc(unsigned char c)
{
    // Wait for transmitter ready (TXRDY) (bit 5 of LSR)
    while ((inb(0x3f8+5) & (1<<5)) == 0);

    // Send the byte
    outb(c, 0x3f8);
}

void serial_outstr(const char *s)
{
    while (*s != '\0') {
        serial_putc((unsigned char) *s);
        s++;
    }
}


#include "serial.h"
#include "superio.h"
#include "portio.h"
#include "printf.h"
#include "propaganda.h"

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

void serial_init(void)
{
    // Serial
    printf("Configuring serial port for 115200 8N1\n");
    // Set baud rate to 115200 bps (divisor=1)
    outb(0x80 | inb(0x3f8+3), 0x3f8+3);     // Set LCR:DLAB
    outb(0x01, 0x3f8);   // low byte
    outb(0x00, 0x3f8+1);   // high byte
    outb(~0x80 & inb(0x3f8+3), 0x3f8+3);     // Clear LCR:DLAB
    // Set 8N1
    outb(0x03, 0x3f8+3);    // LCR
    // Enable FIFO (XXX: Does this actually do anything on this chip?)
    outb(0xc1, 0x3f8+2);    // FCR

    serial_outstr("*** Serial port enabled\r\n");
    serial_outstr(propaganda);
}


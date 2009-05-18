#include "pcspkr.h"
#include "portio.h"

// PC Speaker stuff
//
// See http://www.dcc.unicamp.br/~celio/mc404s2-03/8253timer.html
// for information about programming the PC speaker.

void pcspkr_init(void)
{
    outb(182, 0x43);
}

static inline void pcspkr_off(void)
{
    outb(inb(0x61) & ~3, 0x61);
}

static inline void pcspkr_on(void)
{
    outb(inb(0x61) | 3, 0x61);
}

void pcspkr_beep(uint16_t period, uint16_t duration)
{
    outb(period & 0xff, 0x42);
    outb(period >> 8, 0x42);
    pcspkr_on();
    for (volatile int j = 0; j < (duration << 16); j++);   // delay
    pcspkr_off();
}

// Play a short tune from the Maple Leaf Rag
void pcspkr_boot_tune(void)
{
    pcspkr_init();

    pcspkr_beep(5746>>1, 50); // G#
    pcspkr_beep(5119>>1, 50); // A#
    pcspkr_beep(5746>>1, 50); // G#
    pcspkr_beep(4560>>1, 50); // C
    pcspkr_beep(5746>>1, 50); // G#
    pcspkr_beep(5119>>1, 50); // A#
    pcspkr_beep(4560>>1, 83); // C
    pcspkr_beep(6087>>1, 50); // G
    pcspkr_beep(5119>>1, 83); // A#
    pcspkr_beep(5746>>1, 83); // G#
}

void pcspkr_error_tune(void)
{
    pcspkr_init();

    pcspkr_beep(4560>>2, 500); // C
}

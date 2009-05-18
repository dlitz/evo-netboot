#include "led.h"
#include "superio.h"
#include "portio.h"

// GPIO "I/O Port 1" consists of GPIO17-10
#define GPIO_PORT1_DATA (gpio_io_base_port+0x00)

void led_set(enum led_state value)
{
    // Make the LED green (instead of amber):
    // - According to Nick Innes, the bi-colour LED is connected to GPIO10 and
    //   GPIO11 on the SuperI/O chip.
    //     http://www.coreboot.org/pipermail/coreboot/2008-May/035283.html
    // - According to the PC97307 datasheet:
    //   - "I/O Port 1" consists of GPIO17-10
    //   - Bit 7 of "SuperI/O Configuration Register 2" is the "GPIO Bank
    //     Select" bit

    uint8_t v = inb(GPIO_PORT1_DATA);
    v &= ~3; // mask out GPIO11-10
    v |= ((uint16_t)value & 3); // Set GPIO11-10 to the new value
    outb(v, GPIO_PORT1_DATA);
}

enum led_state led_get(void)
{
    return (enum led_state)(inb(GPIO_PORT1_DATA) & 3);
}

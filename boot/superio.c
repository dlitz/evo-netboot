#include "superio.h"
#include "printf.h"
#include "portio.h"
#include "main.h"

// PC97307 SuperI/O
//
// To access registers on the PC97307, we first write the index of the
// register to SUPERIO_INDEX_PORT, then read/write the data from/to
// SUPERIO_DATA_PORT.
//
// On the Evo T30, BADDR1=0 and BADDR0=0, so the index and data ports are
// 015Ch and 015Dh.
#define SUPERIO_INDEX_PORT 0x15c
#define SUPERIO_DATA_PORT 0x15d

// Possible alternative configuration (BADDR1=1, BADDR0=1)
//#define SUPERIO_INDEX_PORT 0x2e
//#define SUPERIO_DATA_PORT 0x2f

// Another possible alternative configuration (BADDR1=0, BADDR0=x)
//#define SUPERIO_INDEX_PORT 0x279
//#define SUPERIO_DATA_PORT 0xa79

#define SUPERIO_SID_REG 0x20    // SID register
#define SUPERIO_SRID_REG 0x27    // SRID register (PC97307 only)

uint16_t gpio_io_base_port;

uint8_t superio_inb(uint8_t index)
{
    outb(index, SUPERIO_INDEX_PORT);
    return inb(SUPERIO_DATA_PORT);
}

void superio_outb(uint8_t data, uint8_t index)
{
    outb(index, SUPERIO_INDEX_PORT);
    outb(data, SUPERIO_DATA_PORT);
}

void superio_select_logical_device(uint8_t devno)
{
    superio_outb(devno, 0x07);  // 07h: logical device number
}

void dump_superio(void)
{
    printf("PC97307 SuperI/O: SID(20h)=0x%02x SRID(27h)=0x%02x CR2(22h)=0x%02x\n", superio_inb(0x20), superio_inb(0x27), superio_inb(0x22));
}

void superio_init(void)
{
    // Enable PC97307 UART1
    if (debug_mode) printf("PC97307 SuperI/O: Activating serial port\n");
    superio_select_logical_device(6); // logical device 6 (UART1)
    superio_outb(superio_inb(0x30) | 1, 0x30);  // 0x30: Activate

    // Enable parallel port
    if (debug_mode) printf("PC97307 SuperI/O: Activating parallel port\n");
    superio_select_logical_device(6); // logical device 4 (Parallel port)
    superio_outb(superio_inb(0x30) | 1, 0x30);  // 0x30: Activate

    // Determine the I/O base address for GPIO.  This will have already been
    // set by NETXFER.
    superio_select_logical_device(7);   // Logical device 7: GPIO
    gpio_io_base_port = (superio_inb(0x60) << 8) | superio_inb(0x61);
    if (debug_mode) printf("PC97307 SuperI/O: GPIO I/O base port: 0x%04x\n", gpio_io_base_port);
}

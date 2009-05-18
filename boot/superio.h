#ifndef SUPERIO_H
#define SUPERIO_H

#include <stdint.h>

extern uint16_t gpio_io_base_port;

extern uint8_t superio_inb(uint8_t index);
extern void superio_outb(uint8_t index, uint8_t data);
extern void superio_select_logical_device(uint8_t devno);
extern void dump_superio(void);
extern void superio_init(void);

#endif /* SUPERIO_H */

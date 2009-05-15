#ifndef PCSPKR_H
#define PCSPKR_H

#include <stdint.h>

extern void pcspkr_init(void);
extern void pcspkr_beep(uint16_t period, uint16_t duration);
extern void pcspkr_boot_tune(void);
extern void pcspkr_error_tune(void);

#endif /* PCSPKR_H */

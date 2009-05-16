#ifndef LOADLINUX_H
#define LOADLINUX_H

#include <stdint.h>

extern void load_linux(void);
extern void *bzImage_start;
extern void *initrd_start;
extern uint32_t initrd_size;
extern void *e820_start;
extern uint32_t e820_size;
extern char *kernel_command_line;

#endif /* LOADLINUX_H */


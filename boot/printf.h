#ifndef PRINTF_H
#define PRINTF_H

#include <stdbool.h>

struct printf_config_struct {
    bool screen_output;     // Enable output to screen
    bool serial_output;     // Enable output to serial port
};

extern struct printf_config_struct printf_config;

extern int printf(const char *fmt, ...);

#endif

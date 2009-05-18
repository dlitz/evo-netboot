#ifndef LED_H
#define LED_H

#include <stdint.h>

enum led_state {
    LED_OFF = 0,    // 00
    LED_GREEN = 1,  // 01
    LED_AMBER = 2   // 10
};

extern void led_set(enum led_state value);
extern enum led_state led_get(void);

#endif /* LED_H */

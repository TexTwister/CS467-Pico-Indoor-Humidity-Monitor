
#ifndef LED_LOGIC_H // If not defined, define. Else, skip
#define LED_LOGIC_H

#include "pico/stdlib.h" // Pico SDK uint type


// Prototypes:
int humidity_to_led(float humidity);

void led_init_hardware(void);

#endif
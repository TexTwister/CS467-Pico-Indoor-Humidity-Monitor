
#ifndef LED_LOGIC_H // If not defined, define. Else, skip
#define LED_LOGIC_H

#include "pico/stdlib.h" // Pico SDK uint type

//Constants
#define LED_COUNT 6

// Prototypes:

void led_init_hardware(void);

void led_startup_sequence(void);

int humidity_to_led(float humidity);

void led_humidity_lightup(float humidity);

void led_error_flash(void);


#endif
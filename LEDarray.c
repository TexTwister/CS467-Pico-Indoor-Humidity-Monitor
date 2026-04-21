#include <stdio.h>
#include "pico/stdlib.h"

// Mapping out the hardware:
//LED 1 is physically connected via jumper wire to GPIO 14, LED 2 to GPIO 12, LED 3 to GPIO 10, 
//LED 4 to GPIO 8, LED 5 to GPIO 6, and LED 6 to GPIO 4
const int LED_PINS[] = {14, 12, 10, 8, 6, 4};
const int LED_COUNT = 6;

int main () {

    // Allows for USB communication from the Pico to the local device; starts serial connection to PC
    stdio_init_all();

    for (int i = 0; i < LED_COUNT; i++) {
        gpio_init(LED_PINS[i]);
        gpio_set_dir(LED_PINS[i], GPIO_OUT);
    }

    while (true) {
        for (int i = 0; i < LED_COUNT; i++) {
            // 1 = ON; 0 = OFF
            gpio_put(LED_PINS[i], 1);
            sleep_ms(150);
            gpio_put(LED_PINS[i], 0);
        }

    }
}
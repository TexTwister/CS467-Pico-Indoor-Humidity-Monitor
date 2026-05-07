#include "ledlib/include/led_logic.h"
#include "pico/stdlib.h"
#include <stdio.h>

// Testing light patterns
int main() {

    // Initialize I/O
    stdio_init_all();

    // Setup the GP pins for LEDs
    led_init_hardware();

    // Run startup pattern
    led_startup_sequence();

    while (true) {
        // Test environment of 10% (Should light up LED 1)
        printf("Testing humidity at 10.0%%\n");
        led_humidity_lightup(10.00f);
        sleep_ms(2000);

        // Test environment of 10% (Should light up LED 3)
        printf("Testing humidity at 40.0%%\n");
        led_humidity_lightup(40.00f);
        sleep_ms(2000);

        // Test environment of 50% (Should light up LED 4)
        printf("Testing humidity at 50.0%%\n");
        led_humidity_lightup(50.00f);
        sleep_ms(2000);

        // Test environment of 80% (Should light up LED 5)
        printf("Testing humidity at 80.0%%\n");
        led_humidity_lightup(80.00f);
        sleep_ms(2000);
        
        // Test error state pattern
        printf("Testing Error State Sequence...\n");
        led_error_flash();
    }
    
}


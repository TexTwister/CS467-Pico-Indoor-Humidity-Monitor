#include "ledlib/include/led_logic.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <math.h>

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

        // Test environment of 40% (Should light up LED 3)
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

        // Boundary Testing for humidity thresholds
        float boundaries[] = {
            0.00f, 20.49f,              // LED 1  
            20.50f, 30.49f,             // LED 2
            30.50f, 40.49f,             // LED 3
            40.50f, 60.49f,             // LED 4
            60.50f, 80.49f,             // LED 5
            80.50f, 99.99f, 100.00f};   // LED 6
        int num_bounds = sizeof(boundaries) / sizeof(boundaries[0]);
        
        for (int i = 0; i <num_bounds; i++) {
            printf("Testing Threshold Boundary: %.2f%%\n", boundaries[i]);
            led_humidity_lightup(boundaries[i]);
            sleep_ms(1500);
        }   
        
        // Edge case testing to trigger error state (<0.00, >100.00, NaN)
        float edge_cases[] = {-15.0f, 100.10f, 299.12f, NAN};
        int num_edges = sizeof(edge_cases) / sizeof(edge_cases[0]);

        for (int i = 0; i <num_edges; i++) {
            printf("Edge Case Testing: %.2f%%\n", edge_cases[i]);
            led_humidity_lightup(edge_cases[i]);
            sleep_ms(1500);
        }  

        // Test error state pattern
        printf("Testing Error State Sequence...\n");
        led_error_flash();
    }
}


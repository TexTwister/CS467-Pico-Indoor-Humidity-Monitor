#include "led_logic.h"
#include "pico/stdlib.h"
#include <stdio.h>

// Hardware Pin Mapping Definitions (GPIO Numbers)
#define LED_1_PIN 14
#define LED_2_PIN 12
#define LED_3_PIN 10
#define LED_4_PIN 8
#define LED_5_PIN 6
#define LED_6_PIN 4

const uint LED_PINS[6] = {
    LED_1_PIN,
    LED_2_PIN, 
    LED_3_PIN,
    LED_4_PIN,
    LED_5_PIN, 
    LED_6_PIN
};

void led_init_hardware(void) {
    // Initializes 6 LED pins 

    for (int i = 0; i < 6; i++) {
        gpio_init(LED_PINS[i]); // Wakes up the pins
        gpio_set_dir(LED_PINS[i], GPIO_OUT);  // Send power from pin to LED
        gpio_put(LED_PINS[i], 0); // Pins start out OFF
    }
}

void led_startup_sequence(void) {
    //Ping Pong pattern: single LED starting at LED 1 -> 6, LED 6-> 1 (3x) 
    for (int hop = 0; hop < 3; hop++) {

        // Left to right movement
        for (int i = 0; i < 6; i++) {
            gpio_put(LED_PINS[i], 1);
            sleep_ms(200);
            gpio_put(LED_PINS[i], 0);
    }

        // Right to left movement
        // Start at index 4 to skip double blink on end
        for (int i = 4; i > 0; i--) {
            gpio_put(LED_PINS[i], 1);
            sleep_ms(200);
            gpio_put(LED_PINS[i], 0);
        }
    }

        // All LEDs ON
        for (int i = 0; i < 6; i++) {
            gpio_put(LED_PINS[i], 1);
        }

        sleep_ms(700); 

        // All LEDs OFF
        for (int i = 0; i < 6; i++) {
            gpio_put(LED_PINS[i], 0);
        }
        sleep_ms(700);
}
 

void led_humidity_lightup(float humidity) {
    // Lights up a single LED based on humidity level.
    // LED 1 (Red), LED 2 (Yellow), LED 3 (Green), LED 4 (Green), LED 5 (Yellow), LED 6 (Red)
    
    int stage = humidity_to_led(humidity);
    int target_index = stage - 1; //Maps LED 1-6 to array index  0-5
   
    for (int i = 0; i < 6; i++) {
        if (i == target_index) {
            gpio_put(LED_PINS[i], 1); // (ON) Light up specific LED
        } else {

            gpio_put(LED_PINS[i], 0); // (OFF) All non-target lights are dark
        }
    }
}

int humidity_to_led(float humidity) {
    // Converts a float humidity percentage (relative humidity) into a specific light level on a 6-stage array.

    if (humidity < 20.50f) return 1;    // Extremely dry
    if (humidity < 30.50f) return 2;    // Dry
    if (humidity < 40.50f) return 3;    // Ideal Winter target zone
    if (humidity < 60.50f) return 4;    // Ideal Summer target zone  
    if (humidity < 80.50f) return 5;    // Humid
    
    return 6;                           // Extremely humid

}


void led_error_flash(void) {
    // Error state pattern: flash, flash, flash,  pause 
    // To be used for any sensor hardware or sensor issue
    for (int flash_count = 0; flash_count < 3; flash_count++) {
        
        // All 6 LEDs ON      
        for (int i = 0; i < 6; i++) {
            gpio_put(LED_PINS[i], 1);
    }
        sleep_ms(500);
        
        // All 6 LEDs OFF
        for (int i = 0; i < 6; i++) {
            gpio_put(LED_PINS[i], 0);
        }
        sleep_ms(500);
    }
    // Longer pause to finish off the error signaling pattern
    sleep_ms(750);
}

// Testing light patterns
int led_testing_main() {

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

// Original pattern option for startup:
// void led_startup_sequence(void) {
//     // LED array pattern = outer Red LEDs, middle yellow LEDs, inner green LEDs, all LEDs ON then OFF
//     // Sequence repeats 3x

//     for (int cycle = 0; cycle < 3; cycle++) {
       
//         // Paired color LED ON pattern
//         for (int i = 0; i < 3; i++) {
//             gpio_put(LED_PINS[i], 1); // Light up left side of array
//             gpio_put(LED_PINS[5 - i], 1); // Light up right side of array
            
//             sleep_ms(500); // Hold with color ON
            
//             // Clear color matching LED pair
//             gpio_put(LED_PINS[i], 0);   
//             gpio_put(LED_PINS[5 - i], 0); 
//             sleep_ms(200);
//         }

//         // All LEDs ON
//         for (int i = 0; i < 6; i++) {
//             gpio_put(LED_PINS[i], 1);
//         }

//         sleep_ms(400); 

//         // All LEDs OFF
//         for (int i = 0; i < 6; i++) {
//             gpio_put(LED_PINS[i], 0);
//         }
//         sleep_ms(400);
//     }
// }

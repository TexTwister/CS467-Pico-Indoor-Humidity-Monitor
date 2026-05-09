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

const uint LED_PINS[LED_COUNT] = {LED_1_PIN, LED_2_PIN, LED_3_PIN,
                                  LED_4_PIN, LED_5_PIN, LED_6_PIN};

void led_init_hardware(void) {
    // Initializes 6 LED pins and allows for slight delay as sensors power up
    sleep_ms(2000);

    for (int i = 0; i < LED_COUNT; i++) {
        gpio_init(LED_PINS[i]);               // Wakes up the pins
        gpio_set_dir(LED_PINS[i], GPIO_OUT);  // Send power from pin to LED
        gpio_put(LED_PINS[i], 0);             // Pins start out OFF
    }
}

void led_startup_sequence(void) {
    // Ping Pong pattern: single LED starting at LED 1 -> 6, LED 6-> 1 (3x)
    for (int hop = 0; hop < 3; hop++) {
        // Left to right movement
        for (int i = 0; i < LED_COUNT; i++) {
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
    for (int i = 0; i < LED_COUNT; i++) {
        gpio_put(LED_PINS[i], 1);
    }

    sleep_ms(700);

    // All LEDs OFF
    for (int i = 0; i < LED_COUNT; i++) {
        gpio_put(LED_PINS[i], 0);
    }
    sleep_ms(700);
}

void led_humidity_lightup(float humidity) {
    // Lights up a single LED based on humidity level.
    // LED 1 (Red), LED 2 (Yellow), LED 3 (Green), LED 4 (Green), LED 5
    // (Yellow), LED 6 (Red)

    int stage        = humidity_to_led(humidity);
    int target_index = stage - 1;  // Maps LED 1-6 to array index  0-5

    for (int i = 0; i < LED_COUNT; i++) {
        if (i == target_index) {
            gpio_put(LED_PINS[i], 1);  // (ON) Light up specific LED
        } else {
            gpio_put(LED_PINS[i], 0);  // (OFF) All non-target lights are dark
        }
    }
}

int humidity_to_led(float humidity) {
    // Converts a float humidity percentage (relative humidity) into a specific
    // light level on a 6-stage array.

    if (humidity < 20.50f) return 1;  // Extremely dry
    if (humidity < 30.50f) return 2;  // Dry
    if (humidity < 40.50f) return 3;  // Ideal Winter target zone
    if (humidity < 60.50f) return 4;  // Ideal Summer target zone
    if (humidity < 80.50f) return 5;  // Humid

    return 6;  // Extremely humid
}

void led_error_flash(void) {
    // Error state pattern: flash, flash, flash,  pause
    // To be used for any sensor hardware or sensor issue
    for (int flash_count = 0; flash_count < 3; flash_count++) {
        // All 6 LEDs ON
        for (int i = 0; i < LED_COUNT; i++) {
            gpio_put(LED_PINS[i], 1);
        }

        sleep_ms(500);

        // All 6 LEDs OFF
        for (int i = 0; i < LED_COUNT; i++) {
            gpio_put(LED_PINS[i], 0);
        }

        sleep_ms(500);
    }

    // Longer pause to finish off the error signaling pattern
    sleep_ms(750);
}

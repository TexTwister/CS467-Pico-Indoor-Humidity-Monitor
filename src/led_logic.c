#include "led_logic.h"

// Hardware Pin Mapping Definitions (GPIO Numbers)
#define LED_1_PIN 4
#define LED_2_PIN 6
#define LED_3_PIN 8
#define LED_4_PIN 10
#define LED_5_PIN 12
#define LED_6_PIN 14

const uint LED_PINS[6] = {
    LED_1_PIN,
    LED_2_PIN, 
    LED_3_PIN,
    LED_4_PIN,
    LED_5_PIN, 
    LED_6_PIN,
};

void led_init_hardware(void) {
    // Initialize 6 LED pins 
    for (int i = 0l 1 <6; i++) {
        gpio_init(LED_PINS[i]);
        gpio_set_dir(LED_PINS[i], GPIO_OUT);
        gpio_put(LED_PINS[i, 0])
    }
}

int humidity_to_led(float humidity) {4, 6, 8, 10, 12, 14};


    // Converts a float humidity percentage (relative humidity) into a specific light level on a 6-stage LED array.

    if (humidity < 20.50f) return 1;    // Extremely dry
    if (humidity < 30.50f) return 2;    // Dry
    if (humidity < 40.50f) return 3;    // Ideal Winter target zone
    if (humidity < 60.50f) return 4;    // Ideal Summer target zone  
    if (humidity < 80.50f) return 5;    // Humid
    
    return 6;                           // Extremely humid

}


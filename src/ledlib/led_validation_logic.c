#include "led_logic.h"
#include <math.h>
#include <stdio.h>
#include "pico/stdlib.h"

bool is_humidity_valid(float humidity) {
    // Check 1: Hardware/Sensor Failure (NaN)

    if(isnan(humidity)) {
        printf("[Validation Error]: Sensor returned NaN -- Check wiring and breadboard)\n");
        return false;
    }

    // Check 2: Impossible number (negative)
    if (humidity < 0.0f) {
        printf("[Validation Error]: Negative humidity (%.2f%%) detected.\n", humidity);
    }
    
    //Check 3: Impossible number or outside range 0-100)
    if (humidity > 100.0f) {
        printf("[Validation Error]: Humidity above 100%% (%.2f%%) detected.\n", humidity);
    }

    return true;
}

void led_validated_display(float humidity) {
    if (is_humidity_valid(humidity)) {
        // Sensor data is good --> LED logic
        led_humidity_lightup(humidity);
    } else {
        // Sensor data is impossible or NaN --> error flash pattern
        led_error_flash();
    }
}

// Scripted demo to show validation and LED logic
void run_validation_demo(void) {
    printf("...Starting Validation Demo..\n");

    // Test 1: Impossibly Low Reading
    printf("Testing -15.0%% (Expected: Error Flash)\n");
    led_validated_display(-15.0f);
    sleep_ms(2000);

    // Test 2: Impossibly High Reading
    printf("Testing 115.0%% (Expected: Error Flash)\n");
    led_validated_display(115.0f);
    sleep_ms(2000);

    // Test 3: Ideal Humidity Reading
    printf("Testing 40.0%% (Expected: LED 4 (Green))\n");
    led_validated_display(40.0f);
    sleep_ms(2000);

    printf("...Demo Complete: Resuming Environmental Sensor Readings...\n");
}

// Scripted demo to test LED mapping for each of the 6 stages
void run_led_sweep_test(void) {
    float humidity_vals[] = {5.0f, 25.0f, 37.0f, 52.0f, 75.0f, 90.0f};

    printf("...Starting LED Calibration Sweep...\n");

    for (int i = 0; i < LED_COUNT; i++) {
        printf("Testing Stage %d: %.1f%%\n", i + 1, humidity_vals[i]);
        led_validated_display(humidity_vals[i]);
        sleep_ms(1000);
    }

    printf("...LED Sweep Complete...\n");
}

// Begins validation testing followed by a sequential sweep from stage 1 to 6. 
void run_full_LED_verification(void) {
    printf("\n...Starting Full LED System Verification...");

    // Tests logic and NaN handling
    run_validation_demo();
    sleep_ms(2000);

    //Tests LED mapping and wiring
    run_led_sweep_test();

    printf("...All Tests Complete: System Stable...");
}

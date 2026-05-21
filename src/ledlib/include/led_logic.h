#ifndef LED_LOGIC_H // If not defined, define. Else, skip
#define LED_LOGIC_H

#include "pico/stdlib.h" // Pico SDK uint type

/**
 * @brief Total number of LEDs in the hardware array.
 */
#define LED_COUNT 6

/**
 * @brief Initializes the GPIO pins for the LED array.
 * * Sets the pins defined in LED_PINS to output mode and
 * ensures they start in the OFF state.
 */

void led_init_hardware(void);

/**
 * @brief Runs a ping pong style startup display pattern.
 * * Sequentially toggles LEDs from 1 to 6 and back to 1.
 * Used to verify all LEDs are wired correctly upon startup.
 */
void led_startup_sequence(void);

/**
 * @brief Maps a humidity value to a specific LED stage.
 *  * @param humidity The relative humidity percentage. 
 *  @return int The LED index (1 through LED_COUNT)
 */
int humidity_to_led(float humidity);

/**
 * @brief Updates the LED array based on the humidity percentage.
 *  * @param humidity The relative humidity percentage. 
 */
void led_humidity_lightup(float humidity);

/**
 * @brief Flashes all LEDs to signal a hardware or sensor error.
 */
void led_error_flash(void);

/**
 * @brief Validates raw humidity float before LED array updates stage level.
 * Returns true if value is within possible limits (0-100%).
 */
bool is_humidity_valid(float humidity);

/**
 * Wrapper function to be used in main.c or test suites.
 * Decides whether to show the humidity level or the error pattern on the LED array.
 */
void led_validated_display(float humidity);

/**
 * @brief Runs a scripted test passing invalid data (0.0% > humidity > 100%, NaN value)
 * to verify the prototype triggers error states correctly.
 */
void run_validation_demo(void);

/**
 * @brief Runs a script that sequentially sweeps LED 1 --> 6 using
 * hardcoded values to verify hardware mapping and LED staging accuracy.
 */
void run_led_sweep_test(void);

/**
 * @brief Starts a full system validation check and hardware sweep demo.
 */
void run_full_LED_verification(void);

#endif // LED_LOGIC_H
#ifndef LED_LOGIC_H // If not defined, define. Else, skip
#define LED_LOGIC_H

#include "pico/stdlib.h" // Pico SDK uint type

/**
 * @brief Ttoal number of LEDs in the harware array.
 */
#define LED_COUNT 6

/**
 * @brief Initialized the GPIO pins for the LED array.
 * * Sets the pins defined in LED_PINS to output mode and
 * ensures they start in the OFF state.
 */

void led_init_hardware(void);

/**
 * @brief Runs a ping pong style startup display pattern.
 * * Sequentially toggles LEDs form 1 to 6 and back to 1.
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

#endif // LED_LOGIC_H
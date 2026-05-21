#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/stdlib.h"

#include "dht20.h"
#include "lcdlib.h"
#include "led_logic.h"

#define DHT20_I2C_PORT            i2c0
#define DHT20_I2C_BAUDRATE        100000
#define DHT20_SDA_PIN             20  // GPIO pin number for DHT20 SDA line
#define DHT20_SCL_PIN             21  // GPIO pin number for DHT20 SCL line
#define LCD_I2C_BAUDRATE          100000
#define LCD_I2C_CONTROLLER        1
#define LCD_SDA_PIN               26    // I2C SDA pin number (physical pin 31)
#define LCD_SCL_PIN               27    // I2C SCL pin number (physical pin 32)
#define SENSOR_UPDATE_INTERVAL_MS 5000  // 5 seconds between sensor reads.
#define LCD_ROW_2                 0x40
#define DEGREE_SYMB               0b11011111  // See HD44780 data sheet page 17
#define TEMP_TOGGLE_BUTTON_PIN    16  // Physical pin 21; connect button to GND
// Debounce waits for switch contacts to settle so one press toggles only once.
#define BUTTON_DEBOUNCE_MS 50
// Polling every 20 ms lets the app catch button presses between sensor reads.
#define BUTTON_POLL_INTERVAL_MS 20

/**
 * @brief Configures the Pico GPIO pins used by the DHT20 as an I2C bus.
 */
static void configure_dht20_i2c_bus(void) {
    i2c_init(DHT20_I2C_PORT, DHT20_I2C_BAUDRATE);
    gpio_set_function(DHT20_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(DHT20_SCL_PIN, GPIO_FUNC_I2C);

    /** @brief Enables pull-ups on the DHT20 I2C lines. */
    gpio_pull_up(DHT20_SDA_PIN);
    gpio_pull_up(DHT20_SCL_PIN);
}

/**
 * @brief Configures the C/F toggle button as an active-low GPIO input.
 */
static void configure_temperature_toggle_button(void) {
    gpio_init(TEMP_TOGGLE_BUTTON_PIN);
    gpio_set_dir(TEMP_TOGGLE_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(TEMP_TOGGLE_BUTTON_PIN);
}

/**
 * @brief Converts Celsius to Fahrenheit.
 */
static float celsius_to_fahrenheit(float temperature_c) {
    return (temperature_c * 9.0f / 5.0f) + 32.0f;
}

/**
 * @brief Debounces the temperature unit button and reports new presses.
 *
 * Mechanical button contacts can bounce rapidly between pressed and released.
 * This waits briefly, confirms the button is still pressed, then ignores
 * additional pressed readings until the button is released.
 */
static bool temperature_toggle_button_was_pressed(bool* button_was_pressed) {
    const bool button_is_pressed = !gpio_get(TEMP_TOGGLE_BUTTON_PIN);

    if (button_is_pressed && !*button_was_pressed) {
        sleep_ms(BUTTON_DEBOUNCE_MS);

        if (!gpio_get(TEMP_TOGGLE_BUTTON_PIN)) {
            *button_was_pressed = true;
            return true;
        }
    } else if (!button_is_pressed) {
        *button_was_pressed = false;
    }

    return false;
}

/**
 * @brief Polls the button during idle time so short presses are still caught.
 *
 * The sensor only updates every few seconds, but a user can press the button
 * at any time. Sleeping in short poll intervals keeps the button responsive
 * without changing the sensor update interval.
 */
static void sleep_and_check_temperature_button(uint32_t duration_ms,
                                               bool*    use_fahrenheit,
                                               bool*    button_was_pressed) {
    uint32_t elapsed_ms = 0;

    while (elapsed_ms < duration_ms) {
        if (temperature_toggle_button_was_pressed(button_was_pressed)) {
            *use_fahrenheit = !*use_fahrenheit;
            printf("Temperature unit changed to %c\n",
                   *use_fahrenheit ? 'F' : 'C');
        }

        const uint32_t remaining_ms = duration_ms - elapsed_ms;
        const uint32_t delay_ms     = remaining_ms < BUTTON_POLL_INTERVAL_MS
                                          ? remaining_ms
                                          : BUTTON_POLL_INTERVAL_MS;
        sleep_ms(delay_ms);
        elapsed_ms += delay_ms;
    }
}

int main(void) {
    dht20_t         sensor = {0};
    struct lcd_inst lcd    = {
        LCD_I2C_BAUDRATE, LCD_I2C_CONTROLLER, LCD_SDA_PIN, LCD_SCL_PIN, NULL,
    };
    bool use_fahrenheit     = false;
    bool button_was_pressed = false;

    stdio_init_all();
    sleep_ms(1000);

    lcd_init(&lcd);
    configure_dht20_i2c_bus();
    configure_temperature_toggle_button();

    // LED basic hardware check
    // Initialize LED pins and begin ping pong light pattern
    led_init_hardware();
    led_startup_sequence();

    printf("Starting DHT20 Humidity Sensor (SDA=GP%d, SCL=GP%d)\n",
           DHT20_SDA_PIN, DHT20_SCL_PIN);
    lcd_write_string_two_lines(&lcd, "DHT20 starting", "Waiting...");

    /**
     * @brief Retries startup so the device can recover if wiring or power
     * settles late.
     */
    while (!dht20_init(&sensor, DHT20_I2C_PORT, DHT20_DEFAULT_ADDRESS)) {
        printf(
            "DHT20 not ready at 0x%02X. Check to ensure it is properly "
            "connected to the I2C.\n",
            DHT20_DEFAULT_ADDRESS);
        lcd_write_string_two_lines(&lcd, "DHT20 not ready", "Check wiring");

        // Display error pattern on LEDs if sensor is not found
        led_error_flash();

        sleep_ms(SENSOR_UPDATE_INTERVAL_MS);
    }

    printf("DHT20 found at 0x%02X\n", DHT20_DEFAULT_ADDRESS);
    lcd_write_string_two_lines(&lcd, "DHT20 found", "Reading...");

    while (true) {
        /** @brief LCD rows are 16 characters plus a null terminator. */
        char  line1[17]     = {0};
        char  line2[17]     = {0};
        float temperature_c = 0.0f;
        float humidity_rh   = 0.0f;

        if (temperature_toggle_button_was_pressed(&button_was_pressed)) {
            use_fahrenheit = !use_fahrenheit;
            printf("Temperature unit changed to %c\n",
                   use_fahrenheit ? 'F' : 'C');
        }

        if (dht20_read(&sensor, &temperature_c, &humidity_rh)) {
            const float displayed_temperature =
                use_fahrenheit ? celsius_to_fahrenheit(temperature_c)
                               : temperature_c;
            const char temperature_unit = use_fahrenheit ? 'F' : 'C';

            printf("Temp: %.2f %c  Humidity: %.2f %%RH\n",
                   displayed_temperature, temperature_unit, humidity_rh);
            // Update LED display based on humidity reading
            led_humidity_lightup(humidity_rh);

            snprintf(line1, sizeof(line1), "Temp %c%c: %5.1f", DEGREE_SYMB,
                     temperature_unit, displayed_temperature);
            snprintf(line2, sizeof(line2), "Humidity: %5.1f%%", humidity_rh);
            lcd_write_string_two_lines(&lcd, line1, line2);
        } else {
            printf("DHT20 read failed\n");
            lcd_write_string_two_lines(&lcd, "DHT20 read fail", "Retrying...");

            // Blinking error if a read fails
            led_error_flash();
        }

        sleep_and_check_temperature_button(
            SENSOR_UPDATE_INTERVAL_MS, &use_fahrenheit, &button_was_pressed);
    }
}

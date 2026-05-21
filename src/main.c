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
#define BUTTON_DEBOUNCE_MS      50
// Checking every 20 ms lets the app exit idle sleep soon after a button event.
#define BUTTON_POLL_INTERVAL_MS 20

static volatile bool temperature_button_event_pending = false;

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
 * @brief Records button press edges so the main loop can handle them later.
 */
static void gpio_irq_callback(uint gpio, uint32_t events) {
    if (gpio == TEMP_TOGGLE_BUTTON_PIN && (events & GPIO_IRQ_EDGE_FALL)) {
        temperature_button_event_pending = true;
    }
}

/**
 * @brief Configures the C/F toggle button as an active-low GPIO input.
 */
static void configure_temperature_toggle_button(void) {
    gpio_init(TEMP_TOGGLE_BUTTON_PIN);
    gpio_set_dir(TEMP_TOGGLE_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(TEMP_TOGGLE_BUTTON_PIN);
    gpio_set_irq_enabled_with_callback(
        TEMP_TOGGLE_BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_callback);
}

/**
 * @brief Converts Celsius to Fahrenheit.
 */
static float celsius_to_fahrenheit(float temperature_c) {
    return (temperature_c * 9.0f / 5.0f) + 32.0f;
}

/**
 * @brief Toggles the displayed temperature unit.
 */
static void toggle_temperature_unit(bool* use_fahrenheit) {
    *use_fahrenheit = !*use_fahrenheit;
    printf("Temperature unit changed to %c\n", *use_fahrenheit ? 'F' : 'C');
}

/**
 * @brief Debounces pending temperature unit button events.
 *
 * Mechanical button contacts can bounce rapidly between pressed and released.
 * This waits briefly after an interrupt event and confirms the button is still
 * pressed before reporting a toggle request.
 */
static bool temperature_toggle_button_was_pressed(void) {
    static bool button_press_latched = false;

    if (!temperature_button_event_pending) {
        if (button_press_latched && gpio_get(TEMP_TOGGLE_BUTTON_PIN)) {
            button_press_latched = false;
        }

        return false;
    }

    temperature_button_event_pending = false;
    sleep_ms(BUTTON_DEBOUNCE_MS);

    const bool button_is_pressed     = !gpio_get(TEMP_TOGGLE_BUTTON_PIN);
    temperature_button_event_pending = false;

    if (button_is_pressed && !button_press_latched) {
        button_press_latched = true;
        return true;
    }

    if (!button_is_pressed) {
        button_press_latched = false;
    }

    return false;
}

/**
 * @brief Waits for the next sensor read or exits early for a button press.
 *
 * Button presses are recorded by the GPIO interrupt, including presses that
 * happen while the program is updating the sensor, LCD, or LEDs.
 */
static bool sleep_and_check_temperature_button(uint32_t duration_ms) {
    uint32_t elapsed_ms = 0;

    while (elapsed_ms < duration_ms) {
        if (temperature_toggle_button_was_pressed()) {
            return true;
        }

        const uint32_t remaining_ms = duration_ms - elapsed_ms;
        const uint32_t delay_ms     = remaining_ms < BUTTON_POLL_INTERVAL_MS
                                          ? remaining_ms
                                          : BUTTON_POLL_INTERVAL_MS;
        sleep_ms(delay_ms);
        elapsed_ms += delay_ms;
    }

    return false;
}

int main(void) {
    dht20_t         sensor = {0};
    struct lcd_inst lcd    = {
        LCD_I2C_BAUDRATE, LCD_I2C_CONTROLLER, LCD_SDA_PIN, LCD_SCL_PIN, NULL,
    };
    bool use_fahrenheit = false;

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

    // Retry startup so the device can recover if wiring or power settles late.
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
        // LCD rows are 16 characters plus a null terminator.
        char  line1[17]     = {0};
        char  line2[17]     = {0};
        float temperature_c = 0.0f;
        float humidity_rh   = 0.0f;

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

        if (sleep_and_check_temperature_button(SENSOR_UPDATE_INTERVAL_MS)) {
            toggle_temperature_unit(&use_fahrenheit);
        }
    }
}

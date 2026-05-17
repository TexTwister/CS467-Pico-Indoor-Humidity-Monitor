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
// TODO: Choose a GPIO pin for a physical C/F toggle button and define it here.

// Configure the Pico pins used by the DHT20 as an I2C bus.
static void configure_dht20_i2c_bus(void) {
    i2c_init(DHT20_I2C_PORT, DHT20_I2C_BAUDRATE);
    gpio_set_function(DHT20_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(DHT20_SCL_PIN, GPIO_FUNC_I2C);

    // Enable pull-ups on I2C lines
    gpio_pull_up(DHT20_SDA_PIN);
    gpio_pull_up(DHT20_SCL_PIN);
}

int main(void) {
    dht20_t         sensor = {0};
    struct lcd_inst lcd    = {
        LCD_I2C_BAUDRATE, LCD_I2C_CONTROLLER, LCD_SDA_PIN, LCD_SCL_PIN, NULL,
    };

    stdio_init_all();
    sleep_ms(1000);

    lcd_init(&lcd);
    configure_dht20_i2c_bus();
    // TODO: Initialize the C/F toggle button GPIO with a pull-up or pull-down.

    // LED basic hardware check
    // Initialize LED pins and begin ping pong light pattern
    led_init_hardware();
    led_startup_sequence();

    // // For demo purposes on Prog 3/ Final Video
    // run_full_LED_verification();

    printf("Starting DHT20 Humidity Sensor (SDA=GP%d, SCL=GP%d)\n",
           DHT20_SDA_PIN, DHT20_SCL_PIN);
    lcd_write_string_two_lines(&lcd, "DHT20 starting", "Waiting...");

    // Keep retrying startup so the device can recover if wiring or power
    // settles late.
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

        // TODO: Check the button state here and toggle between
        // Celsius/Fahrenheit.
        if (dht20_read(&sensor, &temperature_c, &humidity_rh)) {
            // TODO: Convert temperature_c to Fahrenheit before formatting when
            // Fahrenheit mode is selected.
            printf("Temp: %.2f C  Humidity: %.2f %%RH\n", temperature_c,
                   humidity_rh);
            // Update LED display based on humidity reading       
            led_validated_display(humidity_rh);

            snprintf(line1, sizeof(line1), "Temp (%cC):  %4.1f", DEGREE_SYMB,
                     temperature_c);
            snprintf(line2, sizeof(line2), "Humidity%%:  %4.1f", humidity_rh);
            lcd_write_string_two_lines(&lcd, line1, line2);
        } else {
            printf("DHT20 read failed\n");
            lcd_write_string_two_lines(&lcd, "DHT20 read fail", "Retrying...");
        
            // Blinking error if a read fails
            led_error_flash();
        }

        sleep_ms(SENSOR_UPDATE_INTERVAL_MS);
    }
}

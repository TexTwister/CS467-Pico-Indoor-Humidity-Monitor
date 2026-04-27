#include <stdio.h>

#include "hardware/i2c.h"
#include "pico/stdlib.h"

#include "dht20.h"
#include "lcdlib.h"

#define DHT20_I2C_PORT            i2c0
#define DHT20_I2C_BAUDRATE        100000
#define DHT20_SDA_PIN             20
#define DHT20_SCL_PIN             21
#define LCD_I2C_BAUDRATE          100000
#define LCD_I2C_CONTROLLER        1
#define LCD_SDA_PIN               26
#define LCD_SCL_PIN               27
#define SENSOR_UPDATE_INTERVAL_MS 2000
#define LCD_ROW_2                 0x40

static void configure_dht20_i2c_bus(void) {
    i2c_init(DHT20_I2C_PORT, DHT20_I2C_BAUDRATE);
    gpio_set_function(DHT20_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(DHT20_SCL_PIN, GPIO_FUNC_I2C);

    // Enable pull-ups on I2C lines
    gpio_pull_up(DHT20_SDA_PIN);
    gpio_pull_up(DHT20_SCL_PIN);
}

static void lcd_write_lines(struct lcd_inst* lcd, const char* line1,
                            const char* line2) {
    lcd_clear_screen(lcd);
    sleep_ms(2);
    lcd_set_cursor(lcd, 0);
    lcd_write_string(lcd, (char*)line1);
    lcd_set_cursor(lcd, LCD_ROW_2);
    lcd_write_string(lcd, (char*)line2);
}

int main(void) {
    dht20_t         sensor = {0};
    struct lcd_inst lcd    = {
        LCD_I2C_BAUDRATE, LCD_I2C_CONTROLLER, LCD_SDA_PIN, LCD_SCL_PIN, NULL,
    };
    bool crc_fallback_warned = false;

    stdio_init_all();
    sleep_ms(2000);

    lcd_init(&lcd);
    configure_dht20_i2c_bus();

    printf("Starting DHT20 Humidity Sensor (SDA=GP%d, SCL=GP%d)\n",
           DHT20_SDA_PIN, DHT20_SCL_PIN);
    lcd_write_lines(&lcd, "DHT20 starting", "Waiting...");

    while (!dht20_init(&sensor, DHT20_I2C_PORT, DHT20_DEFAULT_ADDRESS)) {
        printf(
            "DHT20 not ready at 0x%02X. Check to ensure it is properly "
            "connected to the I2C.\n",
            DHT20_DEFAULT_ADDRESS);
        lcd_write_lines(&lcd, "DHT20 not ready", "Check wiring");
        sleep_ms(SENSOR_UPDATE_INTERVAL_MS);
    }

    printf("DHT20 found at 0x%02X\n", DHT20_DEFAULT_ADDRESS);
    lcd_write_lines(&lcd, "DHT20 found", "Reading...");

    while (true) {
        char  line1[17]     = {0};
        char  line2[17]     = {0};
        float temperature_c = 0.0f;
        float humidity_rh   = 0.0f;

        if (dht20_read(&sensor, &temperature_c, &humidity_rh)) {
            printf("Temp: %.2f C  Humidity: %.2f %%RH\n", temperature_c,
                   humidity_rh);
            snprintf(line1, sizeof(line1), "Temp:%6.2f C", temperature_c);
            snprintf(line2, sizeof(line2), "Hum: %6.2f%%RH", humidity_rh);
            lcd_write_lines(&lcd, line1, line2);

            if (sensor.last_read_used_crc_fallback && !crc_fallback_warned) {
                printf(
                    "Warning: accepted DHT20 readings despite CRC mismatch "
                    "(expected 0x%02X, got 0x%02X)\n",
                    sensor.last_crc_expected, sensor.last_crc_received);
                crc_fallback_warned = true;
            }
        } else {
            printf("DHT20 read failed\n");
            lcd_write_lines(&lcd, "DHT20 read fail", "Retrying...");
        }

        sleep_ms(SENSOR_UPDATE_INTERVAL_MS);
    }
}

#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "dht20.h"

#define DHT20_I2C_PORT            i2c0
#define DHT20_I2C_BAUDRATE        100000
#define DHT20_SDA_PIN             4
#define DHT20_SCL_PIN             5
#define SENSOR_UPDATE_INTERVAL_MS 2000

static void configure_i2c_bus(void) {
    i2c_init(DHT20_I2C_PORT, DHT20_I2C_BAUDRATE);
    gpio_set_function(DHT20_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(DHT20_SCL_PIN, GPIO_FUNC_I2C);

    // Enable pull-ups on I2C lines
    gpio_pull_up(DHT20_SDA_PIN);
    gpio_pull_up(DHT20_SCL_PIN);
}

int main(void) {
    dht20_t sensor                  = {0};
    bool    crc_fallback_warned     = false;

    stdio_init_all();
    sleep_ms(2000);

    configure_i2c_bus();

    printf("Starting DHT20 Humidity Sensor (SDA=GP4, SCL=GP5)\n");

    while (!dht20_init(&sensor, DHT20_I2C_PORT, DHT20_DEFAULT_ADDRESS)) {
        printf(
            "DHT20 not ready at 0x%02X. Check to ensure it is properly "
            "connected to the I2C.\n",
            DHT20_DEFAULT_ADDRESS);
        sleep_ms(SENSOR_UPDATE_INTERVAL_MS);
    }

    printf("DHT20 found at 0x%02X\n", DHT20_DEFAULT_ADDRESS);

    while (true) {
        float temperature_c = 0.0f;
        float humidity_rh   = 0.0f;

        if (dht20_read(&sensor, &temperature_c, &humidity_rh)) {
            printf("Temp: %.2f C  Humidity: %.2f %%RH\n", temperature_c,
                   humidity_rh);
            if (sensor.last_read_used_crc_fallback && !crc_fallback_warned) {
                printf(
                    "Warning: accepted DHT20 readings despite CRC mismatch "
                    "(expected 0x%02X, got 0x%02X)\n",
                    sensor.last_crc_expected, sensor.last_crc_received);
                crc_fallback_warned = true;
            }
        } else {
            printf("DHT20 read failed\n");
        }

        sleep_ms(SENSOR_UPDATE_INTERVAL_MS);
    }
    // TODO: Add C/F conversion
}

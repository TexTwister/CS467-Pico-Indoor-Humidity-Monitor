#include "dht20.h"

#include <stddef.h>

#include "pico/stdlib.h"

/**
 * @brief Enables optional DHT20 measurement CRC validation.
 *
 * Most AHT20/DHT20-compatible breakout examples read the 6-byte measurement
 * payload. Compile with -DCHECK_CRC=1 for sensor variants that send the
 * seventh CRC byte.
 */
#ifndef CHECK_CRC
#define CHECK_CRC 0
#endif

#define DHT20_INITIALIZE_COMMAND          0xBE
#define DHT20_TRIGGER_MEASUREMENT_COMMAND 0xAC

#define DHT20_BUSY_MASK       0x80
#define DHT20_CALIBRATED_MASK 0x18

#define DHT20_POWER_ON_DELAY_MS      100
#define DHT20_INIT_DELAY_MS          10
#define DHT20_MEASUREMENT_DELAY_MS   80
#define DHT20_READY_POLL_INTERVAL_MS 5
#define DHT20_READY_TIMEOUT_MS       150

#if CHECK_CRC == 1
#define DHT20_DATA_FRAME_SIZE 7
#else
#define DHT20_DATA_FRAME_SIZE 6
#endif

/**
 * @brief Reads the DHT20 status byte.
 *
 * @param sensor DHT20 sensor configuration.
 * @param status Destination for the returned status byte.
 * @return true when the status byte is read successfully, false otherwise.
 */
static bool dht20_read_status(const dht20_t* sensor, uint8_t* status) {
    return i2c_read_blocking(sensor->i2c, sensor->address, status, 1, false) ==
           1;
}

/**
 * @brief Waits for the DHT20 to clear its busy bit.
 *
 * @param sensor DHT20 sensor configuration.
 * @param timeout_ms Maximum time to wait, in milliseconds.
 * @return true when the sensor is ready, false on timeout or I2C failure.
 */
static bool dht20_wait_until_ready(const dht20_t* sensor, uint32_t timeout_ms) {
    const absolute_time_t deadline = make_timeout_time_ms(timeout_ms);
    uint8_t               status   = 0;

    while (true) {
        if (!dht20_read_status(sensor, &status)) {
            return false;
        }

        if ((status & DHT20_BUSY_MASK) == 0) {
            return true;
        }

        if (time_reached(deadline)) {
            return false;
        }

        sleep_ms(DHT20_READY_POLL_INTERVAL_MS);
    }
}

#if CHECK_CRC == 1
/**
 * @brief Calculates the DHT20 CRC-8 for a measurement frame.
 *
 * @param data Measurement frame bytes.
 * @param length Number of bytes to include in the CRC calculation.
 * @return Calculated CRC-8 value.
 */
static uint8_t dht20_crc8(const uint8_t* data, size_t length) {
    uint8_t crc = 0xFF;

    for (size_t index = 0; index < length; ++index) {
        crc ^= data[index];

        for (uint bit = 0; bit < 8; ++bit) {
            if ((crc & 0x80u) != 0) {
                crc = (uint8_t)((crc << 1) ^ 0x31u);
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}
#endif

/**
 * @brief Stores the I2C configuration and prepares the DHT20 for reads.
 *
 * @param sensor DHT20 sensor state to initialize.
 * @param i2c I2C instance connected to the sensor.
 * @param address Sensor I2C address, or 0 to use DHT20_DEFAULT_ADDRESS.
 * @return true when the sensor is reachable and calibrated, false otherwise.
 */
bool dht20_init(dht20_t* sensor, i2c_inst_t* i2c, uint8_t address) {
    uint8_t status = 0;

    if (sensor == NULL || i2c == NULL) {
        return false;
    }

    // Address 0 means "use the DHT20 default" for convenience at call sites.
    sensor->i2c     = i2c;
    sensor->address = address == 0 ? DHT20_DEFAULT_ADDRESS : address;

    sleep_ms(DHT20_POWER_ON_DELAY_MS);

    if (!dht20_read_status(sensor, &status)) {
        return false;
    }

    if ((status & DHT20_CALIBRATED_MASK) != DHT20_CALIBRATED_MASK) {
        const uint8_t init_command[3] = {DHT20_INITIALIZE_COMMAND, 0x08, 0x00};

        if (i2c_write_blocking(sensor->i2c, sensor->address, init_command, 3,
                               false) < 0) {
            return false;
        }

        sleep_ms(DHT20_INIT_DELAY_MS);

        if (!dht20_read_status(sensor, &status)) {
            return false;
        }
    }

    return (status & DHT20_CALIBRATED_MASK) == DHT20_CALIBRATED_MASK;
}

/**
 * @brief Reads one DHT20 temperature and humidity sample.
 *
 * @param sensor Initialized DHT20 sensor state.
 * @param temperature_c Destination for temperature in degrees Celsius.
 * @param humidity_rh Destination for relative humidity percentage.
 * @return true when a valid sample is read, false otherwise.
 */
bool dht20_read(const dht20_t* sensor, float* temperature_c,
                float* humidity_rh) {
    const uint8_t trigger_command[3] = {DHT20_TRIGGER_MEASUREMENT_COMMAND, 0x33,
                                        0x00};
    uint8_t       data[DHT20_DATA_FRAME_SIZE] = {0};
    uint32_t      raw_humidity                = 0;
    uint32_t      raw_temperature             = 0;

    if (sensor == NULL || temperature_c == NULL || humidity_rh == NULL) {
        return false;
    }

    if (!dht20_wait_until_ready(sensor, DHT20_READY_TIMEOUT_MS)) {
        return false;
    }

    if (i2c_write_blocking(sensor->i2c, sensor->address, trigger_command, 3,
                           false) < 0) {
        return false;
    }

    sleep_ms(DHT20_MEASUREMENT_DELAY_MS);

    if (!dht20_wait_until_ready(sensor, DHT20_READY_TIMEOUT_MS)) {
        return false;
    }

    if (i2c_read_blocking(sensor->i2c, sensor->address, data, sizeof(data),
                          false) != (int)sizeof(data)) {
        return false;
    }

    if ((data[0] & DHT20_BUSY_MASK) != 0) {
        return false;
    }

#if CHECK_CRC == 1
    // Reject corrupted frames instead of displaying questionable readings.
    const uint8_t expected_crc = dht20_crc8(data, 6);
    if (expected_crc != data[6]) {
        return false;
    }
#endif

    // Humidity and temperature are packed as two 20-bit unsigned values.
    raw_humidity = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) |
                   ((uint32_t)data[3] >> 4);

    raw_temperature = (((uint32_t)data[3] & 0x0Fu) << 16) |
                      ((uint32_t)data[4] << 8) | (uint32_t)data[5];

    *humidity_rh   = ((float)raw_humidity * 100.0f) / 1048576.0f;
    *temperature_c = ((float)raw_temperature * 200.0f) / 1048576.0f - 50.0f;

    return true;
}

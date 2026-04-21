#include "dht20.h"

#include <stddef.h>

#include "pico/stdlib.h"

#define DHT20_STATUS_COMMAND              0x71
#define DHT20_INITIALIZE_COMMAND          0xBE
#define DHT20_TRIGGER_MEASUREMENT_COMMAND 0xAC

#define DHT20_BUSY_MASK       0x80
#define DHT20_CALIBRATED_MASK 0x18

#define DHT20_POWER_ON_DELAY_MS      100
#define DHT20_INIT_DELAY_MS          10
#define DHT20_MEASUREMENT_DELAY_MS   80
#define DHT20_READY_POLL_INTERVAL_MS 5
#define DHT20_READY_TIMEOUT_MS       150
#define DHT20_DATA_FRAME_SIZE        7

static bool dht20_measurement_looks_sane(uint8_t status, const uint8_t* data) {
    uint32_t raw_humidity    = 0;
    uint32_t raw_temperature = 0;
    float    humidity_rh     = 0.0f;
    float    temperature_c   = 0.0f;

    if (data == NULL) {
        return false;
    }

    if ((status & DHT20_BUSY_MASK) != 0) {
        return false;
    }

    raw_humidity = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) |
                   ((uint32_t)data[3] >> 4);
    raw_temperature = (((uint32_t)data[3] & 0x0Fu) << 16) |
                      ((uint32_t)data[4] << 8) | (uint32_t)data[5];

    humidity_rh   = ((float)raw_humidity * 100.0f) / 1048576.0f;
    temperature_c = ((float)raw_temperature * 200.0f) / 1048576.0f - 50.0f;

    return humidity_rh >= 0.0f && humidity_rh <= 100.0f &&
           temperature_c >= -40.0f && temperature_c <= 125.0f;
}

static bool dht20_read_status(const dht20_t* sensor, uint8_t* status) {
    uint8_t command = DHT20_STATUS_COMMAND;

    if (sensor == NULL || status == NULL) {
        return false;
    }

    if (i2c_write_blocking(sensor->i2c, sensor->address, &command, 1, true) <
        0) {
        return false;
    }

    return i2c_read_blocking(sensor->i2c, sensor->address, status, 1, false) ==
           1;
}

static bool dht20_wait_until_ready(const dht20_t* sensor, uint32_t timeout_ms) {
    absolute_time_t deadline = make_timeout_time_ms(timeout_ms);
    uint8_t         status   = 0;

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

bool dht20_init(dht20_t* sensor, i2c_inst_t* i2c, uint8_t address) {
    uint8_t status = 0;

    if (sensor == NULL || i2c == NULL) {
        return false;
    }

    sensor->i2c                         = i2c;
    sensor->address                     = address == 0 ? DHT20_DEFAULT_ADDRESS : address;
    sensor->last_read_used_crc_fallback = false;
    sensor->last_crc_expected           = 0;
    sensor->last_crc_received           = 0;

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

bool dht20_read(dht20_t* sensor, float* temperature_c, float* humidity_rh) {
    const uint8_t trigger_command[3] = {DHT20_TRIGGER_MEASUREMENT_COMMAND, 0x33,
                                        0x00};
    uint8_t       data[DHT20_DATA_FRAME_SIZE] = {0};
    uint32_t      raw_humidity                = 0;
    uint32_t      raw_temperature             = 0;
    uint8_t       expected_crc                = 0;

    if (sensor == NULL || temperature_c == NULL || humidity_rh == NULL) {
        return false;
    }

    sensor->last_read_used_crc_fallback = false;
    sensor->last_crc_expected           = 0;
    sensor->last_crc_received           = 0;

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

    expected_crc = dht20_crc8(data, 6);
    sensor->last_crc_expected = expected_crc;
    sensor->last_crc_received = data[6];
    if (expected_crc != data[6]) {
        if (!dht20_measurement_looks_sane(data[0], data)) {
            return false;
        }

        sensor->last_read_used_crc_fallback = true;
    }

    raw_humidity = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) |
                   ((uint32_t)data[3] >> 4);

    raw_temperature = (((uint32_t)data[3] & 0x0Fu) << 16) |
                      ((uint32_t)data[4] << 8) | (uint32_t)data[5];

    *humidity_rh   = ((float)raw_humidity * 100.0f) / 1048576.0f;
    *temperature_c = ((float)raw_temperature * 200.0f) / 1048576.0f - 50.0f;

    return true;
}

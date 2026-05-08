#ifndef DHT20_H
#define DHT20_H

#include <stdbool.h>
#include <stdint.h>

#include "hardware/i2c.h"

/** @brief Standard DHT20 I2C address. */
#define DHT20_DEFAULT_ADDRESS 0x38

/**
 * @brief Minimal DHT20 driver state shared between init and read calls.
 */
typedef struct {
    i2c_inst_t* i2c;
    uint8_t     address;
} dht20_t;

/**
 * @brief Initializes the sensor on the given I2C bus.
 *
 * @param sensor DHT20 sensor state to initialize.
 * @param i2c I2C instance connected to the sensor.
 * @param address Sensor I2C address, or 0 to use DHT20_DEFAULT_ADDRESS.
 * @return true when the sensor is reachable and calibrated, false otherwise.
 */
bool dht20_init(dht20_t* sensor, i2c_inst_t* i2c, uint8_t address);

/**
 * @brief Reads one temperature and humidity sample.
 *
 * @param sensor Initialized DHT20 sensor state.
 * @param temperature_c Destination for temperature in degrees Celsius.
 * @param humidity_rh Destination for relative humidity percentage.
 * @return true when a sample is read successfully; false on I2C, busy, or CRC failure.
 */
bool dht20_read(const dht20_t* sensor, float* temperature_c, float* humidity_rh);

#endif

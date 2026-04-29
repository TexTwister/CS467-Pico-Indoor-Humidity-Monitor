#ifndef DHT20_H
#define DHT20_H

#include <stdbool.h>
#include <stdint.h>

#include "hardware/i2c.h"

#define DHT20_DEFAULT_ADDRESS 0x38 // Standard DHT20 I2C address.

// Minimal DHT20 driver state shared between init/read calls.
typedef struct {
    i2c_inst_t* i2c;
    uint8_t     address;
} dht20_t;

// Initialize the sensor on the given I2C bus; pass address 0 for the default.
bool dht20_init(dht20_t* sensor, i2c_inst_t* i2c, uint8_t address);

// Read one temperature/humidity sample.
// Returns false on I2C, busy, or CRC failure.
bool dht20_read(dht20_t* sensor, float* temperature_c, float* humidity_rh);

#endif

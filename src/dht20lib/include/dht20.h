#ifndef DHT20_H
#define DHT20_H

#include <stdbool.h>
#include <stdint.h>

#include "hardware/i2c.h"

#define DHT20_DEFAULT_ADDRESS 0x38

typedef struct {
    i2c_inst_t* i2c;
    uint8_t     address;
    bool        last_read_used_crc_fallback;
    uint8_t     last_crc_expected;
    uint8_t     last_crc_received;
} dht20_t;

bool dht20_init(dht20_t* sensor, i2c_inst_t* i2c, uint8_t address);
bool dht20_read(dht20_t* sensor, float* temperature_c, float* humidity_rh);

#endif

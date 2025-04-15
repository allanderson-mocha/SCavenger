#ifndef I2C_H
#define I2C_H

#include <stdint.h>

void i2c_init(uint8_t bdiv);
uint8_t i2c_io(uint8_t device_addr, uint8_t *wp, uint16_t wn, uint8_t *rp, uint16_t rn);

#endif

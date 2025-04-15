#ifndef LIGHT_H
#define LIGHT_H

#include <stdint.h>

#define I2C_DEVICE_ADDR 0x29

/**
 * @brief Initializes the TSL2591 light sensor.
 *
 * This function sends an enable signal to the light sensor through I2C for later reading.
 */
void light_init(void);

/**
 * @brief Reads the lux from light sensor.
 *
 * This function retrieves sensor value bytes then performs arithmetic to return a float lux value.
 * @return The measured lux.
 */
float get_light(void);

#endif
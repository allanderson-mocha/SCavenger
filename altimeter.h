#ifndef ALTIMETER_H
#define ALTIMETER_H

#include <stdint.h>

#define F_CPU 8000000UL      // 8 MHz clock speed
#define MPL3115A2_ADDRESS 0x60 // 7-bit address


/**
 * @brief Initializes the MPL3115A2 altimeter.
 *
 * This function configures the MPL3115A2 into altimeter mode with an oversampling rate (OSR) of 128.
 * It sets the device to Standby mode to apply configuration settings before switching it to Active mode.
 */
void altimeter_init(void);

/**
 * @brief Retrieves the current altitude from the MPL3115A2 sensor.
 *
 * Reads three bytes from the sensor, combines them into a 20-bit altitude value,
 * and returns the result as a 32-bit signed integer.
 *
 * @return The measured altitude.
 */
// int32_t get_altitude(void);
float get_altitude(void);
float get_temperature(void);


#endif
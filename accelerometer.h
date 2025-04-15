#ifndef ACC_H
#define ACC_H

#include <stdint.h>

#define LIS3DH_WRITE (0x18 << 1)  /**< I2C address for writing to the LIS3DH accelerometer */

/**
 * @brief Initializes the LIS3DH accelerometer.
 *
 * Configures the LIS3DH to enable all three axes with a sample rate of 100Hz.
 * Must be called before attempting to read acceleration values.
 */
void accel_init(void);

/**
 * @brief Reads acceleration data from the LIS3DH sensor.
 *
 * Retrieves the latest X, Y, and Z acceleration values and stores them
 * as signed 16-bit integers in the array pointed to by `coords`.
 * 
 * @param coords Pointer to an array of 3 int16_t values to store X, Y, and Z axis data.
 */
void get_accel(int16_t* coords);

#endif

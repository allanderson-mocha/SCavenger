#ifndef ESP_H
#define ESP_H

#include <stdint.h>

/**
 * @brief Send GPS coordinates to the ESP32 over I2C and receive up to 3 nearby cafe coordinates.
 * 
 * @param coords         A null-terminated string representing user GPS coordinates, e.g., "34.0522,-118.2437".
 * @param top_coords     A 2D array to store up to 3 received coordinates (each up to 31 characters + null terminator).
 * 
 * Initialize top_coords in main as empty array to get overwritten every time you call.
 */
void get_nearby_cafes(const char* coords, char top_coords[3][32]);

#endif
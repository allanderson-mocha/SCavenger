#ifndef GPS_H
#define GPS_H

#include <stdint.h>

#define F_CPU 8000000UL
// Size of GPS buffer for storing incoming NMEA sentences
#define GPS_BUFFER_SIZE 128

// Buffer to hold the latest full GPS sentence
extern char gps_buffer[GPS_BUFFER_SIZE];

// Index for filling gps_buffer
extern int gps_index;


/**
 * @brief Initializes UART hardware for communication with GPS module.
 *        Sets baud rate to 9600, 8 data bits, no parity, 1 stop bit.
 */
void gps_init(void);

/**
 * @brief Receives a single character from UART.
 * 
 * @return Received character from GPS.
 */
char gps_serial_in(void);

/**
 * @brief Reads a complete GPS NMEA sentence into gps_buffer.
 *        Reads characters until a newline character is detected.
 */
void read_gps_sentence(void);

/**
 * @brief Parses latitude and longitude from a $GPRMC NMEA sentence.
 * 
 * @param sentence Pointer to the GPS NMEA sentence (must start with $GPRMC).
 * @param latitude Pointer to store parsed latitude (in decimal degrees).
 * @param longitude Pointer to store parsed longitude (in decimal degrees).
 */
void parse_gps_coordinates(char* sentence, float* latitude, float* longitude);

#endif
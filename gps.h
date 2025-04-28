#ifndef GPS_H
#define GPS_H

#include <avr/io.h>

#define GPS_BUFFER_SIZE 128

extern char gps_buffer[GPS_BUFFER_SIZE];
extern volatile int gps_index;
extern volatile uint8_t gps_ready;

void gps_init(void);
char gps_serial_in(void);
void read_gps_sentence(void);
void parse_gps_coordinates(char* sentence, float* latitude, float* longitude);
uint8_t gps_sentence_ready(void);
void gps_reset(void);

#endif
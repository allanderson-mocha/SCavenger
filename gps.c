#include "gps.h"
#include <string.h>
#include <stdlib.h>

char gps_buffer[GPS_BUFFER_SIZE];
int gps_index = 0;

// UART initialization
void gps_init(void) {
    unsigned int ubrr = 47; // For 9600 baud with 8MHz clock
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << TXEN0) | (1 << RXEN0); // Enable TX and RX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8-bit, no parity, 1 stop bit
}

// UART receive
char gps_serial_in(void) {
    while (!(UCSR0A & (1 << RXC0))); // Wait for data
    return UDR0;
}

// Read one full GPS sentence
void read_gps_sentence(void) {
    gps_index = 0;
    while (1) {
        char c = gps_serial_in();
        if (c == '\n') {
            gps_buffer[gps_index] = '\0'; // Null terminate
            break;
        }
        if (gps_index < GPS_BUFFER_SIZE - 1) {
            gps_buffer[gps_index++] = c;
        }
    }
}

// Parse latitude and longitude from $GPRMC
void parse_gps_coordinates(char* sentence, float* latitude, float* longitude) {
    if (strncmp(sentence, "$GPRMC", 6) != 0 && strncmp(sentence, "$GPGGA", 6) != 0) {
        return;
    }

    char* token;
    int field = 0;
    char lat_str[16] = {0};
    char lat_dir = 'N';
    char lon_str[16] = {0};
    char lon_dir = 'E';

    token = strtok(sentence, ",");
    while (token != NULL) {
        field++;

        if (field == 3) {
            strncpy(lat_str, token, sizeof(lat_str) - 1);
        } else if (field == 4) {
            lat_dir = token[0];
        } else if (field == 5) {
            strncpy(lon_str, token, sizeof(lon_str) - 1);
        } else if (field == 6) {
            lon_dir = token[0];
            break;
        }
        token = strtok(NULL, ",");
    }

    float lat = atof(lat_str);
    float lon = atof(lon_str);

    int lat_deg = (int)(lat / 100);
    float lat_min = lat - (lat_deg * 100);
    *latitude = lat_deg + lat_min / 60.0;
    if (lat_dir == 'S') *latitude = -*latitude;

    int lon_deg = (int)(lon / 100);
    float lon_min = lon - (lon_deg * 100);
    *longitude = lon_deg + lon_min / 60.0;
    if (lon_dir == 'W') *longitude = -*longitude;
}

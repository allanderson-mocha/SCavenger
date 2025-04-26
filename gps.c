#include <avr/io.h>
#include "gps.h"
#include <string.h>
#include <stdlib.h>

// NEED IN MAIN
#define GPS_BUFFER_SIZE 128
char gps_buffer[GPS_BUFFER_SIZE];
int gps_index = 0;


// Function to initialize UART
void gps_init() {
    unsigned int ubrr = 47;
    UBRR0H = (unsigned char)(ubrr >> 8);  // Set high byte
    UBRR0L = (unsigned char)ubrr;         // Set low byte

    UCSR0B = (1 << TXEN0) | (1 << RXEN0); // Enable TX and RX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8-bit data, no parity, 1 stop bit
}


// Function to receive a character over UART
char gps_serial_in(void) {
    while (!(UCSR0A & (1 << RXC0))); // Wait for data to be received
    return UDR0;  // Read received data
}

// Function to read GPS
void read_gps_sentence() {
    gps_index = 0;
    while (1) {
        char c = gps_serial_in();  // Read a character from GPS

        if (c == '\n') {
            gps_buffer[gps_index] = '\0';  // Null-terminate the string
            break;
        }
        
        if (gps_index < GPS_BUFFER_SIZE - 1) {
            gps_buffer[gps_index++] = c;  // Save the character
        }
    }
}

// Coordinates Parser
void parse_gps_coordinates(char* sentence, float* latitude, float* longitude) {
    if (strncmp(sentence, "$GPRMC", 6) != 0) {
        return; // Not a GPRMC sentence
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

        if (field == 3) { // Latitude
            strncpy(lat_str, token, sizeof(lat_str) - 1);
        } else if (field == 4) { // Latitude direction
            lat_dir = token[0];
        } else if (field == 5) { // Longitude
            strncpy(lon_str, token, sizeof(lon_str) - 1);
        } else if (field == 6) { // Longitude direction
            lon_dir = token[0];
            break; // We have both lat and lon now
        }

        token = strtok(NULL, ",");
    }

    // Convert NMEA format to decimal degrees
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
/* 
*** EXAMPLE MAIN ***
int main(void) {
    gps_init(); // Initialize UART
    led_init()'

    float latitude = 0.0;
    float longitude = 0.0;

    while (1) {
        read_gps_sentence(); // Get a full line into gps_buffer

        if (strncmp(gps_buffer, "$GPRMC", 6) == 0) {
            parse_gps_coordinates(gps_buffer, &latitude, &longitude);

            char buffer[32];
            snprintf(buffer, sizeof(buffer), "Lat: %.6f", latitude);
            led_stringout(buffer)'

            snprintf(buffer, sizeof(buffer), "Lon: %.6f", longitude);
            led_stringout(buffer)'
        }
    }

    return 0;
}
*/
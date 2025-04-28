#include "gps.h"
#include "lcd.h"
#include <string.h>
#include <stdlib.h>
#include <avr/interrupt.h>

char gps_buffer[GPS_BUFFER_SIZE];
volatile int gps_index = 0;
volatile uint8_t gps_ready = 0;

void gps_init(void) {
    unsigned int ubrr = 47; // For 9600 baud with 8MHz clock
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0); // Enable RX, TX, and RX Complete Interrupt
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8-bit data, 1 stop bit, no parity
}

// This function now just returns whether a full sentence is ready
uint8_t gps_sentence_ready(void) {
    return gps_ready;
}

// Reset buffer when done
void gps_reset(void) {
    gps_index = 0;
    gps_ready = 0;
    gps_buffer[0] = '\0';
}

ISR(USART_RX_vect) {
    static char prev_c = 0; // Keep track of the previous character

    char c = UDR0; // Read incoming byte

    if (prev_c == '\r' && c == '\n') {
        // Proper end of GPS sentence
        if (gps_index > 0) gps_buffer[gps_index - 1] = '\0'; // Overwrite '\r' with '\0'
        gps_ready = 1;
        gps_index = 0; // Reset for next sentence
    } else {
        if (gps_index < GPS_BUFFER_SIZE - 1) {
            gps_buffer[gps_index++] = c;
        } else {
            gps_index = 0; // Overflow safety: restart sentence
        }
    }

    prev_c = c; // Update previous character
}



void parse_gps_coordinates(char* sentence, float* latitude, float* longitude) {
    uint8_t is_rmc = 0;
    if (strncmp(sentence, "$GPRMC", 6) == 0) {
        is_rmc = 1;
    } else if (strncmp(sentence, "$GPGGA", 6) == 0) {
        is_rmc = 0;
    } else {
        return; // Not a supported sentence
    }

    // Make a temporary copy so strtok doesn't modify the original
    static char temp[GPS_BUFFER_SIZE];
    strncpy(temp, sentence, GPS_BUFFER_SIZE - 1);
    temp[GPS_BUFFER_SIZE - 1] = '\0';

    char* token;
    int field = 0;
    char lat_str[16] = {0};
    char lat_dir = 'N';
    char lon_str[16] = {0};
    char lon_dir = 'E';

    token = strtok(temp, ",");
    while (token != NULL) {
        field++;

        if (is_rmc) {
            // $GPRMC field numbers
            if (field == 4) {
                strncpy(lat_str, token, sizeof(lat_str) - 1);
            } else if (field == 5) {
                lat_dir = token[0];
            } else if (field == 6) {
                strncpy(lon_str, token, sizeof(lon_str) - 1);
            } else if (field == 7) {
                lon_dir = token[0];
                break;
            }
        } else {
            // $GPGGA field numbers
            if (field == 2) {
                strncpy(lat_str, token, sizeof(lat_str) - 1);
            } else if (field == 3) {
                lat_dir = token[0];
            } else if (field == 4) {
                strncpy(lon_str, token, sizeof(lon_str) - 1);
            } else if (field == 5) {
                lon_dir = token[0];
                break;
            }
        }

        token = strtok(NULL, ",");
    }

    // DEBUG
    // lcd_moveto(0,0);
    // lcd_stringout(lat_str);
    // lcd_moveto(1,0);
    // lcd_stringout(lon_str);

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


// #include "gps.h"
// #include <string.h>
// #include <stdlib.h>

// char gps_buffer[GPS_BUFFER_SIZE];
// int gps_index = 0;

// // UART initialization
// void gps_init(void) {
//     unsigned int ubrr = 47; // For 9600 baud with 8MHz clock
//     UBRR0H = (unsigned char)(ubrr >> 8);
//     UBRR0L = (unsigned char)ubrr;
//     UCSR0B = (1 << TXEN0) | (1 << RXEN0); // Enable TX and RX
//     UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8-bit, no parity, 1 stop bit
// }

// // UART receive
// char gps_serial_in(void) {
//     while (!(UCSR0A & (1 << RXC0))); // Wait for data
//     return UDR0;
// }

// // Read one full GPS sentence
// void read_gps_sentence(void) {
//     gps_index = 0;
//     while (1) {
//         char c = gps_serial_in();
//         if (c == '\n') {
//             gps_buffer[gps_index] = '\0'; // Null terminate
//             break;
//         }
//         if (gps_index < GPS_BUFFER_SIZE - 1) {
//             gps_buffer[gps_index++] = c;
//         }
//     }
// }

// Parse latitude and longitude from $GPRMC or $GPGGA
// void parse_gps_coordinates(char* sentence, float* latitude, float* longitude) {
//     if (strncmp(sentence, "$GPRMC", 6) != 0 && strncmp(sentence, "$GPGGA", 6) != 0) {
//         return; // Not a supported sentence
//     }

//     char* token;
//     int field = 0;
//     char lat_str[16] = {0};
//     char lat_dir = 'N';
//     char lon_str[16] = {0};
//     char lon_dir = 'E';

//     token = strtok(sentence, ",");
//     while (token != NULL) {
//         field++;

//         if (field == 3) {
//             strncpy(lat_str, token, sizeof(lat_str) - 1);
//         } else if (field == 4) {
//             lat_dir = token[0];
//         } else if (field == 5) {
//             strncpy(lon_str, token, sizeof(lon_str) - 1);
//         } else if (field == 6) {
//             lon_dir = token[0];
//             break;
//         }
//         token = strtok(NULL, ",");
//     }

//     float lat = atof(lat_str);
//     float lon = atof(lon_str);

//     int lat_deg = (int)(lat / 100);
//     float lat_min = lat - (lat_deg * 100);
//     *latitude = lat_deg + lat_min / 60.0;
//     if (lat_dir == 'S') *latitude = -*latitude;

//     int lon_deg = (int)(lon / 100);
//     float lon_min = lon - (lon_deg * 100);
//     *longitude = lon_deg + lon_min / 60.0;
//     if (lon_dir == 'W') *longitude = -*longitude;
// }


#define F_CPU 16000000UL  // 16 MHz clock speed
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

// --- UART Initialization ---
void uart_init(unsigned int baud) {
    unsigned int ubrr = F_CPU/16/baud - 1;
    UBRR0H = (ubrr >> 8);
    UBRR0L = ubrr;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);     // Enable receiver and transmitter
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);   // 8-bit data
}

void uart_tx(char c) {
    while (!(UCSR0A & (1 << UDRE0))); // Wait until buffer is empty
    UDR0 = c;
}

char uart_rx() {
    while (!(UCSR0A & (1 << RXC0))); // Wait for data
    return UDR0;
}

void uart_print(const char* str) {
    while (*str) {
        uart_tx(*str++);
    }
}

// --- GPS Parsing ---
#define BUFFER_SIZE 128
char buffer[BUFFER_SIZE];

void read_nmea_sentence() {
    char c;
    uint8_t idx = 0;

    // Wait for start of NMEA sentence
    while ((c = uart_rx()) != '$');

    buffer[idx++] = '$';

    while (idx < BUFFER_SIZE - 1) {
        c = uart_rx();
        buffer[idx++] = c;
        if (c == '\n') break;  // End of NMEA sentence
    }

    buffer[idx] = '\0';  // Null terminate
}

int main(void) {
    uart_init(9600);
    _delay_ms(1000);

    uart_print("GPS UART Test Start\r\n");

    while (1) {
        read_nmea_sentence();

        // Filter only GPRMC or GPGGA sentences
        if (strstr(buffer, "$GPRMC") || strstr(buffer, "$GPGGA")) {
            uart_print(">> ");
            uart_print(buffer);
        }
    }
}

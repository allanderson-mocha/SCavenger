#include <avr/io.h>
#include "gps.h"

// Function to initialize UART
void gps_init() {
    unsigned int ubrr = 47;
    UBRR0H = (unsigned char)(ubrr >> 8);  // Set high byte
    UBRR0L = (unsigned char)ubrr;         // Set low byte

    UCSR0B = (1 << TXEN0) | (1 << RXEN0); // Enable TX and RX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8-bit data, no parity, 1 stop bit
}

// Function to transmit a character over UART
void gps_serial_out(char data) {
    while (!(UCSR0A & (1 << UDRE0))); // Wait for empty transmit buffer
    UDR0 = data;  // Load data into the UART register
}

// Function to receive a character over UART
char gps_serial_in(void) {
    while (!(UCSR0A & (1 << RXC0))); // Wait for data to be received
    return UDR0;  // Read received data
}

/* 
*** EXAMPLE MAIN ***
int main(void) {
    // Set PD2 and PD3 as outputs
    DDRD |= (1 << PD2) | (1 << PD3);

    // Set PD2 and PD3 LOW
    PORTD &= ~((1 << PD2) | (1 << PD3));

    while (1) {
        char received = gps_serial_in();  // Read character from UART
        gps_serial_out(received);         // Send the received character back
    }

    return 0;
}
*/
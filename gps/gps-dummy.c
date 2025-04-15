#define F_CPU 16000000UL  // System clock frequency (16MHz)
#include <avr/io.h>

// Function to initialize UART
void serial_init(unsigned int ubrr) {
    UBRR0H = (unsigned char)(ubrr >> 8);  // Set high byte
    UBRR0L = (unsigned char)ubrr;         // Set low byte

    UCSR0B = (1 << TXEN0) | (1 << RXEN0); // Enable TX and RX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8-bit data, no parity, 1 stop bit
}

// Function to transmit a character over UART
void serial_out(char data) {
    while (!(UCSR0A & (1 << UDRE0))); // Wait for empty transmit buffer
    UDR0 = data;  // Load data into the UART register
}

// Function to receive a character over UART
char serial_in(void) {
    while (!(UCSR0A & (1 << RXC0))); // Wait for data to be received
    return UDR0;  // Read received data
}

int main(void) {
    serial_init(47);  // Set baud rate to 9600 (UBRR = 47 for 7.3728MHz)

    // Set PD2 and PD3 as outputs
    DDRD |= (1 << PD2) | (1 << PD3);

    // Set PD2 and PD3 LOW
    PORTD &= ~((1 << PD2) | (1 << PD3));

    while (1) {
        char received = serial_in();  // Read character from UART
        serial_out(received);         // Send the received character back
    }

    return 0;
}

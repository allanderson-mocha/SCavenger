#ifndef GPS_H
#define GPS_H

#define F_CPU 16000000UL

/**
 * @brief Initializes the UART hardware for GPS communication.
 *
 * This sets the baud rate (UBRR0), enables transmitter and receiver,
 * and configures the frame format to 8 data bits, no parity, and 1 stop bit.
 * 
 * Baud rate is hardcoded for 9600 with 8MHz clock (UBRR = 47).
 */
void gps_init(void);

/**
 * @brief Sends a single character over UART.
 *
 * Blocks until the UART transmit buffer is empty, then writes the character
 * to the UART Data Register (UDR0).
 * 
 * @param data The character to transmit.
 */
void gps_serial_out(char data);

/**
 * @brief Receives a single character from UART.
 *
 * Blocks until a character has been received via UART,
 * then returns the received character.
 *
 * @return The received character.
 */
char gps_serial_in(void);

#endif
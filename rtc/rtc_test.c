#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#define F_CPU 16000000UL // 16 MHz clock speed
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1
#define DS3231_ADDRESS 0x68

void uart_init(unsigned int ubrr) {
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_transmit(char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void uart_print(const char *str) {
    while (*str) {
        uart_transmit(*str++);
    }
}

void i2c_init() {
    TWSR = 0x00;
    TWBR = 0x48; // Set SCL to 100kHz (assuming 16MHz CPU clock)
    TWCR = (1 << TWEN);
}

void i2c_start() {
    TWCR = (1 << TWSTA) | (1 << TWEN) | (1 << TWINT);
    while (!(TWCR & (1 << TWINT)));
}

void i2c_stop() {
    TWCR = (1 << TWSTO) | (1 << TWEN) | (1 << TWINT);
}

void i2c_write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWEN) | (1 << TWINT);
    while (!(TWCR & (1 << TWINT)));
}

uint8_t i2c_read_ack() {
    TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWEA);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

uint8_t i2c_read_nack() {
    TWCR = (1 << TWEN) | (1 << TWINT);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

void ds3231_read_time() {
    char buffer[20];
    i2c_start();
    i2c_write((DS3231_ADDRESS << 1) | 0x00); // Write mode
    i2c_write(0x00); // Start register (Seconds)
    i2c_start();
    i2c_write((DS3231_ADDRESS << 1) | 0x01); // Read mode
    uint8_t seconds = i2c_read_ack();
    uint8_t minutes = i2c_read_ack();
    uint8_t hours = i2c_read_nack();
    i2c_stop();
    snprintf(buffer, sizeof(buffer), "Time: %02x:%02x:%02x\r\n", hours, minutes, seconds);
    uart_print(buffer);
}

int main() {
    uart_init(MYUBRR);
    i2c_init();
    _delay_ms(1000);
    uart_print("DS3231 Test\r\n");
    while (1) {
        ds3231_read_time();
        _delay_ms(1000);
    }
}

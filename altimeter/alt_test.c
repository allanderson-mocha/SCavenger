#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#define F_CPU 16000000UL // 16 MHz clock speed
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1
#define MPL3115A2_ADDRESS 0x60

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
    TWBR = 0x48;
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

void mpl3115a2_init() {
    i2c_start();
    i2c_write((MPL3115A2_ADDRESS << 1) | 0x00);
    i2c_write(0x26); // Control register
    i2c_write(0xB9); // Altimeter mode, OSR=128
    i2c_stop();
    _delay_ms(500);
}

void mpl3115a2_read_altitude() {
    char buffer[20];
    i2c_start();
    i2c_write((MPL3115A2_ADDRESS << 1) | 0x00);
    i2c_write(0x01); // OUT_P_MSB register
    i2c_start();
    i2c_write((MPL3115A2_ADDRESS << 1) | 0x01);
    uint8_t msb = i2c_read_ack();
    uint8_t csb = i2c_read_ack();
    uint8_t lsb = i2c_read_nack();
    i2c_stop();
    
    int32_t altitude = ((int32_t)msb << 10) | ((int32_t)csb << 2) | ((lsb >> 6) & 0x03);
    snprintf(buffer, sizeof(buffer), "Altitude: %ldm\r\n", altitude);
    uart_print(buffer);
}

int main() {
    uart_init(MYUBRR);
    i2c_init();
    _delay_ms(1000);
    uart_print("MPL3115A2 Test\r\n");
    mpl3115a2_init();
    while (1) {
        mpl3115a2_read_altitude();
        _delay_ms(1000);
    }
}

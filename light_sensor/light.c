#define F_CPU 16000000UL // tells compiler running on 16 mHz

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include "lcd.h"

// === I2C Setup ===
// I2C Setup Functions
void i2c_init(void) {
    TWSR = 0x00; // Disables prescaler ->prescale = 1
    TWBR = 0x48; // Sets I2C speed (72 in decimal)
    TWCR = (1 << TWEN); // Enables I2C (TWI) hardware
}

uint8_t i2c_start(uint8_t addr) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); // sends a START condition on the I2C bus
    uint16_t timeout = 10000;
    while (!(TWCR & (1 << TWINT)) && --timeout); // Wait for the start to complete
    if (timeout == 0) return 0;

    TWDR = addr; // loads 8 bit address (device address + R/W bit)
    TWCR = (1 << TWINT) | (1 << TWEN); // Starts transmission
    timeout = 10000;
    while (!(TWCR & (1 << TWINT)) && --timeout); // Wait for ACK
    if (timeout == 0) return 0;

    uint8_t status = TWSR & 0xF8; // mask out status bits
    return (status == 0x18 || status == 0x40);
}

void i2c_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO); // Send STOP condition
}

int main(void) {
    lcd_init();
    i2c_init();
    _delay_ms(100);

    lcd_moveto(0, 0);
    //lcd_stringout("Checking I2C...");
    _delay_ms(500);

    lcd_moveto(1, 0);

    if (i2c_start((0x29 << 1) | 0)) {
        lcd_stringout("Sensor Found");
    } else {
        lcd_stringout("I2C FAIL     ");
    }
    i2c_stop();

    while (1);
    return 0;
}


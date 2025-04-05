#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include "lcd.h"     // Include the LCD functions from lcd-test.c
#include "i2c.h"     // Include the I2C functions from i2c.c

// #define I2C_DEVICE_ADDR 0x28  // Example I2C address (you may need to change this)
#define I2C_DEVICE_ADDR 0x52

char buffer[17];  // For printing to LCD

int main(void) {
    uint8_t status;
    uint8_t rbuf[1];               // Buffer to hold I2C response
    // uint8_t wbuf[1] = {0x00};      // Register address or command to send (customize as needed)
    // uint8_t wbuf[1] = {0x12};    // <- correct register for ID
    uint8_t wbuf[1] = {0xB2};   // COMMAND_BIT (0xA0) | 0x12



    lcd_init();                    // Initialize LCD
    i2c_init(72);                  // Initialize I2C with BRDIV = 72 (~100kHz for 8MHz clock)

    lcd_moveto(0, 0);
    lcd_stringout("Reading I2C...");

    _delay_ms(500);

    status = i2c_io(I2C_DEVICE_ADDR, wbuf, 1, rbuf, 1);  // Write then read 1 byte

    lcd_moveto(1, 0);
    if (status == 0) {
        snprintf(buffer, sizeof(buffer), "Data: 0x%02X", rbuf[0]);
        lcd_stringout(buffer);
    } else {
        snprintf(buffer, sizeof(buffer), "Err: 0x%02X", status);
        lcd_stringout(buffer);
    }

    while (1) {
        // Loop forever
    }

    return 0;
}

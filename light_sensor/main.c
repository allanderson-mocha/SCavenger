#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include "lcd.h"     // Include the LCD functions from lcd-test.c
#include "i2c.h"     // Include the I2C functions from i2c.c

#define I2C_DEVICE_ADDR 0x29  // Example I2C address (you may need to change this)

char buffer[17];  // For printing to LCD

int main(void) {
    uint8_t write_addr = I2C_DEVICE_ADDR << 1; // 0x52
    uint8_t read_addr = (I2C_DEVICE_ADDR << 1) | 0x01; // 0x53

    uint8_t status;
    uint8_t rbuf[1];               // Buffer to hold I2C response
    uint8_t wbuf[1] = {0xB2};   // COMMAND_BIT (0xA0) | 0x12

    lcd_init();                    // Initialize LCD
    i2c_init(72);                  // Initialize I2C with BRDIV = 72 (~100kHz for 8MHz clock)

    lcd_moveto(0, 0);
    lcd_stringout("Reading Light...");

    _delay_ms(500);

    status = i2c_io(write_addr, wbuf, 1, rbuf, 1);  // Write then read 1 byte

    lcd_moveto(1, 0);
    if (status == 0) {
        snprintf(buffer, sizeof(buffer), "Data: 0x%02X", rbuf[0]);
        lcd_stringout(buffer);
    } else {
        snprintf(buffer, sizeof(buffer), "Err: 0x%02X", status);
        lcd_stringout(buffer);
    }

    _delay_ms(500);

    uint8_t enable_buf[] = { 0xA0, 0x03 }; // addr = ENABLE; AEN | PON
    status = i2c_io(write_addr, enable_buf, 2, NULL, 0);
    if (status != 0) {
        lcd_moveto(2, 0);
        snprintf(buffer, sizeof(buffer), "Enable Err: %02X", status);
        lcd_stringout(buffer);
        while (1);
    }

    _delay_ms(120);  // Wait for sensor to collect light data

    while (1) {
        uint8_t reg_addr[1] = {0xA0 | 0x14}; // CMD | starting at 0x14 with auto-increment
        uint8_t light_buf[4] = {0};
        status = i2c_io(write_addr, reg_addr, 1, light_buf, 4);
        if (status != 0) {
            lcd_moveto(2, 0);
            snprintf(buffer, sizeof(buffer), "Read Err: %02X", status);
            lcd_stringout(buffer);
            while (1);
        }
    
        uint16_t ch0 = (light_buf[1] << 8) | light_buf[0];
        uint16_t ch1 = (light_buf[3] << 8) | light_buf[2];
    
        float atime = 100.0;    // ms (default)
        float again = 25.0;     // default gain
        float cpl = (atime * again) / 408.0;
        float lux = (ch0 - ch1) * (1.0 / cpl);
    
        lcd_moveto(0, 20);
        snprintf(buffer, sizeof(buffer), "Lux: %3d", (int)lux);
        lcd_stringout(buffer);
    }

    return 0;
}

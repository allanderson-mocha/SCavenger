#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include "lcd.h"     // Include LCD functions
#include "i2c.h"     // Include I2C functions

#define I2C_DEVICE_ADDR 0x30  // 8-bit I2C address for LIS3DH

char buffer[17];  // For printing to LCD

int main(void) {
    uint8_t status;
    uint8_t wbuf[2];     // Write buffer (register + value or register address)
    uint8_t rbuf[6];     // Read buffer for acceleration data

    lcd_init();                    // Initialize LCD
    i2c_init(72);                  // Initialize I2C (~100kHz for 8MHz clock)

    lcd_moveto(0, 0);
    lcd_stringout("Configuring...");

    _delay_ms(500);

    // --- Write CTRL_REG1 (0x20) ---
    wbuf[0] = 0x20;  // CTRL_REG1 address
    wbuf[1] = 0x27;  // Value: 10Hz, normal mode, XYZ enabled
    status = i2c_io(I2C_DEVICE_ADDR, wbuf, 2, NULL, 0);

    // --- Write CTRL_REG4 (0x23) ---
    wbuf[0] = 0x23;  // CTRL_REG4 address
    wbuf[1] = 0x88;  // Value: block data update, high resolution
    status = i2c_io(I2C_DEVICE_ADDR, wbuf, 2, NULL, 0);

    _delay_ms(500);

    // lcd_moveto(1, 0);
    // lcd_stringout("Setup Done");   
    // _delay_ms(500);

    // --- Main loop: read sensor values ---
    while (1) {
        uint8_t status_reg;

        // Step 1: Read STATUS_REG (0x27)
        wbuf[0] = 0x27;  // STATUS_REG address
        status = i2c_io(I2C_DEVICE_ADDR, wbuf, 1, &status_reg, 1);

        lcd_moveto(0, 0);
        if (status == 0) {
            snprintf(buffer, sizeof(buffer), "Data: 0x%02X", status_reg);
            lcd_stringout(buffer);
        } else {
            snprintf(buffer, sizeof(buffer), "Err: 0x%02X", status);
            lcd_stringout(buffer);
        }

        // Step 2: If data is ready (ZYXDA == 1), read acceleration
        if ((status == 0) && (status_reg & 0x08)) {
            wbuf[0] = 0x28;  // OUT_X_L address
            status = i2c_io(I2C_DEVICE_ADDR, wbuf, 1, rbuf, 6);

            lcd_moveto(1, 0);
            if (status == 0) {
                int16_t x = (rbuf[1] << 8) | rbuf[0];
                int16_t y = (rbuf[3] << 8) | rbuf[2];
                int16_t z = (rbuf[5] << 8) | rbuf[4];

                snprintf(buffer, sizeof(buffer), "X:%d", x);   // Showing X value
                lcd_stringout(buffer);
            } else {
                snprintf(buffer, sizeof(buffer), "Err: 0x%02X", status);
                lcd_stringout(buffer);
            }
        } else {
            // No new data ready
            lcd_moveto(1, 0);
            lcd_stringout("No new data  ");
        }

        _delay_ms(200);  // Polling delay
    }

    return 0;
}

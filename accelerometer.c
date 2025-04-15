#include "accelerometer.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "lcd.h"
#include "i2c.h"

#define LIS3DH_WRITE (0x18 << 1)

void accel_init() {
    char buffer[17];
    uint8_t init_buf[] = {0x20, 0x57};
    uint8_t status = i2c_io(LIS3DH_WRITE, init_buf, 2, NULL, 0);
    if (status != 0) {
        lcd_moveto(0, 0);
        snprintf(buffer, sizeof(buffer), "Acc Error: %02X", status);
        lcd_stringout(buffer);
        _delay_ms(2000);
        return;
    }
}

void get_accel(int16_t* coords) {
    char buffer[17];
    uint8_t reg_addr = 0x28 | 0x80; // auto-increment bit
    uint8_t data[6] = {0};
    uint8_t status = i2c_io(LIS3DH_WRITE, &reg_addr, 1, data, 6);
    if (status != 0) {
        lcd_moveto(1, 0);
        snprintf(buffer, sizeof(buffer), "Read Err: %02X", status);
        lcd_stringout(buffer);
        _delay_ms(2000);
        return;
    }

        // Combine low/high bytes (data is in 2's complement, 10-bit left-aligned in 16-bit)
    int16_t x = (int16_t)((data[1] << 8) | data[0]) >> 6;
    int16_t y = (int16_t)((data[3] << 8) | data[2]) >> 6;
    int16_t z = (int16_t)((data[5] << 8) | data[4]) >> 6;

    coords[0] = x;
    coords[1] = y;
    coords[2] = z;

}

/*
***EXAMPLE USE***

int main(void) {
    lcd_init();
    i2c_init(72);  // 100kHz if F_CPU = 8MHz

    _delay_ms(100); // wait for LIS3DH to power up

    // Configure CTRL_REG1 (0x20) = 0x57 (ODR=100Hz, XYZ enabled)
    uint8_t init_buf[] = {0x20, 0x57};
    uint8_t status = i2c_io(LIS3DH_WRITE, init_buf, 2, NULL, 0);
    if (status != 0) {
        lcd_moveto(0, 0);
        snprintf(buffer, sizeof(buffer), "Init Error: %02X", status);
        lcd_stringout(buffer);
        while (1);
    }

    lcd_moveto(0, 0);
    lcd_stringout("Accel Ready");

    _delay_ms(500);

    while (1) {
        // Read 6 bytes from OUT_X_L (0x28) with auto-increment
        uint8_t reg_addr = 0x28 | 0x80; // auto-increment bit
        uint8_t data[6] = {0};
        status = i2c_io(LIS3DH_WRITE, &reg_addr, 1, data, 6);
        if (status != 0) {
            lcd_moveto(1, 0);
            snprintf(buffer, sizeof(buffer), "Read Err: %02X", status);
            lcd_stringout(buffer);
            continue;
        }

        // Combine low/high bytes (data is in 2's complement, 10-bit left-aligned in 16-bit)
        int16_t x = (int16_t)((data[1] << 8) | data[0]) >> 6;
        int16_t y = (int16_t)((data[3] << 8) | data[2]) >> 6;
        int16_t z = (int16_t)((data[5] << 8) | data[4]) >> 6;

        lcd_moveto(0, 0);
        snprintf(buffer, sizeof(buffer), "X:%4d", x);
        lcd_stringout(buffer);

        lcd_moveto(1, 0);
        snprintf(buffer, sizeof(buffer), "Y:%4d", y);
        lcd_stringout(buffer);

        lcd_moveto(0, 20);
        snprintf(buffer, sizeof(buffer), "Z:%4d", z);
        lcd_stringout(buffer);

        _delay_ms(250); // update rate
    }

    return 0;
}
*/

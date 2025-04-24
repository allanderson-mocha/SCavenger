#include "accelerometer.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "i2c.h"
#include "lcd.h"  // Optional: only needed for debugging

#define LIS3DH_ADDR 0x18
#define LIS3DH_WRITE (LIS3DH_ADDR << 1)

static int16_t prev_z = 0;
static uint8_t step_cooldown = 0;

void accel_init() {
    uint8_t init_buf[] = {0x20, 0x57}; // ODR=100Hz, enable XYZ
    i2c_io(LIS3DH_WRITE, init_buf, 2, NULL, 0);
}

void get_accel(int16_t* coords) {
    uint8_t reg_addr = 0x28 | 0x80; // Auto-increment starting at OUT_X_L
    uint8_t data[6] = {0};
    uint8_t status = i2c_io(LIS3DH_WRITE, &reg_addr, 1, data, 6);

    if (status != 0) {
        coords[0] = coords[1] = coords[2] = 0;
        return;
    }

    coords[0] = (int16_t)((data[1] << 8) | data[0]) >> 6;
    coords[1] = (int16_t)((data[3] << 8) | data[2]) >> 6;
    coords[2] = (int16_t)((data[5] << 8) | data[4]) >> 6;
}

uint8_t detect_step(int16_t z_val) {
    int16_t delta = z_val - prev_z;
    prev_z = z_val;

    if (step_cooldown > 0) {
        step_cooldown--;
        return 0;
    }

    if (delta > 30 || delta < -30) { // Tune as needed
        step_cooldown = 10;
        return 1;
    }

    return 0;
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

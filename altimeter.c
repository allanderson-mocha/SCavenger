#include "altimeter.h"
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "i2c.h"
#include "lcd.h"

// Initialize the MPL3115A2 altimeter in altimeter mode, OSR = 128
void altimeter_init(void) {
    uint8_t wbuf[2];

    // Enter Standby mode (SBYB=0) to configure settings
    wbuf[0] = 0x26; // CTRL_REG1 address
    wbuf[1] = 0xB8; 
    if (i2c_io(MPL3115A2_ADDRESS << 1, wbuf, 2, NULL, 0) != 0) {
        lcd_moveto(1, 0);
        lcd_stringout("Alt. write error");
        return;
    }
    _delay_ms(10);

    // Set Active mode (SBYB=1) to start measurement
    wbuf[1] = 0xB9; // Same as before, but now SBYB=1
    if (i2c_io(MPL3115A2_ADDRESS << 1, wbuf, 2, NULL, 0) != 0) {
        lcd_moveto(1, 0);
        lcd_stringout("Alt. start error");
        return;
    }

    _delay_ms(500); // Let sensor stabilize
}

// Read altitude from the MPL3115A2 and display the value on the LCD
int32_t get_altitude(void) {
    char buffer[17]; // Enough for a 16-character LCD line + null terminator
    uint8_t wbuf[1] = { 0x01 }; // Register address for OUT_P_MSB
    uint8_t rbuf[3];
    
    // Write one byte (the register address) then read 3 bytes from the device
    if(i2c_io(MPL3115A2_ADDRESS << 1, wbuf, 1, rbuf, 3) != 0) {
        lcd_moveto(1, 0);
        lcd_stringout("I2C read error  ");
        return;
    }
    
    // Combine the three bytes into a 20-bit altitude value
    int32_t altitude = ((int32_t)rbuf[0] << 10) | ((int32_t)rbuf[1] << 2) | ((rbuf[2] >> 6) & 0x03);
    
    /*
    EXAMPLE ALTITUDE LCD DISPLAY
    // Format the altitude string (ensure it fits on 16 characters)
    snprintf(buffer, sizeof(buffer), "Alt: %ld m   ", altitude);
    
    // Display the altitude on the first line of the LCD
    lcd_moveto(1, 0);
    lcd_stringout(buffer);
    */
   return altitude;
}
// Read temperature from MPL3115A2 in Celsius
float get_temperature(void) {
    uint8_t wbuf[1] = { 0x04 };  // OUT_T_MSB register
    uint8_t rbuf[2];

    if (i2c_io(MPL3115A2_ADDRESS << 1, wbuf, 1, rbuf, 2) != 0) {
        lcd_moveto(1, 0);
        lcd_stringout("Temp read error ");
        return -1000.0;  // Return an out-of-range value on error
    }

    int16_t temp_raw = ((int16_t)rbuf[0] << 8) | rbuf[1];
    temp_raw >>= 4;  // 12-bit signed

    return temp_raw / 16.0;  // Convert to °C
}

/*
TEST CODE EXAMPLE

int main(void) {
    lcd_init();          // Initialize the LCD display
    i2c_init(72);        // Initialize I2C (from i2c.c) with bit rate divisor 72
    _delay_ms(1000);     // Wait for devices to settle
    
    // Display a startup message on the LCD
    lcd_moveto(0, 0);
    lcd_stringout("MPL3115A2 Test ");
    
    altimeter_init();    // Initialize the altimeter

    // Example when just displaying and not saving altitude value
    while (1) {
        get_altitude(); // Read and display altitude
        _delay_ms(1000);
    }
    
    return 0;
}
*/ 
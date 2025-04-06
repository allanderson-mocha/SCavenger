#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "i2c.h"
#include "lcd.h"

#define F_CPU 16000000UL      // 16 MHz clock speed
#define MPL3115A2_ADDRESS 0x60 // 7-bit address

// Initialize the MPL3115A2 altimeter in altimeter mode, OSR = 128
void mpl3115a2_init(void) {
    uint8_t wbuf[2];
    wbuf[0] = 0x26; // Control register address
    wbuf[1] = 0xB9; // Altimeter mode, OSR=128
    // Write 2 bytes: register address then the configuration data
    if(i2c_io(MPL3115A2_ADDRESS << 1, wbuf, 2, NULL, 0) != 0) {
        lcd_moveto(1, 0);
        lcd_stringout("I2C write error");
    }
    _delay_ms(500);
}

// Read altitude from the MPL3115A2 and display the value on the LCD
void mpl3115a2_read_altitude(void) {
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
    // Format the altitude string (ensure it fits on 16 characters)
    snprintf(buffer, sizeof(buffer), "Alt: %ld m   ", altitude);
    
    // Display the altitude on the first line of the LCD
    lcd_moveto(1, 0);
    lcd_stringout(buffer);
}

int main(void) {
    lcd_init();          // Initialize the LCD display
    i2c_init(72);        // Initialize I2C (from i2c.c) with bit rate divisor 72
    _delay_ms(1000);     // Wait for devices to settle
    
    // Display a startup message on the LCD
    lcd_moveto(0, 0);
    lcd_stringout("MPL3115A2 Test ");
    
    mpl3115a2_init();    // Initialize the altimeter
    
    while (1) {
        mpl3115a2_read_altitude(); // Read and display altitude
        _delay_ms(1000);
    }
    
    return 0;
}

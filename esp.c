#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "i2c.h"
#include "lcd.h"  // If you want to display on LCD
#include "esp.h"

#define ESP32_ADDR 0x28
#define F_CPU 8000000UL
#define BAUD 9600

void get_nearby_cafes(const char *coords, char top_coords[3][32]) {
    uint8_t status;
    char send_buf[32];
    char read_buf[32];
    strncpy(send_buf, coords, sizeof(send_buf) - 1);
    send_buf[sizeof(send_buf) - 1] = '\0';

    // Send coordinates to ESP
    status = i2c_io(ESP32_ADDR << 1, (uint8_t*)send_buf, strlen(send_buf), (uint8_t*)read_buf, 3);
    read_buf[3] = '\0';

    _delay_ms(3000);

    for (int i = 0; i < 3; i++) {
        strcpy(send_buf, "NEXT");
        memset(read_buf, 0, sizeof(read_buf));
        status = i2c_io(ESP32_ADDR << 1, (uint8_t*)send_buf, 4, (uint8_t*)read_buf, 31);
        read_buf[31] = '\0';

        if (strncmp(read_buf, "DONE", 4) == 0) break;
        
        strncpy(top_coords[i], read_buf, sizeof(top_coords[i]) - 1);    // Save coords
        top_coords[i][31] = '\0';   // Ensure null-terminated
        
        _delay_ms(2000);
    }
}

int main(void) {
    char top_coords[3][32];
    

    i2c_init(72);  // 100kHz with TWBR=72 @ 16MHz
    lcd_init();
    _delay_ms(1000);

    lcd_moveto(0, 0);
    lcd_stringout("Coordinates:")
    get_nearby_cafes("34.0522,-118.2437", top_coords);
    
    while (1){
        for(int i = 0, i < 3; i++) {
            lcd_moveto(1,0);
            lcd_stringout(top_coords[i]);
        }
    };
}

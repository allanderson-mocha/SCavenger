#include "altimeter.h"
#include "light_sensor.h"
#include "accelerometer.h"
#include "i2c.h"
#include "lcd.h"

void main() {
    // Device initializations
    lcd_init();
    i2c_init(72);
    altimeter_init();
    light_init();
    accel_init();
    int16_t coords[3];  // Array containing coords to be updated by get_accel
    get_accel(coords);

    
}
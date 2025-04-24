#ifndef LED_H
#define LED_H

#include <avr/io.h>
#include <stdint.h>

// LED pin assignments
#define LED1 PB0
#define LED2 PB7
#define LED3 PD2

// Function declarations
void led_init(void);
void morse_update(uint16_t elapsed_ms);
void morse_led_on(void);
void morse_led_off(void);

#endif

#ifndef LED_H
#define LED_H

#include <avr/io.h>
#include <stdint.h>

// LED pin assignments
// #define LED1 PB0
#define LED2 PB7

// Function declarations
void led_init(void);
void led_on(uint8_t port, uint8_t pin);
void led_off(uint8_t port, uint8_t pin);
void led_blink(uint8_t port, uint8_t pin, uint16_t duration_ms);

#endif

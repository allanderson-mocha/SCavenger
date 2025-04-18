#include "led.h"
#include <util/delay.h>

void led_init(void) {
    // Set LED pins as output
    DDRB |= (1 << LED1); 
    DDRB |= (1 << LED2);
}

void led_on(uint8_t port, uint8_t pin) {
    if (port == PORTB) PORTB |= (1 << pin);
    // else if (port == PORTC) PORTC |= (1 << pin);
}

void led_off(uint8_t port, uint8_t pin) {
    if (port == PORTB) PORTB &= ~(1 << pin);
    // else if (port == PORTC) PORTC &= ~(1 << pin);
}

void led_blink(uint8_t port, uint8_t pin, uint16_t duration_ms) {
    led_on(port, pin);

    for (uint16_t i = 0; i < duration_ms; i++) {
        _delay_ms(1);
    }

    led_off(port, pin);
    _delay_ms(100);
}


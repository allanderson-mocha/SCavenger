#include "led.h"
#include <util/delay.h>

#define DOT_DURATION 200
#define DASH_DURATION 600
#define SYMBOL_GAP 200
#define LETTER_GAP 600
#define WORD_GAP 1000

void led_init(void) {
    // Set LED pins as output
    DDRB |= (1 << LED1) | (1 << LED2); 
    DDRD |= (1 << LED3);
}

void led_on(uint8_t port, uint8_t pin) {
    if (port == PORTB) PORTB |= (1 << pin);
    else if (port == PORTD) PORTD |= (1 << pin);
}

void led_off(uint8_t port, uint8_t pin) {
    if (port == PORTB) PORTB &= ~(1 << pin);
    else if (port == PORTD) PORTD &= ~(1 << pin);
}

void led_blink(uint8_t port, uint8_t pin, uint16_t duration_ms) {
    led_on(port, pin);

    for (uint16_t i = 0; i < duration_ms; i++) {
        _delay_ms(1);
    }

    led_off(port, pin);
    _delay_ms(100);
}
void morse_led_on() {
    PORTB |= (1 << LED1);
    PORTB |= (1 << LED2);
    PORTD |= (1 << LED3);
}

void morse_led_off() {
    PORTB &= ~(1 << LED1);
    PORTB &= ~(1 << LED2);
    PORTD &= ~(1 << LED3);
}

void morse_dot() {
    morse_led_on();
    _delay_ms(DOT_DURATION);
    morse_led_off();
    _delay_ms(SYMBOL_GAP);
}

void morse_dash() {
    morse_led_on();
    _delay_ms(DASH_DURATION);
    morse_led_off();
    _delay_ms(SYMBOL_GAP);
}

void morse_letter_gap() {
    _delay_ms(LETTER_GAP);
}

void morse_word_gap() {
    _delay_ms(WORD_GAP);
}
void morse_freeze_me() {
    // Morse code for "FREEZE ME"

    // F: ..-.
    morse_dot(); morse_dot(); morse_dash(); morse_dot();
    morse_letter_gap();

    // R: .-.
    morse_dot(); morse_dash(); morse_dot();
    morse_letter_gap();

    // E
    morse_dot();
    morse_letter_gap();

    // E
    morse_dot();
    morse_letter_gap();

    // Z: --..
    morse_dash(); morse_dash(); morse_dot(); morse_dot();
    morse_letter_gap();

    // E
    morse_dot();
    morse_word_gap();

    // M: --
    morse_dash(); morse_dash();
    morse_letter_gap();

    // E
    morse_dot();
    morse_word_gap();
}


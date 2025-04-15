#include "buzz.h"
#include <avr/io.h>
#include <util/delay.h>


void buzzer_init() {
    // Set BUZZER_PIN as an output
    DDRD |= (1 << BUZZER_PIN);
}

void buzzer_on() {
    // Set BUZZER_PIN high
    PORTD |= (1 << BUZZER_PIN);
}

void buzzer_off() {
    // Set BUZZER_PIN low
    PORTD &= ~(1 << BUZZER_PIN);
}

void beep(uint16_t time) {
    buzzer_on();
    _delay_ms(time);    // Wait 500ms
    buzzer_off();      // Turn buzzer OFF
    _delay_ms(time);
}

/*
EXAMPLE USE

int main(void) {
    buzzer_init();  // Set up the buzzer pin

    while (1) {
        buzzer_on();       // Turn buzzer ON
        _delay_ms(500);    // Wait 500ms
        buzzer_off();      // Turn buzzer OFF
        _delay_ms(500);    // Wait 500ms
    }
}
*/
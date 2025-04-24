#include "buzz.h"
#include <avr/io.h>
#include <util/delay.h>


void buzzer_init() {
    // Set BUZZER_PIN as an output
    DDRB |= (1 << BUZZER_PIN);
}

void buzzer_on() {
    // Set BUZZER_PIN high
    PORTB |= (1 << BUZZER_PIN);
}

void buzzer_off() {
    // Set BUZZER_PIN low
    PORTB &= ~(1 << BUZZER_PIN);
}
void success_sound() {
    play_note(880, 100);   // A5, short
    _delay_ms(50);
    play_note(988, 100);   // B5
    _delay_ms(50);
    play_note(1046, 100);  // C6
}


void error_sound() {
    play_note(220, 150);  // Low buzz
    _delay_ms(50);
    play_note(220, 150);
}


// void play_note(unsigned short freq)
// {
//     unsigned long period = 1000000UL / freq;
//     unsigned long loop_delay = (period / 2) / 10;

//     for (unsigned short i = 0; i < freq; i++) {
//         PORTB |= (1 << PB1);

//         for (unsigned long d = 0; d < loop_delay; d++) {
//             _delay_us(10);
//         }

//         PORTB &= ~(1 << PB1);

//         for (unsigned long d = 0; d < loop_delay; d++) {
//             _delay_us(10);
//         }
//     }
// }

void play_note(unsigned short freq, unsigned short duration_ms) {
    unsigned long period = 1000000UL / freq;
    unsigned long half_period = period / 2;
    unsigned long cycles = (1000UL * duration_ms) / period;

    for (unsigned long i = 0; i < cycles; i++) {
        PORTB |= (1 << PB1);

        for (unsigned long d = 0; d < (half_period / 10); d++) {
            _delay_us(10);
        }

        PORTB &= ~(1 << PB1);

        for (unsigned long d = 0; d < (half_period / 10); d++) {
            _delay_us(10);
        }
    }
}

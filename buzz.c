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


// void play_note(unsigned short freq)
// {
//     unsigned long period = 1000000UL / freq;

//     for (unsigned short i = 0; i < freq; i++) {
//         PORTB |= (1 << PB1);
//         _delay_us(period / 2);
//         PORTB &= ~(1 << PB1);
//         _delay_us(period / 2);
//     }
// }
void play_note(unsigned short freq)
{
    unsigned long period = 1000000UL / freq;
    unsigned long loop_delay = (period / 2) / 10;

    for (unsigned short i = 0; i < freq; i++) {
        PORTB |= (1 << PB1);

        for (unsigned long d = 0; d < loop_delay; d++) {
            _delay_us(10);
        }

        PORTB &= ~(1 << PB1);

        for (unsigned long d = 0; d < loop_delay; d++) {
            _delay_us(10);
        }
    }
}

// void play_note(unsigned short freq)
// {
//     unsigned long period;
//     period = 1000000UL / freq;  // Period in microseconds

//     // Play for approx 1 second (freq cycles)
//     for (unsigned short i = 0; i < freq; i++) {
//         PORTB |= (1 << PB1);             // Set PB1 high
//         variable_delay_us(period / 2);   // Delay half period
//         PORTB &= ~(1 << PB1);            // Set PB1 low
//         variable_delay_us(period / 2);   // Delay half period
//     }
// }



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
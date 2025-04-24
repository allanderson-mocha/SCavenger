#include "led.h"

// Morse struct and sequence
typedef struct {
    uint8_t on;        // 1 = LED ON, 0 = LED OFF
    uint16_t duration; // Duration in milliseconds
} MorseStep;

#define UNIT 200
#define PAUSE_DURATION 10000

MorseStep morse_freeze_me_sequence[] = {
    // F: ..-.
    {1, UNIT}, {0, UNIT}, {1, UNIT}, {0, UNIT},
    {1, UNIT * 3}, {0, UNIT}, {1, UNIT}, {0, UNIT * 3},
    // R: .-.
    {1, UNIT}, {0, UNIT}, {1, UNIT * 3}, {0, UNIT},
    {1, UNIT}, {0, UNIT * 3},
    // E
    {1, UNIT}, {0, UNIT * 3},
    // E
    {1, UNIT}, {0, UNIT * 3},
    // Z: --..
    {1, UNIT * 3}, {0, UNIT}, {1, UNIT * 3}, {0, UNIT},
    {1, UNIT}, {0, UNIT}, {1, UNIT}, {0, UNIT * 3},
    // E
    {1, UNIT}, {0, UNIT * 7},
    // M: --
    {1, UNIT * 3}, {0, UNIT}, {1, UNIT * 3}, {0, UNIT * 3},
    // E
    {1, UNIT}, {0, UNIT * 7},
};
const uint8_t morse_sequence_length = sizeof(morse_freeze_me_sequence) / sizeof(MorseStep);

// Internal state variables
static uint16_t morse_timer = 0;
static uint32_t pause_timer = 0;
static uint8_t morse_index = 0;
static uint8_t in_pause = 0;

void led_init(void) {
    DDRB |= (1 << LED1) | (1 << LED2); 
    DDRD |= (1 << LED3);
}

void morse_update(uint16_t elapsed_ms) {
    if (in_pause) {
        pause_timer += elapsed_ms;
        if (pause_timer >= PAUSE_DURATION) {
            pause_timer = 0;
            morse_index = 0;
            in_pause = 0;
        }
        return;
    }

    MorseStep step = morse_freeze_me_sequence[morse_index];

    if (step.on) morse_led_on();
    else morse_led_off();

    morse_timer += elapsed_ms;

    if (morse_timer >= step.duration) {
        morse_timer = 0;
        morse_index++;
        if (morse_index >= morse_sequence_length) {
            morse_led_off();
            in_pause = 1;
        }
    }
}

void morse_led_on(void) {
    PORTB |= (1 << LED1);
    PORTB |= (1 << LED2);
    PORTD |= (1 << LED3);
}

void morse_led_off(void) {
    PORTB &= ~(1 << LED1);
    PORTB &= ~(1 << LED2);
    PORTD &= ~(1 << LED3);
}

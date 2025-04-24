#include <avr/io.h>
#include <stdint.h>
#include "buzz.h"

typedef struct {
    uint16_t freq;
    uint16_t duration;
} Note;

#define VICTORY_NOTE_COUNT 4

Note victory_notes[VICTORY_NOTE_COUNT] = {
    {1318, 100},  // E6
    {1568, 100},  // G6
    {1760, 150},  // A6
    {1396, 200}   // F#6
};

uint8_t current_note_index = 0;
uint8_t victory_playing = 0;

volatile uint8_t sound_active = 0;
uint16_t sound_timer = 0;  // milliseconds remaining
volatile SoundMode current_sound_mode = SOUND_NONE;

void sound_play(uint16_t freq, uint16_t duration_ms, SoundMode mode) {
    // If a higher-priority sound is playing, ignore lower ones
    if (current_sound_mode == SOUND_VICTORY && mode != SOUND_VICTORY)
        return;

    uint32_t top = F_CPU / (2UL * freq);  // Calculate TOP value for CTC

    DDRB |= (1 << PB1);  // Set PB1 (OC1A) as output

    // Configure Timer1 in CTC mode, toggle OC1A on compare match
    TCCR1A = (1 << COM1A0);              // Toggle OC1A on compare match
    TCCR1B = (1 << WGM12) | (1 << CS10); // CTC mode, no prescaler

    OCR1A = (uint16_t)(top - 1);

    sound_timer = duration_ms;
    sound_active = 1;
    current_sound_mode = mode;
}

void sound_update(uint16_t elapsed_ms) {
    if (sound_active) {
        if (elapsed_ms >= sound_timer) {
            sound_stop();
        } else {
            sound_timer -= elapsed_ms;
        }
    }
}

void sound_stop() {
    TCCR1A = 0;
    TCCR1B = 0;
    PORTB &= ~(1 << PB1);  // Ensure buzzer is off
    sound_active = 0;
    current_sound_mode = SOUND_NONE;
}

void buzzer_init() {
    DDRB |= (1 << PB1); // Set PB1 as output
    PORTB &= ~(1 << PB1); // Ensure buzzer is off initially
}

void play_victory_sound() {
    current_note_index = 0;
    current_sound_mode = SOUND_VICTORY;
    victory_playing = 1;
    sound_play(victory_notes[0].freq, victory_notes[0].duration, SOUND_VICTORY);
}

void update_victory_sound(uint16_t elapsed_ms) {
    sound_update(elapsed_ms);

    if (victory_playing && !sound_active) {
        current_note_index++;
        if (current_note_index < VICTORY_NOTE_COUNT) {
            Note note = victory_notes[current_note_index];
            sound_play(note.freq, note.duration, SOUND_VICTORY);
        } else {
            victory_playing = 0;  // Done
        }
    }
}

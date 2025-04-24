#ifndef BUZZ_H
#define BUZZ_H

#include <stdint.h>
#include <avr/io.h>

#define BUZZER_PIN PB1  // Connect buzzer to PB1

// Sound priority levels
typedef enum {
    SOUND_NONE,
    SOUND_VOICE,
    SOUND_VICTORY
} SoundMode;

void buzzer_init(void);

// Sound control with priority support
void sound_play(uint16_t freq, uint16_t duration_ms, SoundMode mode);
void sound_update(uint16_t elapsed_ms);
void sound_stop(void);

// Victory jingle
void play_victory_sound(void);
void update_victory_sound(uint16_t elapsed_ms);

#endif

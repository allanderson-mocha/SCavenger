#ifndef BUZZ_H
#define BUZZ_H

#include <stdint.h>
#include <avr/io.h>

#define BUZZER_PIN PB1  // Connect buzzer to PD6

/**
 * @brief Initializes the buzzer pin.
 *
 * Configures the designated buzzer pin as an output.
 * This must be called before using other buzzer functions.
 */
void buzzer_init(void);

/**
 * @brief Turns the buzzer on.
 *
 * Sets the buzzer pin high to activate the buzzer.
 */
void buzzer_on(void);

/**
 * @brief Turns the buzzer off.
 *
 * Sets the buzzer pin low to deactivate the buzzer.
 */
void buzzer_off(void);

/**
 * @brief Activates the buzzer for a specified duration.
 *
 * Turns the buzzer on, waits for the specified time (in milliseconds),
 * then turns it off.
 *
 * @param time Duration to beep in milliseconds.
 */
// void beep(uint16_t time);
void play_note(unsigned short freq);
#endif
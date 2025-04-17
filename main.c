/* Puzzle Box Main - Puzzle 1: Light Sensor Challenge */

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "light_sensor.h"
#include "lcd.h"

#define BRIGHT_THRESHOLD 5.0  // Adjustable light threshold
#define DELAY_MS 1000
#define MAX_CHARS_PER_LINE 20
#define MAX_LCD_LINES 4

typedef struct {
    const char* fullText;
    int charIndex;
    int row;
    int col;
    uint8_t finished;
} DialogueState;

void init_say(DialogueState* state, const char* text) {
    lcd_writecommand(0x01); // Clear display
    state->fullText = text;
    state->charIndex = 0;
    state->row = 0;
    state->col = 0;
    state->finished = 0;
}

void say_step(DialogueState* state) {
    if (state->finished || state->fullText[state->charIndex] == '\0') {
        state->finished = 1;
        return;
    }

    char c = state->fullText[state->charIndex++];

    // Auto-wrap after 20 characters per LCD segment
    if (state->col >= MAX_CHARS_PER_LINE) {
        state->col = 0;
        state->row++;
    }

    if (state->row >= MAX_LCD_LINES) {
        state->finished = 1;
        return;
    }

    int lcd_row = state->row % 2;
    int lcd_col_offset = (state->row / 2) * MAX_CHARS_PER_LINE;
    lcd_moveto(lcd_row, lcd_col_offset + state->col);
    lcd_writedata(c);
    state->col++;
}

int main(void) {
    lcd_init();
    light_init();

    DialogueState narrator;
    init_say(&narrator, "Waking up...");

    while (!narrator.finished) {
        say_step(&narrator);
        _delay_ms(50); // Simulate timed char-by-char step (replace with non-blocking logic if needed)
    }

    float initial_light = get_light();
    uint8_t narrator_prefers_bright = (initial_light < BRIGHT_THRESHOLD);

    if (narrator_prefers_bright) {
        init_say(&narrator, "It's too dark in here! Turn on the lights.");
    } else {
        init_say(&narrator, "It's too bright in here! Turn off the lights.");
    }

    while (!narrator.finished) {
        say_step(&narrator);
        _delay_ms(50);
    }

    // Wait until player adjusts light to match narrator's preference
    while (1) {
        float current_light = get_light();

        if ((narrator_prefers_bright && current_light > BRIGHT_THRESHOLD) ||
            (!narrator_prefers_bright && current_light < BRIGHT_THRESHOLD)) {
            break;
        }
    }

    init_say(&narrator, "Ah, that's better.");
    while (!narrator.finished) {
        say_step(&narrator);
        _delay_ms(50);
    }

    // TODO: Puzzle 2 will go here

    while (1) {}
} 

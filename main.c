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

void say(const char* line) {
    lcd_writecommand(0x01); // Clear display

    int line_num = 0;
    int col = 0;
    int start = 0;
    int len = strlen(line);

    while (start < len && line_num < MAX_LCD_LINES) {
        // Find the end of the next word
        int end = start;
        while (end < len && line[end] != ' ') {
            end++;
        }
        int word_len = end - start;

        // If word doesn't fit in current line, move to next line
        if (col + word_len > MAX_CHARS_PER_LINE) {
            line_num++;
            col = 0;
        }

        if (line_num >= MAX_LCD_LINES) break;

        // Set cursor to correct position
        int row = line_num % 2;
        int col_offset = (line_num / 2) * MAX_CHARS_PER_LINE;
        lcd_moveto(row, col_offset + col);

        // Write the word
        for (int i = start; i < end; i++) {
            lcd_writedata(line[i]);
            col++;
        }

        // Add space if there's room and it's not the end of the string
        if (line[end] == ' ' && col + 1 <= MAX_CHARS_PER_LINE) {
            lcd_writedata(' ');
            col++;
            end++;  // Skip the space
        }

        start = end;
    }
}

void wait_ms(unsigned int ms) {
    while (ms > 0) {
        _delay_ms(1);
        ms--;
    }
}

int main(void) {
    lcd_init();
    light_init();

    say("Waking up...");
    wait_ms(2000);

    float initial_light = get_light();
    uint8_t narrator_prefers_bright = (initial_light < BRIGHT_THRESHOLD);

    if (narrator_prefers_bright) {
        say("It's too dark in here! Turn on the lights.");
    } else {
        say("It's too bright in here! Turn off the lights.");
    }

    // Wait until player adjusts light to match narrator's preference
    while (1) {
        float current_light = get_light();

        if ((narrator_prefers_bright && current_light > BRIGHT_THRESHOLD) ||
            (!narrator_prefers_bright && current_light < BRIGHT_THRESHOLD)) {
            break;
        }

        wait_ms(1000);  // Check once per second
    }

    say("Ah, that's better.");
    wait_ms(2000);

    // TODO: Puzzle 2 will go here

    while (1) {}
} 
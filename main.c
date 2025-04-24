/* Puzzle Box Main - Puzzle 1: Light Sensor Challenge */
/* Puzzle Box Main - Puzzle 2: Morse Code/Temperature Sensor Challenge */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include "light_sensor.h"
#include "lcd.h"
#include "buzz.h"
#include "led.h"
#include "altimeter.h"


#define BRIGHT_THRESHOLD 5.0
#define MAX_CHARS_PER_LINE 20
#define MAX_LCD_LINES 4
#define PUZZLE_COUNT 6

#define POWER_BUTTON_PIN PC2
#define CLUE_BUTTON_PIN  PC1
#define BACK_BUTTON_PIN  PC0
#define NEXT_BUTTON_PIN  PC3

#define SUCCESS_TEXT "Well done! Continue my story by pressing NEXT."

volatile uint8_t power_button_pressed = 0;
volatile uint8_t clue_button_pressed = 0;
volatile uint8_t back_button_pressed = 0;
volatile uint8_t next_button_pressed = 0;

uint8_t display_dirty = 1;

typedef enum {
    MODE_DIALOGUE,
    MODE_PUZZLE
} SystemMode;

typedef enum {
    SCREEN_PROMPT,
    SCREEN_CLUE1,
    SCREEN_CLUE2,
    SCREEN_CLUE3,
    SCREEN_CLUE_CONFIRM,
    SCREEN_NO_CLUES,
    SCREEN_CLUE_MENU
} PuzzleScreen;

typedef struct {
    uint8_t puzzle_index;
    SystemMode mode;
    uint8_t clue_progress;
    PuzzleScreen current_screen;
    uint8_t dialogue_index;
    uint8_t power_on;
    uint8_t puzzle_complete;
    float initial_light;
    uint8_t clue_menu_open;
} GameState;

GameState game;

const char* puzzle0_dialogue[] = {
    "Waking up... you're here! The journal worked!",
    "When I was young, I loved building things.",
    "I used to take apart clocks just to see how they ticked.",
    "This journal is the last thing I built.",
    "Will you help me finish it?", NULL
};

const char* puzzle1_dialogue[] = {
    SUCCESS_TEXT,
    "I met someone once. They smelled like old paper and hope.",
    "We dreamed of seeing the world, but time got in the way.",
    "When love finds you, don't waste it.", NULL
};

const char* puzzle2_dialogue[] = {
    SUCCESS_TEXT,
    "When the doctor said 'terminal,' I just stared at the ceiling.",
    "I felt still. Like I had already left.",
    "That's why I made this. To move when I couldn't.", NULL
};

const char* puzzle3_dialogue[] = {
    SUCCESS_TEXT,
    "I missed more sunsets than I can count.",
    "Do one thing for me. Don't wait for a 'better time.'",
    "You're alive. That's all the permission you need.", NULL
};

const char* puzzle4_dialogue[] = {
    SUCCESS_TEXT,
    "I used to fear being forgotten.",
    "But maybe this is enough. A small light, passed on.",
    "If you're still listening, maybe I'm still here.", NULL
};

const char* puzzle5_dialogue[] = {
    SUCCESS_TEXT,
    "I never saw the ocean, but I dreamed of it.",
    "Let the tide carry me where I never got to go.",
    "Thank you. Now go live something beautiful.", NULL
};

const char** puzzle_dialogues[PUZZLE_COUNT] = {
    puzzle0_dialogue,
    puzzle1_dialogue,
    puzzle2_dialogue,
    puzzle3_dialogue,
    puzzle4_dialogue,
    puzzle5_dialogue
};

const char* puzzle_prompts[PUZZLE_COUNT] = {
    NULL,
    "Puzzle #2",
    "Puzzle 3 prompt",
    "Puzzle 4 prompt",
    "Puzzle 5 prompt",
    "Puzzle 6 prompt"
};

const char* puzzle_clues[PUZZLE_COUNT][3] = {
    {"You might have to get up...", "Something in your room controls brightness!", "Try flipping a light switch."},
    {"Do you see that?", "I recognize that pattern... Morse, right?", "Do as it says! FREEZE me!"},
    {"Clue 1", "Clue 2", "Clue 3"},
    {"Clue 1", "Clue 2", "Clue 3"},
    {"Clue 1", "Clue 2", "Clue 3"},
    {"Clue 1", "Clue 2", "Clue 3"}
};

const char* clue_menu_text = "Click NEXT to unlock a clue.";
const char* clue_confirm_text = "Are you sure you want a clue?";
const char* no_clues_text = "No clues remain.";

typedef struct {
    const char* fullText;
    int charIndex;
    int row;
    int col;
    uint8_t finished;
} DialogueState;

DialogueState dialogue;

void morse_update(uint16_t elapsed_ms);
void init_say(DialogueState* state, const char* text);
void say_step(DialogueState* state);
void reset_game(void);
void setup_buttons(void);
void update_display(void);
void morse_update(uint16_t elapsed_ms);

int main(void) {
    lcd_init();
    buzzer_init();
    led_init();
    light_init();
    altimeter_init();
    setup_buttons();
    reset_game();

    while (1) {
        if (power_button_pressed) {
            power_button_pressed = 0;
            game.power_on = !game.power_on;
            if (game.power_on) reset_game();
            else lcd_writecommand(0x01);
        }

        if (!game.power_on) continue;

        if (next_button_pressed) {
            next_button_pressed = 0;
            if (game.mode == MODE_DIALOGUE && dialogue.finished) {
                if (puzzle_dialogues[game.puzzle_index][game.dialogue_index + 1] != NULL) {
                    game.dialogue_index++;
                } else {
                    game.mode = MODE_PUZZLE;
                    game.current_screen = SCREEN_PROMPT;
                    game.puzzle_complete = 0;
                }
                display_dirty = 1;
            } else if (game.mode == MODE_PUZZLE && game.clue_menu_open) {
                if (game.current_screen == SCREEN_CLUE_MENU) {
                    game.current_screen = SCREEN_CLUE_CONFIRM;
                } else if (game.current_screen == SCREEN_CLUE_CONFIRM) {
                    if (game.clue_progress < 3) {
                        game.clue_progress++;
                        game.current_screen = (PuzzleScreen)(SCREEN_CLUE1 + game.clue_progress - 1);
                    }
                } else if (game.current_screen >= SCREEN_CLUE1 && game.current_screen <= SCREEN_CLUE3) {
                    uint8_t current_clue = game.current_screen - SCREEN_CLUE1;
                    if (current_clue + 1 < game.clue_progress) {
                        game.current_screen++;
                    } else if (game.clue_progress < 3) {
                        game.current_screen = SCREEN_CLUE_CONFIRM;
                    } else {
                        game.current_screen = SCREEN_NO_CLUES;
                    }
                }
                display_dirty = 1;
            }
        }

        if (clue_button_pressed) {
            clue_button_pressed = 0;
            if (game.mode == MODE_PUZZLE) {
                game.clue_menu_open = !game.clue_menu_open;
                game.current_screen = (game.clue_menu_open ? (game.clue_progress > 0 ? SCREEN_CLUE1 : SCREEN_CLUE_MENU) : SCREEN_PROMPT);
                display_dirty = 1;
            }
        }

        if (back_button_pressed) {
            back_button_pressed = 0;
            if (game.mode == MODE_PUZZLE) {
                if (game.current_screen == SCREEN_CLUE2 || game.current_screen == SCREEN_CLUE3) {
                    game.current_screen--;
                    display_dirty = 1;
                } else if (game.current_screen == SCREEN_CLUE1 || game.current_screen == SCREEN_CLUE_CONFIRM || game.current_screen == SCREEN_NO_CLUES || game.current_screen == SCREEN_CLUE_MENU) {
                    game.current_screen = SCREEN_PROMPT;
                    game.clue_menu_open = 0;
                    display_dirty = 1;
                } else if (game.current_screen == SCREEN_PROMPT) {
                    game.mode = MODE_DIALOGUE;
                    display_dirty = 1;
                }
            } else if (game.mode == MODE_DIALOGUE && game.dialogue_index > 0) {
                game.dialogue_index--;
                display_dirty = 1;
            }
        }

        // Puzzle #1: Light Sensor Tutorial
        if (game.mode == MODE_PUZZLE && game.puzzle_index == 0 && !game.puzzle_complete) {
            float current_light = get_light();
            if ((game.initial_light > BRIGHT_THRESHOLD && current_light < BRIGHT_THRESHOLD) ||
                (game.initial_light < BRIGHT_THRESHOLD && current_light > BRIGHT_THRESHOLD)) {
                game.puzzle_complete = 1;
                game.mode = MODE_DIALOGUE;
                game.puzzle_index++;
                game.dialogue_index = 0;
                game.clue_menu_open = 0;
                init_say(&dialogue, puzzle_dialogues[game.puzzle_index][game.dialogue_index]);
            }
        }
        
        // Puzzle 2: Morse Code Challenge - "Freeze Me"
        if (game.puzzle_index == 1 && game.mode == MODE_PUZZLE && !game.puzzle_complete) {
            morse_update(25); // Flash Morse code on all 3 LEDs
            float tempC = get_temperature();  // Temperature check using MPL3115A2
            if (tempC > 24.0) {  // hot enough to complete puzzle (TODO)
                game.puzzle_complete = 1;
                game.mode = MODE_DIALOGUE;
                game.puzzle_index++;
                game.dialogue_index = 0;
                game.clue_menu_open = 0;
                init_say(&dialogue, puzzle_dialogues[game.puzzle_index][0]);
            }
        }

        if (!dialogue.finished) say_step(&dialogue);

        update_display();
        _delay_ms(25);
    }
}

void init_say(DialogueState* state, const char* text) {
    lcd_writecommand(0x01);
    state->fullText = text;
    state->charIndex = 0;
    state->row = 0;
    state->col = 0;
    state->finished = 0;
    display_dirty = 0;
}

void say_step(DialogueState* state) {
    if (state->finished || state->fullText[state->charIndex] == '\0') {
        state->finished = 1;
        return;
    }

    // Peek ahead to see how long the next word is
    int lookahead = 0;
    while (state->fullText[state->charIndex + lookahead] != '\0' &&
           state->fullText[state->charIndex + lookahead] != ' ') {
        lookahead++;
    }

    // If the word won't fit in the current line, move to the next line
    if (state->col + lookahead >= MAX_CHARS_PER_LINE) {
        state->col = 0;
        state->row++;
    }

    if (state->row >= MAX_LCD_LINES) {
        state->finished = 1;
        return;
    }

    char c = state->fullText[state->charIndex++];

    // If character is a space at the beginning of a line, skip it
    if (c == ' ' && state->col == 0) return;

    int lcd_row = state->row % 2;
    int lcd_col_offset = (state->row / 2) * MAX_CHARS_PER_LINE;
    lcd_moveto(lcd_row, lcd_col_offset + state->col);
    lcd_writedata(c);
    state->col++;
}


void reset_game(void) {
    game.puzzle_index = 0;
    game.mode = MODE_DIALOGUE;
    game.clue_progress = 0;
    game.current_screen = SCREEN_PROMPT;
    game.dialogue_index = 0;
    game.puzzle_complete = 0;
    game.initial_light = get_light();
    game.clue_menu_open = 0;
    init_say(&dialogue, puzzle_dialogues[game.puzzle_index][0]);
}

void setup_buttons(void) {
    DDRC &= ~((1 << POWER_BUTTON_PIN) | (1 << CLUE_BUTTON_PIN) |
              (1 << BACK_BUTTON_PIN) | (1 << NEXT_BUTTON_PIN));
    PORTC |= (1 << POWER_BUTTON_PIN) | (1 << CLUE_BUTTON_PIN) |
             (1 << BACK_BUTTON_PIN) | (1 << NEXT_BUTTON_PIN);
    PCICR |= (1 << PCIE1);
    PCMSK1 |= (1 << PCINT10) | (1 << PCINT9) | (1 << PCINT8) | (1 << PCINT11);
    sei();
}

void update_display(void) {
    if (!display_dirty) return;

    if (game.mode == MODE_PUZZLE) {
        if (game.puzzle_index == 0 && game.current_screen == SCREEN_PROMPT) {
            if (game.initial_light > BRIGHT_THRESHOLD) {
                init_say(&dialogue, "Puzzle #1: It's bright in here!");
            } else {
                init_say(&dialogue, "Puzzle #1: It's dark in here!");
            }
        } else if (game.current_screen == SCREEN_CLUE_MENU) {
            init_say(&dialogue, clue_menu_text);
        } else if (game.current_screen == SCREEN_CLUE_CONFIRM) {
            init_say(&dialogue, clue_confirm_text);
        } else if (game.current_screen == SCREEN_NO_CLUES) {
            init_say(&dialogue, no_clues_text);
        } else {
            switch (game.current_screen) {
                case SCREEN_PROMPT:
                    init_say(&dialogue, puzzle_prompts[game.puzzle_index]); break;
                case SCREEN_CLUE1:
                    init_say(&dialogue, puzzle_clues[game.puzzle_index][0]); break;
                case SCREEN_CLUE2:
                    init_say(&dialogue, puzzle_clues[game.puzzle_index][1]); break;
                case SCREEN_CLUE3:
                    init_say(&dialogue, puzzle_clues[game.puzzle_index][2]); break;
                default: break;
            }
        }
    } else if (game.mode == MODE_DIALOGUE) {
        init_say(&dialogue, puzzle_dialogues[game.puzzle_index][game.dialogue_index]);
    }
    display_dirty = 0;
}

ISR(PCINT1_vect) {
    if (!(PINC & (1 << POWER_BUTTON_PIN))) power_button_pressed = 1;
    if (!(PINC & (1 << CLUE_BUTTON_PIN)))  clue_button_pressed = 1;
    if (!(PINC & (1 << BACK_BUTTON_PIN)))  back_button_pressed = 1;
    if (!(PINC & (1 << NEXT_BUTTON_PIN)))  next_button_pressed = 1;
    if (power_button_pressed || clue_button_pressed || back_button_pressed || next_button_pressed) {
        _delay_ms(5);
        while (!(PINC & (1 << POWER_BUTTON_PIN)) || !(PINC & (1 << CLUE_BUTTON_PIN)) || !(PINC & (1 << BACK_BUTTON_PIN)) || !(PINC & (1 << NEXT_BUTTON_PIN))) {}
        _delay_ms(5);
    }
}

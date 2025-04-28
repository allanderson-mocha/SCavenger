#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "light_sensor.h"
#include "lcd.h"
#include "buzz.h"
#include "led.h"
#include "i2c.h"
#include "altimeter.h"
#include "accelerometer.h"
#include "solenoid.h"
#include "gps.h"
#include "main.h"

#define STEP_GOAL 69
#define ALTITUDE_THRESHOLD 10
#define TEMPERATURE_THRESHOLD 24.0
#define BUFFER_SIZE 64

char progmem_buffer[BUFFER_SIZE];

volatile uint8_t power_button_pressed = 0;
volatile uint8_t clue_button_pressed = 0;
volatile uint8_t back_button_pressed = 0;
volatile uint8_t next_button_pressed = 0;

uint8_t EEMEM stored_puzzle_index;

uint8_t display_dirty = 1;
uint8_t just_transitioned = 0;
uint16_t step_count = 0;

GameState game;

const char puzzle0_line0[] PROGMEM = "Hey... you're here! That means the journal worked.";
const char puzzle0_line1[] PROGMEM = "When I was young, I loved building things.";
const char puzzle0_line2[] PROGMEM = "I'd take apart clocks just to see how they ticked.";
const char puzzle0_line3[] PROGMEM = "This journal is the last thing I ever made.";
const char puzzle0_line4[] PROGMEM = "I hope you'll help me finish it.";
const char puzzle0_line5[] PROGMEM = "There's a story buried inside.";
const char puzzle0_line6[] PROGMEM = "It starts small... but don't they all?";

const char* const puzzle0_dialogue[] PROGMEM = {
    puzzle0_line0,
    puzzle0_line1,
    puzzle0_line2,
    puzzle0_line3,
    puzzle0_line4,
    puzzle0_line5,
    puzzle0_line6,
    NULL
};

const char puzzle1_line0[] PROGMEM = "Well done! Continue my story by pressing NEXT.";
const char puzzle1_line1[] PROGMEM = "Have you ever watched someone you love walk away...";
const char puzzle1_line2[] PROGMEM = "...knowing it's the right thing, but wishing it weren't?";
const char puzzle1_line3[] PROGMEM = "I met them in a bookstore.";
const char puzzle1_line4[] PROGMEM = "They smelled like old paper and hope.";
const char puzzle1_line5[] PROGMEM = "We dreamed of seeing the world together.";
const char puzzle1_line6[] PROGMEM = "But time got in the way.";
const char puzzle1_line7[] PROGMEM = "Time always does.";
const char puzzle1_line8[] PROGMEM = "Love is a little miracle. Don't waste it.";

const char* const puzzle1_dialogue[] PROGMEM = {
    puzzle1_line0,
    puzzle1_line1,
    puzzle1_line2,
    puzzle1_line3,
    puzzle1_line4,
    puzzle1_line5,
    puzzle1_line6,
    puzzle1_line7,
    puzzle1_line8,
    NULL
};

const char puzzle2_line0[] PROGMEM = "Well done! Continue my story by pressing NEXT.";
const char puzzle2_line1[] PROGMEM = "The day the doctor said 'terminal,' I didn't cry.";
const char puzzle2_line2[] PROGMEM = "I just stared at the ceiling tiles.";
const char puzzle2_line3[] PROGMEM = "People think you feel sadness, or rage.";
const char puzzle2_line4[] PROGMEM = "But all I felt was stillness.";
const char puzzle2_line5[] PROGMEM = "Like I had already left.";
const char puzzle2_line6[] PROGMEM = "Everything I wanted to do suddenly seemed... urgent.";
const char puzzle2_line7[] PROGMEM = "But my body couldn't keep up.";
const char puzzle2_line8[] PROGMEM = "That's why I made this journal. To move when I no longer could.";

const char* const puzzle2_dialogue[] PROGMEM = {
    puzzle2_line0,
    puzzle2_line1,
    puzzle2_line2,
    puzzle2_line3,
    puzzle2_line4,
    puzzle2_line5,
    puzzle2_line6,
    puzzle2_line7,
    puzzle2_line8,
    NULL
};

const char puzzle3_line0[] PROGMEM = "Well done! Continue my story by pressing NEXT.";
const char puzzle3_line1[] PROGMEM = "There's a tree outside my old bedroom window.";
const char puzzle3_line2[] PROGMEM = "I always said I'd climb it one day. I never did.";
const char puzzle3_line3[] PROGMEM = "I missed more sunsets than I can count.";
const char puzzle3_line4[] PROGMEM = "Chose work. Chose comfort.";
const char puzzle3_line5[] PROGMEM = "Chose fear.";
const char puzzle3_line6[] PROGMEM = "Do one thing for me. Don't wait for a 'better time.'";
const char puzzle3_line7[] PROGMEM = "It never comes.";
const char puzzle3_line8[] PROGMEM = "You're alive. That's all the permission you need.";


const char* const puzzle3_dialogue[] PROGMEM = {
    puzzle3_line0,
    puzzle3_line1,
    puzzle3_line2,
    puzzle3_line3,
    puzzle3_line4,
    puzzle3_line5,
    puzzle3_line6,
    puzzle3_line7,
    puzzle3_line8,
    NULL
};

const char puzzle4_line0[] PROGMEM = "Well done! Continue my story by pressing NEXT.";
const char puzzle4_line1[] PROGMEM = "I used to wonder if anyone would remember me.";
const char puzzle4_line2[] PROGMEM = "I was so scared of disappearing.";
const char puzzle4_line3[] PROGMEM = "But maybe we don't need monuments.";
const char puzzle4_line4[] PROGMEM = "Maybe we just need to be a small light for someone else.";
const char puzzle4_line5[] PROGMEM = "If this journal makes you pause, even once...";
const char puzzle4_line6[] PROGMEM = "...maybe that's enough.";
const char puzzle4_line7[] PROGMEM = "Maybe I'm not gone, if you're still listening.";

const char* const puzzle4_dialogue[] PROGMEM = {
    puzzle4_line0,
    puzzle4_line1,
    puzzle4_line2,
    puzzle4_line3,
    puzzle4_line4,
    puzzle4_line5,
    puzzle4_line6,
    puzzle4_line7,
    NULL
};

const char puzzle5_line0[] PROGMEM = "Well done! Continue my story by pressing NEXT.";
const char puzzle5_line1[] PROGMEM = "I never saw the ocean, but I dreamed of it.";
const char puzzle5_line2[] PROGMEM = "Take my picture. Walk to the shore.";
const char puzzle5_line3[] PROGMEM = "Let the tide carry me somewhere I never got to go.";
const char puzzle5_line4[] PROGMEM = "I don't want to haunt memories.";
const char puzzle5_line5[] PROGMEM = "I want to become one. Soft, and blue, and free.";
const char puzzle5_line6[] PROGMEM = "Thank you for giving me peace.";
const char puzzle5_line7[] PROGMEM = "Now go live something...";
const char puzzle5_line8[] PROGMEM = "...beautiful.";

const char* const puzzle5_dialogue[] PROGMEM = {
    puzzle5_line0,
    puzzle5_line1,
    puzzle5_line2,
    puzzle5_line3,
    puzzle5_line4,
    puzzle5_line5,
    puzzle5_line6,
    puzzle5_line7,
    puzzle5_line8,
    NULL
};

const char* const* const puzzle_dialogues[PUZZLE_COUNT] PROGMEM = {
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
    "Puzzle #3:",
    "Puzzle #4: .",
    "Puzzle #5: Go outside!",
    "..."
};

const char* puzzle_clues[PUZZLE_COUNT][3] = {
    {"You might have to get up...", "Something in your room controls brightness!", "Try flipping a light switch."},
    {"Do you see that?", "I recognize that pattern... Morse, right?", "Do as it says! FREEZE me!"},
    {"The sky is the limit!", "See any hills?", "Perhaps a lil' higher?"},
    {"Blackout!", "Don't stop!", "Shake it!"},
    {"You might need a map.", "Looks like a destination to me...", "Visit the coordinates!"},
    {"...", "...", "..."}
};

const char* clue_menu_text = "Click NEXT to unlock a clue.";
const char* clue_confirm_text = "Are you sure you want a clue?";
const char* no_clues_text = "No clues remain.";

DialogueState dialogue;

void morse_update(uint16_t elapsed_ms);
void init_say(DialogueState* state, const char* text);
void init_current_dialogue(DialogueState* state);
void say_step(DialogueState* state);
void reset_game(void);
void setup_buttons(void);
void update_display(void);
void morse_update(uint16_t elapsed_ms);

int main(void) {
    i2c_init(72);
    lcd_init();
    buzzer_init();
    led_init();
    light_init();
    altimeter_init();
    accel_init(); 
    setup_buttons();
    solenoid_init();
    reset_game();
    gps_init();
    
    int16_t accel_coords[3]; // For accel data
    
    while (1) {
        // Debounce and handle Power button
        if (power_button_pressed) {
            _delay_ms(5);  // Wait a little
            if (!(PINC & (1 << POWER_BUTTON_PIN))) {  // Still pressed?
                power_button_pressed = 0;

                game.power_on = !game.power_on;
                if (game.power_on) reset_game();
                else lcd_writecommand(0x01);
            } else {
                // If bounce released, ignore
                power_button_pressed = 0;
            }
        }

        if (!game.power_on) continue;

        // Debounce and handle Clue button
        if (clue_button_pressed) {
            _delay_ms(5);
            if (!(PINC & (1 << CLUE_BUTTON_PIN))) {
                clue_button_pressed = 0;

                if (game.mode == MODE_PUZZLE) {
                    game.clue_menu_open = !game.clue_menu_open;
                    game.current_screen = (game.clue_menu_open ? (game.clue_progress > 0 ? SCREEN_CLUE1 : SCREEN_CLUE_MENU) : SCREEN_PROMPT);
                    display_dirty = 1;
                }
            } else {
                clue_button_pressed = 0;
            }
        }

        // Debounce and handle Back button
        if (back_button_pressed) {
            _delay_ms(5);
            if (!(PINC & (1 << BACK_BUTTON_PIN))) {
                back_button_pressed = 0;

                if (game.mode == MODE_PUZZLE) {
                    if (game.current_screen == SCREEN_CLUE2 || game.current_screen == SCREEN_CLUE3) {
                        game.current_screen--;
                        display_dirty = 1;
                    } else if (game.current_screen == SCREEN_CLUE1 || game.current_screen == SCREEN_CLUE_CONFIRM ||
                            game.current_screen == SCREEN_NO_CLUES || game.current_screen == SCREEN_CLUE_MENU) {
                        game.current_screen = SCREEN_PROMPT;
                        game.clue_menu_open = 0;
                        display_dirty = 1;
                    } else if (game.current_screen == SCREEN_PROMPT) {
                        game.mode = MODE_DIALOGUE;
                        step_count = 0;
                        display_dirty = 1;
                    }
                } else if (game.mode == MODE_DIALOGUE && game.dialogue_index > 0) {
                    game.dialogue_index--;
                    display_dirty = 1;
                }
            } else {
                back_button_pressed = 0;
            }
        }

        // Debounce and handle Next button
        if (next_button_pressed) {
            _delay_ms(5);
            if (!(PINC & (1 << NEXT_BUTTON_PIN))) {
                next_button_pressed = 0;

                if (game.mode == MODE_DIALOGUE && dialogue.finished) {
                    const char* const* dialogue_array = (const char* const*)pgm_read_word(&(puzzle_dialogues[game.puzzle_index]));
                    const char* next_line_ptr = (const char*)pgm_read_word(&(dialogue_array[game.dialogue_index + 1]));
                    if (next_line_ptr != NULL) {
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
            } else {
                next_button_pressed = 0;
            }
        }

        // Load Puzzle Index
        game.puzzle_index = eeprom_read_byte(&stored_puzzle_index);


        // Puzzle #1: Light Sensor Tutorial
        if (game.mode == MODE_PUZZLE && game.puzzle_index == 0 && !game.puzzle_complete) {
            float current_light = get_light();
            if ((game.initial_light > BRIGHT_THRESHOLD && current_light < BRIGHT_THRESHOLD) ||
                (game.initial_light < BRIGHT_THRESHOLD && current_light > BRIGHT_THRESHOLD)) {
                play_victory_sound();
                game.puzzle_complete = 1;
                game.mode = MODE_DIALOGUE;
                game.puzzle_index++;
                game.dialogue_index = 0;
                game.clue_menu_open = 0;
                // init_say_from_progmem(&dialogue, puzzle_dialogues[game.puzzle_index][game.dialogue_index]);
                init_current_dialogue(&dialogue);
                eeprom_update_byte(&stored_puzzle_index, game.puzzle_index);

            }
        }
        
        // Puzzle 2: Morse Code Challenge - "Freeze Me"
        if (game.puzzle_index == 1 && game.mode == MODE_PUZZLE && !game.puzzle_complete) {
            morse_update(25); // Flash Morse code on all 3 LEDs
            float tempC = get_temperature();  // Temperature check using MPL3115A2
            if (tempC > TEMPERATURE_THRESHOLD) {  // hot enough to complete puzzle (TODO)
                play_victory_sound();
                game.puzzle_complete = 1;
                game.mode = MODE_DIALOGUE;
                game.puzzle_index++;
                game.dialogue_index = 0;
                game.clue_menu_open = 0;
                // init_say_from_progmem(&dialogue, puzzle_dialogues[game.puzzle_index][0]);
                init_current_dialogue(&dialogue);
                morse_led_off();
                eeprom_update_byte(&stored_puzzle_index, game.puzzle_index);
            }
        }

        // Puzzle #3: Altitude
        if (game.puzzle_index == 2 && game.mode == MODE_PUZZLE && !game.puzzle_complete && game.current_screen == SCREEN_PROMPT) {
            static uint8_t initialized = 0;
            if (!initialized) {
                game.base_altitude = get_altitude();
                initialized = 1;
            }
            
            int32_t current_alt = get_altitude();
            int32_t gain = labs((int32_t)(current_alt - game.base_altitude));

            char buffer[17];
            snprintf(buffer, sizeof(buffer), "%3d", (int)gain);
            lcd_moveto(0, 11);
            lcd_stringout(buffer);
        
            if (gain >= ALTITUDE_THRESHOLD) {
                play_victory_sound();
                game.puzzle_complete = 1;
                game.mode = MODE_DIALOGUE;
                game.puzzle_index++;  // move to Puzzle 4
                game.dialogue_index = 0;
                game.clue_menu_open = 0;
                init_current_dialogue(&dialogue);
                eeprom_update_byte(&stored_puzzle_index, game.puzzle_index);
            }
        }
        
        // Puzzle #4: Step Counter
        static uint16_t prev_step_count = 0;
        static uint8_t elapsed_ms = 0;

        if (game.puzzle_index == 3 && game.mode == MODE_PUZZLE && !game.puzzle_complete && game.current_screen == SCREEN_PROMPT) {
            get_accel(accel_coords);

            if (detect_step(accel_coords[2])) {
                step_count++;
            }

            elapsed_ms++;  // We delay 25 ms per loop
            if (elapsed_ms >= 40) { // 25 ms * 40 = 1000ms = 1 second
                elapsed_ms = 0;
                if (step_count > 0) {
                    step_count--;
                }
            }

            if (step_count != prev_step_count) {
                uint8_t row = (step_count+10) / 40;
                uint8_t col = (step_count+10) - (40 * row);
                if (step_count > prev_step_count) {
                    lcd_moveto(row, col);
                    lcd_writedata('.');
                } else {
                    row = (prev_step_count+10) / 40;
                    col = (prev_step_count+10) - (40 * row);
                    lcd_moveto(row, col);
                    lcd_writedata(' ');
                }
                prev_step_count = step_count;  // Update after
            }
            

            if (step_count >= STEP_GOAL) {
                play_victory_sound();
                game.puzzle_complete = 1;
                game.mode = MODE_DIALOGUE;
                game.puzzle_index++;
                game.dialogue_index = 0;
                game.clue_menu_open = 0;
                init_current_dialogue(&dialogue);
                eeprom_update_byte(&stored_puzzle_index, game.puzzle_index);

            }
        } else if (game.puzzle_index == 3 && game.mode == MODE_PUZZLE && !game.puzzle_complete && game.current_screen != SCREEN_PROMPT) {
            just_transitioned = 1;
        }

        // Puzzle #5: GPS Puzzle
        static float latitude = 0.0;
        static float longitude = 0.0;
        static uint8_t target_initialized = 0;

        if (game.puzzle_index == 4 && game.mode == MODE_PUZZLE && !game.puzzle_complete && game.current_screen == SCREEN_PROMPT) {
            if (gps_sentence_ready()) {
                gps_ready = 0; // reset manually if needed
                parse_gps_coordinates(gps_buffer, &latitude, &longitude);
                gps_reset();   // clear buffer for next sentence

                if (latitude > 29.97f && latitude < 49.73f && longitude > -127.44f && longitude < -63.76) {
                    if (!target_initialized) {
                        game.target_latitude = latitude;
                        game.target_longitude = longitude + 0.0010f; // realistic
                        // game.target_longitude = longitude + 0.0180f; // 1 mile
                        // game.target_longitude = longitude + 1.0f; // obvious
                        target_initialized = 1;
                        display_dirty = 1;
                    }

                    float lat_diff = latitude - game.target_latitude;
                    if (lat_diff < 0) lat_diff = -lat_diff;

                    float lon_diff = longitude - game.target_longitude;
                    if (lon_diff < 0) lon_diff = -lon_diff;

                    const float threshold = 0.0005f;

                    if (lat_diff < threshold && lon_diff < threshold) {
                        play_victory_sound();
                        game.puzzle_complete = 1;
                        game.mode = MODE_DIALOGUE;
                        game.puzzle_index++;
                        game.dialogue_index = 0;
                        game.clue_menu_open = 0;
                        init_current_dialogue(&dialogue);
                        eeprom_update_byte(&stored_puzzle_index, game.puzzle_index);

                    }
                }
            }
        }

        // Puzzle #6: Secret Compartment
        static uint8_t solenoid_fired = 0;
        static uint16_t solenoid_timer = 0;

        if (game.puzzle_index == 5 && game.mode == MODE_PUZZLE && !game.puzzle_complete && game.current_screen == SCREEN_PROMPT) {
            if (!solenoid_fired) {
                solenoid_on();       // Turn solenoid ON
                solenoid_fired = 1;  // Mark that we fired it
            }

            solenoid_timer++;  // We delay 25ms per loop
            if (solenoid_timer >= 80) { // 25ms * 80 = 2000ms = 2 seconds
                solenoid_off();
                lcd_writecommand(0x01);
                game.power_on = 0; 
                solenoid_fired = 0;
                _delay_ms(100);  
            }
        }

        if (!dialogue.finished) say_step(&dialogue);

        update_display();
        update_victory_sound(25);
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

void init_say_from_progmem(DialogueState* state, const char* progmem_ptr) {
    lcd_writecommand(0x01);
    copy_progmem_string(progmem_buffer, progmem_ptr);  // Copy to RAM
    state->fullText = progmem_buffer;
    state->charIndex = 0;
    state->row = 0;
    state->col = 0;
    state->finished = 0;
    display_dirty = 0;
}
void init_current_dialogue(DialogueState* state) {
    const char* const* dialogue_array = (const char* const*)pgm_read_word(&(puzzle_dialogues[game.puzzle_index]));
    const char* line_ptr = (const char*)pgm_read_word(&(dialogue_array[game.dialogue_index]));
    init_say_from_progmem(state, line_ptr);
}

void say_voice_click(void) {
    static uint8_t toggle = 0;
    toggle = !toggle;
    if (toggle) sound_play(1000, 30, SOUND_VOICE); // short subtle tick
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

    // Optional voice tick for letters only
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
        say_voice_click();
    }
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

    // Read correct line from PROGMEM
    const char* const* dialogue_array = (const char* const*)pgm_read_word(&(puzzle_dialogues[game.puzzle_index]));
    const char* line_ptr = (const char*)pgm_read_word(&(dialogue_array[0]));
    init_say_from_progmem(&dialogue, line_ptr);  // <-- use _from_progmem
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
        } else if (game.puzzle_index == 4 && game.current_screen == SCREEN_PROMPT) {
            char buffer[BUFFER_SIZE];
            int32_t lat_int = (int32_t)(game.target_latitude * 10000.0f);
            int32_t lat_whole = lat_int / 10000;
            int32_t lat_frac = lat_int % 10000;
            if (lat_frac < 0) lat_frac = -lat_frac;
    
            int32_t lon_int = (int32_t)(game.target_longitude * 10000.0f);
            int32_t lon_whole = lon_int / 10000;
            int32_t lon_frac = lon_int % 10000;
            if (lon_frac < 0) lon_frac = -lon_frac;

            if (abs(game.target_latitude) < 0.01f || abs(game.target_longitude) < 0.01f) {
                snprintf(buffer, sizeof(buffer), "Puzzle #5: Go outside!");
            } else {
                snprintf(buffer, sizeof(buffer), "Puzzle #5: %ld.%04ld,%ld.%04ld", lat_whole, lat_frac, lon_whole, lon_frac);
            }
    
            init_say(&dialogue, buffer);
        } else if (game.current_screen == SCREEN_CLUE_MENU) {
            init_say(&dialogue, clue_menu_text);
        } else if (game.current_screen == SCREEN_CLUE_CONFIRM) {
            init_say(&dialogue, clue_confirm_text);
        } else if (game.current_screen == SCREEN_NO_CLUES) {
            init_say(&dialogue, no_clues_text);
        } else {
            switch (game.current_screen) {
                case SCREEN_PROMPT:
                    init_say(&dialogue, puzzle_prompts[game.puzzle_index]);
                    break;
                case SCREEN_CLUE1:
                    init_say(&dialogue, puzzle_clues[game.puzzle_index][0]);
                    break;
                case SCREEN_CLUE2:
                    init_say(&dialogue, puzzle_clues[game.puzzle_index][1]);
                    break;
                case SCREEN_CLUE3:
                    init_say(&dialogue, puzzle_clues[game.puzzle_index][2]);
                    break;
                default:
                    break;
            }
        }
    } else if (game.mode == MODE_DIALOGUE) {
        const char* const* dialogue_array = (const char* const*)pgm_read_word(&(puzzle_dialogues[game.puzzle_index]));
        const char* line_ptr = (const char*)pgm_read_word(&(dialogue_array[game.dialogue_index]));
        init_say_from_progmem(&dialogue, line_ptr);  // <-- use _from_progmem
    }

    if (game.puzzle_index == 3 && game.current_screen == SCREEN_PROMPT && just_transitioned) {
        just_transitioned = 0;

        char buffer[81];  // 80 max (20x4 LCD), plus null terminator
        uint8_t count = step_count;
        if (count > 80) count = 80;  // Safety cap
    
        strcpy(buffer, "Puzzle 4: ");
        uint8_t len = strlen(buffer);
        for (uint8_t i = 0; i < count; i++) {
            buffer[len + i] = '.';
        }
        buffer[len + count] = '\0';
    
        lcd_writecommand(0x01); // Clear LCD
        lcd_moveto(0, 0);
        lcd_stringout(buffer);
    }

    display_dirty = 0;
}


void copy_progmem_string(char* dest, const char* progmem_ptr) {
    uint8_t i = 0;
    char c;
    while ((c = pgm_read_byte(&(progmem_ptr[i]))) != '\0' && i < BUFFER_SIZE - 1) {
        dest[i++] = c;
    }
    dest[i] = '\0';
}

ISR(PCINT1_vect) {
    if (!(PINC & (1 << POWER_BUTTON_PIN))) power_button_pressed = 1;
    if (!(PINC & (1 << CLUE_BUTTON_PIN)))  clue_button_pressed = 1;
    if (!(PINC & (1 << BACK_BUTTON_PIN)))  back_button_pressed = 1;
    if (!(PINC & (1 << NEXT_BUTTON_PIN)))  next_button_pressed = 1;
}

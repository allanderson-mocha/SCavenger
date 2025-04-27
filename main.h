#ifndef MAIN_H
#define MAIN_H

#include <avr/io.h>
#include <stdint.h>
#include <avr/pgmspace.h>

#define BRIGHT_THRESHOLD 5.0
#define MAX_CHARS_PER_LINE 20
#define MAX_LCD_LINES 4
#define PUZZLE_COUNT 6

#define POWER_BUTTON_PIN PC2
#define CLUE_BUTTON_PIN  PC1
#define BACK_BUTTON_PIN  PC0
#define NEXT_BUTTON_PIN  PC3

#define SUCCESS_TEXT "Well done! Continue my story by pressing NEXT."

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
    uint16_t base_altitude;
} GameState;

extern GameState game;

extern const char* const* const puzzle_dialogues[PUZZLE_COUNT] PROGMEM;
extern const char* puzzle_prompts[PUZZLE_COUNT];
extern const char* puzzle_clues[PUZZLE_COUNT][3];
extern const char* clue_menu_text;
extern const char* clue_confirm_text;
extern const char* no_clues_text;

typedef struct {
    const char* fullText;
    int charIndex;
    int row;
    int col;
    uint8_t finished;
} DialogueState;

extern DialogueState dialogue;

void morse_update(uint16_t elapsed_ms);
void init_say(DialogueState* state, const char* text);
void init_say_from_progmem(DialogueState* state, const char* progmem_ptr);
void say_step(DialogueState* state);
void say_voice_click(void);
void reset_game(void);
void setup_buttons(void);
void update_display(void);
void copy_progmem_string(char* dest, const char* progmem_ptr);

#endif // MAIN_H
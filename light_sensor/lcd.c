#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"

#define LCD_RS      (1 << PB4)
#define LCD_RW      (1 << PB3)
#define LCD_E       (1 << PB2)
#define LCD_CTRL    (LCD_RS | LCD_RW | LCD_E)

#define LCD_DATA    0xf0    // PD4–PD7

void lcd_writenibble(unsigned char data);
void lcd_writebyte(unsigned char data);
void lcd_wait(void);

void lcd_init(void) {
    DDRD |= LCD_DATA;        // Data lines output
    DDRB |= LCD_CTRL;        // Control lines output

    PORTB &= ~LCD_RS;

    _delay_ms(15);
    lcd_writenibble(0x30); _delay_ms(5);
    lcd_writenibble(0x30); _delay_us(120);
    lcd_writenibble(0x30); _delay_ms(2);
    lcd_writenibble(0x20); _delay_ms(2);  // Set 4-bit mode

    lcd_writecommand(0x28);  // 4-bit, 2 lines
    lcd_writecommand(0x0f);  // Display on, cursor on
    lcd_writecommand(0x01);  // Clear display
    _delay_ms(2);
}

void lcd_moveto(unsigned char row, unsigned char col) {
    unsigned char addr = row == 0 ? 0x00 : 0x40;
    lcd_writecommand(0x80 | (addr + col));
}

void lcd_stringout(char *str) {
    while (*str) {
        lcd_writedata(*str++);
    }
}

void lcd_writecommand(unsigned char x) {
    PORTB &= ~LCD_RS;
    lcd_writebyte(x);
    _delay_ms(2);
}

void lcd_writedata(unsigned char x) {
    PORTB |= LCD_RS;
    lcd_writebyte(x);
    _delay_ms(2);
}

void lcd_writebyte(unsigned char data) {
    lcd_writenibble(data);
    lcd_writenibble(data << 4);
}

void lcd_writenibble(unsigned char data) {
    PORTD &= ~LCD_DATA;
    PORTD |= (data & LCD_DATA);

    PORTB &= ~(LCD_RW | LCD_E);
    PORTB |= LCD_E;
    _delay_us(1);         // E pulse width
    PORTB &= ~LCD_E;
}

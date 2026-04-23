#include <string.h>
#include <hardware/i2c.h>
#include <pico/stdlib.h>
#include <pico/time.h>

#include "lcdlib.h"

/* Bus address of LCD */
#define LCD_ADDR 0x27

/* Used to create byte messages for 4-bit mode */
#define LOW_BITMASK 0xF0

/* Codes used during manual LCD initiation (which we have to do since we want to
 * use 4-bit mode) */
#define LCD_SET_4BIT  0x20
#define LCD_INIT_CODE 0x30

/* DDRAM Address Boundaries */
#define DDRAM_ROW1_BEGIN 0x0
#define DDRAM_ROW1_END   0x27
#define DDRAM_ROW2_BEGIN 0x40
#define DDRAM_ROW2_END   0x67

// INSTRUCTION DEFINITIONS/////////////////////////////////////////////////////
//                                                                            /
// Definitions of various instructions that can written to the instruction    /
// register.                                                                  /
///////////////////////////////////////////////////////////////////////////////

#define LCD_CLEAR               0x01
#define LCD_ENTRY_SET           0x06  // increment address counter, no shift
#define LCD_DISPLAY_SET         0x0C  // display on, cursor off, no blink
#define LCD_SHIFT_CURSOR_LEFT   0x10
#define LCD_SHIFT_CURSOR_RIGHT  0x14
#define LCD_SHIFT_DISPLAY_LEFT  0x18
#define LCD_SHIFT_DISPLAY_RIGHT 0x1C
#define LCD_FUNCTION_SET        0x28  // 4-bit mode, 2 lines, 5x8 characters
#define LCD_SET_DDRAM_ADDR      0x80

// CONTROL SIGNALS/////////////////////////////////////////////////////////////
//                                                                            /
//  Working in 4-bit mode, each command or data byte is split up into two     /
//  separate bytes. The high nibble of the command/data byte is sent as the   /
//  high nibble in the first byte, and the low nibble of the command/data is  /
//  sent as the high nibble in the second byte. In each byte, the low nibble  /
//  holds the control signals as follows:                                     /
///////////////////////////////////////////////////////////////////////////////

/* Register Select (RS) - Determines whether to access instruction (command) or
 * data register */
#define RS_INSTR 0b00000000
#define RS_DATA  0b00000001
/* R/W - Whether we want to read from or write to register */
#define RW_WRITE 0b00000000
#define RW_READ  0b00000010
/* Enable - bit must be toggled (pulsed) before any data read or write */
#define PULSE 0b00000100
/* Backlight - must be set on every send or else backlight will turn off */
#define LCD_BACKLIGHT 0b00001000
/* Enable toggle needs to be set high for for minimum 450us and the entire pulse
 * cycle must last at least 1000us */
#define ENABLE_CYCLE_TIME 1000  // data sheet page 49

// PRIVATE FUNCTIONS///////////////////////////////////////////////////////////
//                                                                            /
///////////////////////////////////////////////////////////////////////////////

void write_byte(struct lcd_inst* lcd, uint8_t b) {
    i2c_write_blocking(lcd->i2c_inst, LCD_ADDR, &b, 1, false);
}

void pulse_enable(struct lcd_inst* lcd, uint8_t val) {
    write_byte(lcd, val | PULSE);
    sleep_us(ENABLE_CYCLE_TIME / 2);
    write_byte(lcd, val & ~PULSE);
    sleep_us(ENABLE_CYCLE_TIME / 2);
}

void send_nibble(struct lcd_inst* lcd, uint8_t val) {
    write_byte(lcd, val);
    pulse_enable(lcd, val);
}

void send_byte_as_nibbles(struct lcd_inst* lcd, uint8_t b, uint8_t mode) {
    uint8_t high = (b & LOW_BITMASK) | mode | LCD_BACKLIGHT;
    uint8_t low  = ((b << 4) & LOW_BITMASK) | mode | LCD_BACKLIGHT;
    send_nibble(lcd, high);
    send_nibble(lcd, low);
}

void do_hd44780_init_seq(struct lcd_inst* lcd) {
    send_nibble(lcd, LCD_INIT_CODE | LCD_BACKLIGHT);
    sleep_us(5);
    send_nibble(lcd, LCD_INIT_CODE | LCD_BACKLIGHT);
    sleep_us(150);
    send_nibble(lcd, LCD_INIT_CODE | LCD_BACKLIGHT);
    sleep_us(150);
    send_nibble(lcd, LCD_SET_4BIT | LCD_BACKLIGHT);
}

void __i2c_init(struct lcd_inst* lcd) {
    lcd->i2c_inst = i2c_get_instance(lcd->hw_num);
    i2c_init(lcd->i2c_inst, lcd->baudrate);

    gpio_set_function(lcd->i2c_sda, GPIO_FUNC_I2C);
    gpio_set_function(lcd->i2c_scl, GPIO_FUNC_I2C);

    gpio_pull_up(lcd->i2c_sda);
    gpio_pull_up(lcd->i2c_scl);
}

void send_command(struct lcd_inst* lcd, uint8_t cmd) {
    send_byte_as_nibbles(lcd, cmd, RS_INSTR);
}

void write_char(struct lcd_inst* lcd, char c) {
    send_byte_as_nibbles(lcd, (uint8_t)c, RS_DATA);
}

uint8_t read_busy_flag(struct lcd_inst* lcd) {
    // mask data bits (high nibble) to tell expander we want to change to read
    // mode, and tell LCD's instruction register we want to read.
    uint8_t control = LOW_BITMASK | RS_INSTR | RW_READ | LCD_BACKLIGHT;
    write_byte(lcd, control);

    // pulse high
    write_byte(lcd, control | PULSE);

    uint8_t b;
    uint8_t result;
    // read high nibble into byte
    i2c_read_blocking(lcd->i2c_inst, LCD_ADDR, &b, 1, false);
    // pulse low
    write_byte(lcd, control & ~PULSE);

    // high nibble into result
    result = b & LOW_BITMASK;

    // pulse high
    write_byte(lcd, control | PULSE);
    // read low nibble into byte
    i2c_read_blocking(lcd->i2c_inst, LCD_ADDR, &b, 1, false);
    // pulse low
    write_byte(lcd, control & ~PULSE);

    // low nibble into result
    result |= (b >> 4);

    // tell expander to go back to write mode
    write_byte(lcd, LCD_BACKLIGHT);

    return result;
}

// EXPORTED FUNCTIONS//////////////////////////////////////////////////////////
//                                                                            /
///////////////////////////////////////////////////////////////////////////////

void lcd_clear_screen(struct lcd_inst* lcd) { send_command(lcd, LCD_CLEAR); }

void lcd_init(struct lcd_inst* lcd) {
    __i2c_init(lcd);

    do_hd44780_init_seq(lcd);

    send_command(lcd, LCD_FUNCTION_SET);
    send_command(lcd, LCD_DISPLAY_SET);
    send_command(lcd, LCD_ENTRY_SET);
    send_command(lcd, LCD_CLEAR);
}

uint8_t lcd_get_ac(struct lcd_inst* lcd) {
    uint8_t result = read_busy_flag(lcd);

    return result & 0b00111111;
}

void lcd_set_cursor(struct lcd_inst* lcd, uint8_t addr) {
    if ((addr > DDRAM_ROW1_END && addr < DDRAM_ROW2_BEGIN) ||
        addr > DDRAM_ROW2_END) {
        return;
    }

    uint8_t cmd = LCD_SET_DDRAM_ADDR | addr;
    send_command(lcd, cmd);
}

void lcd_shift_left(struct lcd_inst* lcd, uint8_t n) {
    for (int i = 0; i < n; i++) {
        send_command(lcd, LCD_SHIFT_DISPLAY_LEFT);
    }
}

void lcd_write_char(struct lcd_inst* lcd, char c) { write_char(lcd, c); }

void lcd_write_string(struct lcd_inst* lcd, char* s) {
    uint8_t ac        = lcd_get_ac(lcd);
    uint8_t high_addr = DDRAM_ROW1_END;
    if (ac > DDRAM_ROW1_END) {
        high_addr = DDRAM_ROW2_END;
    }

    uint8_t max_len = high_addr - ac;
    if (strlen(s) < max_len) {
        max_len = strlen(s);
    }

    for (int i = 0; i < max_len; i++) {
        write_char(lcd, s[i]);
    }
}
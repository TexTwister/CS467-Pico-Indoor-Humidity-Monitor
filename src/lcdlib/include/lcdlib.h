#ifndef LCDLIB_H
#define LCDLIB_H

#include <hardware/i2c.h>

/**
 * Holds configuration and I2C instance for the LCD screen.
 */
struct lcd_inst {
    uint32_t    baudrate;  /// I2C baud rate
    uint32_t    hw_num;    /// I2C controller number (0 or 1)
    uint32_t    i2c_sda;   /// I2C SDA pin number
    uint32_t    i2c_scl;   /// I2C SCL pin number
    i2c_inst_t* i2c_inst;  /// Pico-SDK I2C instance; will be set by lcd_init()
};

/**
 * Clear the LCD's screen and return Address Counter to 0.
 *
 * @param lcd An initialized lcd_inst struct
 */
void lcd_clear_screen(struct lcd_inst* lcd);
/**
 * Initialize the LCD screen with the given I2C parameters. Sets up I2C on the
 * Pico, and initializes the LCD screen in 4-bit mode and with no cursor, 2
 * lines, and 5x8 characters.
 *
 * @param lcd An lcd_inst struct holding values for baudrate, I2C controller
 * number, and SDA and SDL pin numbers. The function will populate i2c_inst with
 * the i2c instance returned by the Pico I2C hardware library.
 */
void lcd_init(struct lcd_inst* lcd);
/**
 * Get the LCD's current address counter value.
 *
 * @param lcd An initialized lcd_inst struct.
 * @return The current address counter.
 */
uint8_t lcd_get_ac(struct lcd_inst* lcd);
/**
 * Set the LCD's current at a specific memory location.
 *
 * @param lcd An initialized lcd_inst struct.
 * @param addr A valid DDRAM memory location (0x0-0x27 for top row or 0x40-0x67
 * for bottom row)
 */
void lcd_set_cursor(struct lcd_inst* lcd, uint8_t addr);
/**
 * Shift the display a number of characers to the left.
 *
 * @param lcd An initialized lcd_inst struct.
 * @param n How many characters to shift left by.
 */
void lcd_shift_left(struct lcd_inst* lcd, uint8_t n);
/**
 * Write a single character at the LCD's current Address Counter.
 *
 * @param lcd An initialized lcd_inst struct.
 * @param c The character to write to the LCD.
 */
void lcd_write_char(struct lcd_inst* lcd, char c);
/**
 * Write a string at the LCD's current Address Counter. Strings that are
 * longer than the difference between the current Address Counter and the
 * highest DDRAM address will be cut off.
 *
 * @param lcd An initialized lcd_inst struct.
 * @param s A null-terminated string.
 */
void lcd_write_string(struct lcd_inst* lcd, char* s);

#endif  // LCDLIB_H
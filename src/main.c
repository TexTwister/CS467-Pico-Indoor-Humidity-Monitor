#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lcdlib.h"

int main() {
    // Instantiate the LCD instance with configuration
    struct lcd_inst lcd = {
        100 * 1000,  // baud rate
        0,           // Pico I2C controller number (can be 0 or 1)
        8,           // I2C SDA pin number
        9,           // I2C SCL pin number
        NULL,        // Pico-SDK I2C instance; set NULL because the library will
                     // populate it during lcd_init()
    };

    lcd_init(&lcd);

    char* message1 = "HELLO WORLD!";

    while (true) {
        // write string to LCD and display it
        lcd_write_string(&lcd, message1);

        // get the LCD's current address counter value (i.e., the display's
        // "cursor" position)
        uint8_t ac_addr = lcd_get_ac(&lcd);

        sleep_ms(1000);

        // clear screen (cursor goes back to 0) and write the former ac_addr to
        // display
        lcd_clear_screen(&lcd);
        char message2[32];
        sprintf(message2, "%x", ac_addr);
        lcd_write_string(&lcd, message2);
        sleep_ms(1000);
        lcd_clear_screen(&lcd);
    }
}
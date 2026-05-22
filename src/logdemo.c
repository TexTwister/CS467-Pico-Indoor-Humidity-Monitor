#include "pico/stdlib.h"

#include "logging.h"

int main(void) {
    stdio_init_all();

    while (1) {
        set_log_level(DEBUG);

        printlog(DEBUG, "A debug message");
        printlog(INFO, "An info message");
        printlog(WARNING, "A warning message");
        printlog(ERROR, "An error message");

        printlog(INFO, "Setting loglevel to Warning...");
        sleep_ms(1000);
        set_log_level(WARNING);

        printlog(DEBUG, "You won't see this message");
        printlog(INFO, "Nor will you see this info message");
        printlog(WARNING, "But you will see this warning message");
        printlog(ERROR, "And you will see this error message");
        sleep_ms(1000);

        set_log_level(DEBUG);

        printlog(INFO, "Use it like %s with %s", "printf()", "formatted text");
        sleep_ms(1000);
    }

    return 0;
}
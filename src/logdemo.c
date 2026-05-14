#include "pico/stdlib.h"

#include "logging.h"

int main(void) {
    stdio_init_all();

    while (1) {
        set_log_level(DEBUG);

        printlog("A debug message", DEBUG);
        printlog("An info message", INFO);
        printlog("A warning message", WARNING);
        printlog("An error message", ERROR);

        printlog("Setting loglevel to Warning...", INFO);
        sleep_ms(1000);
        set_log_level(WARNING);

        printlog("You won't see this message", DEBUG);
        printlog("Nor will you see this info message", INFO);
        printlog("But you will see this warning message", WARNING);
        printlog("And you will see this error message", ERROR);
        sleep_ms(1000);

        include_fileinfo();
        printlog("include file and line number where printlog() was called",
                 WARNING);
        disable_fileinfo();
        sleep_ms(1000);
    }

    return 0;
}
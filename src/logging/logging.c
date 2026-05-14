#include "logging.h"
#include "pico/printf.h"
#include "pico/stdlib.h"

#define MAX_MSG_CHARS 1024

static LogLevel _lvl     = INFO;
static int      fileinfo = 0;

void set_log_level(LogLevel lvl) { _lvl = lvl; }

void include_fileinfo(void) { fileinfo = 1; }

void disable_fileinfo(void) { fileinfo = 0; }

void printlog(LogLevel lvl, const char* msg, ...) {
    if (lvl < _lvl) {
        return;
    }

    char* level_str;

    switch (lvl) {
        case DEBUG:
            level_str = "\033[1;92mDEBUG  \033[0m";
            break;
        case INFO:
            level_str = "\033[1;97mINFO   \033[0m";
            break;
        case WARNING:
            level_str = "\033[1;93mWARNING\033[0m";
            break;
        default:
            level_str = "\033[1;91mERROR  \033[0m";
            break;
    }

    va_list list;
    va_start(list, msg);

    char fmesg[MAX_MSG_CHARS];
    vsnprintf(fmesg, MAX_MSG_CHARS, msg, list);
    va_end(list);

    printf("%010d | %s | \"%s\"", to_ms_since_boot(get_absolute_time()),
           level_str, fmesg);

    if (fileinfo) {
        printf(" | file: %s | line: %d", __FILE__, __LINE__);
    }

    printf("\n");
}

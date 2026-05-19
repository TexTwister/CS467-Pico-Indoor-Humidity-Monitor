#ifndef LOGGING_H
#define LOGGING_H

/**
 * Logging level definitions. Default is info.
 */
typedef enum {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
} LogLevel;

/**
 * Set the logging level. Logging statements below this level will not be
 * printed.
 *
 * @param lvl Minimum log level to output
 */
void set_log_level(LogLevel lvl);

/**
 * Log a message to stdout, if the given level is above the minimum defined
 * log level. Use this like you would printf() by passing a formatted string
 * msg and the arguments to insert into the formatted string.
 *
 * @param lvl The log's level.
 * @param msg The message to include in the printed log output.
 * @param ... Variable number of arguments to include in formatted msg,
 * separated by commas.
 *
 * @note the formatted message is limited to 1024 characters; anything longer
 * will be cut off.
 */
void printlog(LogLevel lvl, const char* msg, ...);

#endif  // LOGGING_H
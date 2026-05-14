#ifndef _LOGGING_H
#define _LOGGING_H

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
 * Include filename and line of log function in logging output (useful for
 * debugging).
 */
void include_fileinfo(void);

/**
 * Don't include filename and line of log function in logging output.
 *
 * @note filename and log information are excluded by default. You only need
 * to call this if you previously called include_fileinfo() and now want to
 * keep them out.
 */
void disable_fileinfo(void);

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

#endif  // _LOGGING_H
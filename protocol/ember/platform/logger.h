#pragma once

/**
 * @brief Defines severity levels for logging output.
 *
 * Higher numeric values generally represent lower severity / more verbose logs.
 */
typedef enum emplat_log_level {
    EMBER_LOG_LEVEL_FATAL, /**< Unrecoverable error */
    EMBER_LOG_LEVEL_ERROR, /**< Error condition */
    EMBER_LOG_LEVEL_WARN,  /**< Warning condition */
    EMBER_LOG_LEVEL_INFO,  /**< Informational message */
    EMBER_LOG_LEVEL_TRACE, /**< Detailed trace/debugging information */
} emplat_log_level;

/**
 * @brief Logs a message to the default system logger.
 *
 * @param log_level Severity level of the log message.
 * @param message Message string.
 */
void emplat_print(emplat_log_level log_level, const char* message);

#ifdef EMBER_DEFINE_HELPERS

/**
 * @brief Logs a formatteed message to the default system logger.
 *
 * @param log_level Severity level of the log message.
 * @param message Format string.
 * @param ... Optional format arguments.
 */
void emplat_printf(emplat_log_level log_level, const char* message, ...);

#endif

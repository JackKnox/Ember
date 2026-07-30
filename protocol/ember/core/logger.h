#pragma once

/**
 * @brief Defines severity levels for logging output.
 *
 * Higher numeric values generally represent lower severity / more verbose logs.
 */
typedef enum log_level {
    LOG_LEVEL_FATAL, /**< Critical error causing immediate failure */
    LOG_LEVEL_ERROR, /**< Error condition */
    LOG_LEVEL_WARN,  /**< Warning condition */
    LOG_LEVEL_INFO,  /**< Informational message */
    LOG_LEVEL_TRACE, /**< Detailed trace/debugging information */
    LOG_LEVEL_DEV,   /**< Very verbose developer-only logs (See EMBER_DEV) */
} log_level;

/**
 * @brief Function pointer type for custom log output handlers.
 *
 * @param level Severity level of the log message.
 * @param subsystem Subsystem/category string (e.g. "GPU", "Audio").
 * @param formatted_message Final formatted log message string.
 */
typedef void (*PFN_log_output)(log_level level, const char* subsystem, const char* formatted_message);

/**
 * @brief Logs a formatted message to the default console logger.
 *
 * This function supports printf-style formatting.
 *
 * @param level Severity level of the log.
 * @param subsystem Subsystem/category string (used for filtering/identification).
 * @param message Format string (printf-style).
 * @param ... Optional format arguments.
 */
void emlog_console(log_level level, const char* subsystem, const char* message, ...);

/**
 * @brief Registers a custom log callback.
 *
 * Replaces or overrides the default logging output mechanism.
 *
 * @param func Function pointer receiving log messages.
 */
void emlog_callback(PFN_log_output func);

/**
 * @brief Symbol for default logger in system.
 * 
 * Implemented by current Driver.
 */
void default_logger(log_level level, const char* subsystem, const char* message);
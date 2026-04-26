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
 * @param subsystem Subsystem/category string (e.g. "Renderer", "Audio").
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
void em_log_console(log_level level, const char* subsystem, const char* message, ...);

/**
 * @brief Registers a custom log callback.
 *
 * Replaces or overrides the default logging output mechanism.
 *
 * @param func Function pointer receiving log messages.
 */
void em_log_callback(PFN_log_output func);

#ifndef EM_LOG
// General logging function for Ember.
#define EM_LOG(level, subsystem, message, ...) em_log_console(level, "Ember/" subsystem, message __VA_OPT__(,) __VA_ARGS__)
#endif

#ifndef EM_FATAL
/** @brief Logs an fatal-level message. */
#define EM_FATAL(subsystem, message, ...) EM_LOG(LOG_LEVEL_FATAL, subsystem, message, __VA_ARGS__)
#endif

#ifndef EM_ERROR
/** @brief Logs an error-level message. */
#define EM_ERROR(subsystem, message, ...) EM_LOG(LOG_LEVEL_ERROR, subsystem, message, __VA_ARGS__)
#endif

#ifndef EM_WARN
/** @brief Logs an warn-level message. */
#define EM_WARN(subsystem, message, ...) EM_LOG(LOG_LEVEL_WARN, subsystem, message, __VA_ARGS__)
#endif

#ifndef EM_INFO
/** @brief Logs an info-level message. */
#define EM_INFO(subsystem, message, ...) EM_LOG(LOG_LEVEL_INFO, subsystem, message, __VA_ARGS__)
#endif

#ifndef EM_TRACE
/** @brief Logs an trace-level message. */
#define EM_TRACE(subsystem, message, ...) EM_LOG(LOG_LEVEL_TRACE, subsystem, message, __VA_ARGS__)
#endif

#ifndef EM_DEV
/** @brief Logs an dev-level message. */
#define EM_DEV(subsystem, message, ...) EM_LOG(LOG_LEVEL_DEV, subsystem, message, __VA_ARGS__)
#endif

#if EMBER_DIST
#   define EM_ASSERT(x) ((void)0)
#else
// * NOTE: Assertions in the library are used to validate internal state and function arguments 
// *       that originate from within the library itself. Assertions should only be used to detect 
// *       programming errors or invalid library state. Any user-provided input must be validated explicitly 
// *       and handled gracefully, with clear and descriptive error messages, rather than using assertions.
#   define EM_ASSERT(x) do { if (!(x)) EM_FATAL("ASSERT", #x, __FILE__, __LINE__); } while (0)
#endif
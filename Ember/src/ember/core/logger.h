#pragma once

typedef enum {
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_TRACE = 4
} log_level;

void log_output(log_level level, const char* message, ...);

#if !EM_ENABLE_LOGGING
#   define EM_INFO(...)
#   define EM_TRACE(...)
#endif

#ifndef EM_FATAL
// Logs an fatal-level message.
#define EM_FATAL(message, ...) log_output(LOG_LEVEL_FATAL, message __VA_OPT__(,) __VA_ARGS__)
#endif

#ifndef EM_ERROR
// Logs an error-level message.
#define EM_ERROR(message, ...) log_output(LOG_LEVEL_ERROR, message __VA_OPT__(,) __VA_ARGS__)
#endif

#ifndef EM_WARN
// Logs a warning-level message.
#define EM_WARN(message, ...) log_output(LOG_LEVEL_WARN, message __VA_OPT__(,) __VA_ARGS__)
#endif

#ifndef EM_INFO
// Logs a info-level message.
#define EM_INFO(message, ...) log_output(LOG_LEVEL_INFO, message __VA_OPT__(,) __VA_ARGS__)
#endif

#ifndef EM_TRACE
// Logs a trace-level message.
#define EM_TRACE(message, ...) log_output(LOG_LEVEL_TRACE,message __VA_OPT__(,) __VA_ARGS__)
#endif

#if EM_ENABLE_ASSERTS
// * NOTE: Assertions in the engine are used to validate internal state and function arguments 
// *       that originate from within the engine itself. Assertions should only be used to detect 
// *       programming errors or invalid engine state. Any user-provided input must be validated explicitly 
// *       and handled gracefully, with clear and descriptive error messages, rather than using assertions.
#   define EM_ASSERT(x) do { if (!(x)) EM_FATAL(#x, __FILE__, __LINE__); } while (0)
#else
#   define EM_ASSERT(x) ((void)0)
#endif
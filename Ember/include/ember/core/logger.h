#pragma once

typedef enum {
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_TRACE = 4
} log_level;

void log_output(log_level level, const char* subsystem, const char* message, ...);

#if !EM_ENABLE_LOGGING
#   define EM_INFO(...)
#   define EM_TRACE(...)
#endif

#ifndef EM_LOG
// General logging function for Ember.
#define EM_LOG(level, subsystem, message, ...) log_output(level, "Ember/" subsystem, message __VA_OPT__(,) __VA_ARGS__)
#endif

#ifndef EM_FATAL
// Logs an fatal-level message.
#define EM_FATAL(subsystem, message, ...) EM_LOG(LOG_LEVEL_FATAL, subsystem, message, __VA_ARGS__)
#endif

#ifndef EM_ERROR
// Logs an error-level message.
#define EM_ERROR(subsystem, message, ...) EM_LOG(LOG_LEVEL_ERROR, subsystem, message, __VA_ARGS__)
#endif

#ifndef EM_WARN
// Logs a warning-level message.
#define EM_WARN(subsystem, message, ...) EM_LOG(LOG_LEVEL_WARN, subsystem, message, __VA_ARGS__)
#endif

#ifndef EM_INFO
// Logs a info-level message.
#define EM_INFO(subsystem, message, ...) EM_LOG(LOG_LEVEL_INFO, subsystem, message, __VA_ARGS__)
#endif

#ifndef EM_TRACE
// Logs a trace-level message.
#define EM_TRACE(subsystem, message, ...) EM_LOG(LOG_LEVEL_TRACE, subsystem, message, __VA_ARGS__)
#endif

#if EM_ENABLE_ASSERTS
// * NOTE: Assertions in the engine are used to validate internal state and function arguments 
// *       that originate from within the engine itself. Assertions should only be used to detect 
// *       programming errors or invalid engine state. Any user-provided input must be validated explicitly 
// *       and handled gracefully, with clear and descriptive error messages, rather than using assertions.
#   define EM_ASSERT(x) do { if (!(x)) EM_FATAL("ASSERT", #x, __FILE__, __LINE__); } while (0)
#else
#   define EM_ASSERT(x) ((void)0)
#endif
#include "ember/core.h"

#include <stdarg.h>

static PFN_log_output logger = NULL;

void default_logger(log_level level, const char* subsystem, const char* message) {
    const char* level_strings[] = { "FATAL", "ERROR",  "WARN",  "INFO",  "TRACE", "DEV" };
    static const char* colours[] = { "\033[1;37;101m", "\033[1;31m", "\033[1;33m", "\033[1;32m", "\033[1;36m", "\033[0;30m" };

    emwrite_console("%s[%-5s] %-*s: %s\033[0m\n",
        colours[level],
        level_strings[level],
        16,
        subsystem,
        message);
}

void em_log_console(log_level level, const char* subsystem, const char* message, ...) {
    EM_ASSERT(level <= LOG_LEVEL_DEV && subsystem != NULL && message != NULL && "Invalid arguments passed to em_log_console");

    va_list args;
    va_start(args, message);

    va_list args_copy;
    va_copy(args_copy, args);
    u64 length = (u64)vsnprintf(NULL, 0, message, args_copy);
    va_end(args_copy);

    char* formatted = mem_allocate(NULL, length + 1, MEMORY_TAG_CORE);
    if (!formatted) {
        va_end(args);
        return;
    }

    vsnprintf(formatted, length + 1, message, args);
    formatted[length] = '\0';

    va_end(args);

    if (!logger)
        default_logger(level, subsystem, formatted);
    else
        logger(level, subsystem, formatted);

    mem_free(NULL, formatted, length + 1, MEMORY_TAG_CORE);

    if (level == LOG_LEVEL_FATAL)
        emdebug_break();
}

void em_log_callback(PFN_log_output func) {
    EM_ASSERT(func != NULL && "Invalid arguments passed to em_log_callback");
    logger = func;
}
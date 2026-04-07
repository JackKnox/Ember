#include "ember/core.h"

#include "ember/core/string_utils.h"
#include "ember/platform/global.h"

#include <stdarg.h>

void log_output(log_level level, const char* subsystem, const char* message, ...) {
    const char* level_strings[] = { "FATAL", "ERROR",  "WARN",  "INFO",  "TRACE" };
    static const char* colors[] = { "\033[1;37;101m", "\033[1;31m", "\033[1;33m", "\033[1;32m", "\033[1;36m" };
    EM_ASSERT(level < EM_ARRAYSIZE(level_strings) && subsystem != NULL && message != NULL && "Invalid arguments passed to log_output");

    va_list args;
    va_start(args, message);
    char* formatted = string_format_v(message, args);
    va_end(args);

    emc_console_write("%s[%-5s] %-*s: %s\033[0m\n",
        colors[level],
        level_strings[level],
        16,
        subsystem,
        formatted);
    emc_free(formatted, FALSE);

    if (level == LOG_LEVEL_FATAL)
        emdebug_break();
}
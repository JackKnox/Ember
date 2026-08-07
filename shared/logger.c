#include "ember/core.h"

#include <ember/platform/logger.h>

#include <stdio.h>
#include <stdarg.h>

// TODO: Move to drivers.

void emplat_print(emplat_log_level log_level, const char* message) {
    static const char* colours[] = { "\033[1;37;101m", "\033[1;31m", "\033[1;33m", "\033[1;32m", "\033[1;36m" };

    printf("%s%s\033[0m\n",
        colours[log_level],
        message);
}

static char message_buf[64] = {};

void emplat_printf(emplat_log_level log_level, const char* message, ...) {
    va_list args;
    va_start(args, message);

    va_list args_copy;
    va_copy(args_copy, args);
    u64 length = (u64)vsnprintf(NULL, 0, message, args_copy);
    va_end(args_copy);

    vsnprintf(message_buf, length + 1, message, args);
    message_buf[length] = '\0';

    va_end(args);

    emplat_print(log_level, message_buf);
}

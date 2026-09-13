#include "ember/core.h"

#include <ember/platform/logger.h>

#include <stdio.h>
#include <stdarg.h>

#include <stdlib.h>

void emplat_printf(emplat_log_level log_level, const char* message, ...) {
    va_list args;
    va_start(args, message);

    va_list args_copy;
    va_copy(args_copy, args);
    u64 length = (u64)vsnprintf(NULL, 0, message, args_copy);
    va_end(args_copy);

    char* formatted = malloc(length + 1);

    vsnprintf(formatted, length + 1, message, args);
    formatted[length] = '\0';

    va_end(args);

    free(formatted);

    emplat_print(log_level, formatted);
}

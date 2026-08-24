#pragma once

#include "ember/core.h"

typedef struct emplat_clock_timestamp {
    i64 seconds;

    i32 nanoseconds;
} emplat_clock_timestamp;


/**
 * @brief Returns the current time at UTC+0 time.
 *
 * @return The current time at UTC+0.
 */
emplat_clock_timestamp emplat_clock_utc();

/**
 * @brief Retrieves the current local UTC time.
 *
 * @return The current system time in UTC.
 */
emplat_clock_timestamp emplat_clock_local();

/**
 * @brief Retrieves the system timezone offset.
 *
 * @return The current local timezone offset from UTC, in seconds.
 *
 */
i32 emplat_clock_timezone();

/**
 * @brief Converts the current time to a specified timezone.
 *
 * @param timezone Timezone offset from UTC, in seconds.
 *
 * @return The current time adjusted to the specified timezone.
 */
emplat_clock_timestamp emplat_clock_at_timezone(i32 timezone);

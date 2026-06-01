#pragma once

#include "ember/core.h"

/**
 * @brief High resolution timestamp in seconds.
 *
 * This value is monotonic and suitable for measuring elapsed time.
 * The starting point is unspecified and should not be interpreted
 * as wall-clock time.
 */
typedef f64 emplat_time;

/**
 * @brief Platform timer frequency information.
 *
 * Useful for debugging or exposing timer precision information.
 */
typedef struct emplat_timer_info {
    /**
     * @brief Estimated timer resolution in nanoseconds.
     */
    u64 resolution_ns;

    /**
     * @brief Whether the timer is monotonic.
     */
    b8 monotonic;

    /**
     * @brief Whether the timer is high precision.
     */
    b8 high_precision;
} emplat_timer_info;

/**
 * @brief Returns the current monotonic time in seconds.
 *
 * This timer is intended for profiling, frame timing,
 * animation, and elapsed time calculations.
 *
 * @return Current monotonic time in seconds.
 */
emplat_time emplat_time_now();

/**
 * @brief Returns the current monotonic time in nanoseconds.
 *
 * Useful for high precision profiling.
 *
 * @return Current monotonic time in nanoseconds.
 */
u64 emplat_time_now_ns();

/**
 * @brief Calculates elapsed time in milliseconds.
 *
 * @param start Start timestamp.
 * @param end End timestamp.
 *
 * @return Elapsed time in milliseconds.
 */
f64 emplat_time_elapsed_ms(emplat_time start, emplat_time end);

/**
 * @brief Suspends execution for a specified duration in milliseconds.
 *
 * @param ms Duration to sleep in milliseconds.
 */
void emplat_sleep_ms(f64 ms);

/**
 * @brief Suspends execution for a specified duration in microseconds.
 *
 * This may internally fall back to a less precise sleep
 * depending on platform capabilities.
 *
 * @param us Duration to sleep in microseconds.
 */
void emplat_sleep_us(u64 us);

/**
 * @brief Retrieves information about the active platform timer.
 *
 * @return Timer information structure.
 */
emplat_timer_info emplat_system_timer_info();
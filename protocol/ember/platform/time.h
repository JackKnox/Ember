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
 * @brief Calculates elapsed time in milliseconds.
 *
 * @param start Start timestamp.
 * @param end End timestamp.
 *
 * @return Elapsed time in milliseconds.
 */
f64 emplat_time_elapsed_ms(emplat_time start, emplat_time end);

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
 * @brief Gets the current system time.
 *
 * @return The current time represented as an @ref emplat_time value.
 */
emplat_time emplat_time_now();

/**
 * @brief Gets the current system time in nanoseconds.
 *
 * Uses the highest-resolution clock available on the platform.
 *
 * @return Current time in nanoseconds.
 */
u64 emplat_time_now_ns();

/**
 * @brief Retrieves information about the current platform timer state.
 *
 * @return A populated @ref emplat_timer_info structure containing
 *         platform timing information.
 */
emplat_timer_info emplat_timer_now_info();

/**
 * @brief Platform-specific timer handle.
 */
typedef void* emplat_timer;

/**
 * @brief Creates a timer.
 *
 * @param start Whether the timer should begin running immediately.
 * @param out_timer Pointer to the timer that will receive the created timer state.
 *
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_timer_create(b8 start, emplat_timer* out_timer);

/**
 * @brief Resets a timer to its initial state.
 *
 * Any accumulated elapsed time is discarded.
 *
 * @param timer Pointer to the timer.
 */
void emplat_timer_reset(emplat_timer* timer);

/**
 * @brief Starts a timer.
 *
 * If the timer is already running, this function has no effect.
 *
 * @param timer Pointer to the timer.
 */
void emplat_timer_start(emplat_timer* timer);

/**
 * @brief Stops a timer.
 *
 * Elapsed time remains available for querying after the timer is stopped.
 *
 * @param timer Pointer to the timer.
 */
void emplat_timer_stop(emplat_timer* timer);

/**
 * @brief Restarts a timer.
 *
 * Resets the elapsed time and immediately starts the timer.
 *
 * @param timer Pointer to the timer.
 */
void emplat_timer_restart(emplat_timer* timer);

/**
 * @brief Determines whether a timer is currently running.
 *
 * @param timer Pointer to the timer.
 *
 * @return TRUE if the timer is running; otherwise FALSE.
 */
b8 emplat_timer_running(const emplat_timer* timer);

/**
 * @brief Gets the elapsed time in seconds.
 *
 * @param timer Pointer to the timer.
 *
 * @return Elapsed time in seconds.
 */
f64 emplat_timer_elapsed(const emplat_timer* timer);

/**
 * @brief Gets the elapsed time in milliseconds.
 *
 * @param timer Pointer to the timer.
 *
 * @return Elapsed time in milliseconds.
 */
f64 emplat_timer_elapsed_ms(const emplat_timer* timer);

/**
 * @brief Gets the elapsed time in microseconds.
 *
 * @param timer Pointer to the timer.
 *
 * @return Elapsed time in microseconds.
 */
u64 emplat_timer_elapsed_us(const emplat_timer* timer);

/**
 * @brief Gets the elapsed time in nanoseconds.
 *
 * @param timer Pointer to the timer.
 *
 * @return Elapsed time in nanoseconds.
 */
u64 emplat_timer_elapsed_ns(const emplat_timer* timer);

/**
 * @brief Retrieves detailed information about a timer.
 *
 * @param timer Pointer to the timer.
 *
 * @return A populated @ref emplat_timer_info structure describing the timer.
 */
emplat_timer_info emplat_timer_get_info(emplat_timer* timer);

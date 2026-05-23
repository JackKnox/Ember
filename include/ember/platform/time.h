#pragma once

#include "ember/core.h"

/**
 * @brief Returns the current time in milliseconds.
 *
 * The time is typically measured from an unspecified starting point
 * (e.g., application start or system uptime).
 *
 * @return Current time in milliseconds as a f64.
 */
f64 emplat_current_time();

/**
 * @brief Suspends execution for a specified duration.
 *
 * @param ms Duration to sleep in milliseconds.
 */
void emplat_sleep_ms(f64 ms);
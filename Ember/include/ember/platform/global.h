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

/**
 * @brief Gets the value of an environment variable.
 *
 * @param name The name of the environment variable.
 * @return Pointer to a null-terminated string containing the value,
 *         or NULL if the variable does not exist.
 *
 * @note The returned pointer is managed by the system and must not be freed.
 *       It may become invalid after subsequent environment modifications.
 */
const char* emplat_system_get_env(const char* name);

/**
 * @brief Sets or updates an environment variable.
 *
 * @param name  The name of the environment variable.
 * @param value The value to assign to the variable.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 *
 * @note This function may overwrite an existing variable.
 *       Behavior may vary slightly between platforms.
 */
em_result emplat_system_set_env(const char* name, const char* value);

/**
 * @brief Retrieves the process ID of the current application.
 */
u32 emplat_system_get_pid();

/**
 * @brief Executes a system command.
 *
 * @param command Null-terminated string containing the command to execute.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_system_execute(const char* command);

/**
 * @brief Opaque handle to a dynamically loaded library.
 *
 * This type represents a platform-specific shared library handle.
 */
typedef void* emplat_library;

/**
 * @brief Loads a dynamic/shared library.
 *
 * @param path Path to the library file (e.g., .so, .dll, .dylib).
 * @return A handle to the loaded library, or NULL on failure.
 *
 * @note The returned handle must be released using
 *       emplat_system_library_unload().
 */
emplat_library emplat_system_library_load(const char* path);

/**
 * @brief Retrieves a symbol from a loaded library.
 *
 * @param lib  Handle to the loaded library.
 * @param name Name of the symbol to retrieve.
 * @return Pointer to the symbol, or NULL if not found.
 */
void* emplat_system_library_symbol(emplat_library lib, const char* name);

/**
 * @brief Unloads a previously loaded dynamic library.
 *
 * @param lib Opaque handle to a dynamic library.
 *
 * @note After unloading, the handle becomes invalid and must not be used.
 */
void emplat_system_library_unload(emplat_library lib);

/**
 * @brief Put UTF-8 text into the clipboard.
 *
 * @param text The text to store in the clipboard.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_set_clipboard_text(const char* text);

/**
 * @brief Get UTF-8 text from the clipboard.
 *
 * @return the clipboard text on success, or NULL on failure.
 * 
 * @note The returned string must be freed manually using `emc_free`.
 */
char* emplat_get_clipboard_text();

/**
 * @brief Query whether the clipboard exists and contains a non-empty text string.
 *
 * @return true if the clipboard has text, or false if it does not.
 */
b8 emplat_has_clipboard_text();

/**
 * @brief Get the number of logical CPU cores available.
 *
 * @return The total number of logical CPU cores. On CPUs that include
 *         technologies such as hyperthreading, the number of logical cores
 *         may be more than the number of physical cores.
 */
u32 emplat_get_cpu_cores();

/**
 * @brief Determine the L1 cache line size of the CPU.
 *
 * This is useful for determining multi-threaded structure padding or SIMD
 * prefetch sizes.
 *
 * @return The L1 cache line size of the CPU, in bytes.
 */
u32 emplat_get_cache_line_size();

/**
 * Get the amount of RAM configured in the system.
 *
 * @return The amount of RAM configured in the system in MiB.
 */
u32 emplat_get_system_ram();

/**
 * The basic state for the system's power supply.
 */
typedef enum emplat_powerstate {
    EMBER_POWERSTATE_ERROR = -1,  /**< Error determining power status */
    EMBER_POWERSTATE_UNKNOWN,     /**< Cannot determine power status */
    EMBER_POWERSTATE_ON_BATTERY,  /**< Not plugged in, running on the battery */
    EMBER_POWERSTATE_NO_BATTERY,  /**< Plugged in, no battery available */
    EMBER_POWERSTATE_CHARGING,    /**< Plugged in, charging battery */
    EMBER_POWERSTATE_CHARGED      /**< Plugged in, battery charged */
} emplat_powerstate;

/**
 * @brief Get the current power supply details.
 *
 * You should not take this function as absolute truth. Batteries
 * (especially failing batteries) are delicate hardware, and the values
 * reported here are best estimates based on what that hardware reports.
 *
 * On some platforms, retrieving power supply details might be expensive. If
 * you want to display continuous status you could call this function every
 * minute or so.
 *
 * @param seconds A pointer filled in with the seconds of battery life left.
 *                This will be filled in with -1 if it can't determine a value or there is no battery.
 * @param percent A pointer filled in with the percentage of battery life, between 0 and 100. 
 *                This will be filled in with -1 when we can't determine a value or there is no battery.
 * @return The current battery state or `SDL_POWERSTATE_ERROR` on failure.
 * 
 * @note It's possible a platform can only report battery percentage or time left
 * but not both.
 */
emplat_powerstate emplat_get_powerinfo(i32* seconds, i32* percent);
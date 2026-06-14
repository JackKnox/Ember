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
emplat_time emplat_system_now();

/**
 * @brief Gets the current system time in nanoseconds.
 *
 * Uses the highest-resolution clock available on the platform.
 *
 * @return Current time in nanoseconds.
 */
u64 emplat_system_now_ns();


/**
 * @brief Retrieves information about the current platform timer state.
 *
 * @return A populated @ref emplat_timer_info structure containing
 *         platform timing information.
 */
emplat_timer_info emplat_system_now_info();

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
 * @param filepath Path to the library file (e.g., .so, .dll, .dylib).
 * @return A handle to the loaded library, or NULL on failure.
 *
 * @note The returned handle must be released using
 *       emplat_system_library_unload().
 */
emplat_library emplat_system_library_load(const char* filepath);

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
u32 emplat_system_cache_line_size();

/**
 * @brief Get the amount of RAM configured in the system.
 *
 * @return The amount of RAM configured in the system in MiB.
 */
u32 emplat_system_ram();

/**
 * @brief The type of the OS-provided default folder for a specific purpose.
 */
typedef enum emplat_system_folder {
    EMBER_SYSTEM_FOLDER_HOME,    /**< The folder which contains all of the current user's data or documents */
    EMBER_SYSTEM_FOLDER_ROOT,    /**< The base folder for the rest of the system. May not commonly have permissions for this folder */
    EMBER_SYSTEM_FOLDER_APPDATA, /**< Safe space to store long-standing app data, not likely to be deleted. */
    EMBER_SYSTEM_FOLDER_TEMP,    /**< Temporary data for use by the application. Do not use for important app data */
} emplat_system_folder;

/**
 * @brief Finds the most suitable user folder for a specific purpose.
 * 
 * Many OSes provide certain standard folders for certain purposes, such as
 * storing pictures, music or videos for a certain user. This function gives
 * the path for many of those special locations.
 * 
 * @param folder the type of folder to find.
 * @returns either a null-terminated C string containing the full path to the
 *          folder, or NULL if an error happened.
 */
const char* emplat_get_systemfolder(emplat_system_folder folder);

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
 * @return The current battery state or `EMBER_POWERSTATE_ERROR` on failure.
 * 
 * @note It's possible a platform can only report battery percentage or time left
 * but not both.
 */
emplat_powerstate emplat_system_powerinfo(i32* seconds, i32* percent);
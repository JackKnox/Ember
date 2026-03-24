#pragma once

#include "defines.h"

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
 * @brief Retrieves the name of the current operating system.
 *
 * @return Pointer to a constant string representing the system name.
 *         The returned pointer must not be freed.
 */
const char* emplat_system_name();

/**
 * @brief Gets the value of an environment variable.
 *
 * @param name The name of the environment variable.
 * 
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
 *
 * @return Non-zero on success, zero on failure.
 *
 * @note This function may overwrite an existing variable.
 *       Behavior may vary slightly between platforms.
 */
b8 emplat_system_set_env(const char* name, const char* value);

/**
 * @brief Retrieves the process ID of the current application.
 */
u32 emplat_system_get_pid();

/**
 * @brief Executes a system command.
 *
 * @param command Null-terminated string containing the command to execute.
 *
 * @return Non-zero on success, zero on failure.
 */
b8 emplat_system_execute(const char* command);

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
 *
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
 *
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
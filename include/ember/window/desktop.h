#pragma once

#include "ember/core.h"

#include "ember/window/internal.h"

/**
 * @brief Platform desktop handle.
 * 
 * A desktop represents a connection to a display manager (e.g. Wayland or Win32).
 * To create one you must have at least one window. The desktop controls state such
 * as input state, event callbacks, joysticks and monitor control.
 */
typedef struct emwin_desktop {
    /** @brief Indicates whether the desktop was successfully initialized. */
    b8 initialized;

    /** @brief Platform-specific display state. */
    EMBER_PLATFORM_DESKTOP_STATE;
} emwin_desktop;

/**
 * @brief Returns a platform-specific handle to the underlying window manager.
 * 
 * @param desktop Pointer to the desktop.
 */
void* emwin_desktop_handle(emwin_desktop* desktop);

/**
 * @brief Processes pending all OS events.
 *
 * Polls and dispatches OS events such as input, window movement, resizing,
 * and close requests. This function should be called once per frame.
 *
 * @param desktop Pointer to the desktop.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emwin_desktop_update(emwin_desktop* desktop);

/**
 * @brief Put UTF-8 text into the clipboard.
 *
 * @param text The text to store in the clipboard.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emwin_set_clipboard_text(emwin_desktop* desktop, const char* text);

/**
 * @brief Get UTF-8 text from the clipboard.
 *
 * @return the clipboard text on success, or NULL on failure.
 * 
 * @note The returned string must be freed manually using `mem_free`.
 */
char* emwin_get_clipboard_text(emwin_desktop* desktop);

/**
 * @brief Query whether the clipboard exists and contains a non-empty text string.
 *
 * @return true if the clipboard has text, or false if it does not.
 */
b8 emwin_has_clipboard_text(emwin_desktop* desktop);
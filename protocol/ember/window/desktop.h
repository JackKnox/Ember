#pragma once

#include "ember/core.h"

/*
 * @brief Unique identifier for a window.
 *
 * @note Two windows from different desktops may have the
 *       same window id.
 */
typedef u64 emwin_window_id;

/*
 * @brief Unique identifier for a monitor.
 *
 * @note Two monitors from different desktops may have the 
 *       same monitor id.
 */
typedef u64 emwin_monitor_id;

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
    void* internal_context;
} emwin_desktop;

/**
 * @brief Describes a physical display monitor.
 */
typedef struct emwin_monitor {
    /** @brief Position of the monitor in compositor space. */
    uvec2 position;

    /** @brief Physical dimensions of the monitor in millimeters. */
    uvec2 physical_size;

    /** @brief Logical dimensions of the monitor in pixels. */
    uvec2 size;

    /** @brief Subpixel layout of the monitor. */
    i32 subpixel;

    /** @brief Manufacturer name. */
    const char* make;

    /** @brief Model name. */
    const char* model;

    /** @brief Unique output name assigned by the compositor. */
    const char* name;

    /** @brief Transform applied to the monitor. */
    i32 transform;

    /** @brief Unique identifier for the monitor. */
    emwin_monitor_id id;
} emwin_monitor;

struct emwin_desktop_event;

/**
 * Polls for the next pending desktop event without blocking.
 *
 * If an event is available, it is written to @p out_event and a success
 * result is returned. If no events are pending, the function returns
 * immediately with a result indicating that no event was available.
 *
 * @param desktop The desktop instance to poll.
 * @param out_event Receives the next event if one is available.
 *
 * @return A Ember result code indicating success, no pending events, or an error.
 */
em_result emwin_poll_events(emwin_desktop* desktop, struct emwin_desktop_event* out_event);

/**
 * Waits until the next desktop event becomes available.
 *
 * This function blocks the calling thread until an event is available or
 * an error occurs. The received event is written to @p out_event.
 *
 * @param desktop The desktop instance to wait on.
 * @param out_event Receives the next event.
 *
 * @return A Ember result code indicating success or an error.
 */
em_result emwin_wait_events(emwin_desktop* desktop, struct emwin_desktop_event* out_event);

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

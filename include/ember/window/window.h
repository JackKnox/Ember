#pragma once 

#include "ember/core.h"

#include "ember/window/desktop.h"

#include "ember/window/internal.h"

/**
 * @brief Describes the display mode of a window.
 */
typedef enum emwin_window_mode {
    EMBER_WINDOW_MODE_WINDOWED,   /**< Standard windowed mode. */
    EMBER_WINDOW_MODE_MAXIMIZED,  /**< Window is created maximized. */
    EMBER_WINDOW_MODE_FULLSCREEN  /**< Fullscreen mode covering the entire display. */
} emwin_window_mode;

/**
 * @brief Bitmask flags controlling window creation behavior.
 *
 * These flags can be combined using bitwise OR.
 */
typedef enum emwin_window_flags {
    EMBER_WINDOW_FLAGS_VISIBLE       = 1 << 0, /**< Window is visible immediately after creation. */
    EMBER_WINDOW_FLAGS_NOT_DECORATED = 1 << 1, /**< Window has no title bar / borders. User may want to render them manually. */
    EMBER_WINDOW_FLAGS_LOCKED_SIZE   = 1 << 2, /**< Window can be resized by the user. */
    EMBER_WINDOW_FLAGS_VSYNC         = 1 << 3, /**< Window has VSync enabled (metadata held for the GAPI). */
} emwin_window_flags;

/**
 * @brief Controls how the cursor behaves within the window.
 */
typedef enum emwin_cursor_mode {
    EMBER_CURSOR_MODE_NORMAL,   /**< Cursor is visible and moves freely. */
    EMBER_CURSOR_MODE_HIDDEN,   /**< Cursor is hidden but not locked. */
    EMBER_CURSOR_MODE_DISABLED  /**< Cursor is hidden and locked (relative input mode). */
} emwin_cursor_mode;

/**
 * @brief Configuration used when creating a window.
 *
 * This structure defines all parameters required to initialise a platform window,
 * including size, position, display mode, and behavioral flags.
 */
typedef struct emwin_window_config {
    /** @brief Initial window display mode. */
    emwin_window_mode window_mode;

    /** @brief Initial cursor mode. */
    emwin_cursor_mode cursor_mode;
    
    /** @brief Window creation flags. */
    emwin_window_flags flags;

    /** @brief Window title as a UTF-8 encoded string. */
    const char* title;
    
    union {
        /** @brief Absolute position in screen coordinates (pixels). */
        uvec2 absolute_pos;

        /** @brief If TRUE, window will be centered on the selected display. */
        b8 centered_pos;
    };

    /** @brief Minimum client area size in pixels (0,0 = no limit). */
    uvec2 min_size;

    /** @brief Maximum client area size in pixels (0,0 = no limit). */
    uvec2 max_size;

    /** @brief Initial client area size in pixels. */
    uvec2 size;

    /** 
     * @brief A connection to the global system's WM. 
     * 
     * If NULL, a new desktop object will be created on window open. 
     */
    emwin_desktop* desktop;
} emwin_window_config;

/**
 * @brief Platform window handle.
 *
 * Represents a platform window and its associated state.
 * All platform- and renderer-specific details are stored internally
 * and are opaque to the user.
 */
typedef struct emwin_window {
    /** @brief Current client area size in pixels. Updated on resize events. */
    uvec2 size;

    /**
     * @brief Current window title.
     *
     * This string is owned by the window and may be reallocated when changed.
     */
    char* title;

    /** @brief Owner desktop object, represents a connection to the WM. */
    emwin_desktop* desktop;

    /** @brief Platform-specific window state. */
    EMBER_PLATFORM_WINDOW_STATE
} emwin_window;

/**
 * @brief Creates a default window configuration.
 *
 * The returned configuration contains sensible defaults for all fields.
 *
 * @return A default-initialized @ref emwin_window_config.
 */
emwin_window_config emwin_window_default();

/**
 * @brief Creates and opens a window.
 *
 * Initialises a platform window using the provided configuration and writes
 * the resulting state to @p out_window.
 *
 * @param config Pointer to the window configuration.
 * @param allocator Allocator used for internal allocations.
 * @param out_window Pointer to the window to initialise.
 * @param out_desktop Pointer to already created desktop, may be NULL.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 *
 * @note Passing a valid @p out_desktop is strongly recommended to avoid
 *       reinitialising shared global platform state.
 */
em_result emwin_window_open(const emwin_window_config* config, em_allocator* allocator, emwin_window* out_window, emwin_desktop** out_desktop);

/**
 * @brief Closes and destroys a window.
 *
 * Releases all platform and renderer resources associated with the window.
 *
 * @param window Pointer to the window to destroy.
 */
void emwin_window_close(em_allocator* allocator, emwin_window* window);

/**
 * @brief Checks whether the window has been requested to close.
 *
 * This typically becomes TRUE when the user attempts to close the window
 * (e.g., clicking the close button).
 *
 * @param window Pointer to the window.
 * @return TRUE if the window should close; otherwise FALSE.
 */
b8 emwin_window_should_close(emwin_window* window);
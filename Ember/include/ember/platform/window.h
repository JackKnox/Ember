#pragma once 

#include "ember/core.h"

#include "ember/platform/internal.h"

/**
 * @brief Describes the display mode of a window.
 */
typedef enum emplat_window_mode {
    EMBER_WINDOW_MODE_WINDOWED,   /**< Standard windowed mode. */
    EMBER_WINDOW_MODE_MAXIMIZED,  /**< Window is created maximized. */
    EMBER_WINDOW_MODE_FULLSCREEN  /**< Fullscreen mode covering the entire display. */
} emplat_window_mode;

/**
 * @brief Bitmask flags controlling window creation behavior.
 *
 * These flags can be combined using bitwise OR.
 */
typedef enum emplat_window_flags {
    EMBER_WINDOW_FLAGS_RESIZABLE = 1 << 0, /**< Window can be resized by the user. */
    EMBER_WINDOW_FLAGS_VISIBLE   = 1 << 1, /**< Window is visible immediately after creation. */
    EMBER_WINDOW_FLAGS_DECORATED = 1 << 2, /**< Window has OS-provided decorations (title bar, borders). */
} emplat_window_flags;

/**
 * @brief Controls how the cursor behaves within the window.
 */
typedef enum emplat_cursor_mode {
    EMBER_CURSOR_MODE_NORMAL,   /**< Cursor is visible and moves freely. */
    EMBER_CURSOR_MODE_HIDDEN,   /**< Cursor is hidden but not locked. */
    EMBER_CURSOR_MODE_DISABLED  /**< Cursor is hidden and locked (relative input mode). */
} emplat_cursor_mode;

/**
 * @brief Configuration used when creating a window.
 *
 * This structure defines all parameters required to initialise a platform window,
 * including size, position, display mode, and behavioral flags.
 */
typedef struct emplat_window_config {
    /** @brief Initial window display mode. */
    emplat_window_mode window_mode;

    /** @brief Initial cursor mode. */
    emplat_cursor_mode cursor_mode;
    
    /** @brief Window creation flags. */
    emplat_window_flags flags;

    /** @brief Window title as a UTF-8 encoded string. */
    const char* title;
    
    union {
        /** @brief Absolute position in screen coordinates (pixels). */
        uvec2 window_absolute;

        /** @brief If TRUE, window will be centered on the selected display. */
        b8 window_centered;
    };

    /** @brief Minimum client area size in pixels (0,0 = no limit). */
    uvec2 min_size;

    /** @brief Maximum client area size in pixels (0,0 = no limit). */
    uvec2 max_size;

    /** @brief Initial client area size in pixels. */
    uvec2 size;
} emplat_window_config;

/**
 * @brief Platform window handle.
 *
 * Represents a platform window and its associated state.
 * All platform- and renderer-specific details are stored internally
 * and are opaque to the user.
 */
typedef struct emplat_window {
    /** @brief Renderer-specific state (e.g., swapchain, surface). */
    void* internal_renderer_state;

    /** @brief Current client area size in pixels. Updated on resize events. */
    uvec2 size;

    /**
     * @brief Current window title.
     *
     * This string is owned by the window and may be reallocated when changed.
     */
    char* title;

    /** @brief Platform-specific window state (opaque). */
    EMBER_PLATFORM_WINDOW_STATE
} emplat_window;

/**
 * @brief Creates a default window configuration.
 *
 * The returned configuration contains sensible defaults for all fields.
 *
 * @return A default-initialized @ref emplat_window_config.
 */
emplat_window_config emplat_window_default();

/**
 * @brief Creates and opens a window.
 *
 * Initialises a platform window using the provided configuration and writes
 * the resulting state to @p out_window.
 *
 * @param config Pointer to the window configuration.
 * @param allocator Allocator used for internal allocations.
 * @param last_window Pointer to the previously created window, or NULL if none.
 * @param out_window Pointer to the window to initialise.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 *
 * @note Passing a valid @p last_window is strongly recommended to avoid
 *       reinitialising shared global platform state.
 */
em_result emplat_window_open(const emplat_window_config* config, ember_allocator* allocator, emplat_window* last_window, emplat_window* out_window);

/**
 * @brief Closes and destroys a window.
 *
 * Releases all platform and renderer resources associated with the window.
 *
 * @param window Pointer to the window to destroy.
 */
void emplat_window_close(emplat_window* window);

/**
 * @brief Processes pending window events.
 *
 * Polls and dispatches OS events such as input, window movement, resizing,
 * and close requests. This function should be called once per frame.
 *
 * @param window Pointer to the window.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_window_pump_messages(emplat_window* window);

/**
 * @brief Waits for and processes window events.
 *
 * Blocks the calling thread until at least one event is received, then
 * processes pending events.
 *
 * @param window Pointer to the window.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_window_wait_messages(emplat_window* window);

/**
 * @brief Checks whether the window has been requested to close.
 *
 * This typically becomes TRUE when the user attempts to close the window
 * (e.g., clicking the close button).
 *
 * @param window Pointer to the window.
 * @return TRUE if the window should close; otherwise FALSE.
 */
b8 emplat_window_should_close(emplat_window* window);
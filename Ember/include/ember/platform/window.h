#pragma once 

#include "ember/core.h"

#include "ember/platform/internal.h"

/**
 * @brief Describes the display mode of a window.
 */
typedef enum emplat_window_mode {
    EMBER_WINDOW_MODE_WINDOWED,   /**< Normal windowed mode. */
    EMBER_WINDOW_MODE_MAXIMIZED,  /**< Window starts maximized. */
    EMBER_WINDOW_MODE_FULLSCREEN  /**< Fullscreen mode, covering the display. */
} emplat_window_mode;

/**
 * @brief Configuration used when creating a window.
 *
 * Defines parameters required to initialise a platform window,
 * including size, position, and display mode.
 */
typedef struct emplat_window_config {
    /** @brief Initial window display mode. */
	emplat_window_mode window_mode;

    /** @brief Window title (UTF-8 string). */
	const char* title;
    
	union {
        /** @brief Absolute position in screen coordinates. */
		uvec2 window_absolute;

        /** @brief Whether the window should be centered. */
		b8 window_centered;
	};

    /** @brief Initial window size in pixels. */
	uvec2 size;
} emplat_window_config;

/**
 * @brief Platform window handle.
 *
 * This structure stores internal platform-specific state and renderer-specific
 * state associated with the window. These pointers are opaque to the user.
 */
typedef struct emplat_window {
    /** @brief Renderer-specific state (e.g., swapchain, surface). */
	void* internal_renderer_state;

    /** @brief Current size of the window, changes when resizing. */
    uvec2 size;

    /** @brief Current title of the window, updates when renaming. */
    const char* title;

    /** @brief Platform-specific window state. */
	EMBER_PLATFORM_WINDOW_STATE
} emplat_window;

/**
 * @brief Creates a default window configuration.
 *
 * @return A default-initialized emplat_window_config.
 */
emplat_window_config emplat_window_default();

/**
 * @brief Creates and opens a window.
 *
 * This function initialises the platform window using the provided configuration
 * and stores the resulting state in @p out_window.
 *
 * @param config Pointer to the window configuration.
 * @param last_window Pointer to last window created, may be NULL.
 * @param out_window Pointer to the window to initialise.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 * 
 * @note Accurately setting @p last_window is extremely recommened 
 *       as it avoids creating global state twice across windows.
 */
em_result emplat_window_open(const emplat_window_config* config, ember_allocator* allocator, emplat_window* last_window, emplat_window* out_window);

/**
 * @brief Closes and destroys a window.
 *
 * Frees all platform and renderer resources associated with the window.
 * 
 * @param window Pointer to the relevent window.
 */
void emplat_window_close(emplat_window* window);

/**
 * @brief Processes window events/messages.
 *
 * This should be called once per frame to handle input, OS events, and window updates.
 * 
 * @param window Pointer to the relevent window.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
b8 emplat_window_pump_messages(emplat_window* window);

/**
 * @brief Checks whether the window should close.
 *
 * Typically becomes TRUE when the user attempts to close the window.
 * 
 * @param window Pointer to the relevent window.
 * @return Returns whetever window should now close.
 */
b8 emplat_window_should_close(emplat_window* window);
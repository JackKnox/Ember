// Example when we open a window with defaults.
//
#include <ember/window/window.h>

#include <stdbool.h>

// A small helper macro to cut down on the repetitive error-check boilerplate.
// Calls a function, checks the result, logs a message and jumps to cleanup if it failed.
// em_result_string() converts the result code into something human readable.
#define CHECK_FUNC(func, message)                          \
    {                                                      \
        em_result result = func;                           \
        if (result != EMBER_RESULT_OK) {                   \
            emlog_console(LOG_LEVEL_ERROR, "Examples", message ": %s", \
				em_result_string(result, true));           \
            goto cleanup;                                  \
        }                                                  \
    }

int main(int argc, char** argv) {
    // Ah yes the Allocator API. This is a structure used by all ember subsystems
    // simply holds malloc / free / realloc along with some metadata.
    em_allocator system_malloc = em_allocator_default(); // em_allocator_default() referes to the OS allocator (malloc / free)

    // I need a config structure to open a window
    emwin_window_config window_config = emwin_window_default(); // This sets many helpful values, if we didn't set any values the code would still work.
    window_config.window_mode  = EMBER_WINDOW_MODE_WINDOWED;                            // Just is the standard mode for a window.
    window_config.cursor_mode  = EMBER_CURSOR_MODE_NORMAL;                              // Again normal mode but could use this to hide or lock the cursor (FPS games).
    window_config.flags        = EMBER_WINDOW_FLAGS_VISIBLE | EMBER_WINDOW_FLAGS_VSYNC; // Can actually see the window and VSync is enabled.
    window_config.title        = "Example - Basic Window";                              // The name of the window, see this in the title bar. Uses UTF-8 strings
    window_config.centered_pos = true;                                                  // Center the window to the primary monitor.
    window_config.size         = (uvec2) { 800, 600 };                                  // Initial size of the window when you open it.
    //window_config.desktop = NULL; Have not already created a window.
    //window_config.min_size; These two are already set to zero.
    //window_config.max_size; Use these to set limits to the size of the window.

    // This represents a connection to the systems WM and a collection of managed windows.
    emwin_desktop* desktop = NULL;

    // I will now open a window.
    emwin_window window = {};
    
    CHECK_FUNC(
        emwin_window_open(&window_config, &system_malloc, &window, &desktop),
        "Failed to open ember window");

    // Boom. A window is now open on your desktop. However we still need to update
    // the window every frame. If you don't do this you get those 'X not responding screens' 
    // and your window becomes unresponsive.

    // emwin_window_should_close will become true when the user presses the X in the
    // corner of the window or you send a signal to the window. You may choose to ignore this value.
    while (!emwin_window_should_close(&window)) {

        // This is how we update our window. I told a lie, we do not need to update each
        // window individually but our connection to desktop as a whole. As a result this one
        // call updates all windows and sends window events to relevent callbacks.
        emwin_desktop_update(desktop);
    }

    // Ok the user or yourself have decided to close the window so now we will
    // destroy all resources and exit the program...
cleanup:
    // The allocator used to open and close the window MUST match or you will some very scary errors pummeling into the depths of OS code.
    // Automatically destroys the desktop connection as well.
    emwin_window_close(&system_malloc, &window);
    return 0;
}
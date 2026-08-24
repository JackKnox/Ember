// Example when we open a window with defaults.
//
#define EMBER_DEFINE_HELPERS

#include <ember/platform/logger.h>
#include <ember/platform/system.h>

#include <ember/window/window.h>
#include <ember/window/events.h>

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// A small helper macro to cut down on the repetitive error-check boilerplate.
// Calls a function, checks the result, logs a message and jumps to cleanup if it failed.
// em_result_string() converts the result code into something human readable.
#define CHECK_FUNC(func, message)                          \
    {                                                      \
        em_result result = func;                           \
        if (result != EMBER_RESULT_OK) {                   \
            emplat_printf(EMBER_LOG_LEVEL_ERROR, message ": %s", \
				em_result_string(result, true));           \
            goto cleanup;                                  \
        }                                                  \
    }

int main(int argc, char** argv) {
    // Ah yes the Allocator API. This is a structure used by all ember subsystems
    // simply holds malloc / free / realloc along with some metadata.
    em_allocator system_malloc = emplat_system_allocator(); // em_allocator_default() referes to the OS allocator (malloc / free)

    // I need a config structure to open a window
    emwin_window_config window_config = emwin_window_default(); // This sets many helpful values, if we didn't set any values the code would still work.
    window_config.window_mode  = EMBER_WINDOW_MODE_WINDOWED;                            // Just is the standard mode for a window.
    window_config.cursor_mode  = EMBER_CURSOR_MODE_NORMAL;                              // Again normal mode but could use this to hide or lock the cursor (FPS games).
    window_config.flags        = EMBER_WINDOW_FLAGS_VISIBLE | EMBER_WINDOW_FLAGS_VSYNC; // Can actually see the window and VSync is enabled.
    window_config.title        = "Example - Basic Window";                              // The name of the window, see this in the title bar. Uses UTF-8 strings
    window_config.centered_pos = true;                                                  // Center the window to the primary monitor.
    window_config.size.x       = 800;                                                   // Initial size of the window when you open it.
    window_config.size.y       = 600;
    //window_config.desktop = NULL; Have not already created a window.
    //window_config.min_size; These two are already set to zero.
    //window_config.max_size; Use these to set limits to the size of the window.

    // This represents a connection to the systems WM and a collection of managed windows.
    emwin_desktop* desktop = NULL;

    // I will now open a window.
    emwin_window window = {};
    
    CHECK_FUNC(
        emwin_window_open(&system_malloc, &window_config, 0, &window, &desktop),
        "Failed to open ember window");

    uint64_t stride = window.size.x * sizeof(uint32_t);

    emwin_shm_pool_config pool_config = emwin_shm_pool_default();
    pool_config.size = stride * window.size.y;

    emwin_shm_pool shm_pool = {};
    CHECK_FUNC(
        emwin_shm_pool_create(desktop, &system_malloc, &pool_config, &shm_pool), 
        "Failed to allocate desktop shm pool");

    emwin_shm_buffer_config buffer_config = emwin_shm_buffer_default();
    buffer_config.image_format = EMWIN_FORMAT_BGRA8_UINT;
    buffer_config.size         = window.size;
    buffer_config.stride       = stride;

    emwin_shm_buffer shm_buffer = {};
    CHECK_FUNC(
        emwin_shm_buffer_alloc(&shm_pool, &system_malloc, &buffer_config, &shm_buffer), 
        "Failed to create window shm buffer")

    // Boom. A window is now open on your desktop. However we still need to update
    // the window every frame. If you don't do this you get those 'X not responding screens' 
    // and your window becomes unresponsive.

    b8 running = EMTRUE;
    while (running) {
        emwin_desktop_event desk_event = {};
        while (emwin_poll_events(desktop, &desk_event) == EMBER_RESULT_OK) {
            switch (desk_event.type) {
                case EMWIN_EVENT_WINDOW_CLOSE:
                    running = EMFALSE;
                    break;
                default: break;
            }
        }

        const float hue_step = 360.0f / window.size.x;
        uint32_t* pixels = (uint32_t*)shm_buffer.buffer;

        for (uint32_t x = 0; x < window.size.x; ++x) {
            uint32_t pixel = hsv_to_argb(fmodf(hue + x * hue_step, 360.0f), 1.0f, 1.0f);

            for (uint32_t y = 0; y < window.size.y; ++y)
                pixels[y * window.size.y + x] = pixel;
        }

        CHECK_FUNC(
            emwin_window_attach(&window, &shm_buffer, (uvec2) { 0, 0 }), 
            "Failed to attach window buffer");

        CHECK_FUNC(
            emwin_window_damage(&window, (uvec2) { 0, 0 }, shm_buffer.size), 
            "Failed to damage window buffer");
    }

    // Ok the user or yourself have decided to close the window so now we will
    // destroy all resources and exit the program...
cleanup:
    emwin_shm_pool_destroy(&system_malloc, &shm_pool);
    // The allocator used to open and close the window MUST match or you will some very scary errors pummeling into the depths of OS code.
    // Automatically destroys the desktop connection as well.
    emwin_window_close(&system_malloc, &window);
    return 0;
}

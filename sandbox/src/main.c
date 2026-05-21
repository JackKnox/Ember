#include <ember/window/window.h>

#include <ember/gpu/device.h>

#define LOG_OUTPUT(level, message, ...) log_console(level, "Sandbox", message __VA_OPT__(,) __VA_ARGS__)

#define CHECK_FUNC(func, message)                          \
    {                                                      \
        em_result result = func;                           \
        if (result != EMBER_RESULT_OK) {                   \
            LOG_OUTPUT(LOG_LEVEL_ERROR, message ": %s",    \
					em_result_string(result, EMBER_BUILD_DEBUG)); \
            goto cleanup;                                 \
        }                                                 \
    }

int main(int argc, char** argv) {
    em_allocator allocator = em_allocator_default();

    emwin_window_config window_config = emwin_window_default();
    window_config.size  = (uvec2){ 640, 640 };
    window_config.title = "Test Window";

    emwin_window window = {};
    CHECK_FUNC(
        emwin_window_open(&window_config, &allocator, NULL, &window),
        "Failed to open window");

    emgpu_device_config device_config = emgpu_device_default();
    device_config.required_modes = EMBER_DEVICE_MODE_GRAPHICS;
    device_config.optional_modes = EMBER_DEVICE_MODE_VALIDATION;
    device_config.debug_name     = window_config.title;

    emgpu_device device = {};
    CHECK_FUNC(
        emgpu_device_init(&device_config, &allocator, &device),
        "Failed to create rendering device");

    emgpu_surface_config surface_config = emgpu_surface_default();
    surface_config.preferred_format = EMGPU_FORMAT_BGRA8_UNORM;
    surface_config.force_format     = FALSE;
    
    emgpu_surface surface = {};

cleanup:
    device.destroy_surface(&device, &allocator, &surface);
    emgpu_device_shutdown(&allocator, &device);
    emwin_window_close(&allocator, &window);

    memory_leaks();
    return 0;
}
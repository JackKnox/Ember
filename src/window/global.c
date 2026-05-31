#include "ember/core.h"
#include "ember/window/window.h"

#include "ember/window/embergpu_surface.h"

emwin_window_config emwin_window_default() {
	emwin_window_config config = {};
	config.window_mode  = EMBER_WINDOW_MODE_WINDOWED;
	config.cursor_mode  = EMBER_CURSOR_MODE_NORMAL;
	config.flags        = EMBER_WINDOW_FLAGS_VISIBLE | EMBER_WINDOW_FLAGS_VSYNC;
	config.title        = "EmberWIN test window";
	config.centered_pos = TRUE;
	config.size.width   = 640;
	config.size.height  = 360;
	return config;
}

emwin_gpu_surface_config emwin_gpu_surface_default() {
	emwin_gpu_surface_config config = {};
    config.preferred_format = EMGPU_FORMAT_BGR8_UNORM;
    config.force_format = TRUE;
	return config;
}
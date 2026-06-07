#include "ember/core.h"
#include "ember/window/window.h"

emwin_window_config emwin_window_default() {
	emwin_window_config config = {};
	config.window_mode  = EMBER_WINDOW_MODE_WINDOWED;
	config.cursor_mode  = EMBER_CURSOR_MODE_NORMAL;
	config.flags        = EMBER_WINDOW_FLAGS_VISIBLE | EMBER_WINDOW_FLAGS_VSYNC;
	config.title        = "Basic Window";
	config.centered_pos = TRUE;
	config.size.width   = 640;
	config.size.height  = 360;
	return config;
}
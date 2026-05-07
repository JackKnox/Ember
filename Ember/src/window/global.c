#include "ember/core.h"
#include "ember/window/window.h"

emwin_window_config emwin_window_default() {
	emwin_window_config config = {};
	config.window_mode = EMBER_WINDOW_MODE_WINDOWED;
	config.window_centered = TRUE;
	config.size.width = 640;
	config.size.height = 360;
	return config;
}
#include "ember/core.h"
#include "ember/window/window.h"
#include "ember/window/input.h"

#ifdef EM_PLATFORM_LINUX
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <time.h>
#include <unistd.h>

void GLFWErrorCallback(int error, const char* description) {
	EM_ERROR("Platform", "Error code: %i '%s'", error, description);
}

void* emwin_desktop_handle(emwin_desktop* desktop) {
	return NULL;
}

void* emwin_window_handle(emwin_window* window) {
	return window->wayland.handle;
}

em_result emwin_window_open(const emwin_window_config* config, em_allocator* allocator, emwin_window* last_window, emwin_window* out_window) {
    glfwSetErrorCallback(GLFWErrorCallback);
	if (glfwInit() == 0) {
		EM_ERROR("Platform", "Failed to initialize GLFW.");
		return EMBER_RESULT_UNKNOWN;
	}

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	int monitorX, monitorY;
	glfwGetMonitorPos(monitor, &monitorX, &monitorY);

	uvec2 window_pos = config->window_absolute;
	if (config->window_centered) {
		window_pos.x = monitorX + (mode->width - config->size.x) / 2;
		window_pos.y = monitorY + (mode->height - config->size.y) / 2;
	}

	if (window_pos.x > (u32)monitorX + mode->width || window_pos.y > (u32)monitorY + mode->height) {
		EM_ERROR("Platform", "Specififed window position is outside range of monitior.");
		return EMBER_RESULT_INVALID_VALUE;
	}

	if (config->size.width >= (u32)mode->width || config->size.height >= (u32)mode->height) {
		EM_ERROR("Platform", "Specififed window size is larger than size of monitior.");
		return EMBER_RESULT_INVALID_VALUE;
	}

	glfwWindowHint(GLFW_MAXIMIZED, config->window_mode == EMBER_WINDOW_MODE_MAXIMIZED);
	glfwWindowHint(GLFW_POSITION_X, window_pos.x);
	glfwWindowHint(GLFW_POSITION_Y, window_pos.y);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	EM_INFO("Platform", "Creating window '%s' (%i, %i)", config->title, config->size.width, config->size.height);

	out_window->wayland.handle = glfwCreateWindow(config->size.width, config->size.height, config->title, config->window_mode == EMBER_WINDOW_MODE_FULLSCREEN ? monitor : NULL, NULL);
	if (!out_window->wayland.handle) {
		EM_ERROR("Platform", "Failed to create window.");
		return EMBER_RESULT_UNKNOWN;
	}

	u32 string_length = strlen(config->title) + 1;

	out_window->title = (char*)mem_allocate(NULL, string_length, MEMORY_TAG_PLATFORM);
	em_memcpy(out_window->title, config->title, string_length);

	out_window->size = config->size;
    return EMBER_RESULT_OK;
}

void emwin_window_close(em_allocator* allocator, emwin_window* window) {
	mem_free(NULL, window->title, strlen(window->title) + 1, MEMORY_TAG_PLATFORM);
	window->title = NULL;

    glfwDestroyWindow(window->wayland.handle);
    glfwTerminate();
}

em_result emwin_desktop_pump_messages(emwin_desktop* desktop) {
	glfwPollEvents();
	return EMBER_RESULT_OK;
}

em_result emwin_desktop_wait_messages(emwin_desktop* desktop) {
	glfwWaitEvents();
	return EMBER_RESULT_OK;
}

b8 emwin_window_should_close(emwin_window* window) {
    return glfwWindowShouldClose(window->wayland.handle);
}

#endif
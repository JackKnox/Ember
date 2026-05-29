#include "ember/core.h"
#include "ember/window/window.h"

#include "ember/window/input.h"

#include <ember/window/ember_gpu_surface.h>

#include <ember/gpu/device.h>
#include <ember/gpu/ext/wayland_surface.h>

#ifdef EM_PLATFORM_LINUX
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <time.h>
#include <unistd.h>

struct wl_display* glfwGetWaylandDisplay (void);
struct wl_surface* glfwGetWaylandWindow (GLFWwindow* handle);

void GLFWErrorCallback(int error, const char* description) {
	EM_ERROR("Platform", "Error code: %i '%s'", error, description);
}

void* emwin_desktop_handle(emwin_desktop* desktop) {
	return glfwGetWaylandDisplay();
}

void* emwin_window_handle(emwin_window* window) {
	return glfwGetWaylandWindow(window->wayland.handle);
}

emgpu_extension_desc emwin_gpu_surface_extension(emwin_desktop* desktop) {
	return emgpu_wayland_surface_extension(glfwGetWaylandDisplay());
}

em_result emwin_gpu_surface_create(emgpu_device* device, em_allocator* allocator, emwin_gpu_surface_ext* extension, emwin_gpu_surface_config* config, emgpu_surface* surface) {
	PFN_create_wayland_surface create_surface = (PFN_create_wayland_surface)extension->create_surface_wsi;

	emgpu_wayland_surface_config wayland_config = emgpu_wayland_surface_default();
    wayland_config.preferred_format = config->preferred_format;
    wayland_config.force_format     = config->force_format;
    wayland_config.size             = config->window->size;
    wayland_config.debug_name       = config->window->title;
	wayland_config.display          = glfwGetWaylandDisplay();
    wayland_config.surface          = glfwGetWaylandWindow(config->window->wayland.handle);

	return create_surface(device, allocator, &wayland_config, surface);
}

em_result emwin_window_open(const emwin_window_config* config, em_allocator* allocator, emwin_window* out_window, emwin_desktop** out_desktop) {
    glfwSetErrorCallback(GLFWErrorCallback);
	if (glfwInit() == 0) {
		EM_ERROR("Platform", "Failed to initialize GLFW.");
		return EMBER_RESULT_UNKNOWN;
	}

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	int monitorX, monitorY;
	glfwGetMonitorPos(monitor, &monitorX, &monitorY);

	uvec2 window_pos = config->absolute_pos;
	if (config->centered_pos) {
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

em_result emwin_desktop_update(emwin_desktop* desktop) {
	glfwPollEvents();
	return EMBER_RESULT_OK;
}

b8 emwin_window_should_close(emwin_window* window) {
    return glfwWindowShouldClose(window->wayland.handle);
}

#endif
#include "ember/core.h"

#ifdef EM_PLATFORM_LINUX
#include "ember/platform/global.h"
#include "ember/platform/window.h"
#include "ember/platform/internal.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>       

#include <sys/wait.h>       
#include <dlfcn.h>

f64 emplat_current_time() {
    return 1000.0f * glfwGetTime();
}

const char* emplat_system_name() {
    return "linux-x11";
}

const char* emplat_system_get_env(const char* name) {
    return getenv(name);
}

em_result emplat_system_set_env(const char* name, const char* value) {
    return setenv(name, value, 1) == 0 ? EMBER_RESULT_OK : EMBER_RESULT_UNKNOWN;
}

u32 emplat_system_get_pid() {
    return getpid();
}

b8 emplat_system_execute(const char* command) {
	pid_t new_pid = fork();
	if (new_pid < 0) return FALSE;

	if (new_pid == 0) {
        execl("/bin/sh", "sh", "-c", command, (char*)0);
        _exit(127);
    }

    int status;
    if (waitpid(new_pid, &status, 0) < 0) {
        return FALSE;
    }

    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

emplat_library emplat_system_library_load(const char* path) {
	return dlopen(path, RTLD_LAZY | RTLD_LOCAL);
}

void* emplat_system_library_symbol(emplat_library lib, const char* name) {
    return dlsym(lib, name);
}

void emplat_system_library_unload(emplat_library lib) {
	dlclose(lib);
}

void GLFWErrorCallback(int error, const char* description) {
	EM_ERROR("%s", description);
}

emplat_window_config emplat_window_default() {
	emplat_window_config config = {};
	config.window_mode = EMBER_WINDOW_MODE_WINDOWED;
	config.size = (uvec2) { 640, 360 };
	config.window_centered = TRUE;
	return config;
}

em_result emplat_window_start(
    emplat_window_config* config, 
    emplat_window* out_window) {
    glfwSetErrorCallback(GLFWErrorCallback);
	if (glfwInit() == 0) {
		EM_ERROR("platform_start(): Failed initializing GLFW.");
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

	if (window_pos.x > monitorX + mode->width || window_pos.y > monitorY + mode->height) {
		EM_ERROR("platform_start(): Specififed window position is outside range of monitior");
		return EMBER_RESULT_INVALID_VALUE;
	}

	if (config->size.width >= mode->width || config->size.height >= mode->height) {
		EM_ERROR("platform_start(): Specififed window size is larger than size of monitior");
		return EMBER_RESULT_INVALID_VALUE;;
	}

	glfwWindowHint(GLFW_MAXIMIZED, config->window_mode == EMBER_WINDOW_MODE_MAXIMIZED);
	glfwWindowHint(GLFW_POSITION_X, window_pos.x);
	glfwWindowHint(GLFW_POSITION_Y, window_pos.y);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	out_window->glfw.handle = glfwCreateWindow(config->size.width, config->size.height, config->title, config->window_mode == EMBER_WINDOW_MODE_FULLSCREEN ? monitor : NULL, NULL);
	if (!out_window->glfw.handle) {
		EM_ERROR("platform_start(): Failed creating application window.");
		return EMBER_RESULT_UNKNOWN;
	}

    return TRUE;
}

void emplat_window_close(emplat_window* window) {
    glfwDestroyWindow(window->glfw.handle);
    glfwTerminate();
}

b8 emplat_window_pump_messages(emplat_window* window) {
	glfwPollEvents();
}

b8 emplat_window_should_close(emplat_window* window) {
    return glfwWindowShouldClose(window->glfw.handle);
}

#endif
#pragma once

#include "ember/core.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

typedef struct _emplat_window_wayland {
    GLFWwindow* handle;
} _emplat_window_wayland;

#define EMBER_PLATFORM_WINDOW_STATE _emplat_window_wayland wayland;
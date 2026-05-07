#pragma once

#include "ember/core.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

typedef struct _emwin_window_wayland {
    GLFWwindow* handle;
} _emwin_window_wayland;

#define EMBER_PLATFORM_WINDOW_STATE _emwin_window_wayland wayland;

#define EMBER_PLATFORM_DESKTOP_STATE u8 _pad;

#define EMBER_PLATFORM_JOYSTICK_STATE u8 _pad;
#pragma once

#include "ember/core.h"

#ifdef EM_PLATFORM_LINUX
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

typedef struct _emplat_window_linux {
    GLFWwindow* handle;
} _emplat_window_linux;

#define EMBER_PLATFORM_WINDOW_STATE _emplat_window_linux glfw;

#endif
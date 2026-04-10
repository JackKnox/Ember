#pragma once

#include "ember/core.h"

#ifdef EM_PLATFORM_LINUX
#include "ember/platform/internal/wayland.h"
#endif

#ifdef EM_PLATFORM_POSIX
#include "ember/platform/internal/posix.h"
#endif
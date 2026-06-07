#pragma once

#include "ember/core.h"

#include <pthread.h>

#define EMBER_PLATFORM_SHM_STATE u8

#define EMBER_PLATFORM_FILE_STATE u8

#define EMBER_PLATFORM_FILEWATCHER_STATE u8

#define EMBER_PLATFORM_TIMER_STATE u8

#define EMBER_PLATFORM_MUTEX_STATE pthread_mutex_t

#define EMBER_PLATFORM_COND_STATE pthread_cond_t

#define EMBER_PLATFORM_THREAD_STATE pthread_t
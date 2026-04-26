#pragma once

#include "ember/core.h"

typedef enum ember_data_type {
    EMBER_DATA_TYPE_UINT,
    EMBER_DATA_TYPE_SINT,
    EMBER_DATA_TYPE_FLOAT,
    EMBER_DATA_TYPE_BOOL,
} ember_data_type;

#define EMBER_FORMAT_FLAG_NORMALIZED 1 << 0
#define EMBER_FORMAT_FLAG_BGRA       1 << 1
#define EMBER_FORMAT_FLAG_SRGB       1 << 2
#define EMBER_FORMAT_FLAG_DEPTH      1 << 3
#define EMBER_FORMAT_FLAG_STENCIL    1 << 4

#define EMBER_FORMAT_MAKE(type, bytes, channels, flags) \
    (((flags)              << 24)  | \
     ((type)               << 20)  | \
     (((bytes) / 8)        << 16)  | \
     ((channels)           << 12))

#define EMBER_FORMAT_FLAGS(format)     (((format)  >> 24)  & 0xFF)
#define EMBER_FORMAT_DATA_TYPE(format) (((format)  >> 20)  & 0xF)
#define EMBER_FORMAT_BYTES(format)     ((((format) >> 16)  & 0xF) * 8)
#define EMBER_FORMAT_CHANNELS(format)  (((format)  >> 12)  & 0xF)
#define EMBER_FORMAT_SIZE(format)      (EMBER_FORMAT_BYTES(format) * EMBER_FORMAT_CHANNELS(format))
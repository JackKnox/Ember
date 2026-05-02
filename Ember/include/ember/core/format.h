#pragma once

#include "ember/core.h"

/**
 * @brief Enumeration of supported data types for formats.
 */
typedef enum ember_data_type {
    EMBER_DATA_TYPE_UINT,  /**< Unsigned integer data */
    EMBER_DATA_TYPE_SINT,  /**< Signed integer data */
    EMBER_DATA_TYPE_FLOAT, /**< Floating-point data */
    EMBER_DATA_TYPE_BOOL,  /**< Boolean data */
} ember_data_type;

/** @brief Format flag indicating normalized data (e.g., UNORM/SNORM). */
#define EMBER_FORMAT_FLAG_NORMALIZED (1 << 0)

/** @brief Format flag indicating BGRA channel ordering. */
#define EMBER_FORMAT_FLAG_BGRA       (1 << 1)

/** @brief Format flag indicating sRGB color space. */
#define EMBER_FORMAT_FLAG_SRGB       (1 << 2)

/** @brief Format flag indicating depth component. */
#define EMBER_FORMAT_FLAG_DEPTH      (1 << 3)

/** @brief Format flag indicating stencil component. */
#define EMBER_FORMAT_FLAG_STENCIL    (1 << 4)

/**
 * @brief Packs a format description into a 32-bit integer.
 *
 * @param type Data type (see @ref ember_data_type).
 * @param bytes Number of bits per channel.
 * @param channels Number of channels (e.g., 4 for RGBA).
 * @param flags Bitmask of EMBER_FORMAT_FLAG_* values.
 * @return Encoded format value.
 */
#define EMBER_FORMAT_MAKE(type, bytes, channels, flags) \
    (((flags)              << 24)  | \
     ((type)               << 20)  | \
     (((bytes) / 8)        << 16)  | \
     ((channels)           << 12))

/** @brief Extracts flags from a format. */
#define EMBER_FORMAT_FLAGS(format)     (((format)  >> 24)  & 0xFF)

/** @brief Extracts data type from a format. */
#define EMBER_FORMAT_DATA_TYPE(format) (((format)  >> 20)  & 0xF)

/** @brief Extracts bits per channel from a format. */
#define EMBER_FORMAT_BYTES(format)     ((((format) >> 16)  & 0xF) * 8)

/** @brief Extracts channel count from a format. */
#define EMBER_FORMAT_CHANNELS(format)  (((format)  >> 12)  & 0xF)

/** @brief Computes total size in bits for the format. */
#define EMBER_FORMAT_SIZE(format) \
    (EMBER_FORMAT_BYTES(format) * EMBER_FORMAT_CHANNELS(format))
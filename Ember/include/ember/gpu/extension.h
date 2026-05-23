#pragma once

#include "ember/core.h"

/**
 * @brief Describes an extension to the rendering device.
 *
 * Extensions are optional or required modules provided by the GPU backend
 * and requested by the user during device creation.
 *
 * They may:
 * - Enable additional GPU features
 * - Provide platform-specific integration (WSI)
 * - Expose extra API entry points
 *
 * Most extensions may require additional platform or library dependencies
 * to be linked. See offical documentation for more.
 */ 
typedef struct emgpu_extension_desc {
    /**
     * @brief Internal name of the extension.
     *
     * This name is defined by the system and is used for matching
     * against backend-supported extensions.
     *
     * @note This value is not intended to be used for application logic.
     */
    const char* name;

    /**
     * @brief Version of the extension.
     *
     * Used to ensure compatibility between the requested extension and
     * the backend implementation.
     */
    em_version version;

    /** @brief Type-erased create info for the extension. */
    void* user_data;

    ///**
    // * @brief Indicates whether the extension is optional.
    // *
    // * If set to FALSE, device initialization will fail if the extension
    // * is not supported by the selected backend.
    // */
    //b8 optional;
} emgpu_extension_desc;
#pragma once

#include "ember/core.h"

#include "ember/window/window.h"

#include <ember/gpu/device.h>

/**
 * @brief Returns the ember_window surface extension descriptor.
 */
emgpu_extension_desc emwin_gpu_surface_extension(emwin_desktop* desktop);

/**
 * @brief Configuration for creating a ember_window-backed GPU surface.
 */
typedef struct emwin_gpu_surface_config {
    /** @brief Requested format of the surface texture. */
    emgpu_format preferred_format;

    /** @brief Whetever to exit if exact preferred format isn't found. */
    b8 force_format;

    /** @brief Window to create the GPU surface on. */
    emwin_window* window;

    /** 
     * @brief Indicates whetever to hook into the window's resize events. 
     * 
     * @note Does not override the user's events. 
     */
    b8 set_callbacks;
} emwin_gpu_surface_config;

/**
 * @brief Returns a default ember_window surface configuration.
 *
 * @return A default-initialized emwin_gpu_surface_config.
 */
emwin_gpu_surface_config emwin_gpu_surface_default();

/**
 * @brief ember_window surface extension API interface.
 *
 * Contains function pointers and internal state required to create
 * and manage ember_window-presentable GPU surfaces.
 */
typedef struct emwin_gpu_surface_ext {
    /** @brief If this value is FALSE, the rest of the structure is uninitalized. */
    b8 enabled;

    /** @brief Function pointer to internal WSI create surface func. */
    void* create_surface_wsi;
} emwin_gpu_surface_ext;

/**
 * @brief Creates a ember_window-backed GPU surface.
 * 
 * @param device Pointer to the device instance.
 * @param allocator Allocator used to manage device memory.
 * @param extension Extension object outputted by rendering device.
 * @param config Surface configuration.
 * @param surface Output surface.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emwin_gpu_surface_create(emgpu_device* device, em_allocator* allocator, emwin_gpu_surface_ext* extension, emwin_gpu_surface_config* config, emgpu_surface* surface);
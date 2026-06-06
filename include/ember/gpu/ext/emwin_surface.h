#pragma once

#include "ember/core.h"

#include "ember/gpu/device.h"

#include <ember/window/window.h>

/**
 * @brief Returns the ember_window surface extension descriptor.
 */
emgpu_extension_desc emgpu_emwin_surface_extension(emwin_desktop* desktop);

typedef struct emgpu_emwin_surface_config {
    /** @brief Requested format of the surface texture. */
    emgpu_format preferred_format;

    /** @brief Whetever to exit if exact preferred format isn't found. */
    b8 force_format;

    /** @brief ember_window window to attach GPU surface to. */
    emwin_window* window;

    /** 
     * @brief Indicates whetever to hook into the window's resize events. 
     * 
     * @note Does not override the user's events. 
     */
    b8 set_callbacks;
} emgpu_emwin_surface_config;

/**
 * @brief Creates a default ember_window surface configuration.
 *
 * @return A default-initialized emgpu_emwin_surface_config.
 */
emgpu_emwin_surface_config emgpu_emwin_surface_default();

/**
 * @brief Function pointer type for creating a ember_window surface.
 *
 * @param device GPU device handle.
 * @param allocator Memory allocator used for internal allocations.
 * @param config ember_window surface creation parameters.
 * @param out_surface Output GPU surface object.
 *
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
typedef em_result (*PFN_create_emwin_surface)(
    emgpu_device* device,
    em_allocator* allocator,
    emgpu_emwin_surface_config* config,
    emgpu_surface* out_surface);

/**
 * @brief ember_window surface extension API interface.
 *
 * Contains function pointers and internal state required to create
 * and manage ember-presentable GPU surfaces.
 */
typedef struct emgpu_emwin_surface_ext {
    /** @brief If this value is FALSE, the rest of the structure is uninitalized. */
    b8 enabled;

    /** @brief Creates a ember_window surface backed by the GPU device. */
    PFN_create_emwin_surface create_surface;
} emgpu_emwin_surface_ext;

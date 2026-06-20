#pragma once

#include "ember/core.h"

#include "ember/gpu/surface.h"

#include <ember/window/window.h>

/**
 * @brief Configuration for creating a ember_window-backed GPU surface.
 */
typedef struct emgpu_emwin_surface_config {
    /** @brief Requested format of the surface texture. */
    emgpu_format preferred_format;

    /** @brief Whetever to exit if exact preferred format isn't found. */
    b8 force_format;

    /** @brief ember_window window to attach GPU surface to. */
    emwin_window* window;
} emgpu_emwin_surface_config;

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
    emgpu_extension_desc desc;

    /** @brief Creates a ember_window surface backed by the GPU device. */
    PFN_create_emwin_surface create_surface;
} emgpu_emwin_surface_ext;

#ifdef EMBER_DEFINE_HELPERS

/**
 * @brief Returns the ember_window surface extension descriptor.
 */
emgpu_emwin_surface_ext emgpu_emwin_surface_extension(emwin_desktop* desktop);

/**
 * @brief Creates a default ember_window surface configuration.
 *
 * @return A default-initialized emgpu_emwin_surface_config.
 */
emgpu_emwin_surface_config emgpu_emwin_surface_default();

#endif
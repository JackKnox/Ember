#pragma once

#include "ember/core.h"

#include "ember/gpu/device.h"

#include <wayland-client.h>

/**
 * @brief Returns the Wayland surface extension descriptor.
 */
emgpu_extension_desc emgpu_wayland_surface_extension(struct wl_display* display);

/**
 * @brief Configuration for creating a Wayland-backed GPU surface.
 */
typedef struct emgpu_wayland_surface_config {
    /** @brief Requested format of the surface texture. */
    emgpu_format preferred_format;

    /** @brief Whetever to exit if exact preferred format isn't found. */
    b8 force_format;

    /** @brief Wayland display connection handle. */
    struct wl_display* display;

    /** @brief Wayland surface handle used for presentation. */
    struct wl_surface* surface;
} emgpu_wayland_surface_config;

/**
 * @brief Returns a default Wayland surface configuration.
 *
 * @return A default-initialized emgpu_wayland_surface_config.
 */
emgpu_wayland_surface_config emgpu_surface_default_wayland();

/**
 * @brief Function pointer type for creating a Wayland surface.
 *
 * @param device GPU device handle.
 * @param allocator Memory allocator used for internal allocations.
 * @param surface_config Wayland surface creation parameters.
 * @param out_surface Output GPU surface object.
 *
 * @return em_result indicating success or failure.
 */
typedef em_result (*PFN_create_wayland_surface)(
    emgpu_device* device,
    em_allocator* allocator,
    emgpu_wayland_surface_config* surface_config,
    emgpu_surface* out_surface);

/**
 * @brief Wayland surface extension API interface.
 *
 * Contains function pointers and internal state required to create
 * and manage Wayland-presentable GPU surfaces.
 */
typedef struct emgpu_wayland_surface_api {
    /** @brief If this value is FALSE, the rest of the structure is uninitalized. */
    b8 enabled;

    /** @brief Internal backend-specific state (opaque). */
    void* internal_data;

    /** @brief Creates a Wayland surface backed by the GPU device. */
    PFN_create_wayland_surface create_surface;
} emgpu_wayland_surface_api;
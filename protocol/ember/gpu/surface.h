#pragma once

#include "ember/core.h"

#include "ember/gpu/types.h"
#include "ember/gpu/device.h"

/**
 * @brief Backend-agnostic GPU surface objects.
 *
 * Represents a backend-agnsotic object that connectes a platform surface.
 */
typedef struct emgpu_surface {
    /** @brief Backend-specific internal data. */
    void* internal_data;

    /** @brief Format of the pixel(s) attachted to the platform surface. */
    emgpu_format pixel_format;

    /** @brief Number of owned images used for concurrent rendering. */
    u32 image_count;
} emgpu_surface;

/**
 * @brief Resizes a rendering size to given size.
 * 
 * @param device Pointer to the device instance.
 * @param surface Surface to resize.
 * @param new_size New size of surface. 0, 0 = minimized.
 * @note Surface is not guarenteeed to be resized immediately; In
 *       some backends surface is resized next frame it's rendered to.
 */
em_result emgpu_device_resize_surface(
    emgpu_device* device, 
    emgpu_surface* surface, 
    uvec2 new_size);

/**
 * @brief Destroys a rendering surface.
 *
 * @param device Pointer to the device instance.
 * @param allocator Allocator used to manage device memory.
 * @param surface Surface to destroy.
 */
void emgpu_device_destroy_surface(
    emgpu_device* device, 
    em_allocator* allocator, 
    emgpu_surface* surface);
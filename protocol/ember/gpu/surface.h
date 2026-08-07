#pragma once

#include "ember/core.h"

#include "ember/gpu/types.h"
#include "ember/gpu/device.h"

#include "ember/gpu/command_buffer.h"

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
em_result emgpu_surface_resize(
    const emgpu_device* device, 
    emgpu_surface* surface, 
    uvec2 new_size);

/**
 * @brief Destroys a rendering surface.
 *
 * @param device Pointer to the device instance.
 * @param allocator Allocator used to manage device memory.
 * @param surface Surface to destroy.
 */
void emgpu_surface_destroy(
    const emgpu_device* device, 
    const em_allocator* allocator, 
    emgpu_surface* surface);

/**
 * @brief Acquires the next available surface texture for rendering.
 *
 * Enqueues a presentation acquisition operation and returns a local
 * reference to the acquired surface texture.
 *
 * @param command_buf Pointer to the command buffer.
 * @param surface Surface to acquire the next presentation image from.
 *
 * @return A local framebuffer handle valid for the duration of the command buffer recording.
 */
emgpu_local_framebuffer emgpu_cmd_acquire_surface(emgpu_command_buffer* command_buf, emgpu_surface* surface);

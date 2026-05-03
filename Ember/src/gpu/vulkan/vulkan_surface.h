#pragma once

#include "ember/core.h"

#include "vulkan_types.h"

em_result vulkan_surface_create(
    emgpu_device* device, 
    const emgpu_surface_config* config,
    emgpu_surface* out_surface);

void vulkan_surface_destroy(
    emgpu_device* device, 
    emgpu_surface* surface);

// Recreates swapchain to a new size, but same format.
em_result vulkan_surface_recreate(
    emgpu_device* device, 
    emgpu_surface* surface, 
    uvec2 new_size);

// Gets the next available surface image after calling acquire.
emgpu_texture* vulkan_surface_curr_texture(
    emgpu_device* device,
    emgpu_surface* surface);

// Acquires the next available swapchain image for rendering.
VkResult vulkan_surface_accquire(
    emgpu_device* device,
    emgpu_surface* surface,
    u64 timeout, 
    VkSemaphore signal_semaphore, VkFence signal_fence);
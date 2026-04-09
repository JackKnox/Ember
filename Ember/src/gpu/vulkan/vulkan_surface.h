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

// Acquires the next available swapchain image for rendering.
VkResult vulkan_surface_accquire(
    emgpu_device* device,
    emgpu_surface* surface,
    u32* out_image_index,
    u64 timeout, VkFence wait_fence);

// Presents the current swapchain image to the screen.
VkResult vulkan_surface_present(
    emgpu_device* device, 
    emgpu_surface* surface, 
    u32 image_index, vulkan_queue* present_queue, 
    u32 wait_semaphore_count, VkSemaphore* wait_semaphores);
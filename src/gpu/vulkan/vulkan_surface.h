#pragma once

#include "ember/core.h"

#include "vulkan_types.h"

void vulkan_surface_destroy(
    emgpu_device* device,
    em_allocator* allocator, 
    emgpu_surface* surface);

// Recreates swapchain to a new size, but same format.
em_result vulkan_surface_recreate(
    emgpu_device* device, 
    emgpu_surface* surface, 
    uvec2 new_size);
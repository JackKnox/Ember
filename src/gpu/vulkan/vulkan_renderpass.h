#pragma once

#include "ember/core.h"

#include "vulkan_types.h"

em_result vulkan_renderpass_create(
    emgpu_device* device,
    em_allocator* allocator, 
    const emgpu_renderpass_config* config,
    emgpu_renderpass* out_renderpass);

void vulkan_renderpass_destroy(
    emgpu_device* device,
    em_allocator* allocator, 
    emgpu_renderpass* renderpass);
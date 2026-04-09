#pragma once

#include "ember/core.h"

#include "vulkan_types.h"

em_result vulkan_buffer_create(
    emgpu_device* device, 
    const emgpu_buffer_config* config, 
    emgpu_buffer* out_buffer);

em_result vulkan_buffer_upload(
    emgpu_device* device, 
    emgpu_buffer* buffer, 
    const void* data, 
    u64 start_offset, u64 region);

void vulkan_buffer_destroy(
    emgpu_device* device, 
    emgpu_buffer* buffer);
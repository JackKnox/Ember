#pragma once

#include "defines.h"

#include "vulkan_types.h"

b8 vulkan_texture_create(
    emgpu_device* device,
    emgpu_texture_config* config,
    emgpu_texture* out_texture);

b8 vulkan_texture_upload_data(
    emgpu_device* device,
    emgpu_texture* texture, 
    const void* data, 
    uvec2 offset, 
    uvec2 region);

void vulkan_texture_destroy(
    emgpu_device* device,
    emgpu_texture* texture);

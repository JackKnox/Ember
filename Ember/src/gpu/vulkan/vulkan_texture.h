#pragma once

#include "ember/core.h"

#include "vulkan_types.h"

em_result vulkan_texture_create(
    emgpu_device* device, 
    const emgpu_texture_config* config, 
    emgpu_texture* out_texture);

em_result vulkan_texture_recreate(
    emgpu_device* device, 
    emgpu_texture* texture,
    uvec2 new_size);

em_result vulkan_texture_upload(
    emgpu_device* device, 
    emgpu_texture* texture, 
    const void* data, 
    uvec2 start_offset, uvec2 region);

void vulkan_texture_destroy(
    emgpu_device* device, 
    emgpu_texture* texture);
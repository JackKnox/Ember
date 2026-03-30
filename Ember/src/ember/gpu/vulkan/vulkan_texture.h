#pragma once

#include "defines.h"

#include "vulkan_types.h"

b8 vulkan_texture_create(
    emgpu_device* device,
    emgpu_texture_config* config,
    emgpu_texture* out_texture);

VkResult vulkan_texture_create_internal(
    vulkan_context* context,
    emgpu_texture_config* config,
    VkImage existing_image,
    b8 ownes_provided_image,
    b8 create_memory,
    emgpu_texture* out_texture);

void vulkan_texture_transition_layout(
    vulkan_context* context,
    vulkan_command_buffer* command_buffer,
    emgpu_texture* texture,
    VkImageLayout new_layout);

b8 vulkan_texture_upload_data(
    emgpu_device* device,
    emgpu_texture* texture, 
    const void* data, 
    uvec2 offset, 
    uvec2 region);

void vulkan_texture_destroy(
    emgpu_device* device,
    emgpu_texture* texture);

#pragma once

#include "defines.h"

#include "vulkan_types.h"

VkResult vulkan_image_create(
    vulkan_context* context,
    uvec2 size,
    VkFormat format,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memory_flags,
    b8 create_view,
    VkImageAspectFlags view_aspect_flags,
    vulkan_image* out_image);

void vulkan_image_transition_layout(
    vulkan_context* context,
    vulkan_command_buffer* cmd, 
    vulkan_image* image,
    VkImageLayout new_layout);

void vulkan_image_copy_from_buffer(
	vulkan_context* context,
	vulkan_command_buffer* cmd,
	vulkan_image* image,
	VkBuffer buffer,
    u64 buf_offset,
    uvec2 img_region);

void vulkan_image_destroy(
    vulkan_context* context,
    vulkan_image* image);
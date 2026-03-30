#pragma once

#include "defines.h"

#include "vulkan_types.h"

b8 vulkan_rendertarget_create(
	emgpu_device* device,
    emgpu_rendertarget_config* config,
    emgpu_rendertarget* out_rendertarget);

b8 vulkan_rendertarget_resize(
	emgpu_device* device,
    emgpu_rendertarget* rendertarget,
    uvec2 new_size);

void vulkan_rendertarget_begin(
    vulkan_context* context,
    vulkan_command_buffer* command_buffer,
    emgpu_rendertarget* rendertarget,
    b8 set_viewport, b8 set_scissor);

void vulkan_rendertarget_end(
    vulkan_context* context,
    vulkan_command_buffer* command_buffer, 
    emgpu_rendertarget* rendertarget);

void vulkan_rendertarget_destroy(
	emgpu_device* device,
    emgpu_rendertarget* rendertarget);
#pragma once

#include "defines.h"

#include "vulkan_types.h"

b8 vulkan_renderstage_create_graphic(
	emgpu_device* device,
	emgpu_graphicstage_config* config, 
	emgpu_rendertarget* bound_rendertarget,
	emgpu_renderstage* out_renderstage);

b8 vulkan_renderstage_create_compute(
	emgpu_device* device,
	emgpu_computestage_config* config,
	emgpu_renderstage* out_renderstage);

void vulkan_renderstage_bind(
	vulkan_context* context,
    vulkan_command_buffer* command_buffer,
	emgpu_renderstage* renderstage);

void vulkan_renderstage_destroy(
	emgpu_device* device,
	emgpu_renderstage* renderstage);

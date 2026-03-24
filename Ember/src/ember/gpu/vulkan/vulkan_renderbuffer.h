#pragma once

#include "defines.h"

#include "vulkan_types.h"

b8 vulkan_renderbuffer_create(
	emgpu_device* device,
	emgpu_renderbuffer_config* config,
	emgpu_renderbuffer* out_buffer);

b8 vulkan_renderbuffer_upload_data(
	emgpu_device* device,
	emgpu_renderbuffer* buffer,
	const void* buf_data,
    u64 buf_offset, 
    u64 buf_size);

b8 vulkan_renderbuffer_map_data(
	emgpu_device* device,
	emgpu_renderbuffer* buffer,
	const void* source,
	u64 buf_offset,
	u64 buf_size);

void vulkan_renderbuffer_destroy(
	emgpu_device* device,
	emgpu_renderbuffer* buffer);
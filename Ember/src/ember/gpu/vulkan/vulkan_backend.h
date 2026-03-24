#pragma once

#include "defines.h"

#include "vulkan_types.h"

#include "vulkan_renderbuffer.h"
#include "vulkan_renderstage.h"
#include "vulkan_rendertarget.h"
#include "vulkan_texture.h"

b8 vulkan_device_initialize(emgpu_device* device, emgpu_device_config* config);
void vulkan_device_shutdown(emgpu_device* device);

void vulkan_device_resized(emgpu_device* device, uvec2 new_size);
b8 vulkan_device_update_descriptors(emgpu_device* device, emgpu_update_descriptors* descriptors, u32 descriptor_count);

b8 vulkan_device_begin_frame(emgpu_device* device, f64 delta_time);
void vulkan_device_execute_command(emgpu_device* device, emgpu_playback_context* context, rendercmd_header* hdr, rendercmd_payload* payload);
b8 vulkan_device_end_frame(emgpu_device* device);

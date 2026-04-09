#pragma once

#include "ember/core.h"

#include "vulkan_types.h"
#include "vulkan_surface.h"
#include "vulkan_rendertarget.h"
#include "vulkan_pipeline.h"
#include "vulkan_buffer.h"
#include "vulkan_texture.h"

em_result vulkan_device_initialize(emgpu_device* device, const emgpu_device_config* config);
void vulkan_device_shutdown(emgpu_device* device);

em_result vulkan_device_capabilities(emgpu_device* device, emgpu_device_capabilities* out_capabilities);

em_result vulkan_device_submit_frame(emgpu_device* device, const emgpu_frame* frame);
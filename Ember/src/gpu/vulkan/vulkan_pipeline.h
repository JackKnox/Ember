#pragma once

#include "ember/core.h"

#include "vulkan_types.h"

em_result vulkan_pipeline_create_graphics(
    emgpu_device* device, 
    const emgpu_graphics_pipeline_config* config, 
    emgpu_rendertarget* bound_rendertarget, 
    emgpu_pipeline* out_graphics_pipeline);

em_result vulkan_pipeline_create_compute(
    emgpu_device* device, 
    const emgpu_compute_pipeline_config* config, 
    emgpu_pipeline* out_compute_pipeline);

em_result vulkan_pipeline_update_descriptors(
    emgpu_device* device, 
    emgpu_pipeline* pipeline, 
    emgpu_update_descriptors* descriptors, 
    u32 descriptor_count);

void vulkan_pipeline_destroy(
    emgpu_device* device, 
    emgpu_pipeline* pipeline);
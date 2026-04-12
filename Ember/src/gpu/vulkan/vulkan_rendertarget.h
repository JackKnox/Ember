#pragma once 

#include "ember/core.h"

#include "vulkan_types.h"

em_result vulkan_rendertarget_create(
    emgpu_device* device, 
    const emgpu_rendertarget_config* config,
    emgpu_rendertarget* out_rendertarget);

em_result vulkan_rendertarget_create_present(
    emgpu_device* device, 
    const emgpu_present_target_config* config, 
    emgpu_rendertarget* out_rendertarget);

em_result vulkan_rendertarget_resize(
    emgpu_device* device, 
    emgpu_rendertarget* rendertarget, 
    uvec2 new_size);

void vulkan_rendertarget_destroy(
    emgpu_device* device, 
    emgpu_rendertarget* rendertarget);
#include "ember/core.h"
#include "ember/gpu/resources.h"

#include "ember/gpu/device.h"

emgpu_device_config emgpu_device_default() {
    emgpu_device_config config = {};
    config.debug_name       = "EMBER_GPU";
    config.frame_allocator  = em_allocator_default();
    config.backend_api      = EMBER_DEVICE_BACKEND_VULKAN;
    config.app_version      = EMBER_MAKE_VERSION(0, 0, 1);
    config.required_modes   = EMBER_DEVICE_MODE_GRAPHICS;
    config.frames_in_flight = 3; // Standard in low level GAPIs.
    return config;
}

emgpu_buffer_config emgpu_buffer_default() {
    emgpu_buffer_config config = {};
	return config;
}

emgpu_graphics_pipeline_config emgpu_pipeline_default_graphics() {
	emgpu_graphics_pipeline_config config = {};
    return config;
}

emgpu_compute_pipeline_config emgpu_pipeline_default_compute() {
	emgpu_compute_pipeline_config config = {};
    return config;
}

emgpu_texture_config emgpu_texture_default() {
    emgpu_texture_config config = {};
    config.image_format = EMGPU_FORMAT_RGBA8_SRGB;
    config.filter_type  = EMBER_FILTER_TYPE_NEAREST;
    config.address_mode = EMBER_ADDRESS_MODE_REPEAT;
    config.usage        = EMBER_TEXTURE_USAGE_SAMPLED;   
	return config;
}

u64 emgpu_texture_get_size_in_bytes(emgpu_texture* texture) {
	EM_ASSERT(texture != NULL && "Invalid arguments passed emgpu_texture_get_size_in_bytes");
	return texture->size.x * texture->size.y * EMBER_FORMAT_SIZE(texture->image_format);
}

emgpu_renderpass_config emgpu_renderpass_default() {
    emgpu_renderpass_config config = {};
    return config;
}

emgpu_surface_config emgpu_surface_default() {
    emgpu_surface_config config = {};
    config.preferred_format = EMGPU_FORMAT_BGRA8_UNORM;
	config.force_format     = TRUE;
    return config;
}
#include "ember/core.h"
#include "ember/gpu/device.h"

#include "ember/gpu/raster.h"
#include "ember/gpu/compute.h"

const char* emgpu_backend_type_string(emgpu_device_backend device_backend) {
    switch (device_backend) {
        case EMBER_DEVICE_BACKEND_VULKAN: return "Vulkan";
        case EMBER_DEVICE_BACKEND_OPENGL: return "OpenGL";
        case EMBER_DEVICE_BACKEND_DIRECTX: return "DirectX";
    }
}

const char* emgpu_device_type_string(emgpu_device_type device_type) {
    switch (device_type) {
        case EMBER_DEVICE_TYPE_OTHER:          return "Unknown";
        case EMBER_DEVICE_TYPE_DISCRETE_GPU:   return "Discrete";
        case EMBER_DEVICE_TYPE_CPU:            return "CPU";
        case EMBER_DEVICE_TYPE_INTEGRATED_GPU: return "Intergrated";
        case EMBER_DEVICE_TYPE_VIRTUAL_GPU:    return "Virtual";
    }
}

void emgpu_device_print_capabilities(emgpu_device* device, emgpu_device_capabilities* capabilities, log_level level) {
    EM_LOG(level, "Gpu", "Device capabilities:");
    EM_LOG(level, "Gpu", "  Backend: %s %i.%i.%i - %s [%i.%i.%i]", 
        emgpu_backend_type_string(capabilities->api_type),
        EMBER_VERSION_MAJOR(capabilities->internal_api_version), EMBER_VERSION_MINOR(capabilities->internal_api_version), EMBER_VERSION_PATCH(capabilities->internal_api_version),
        capabilities->vendor_name,
        EMBER_VERSION_MAJOR(capabilities->driver_version), EMBER_VERSION_MINOR(capabilities->driver_version),  EMBER_VERSION_PATCH(capabilities->driver_version));
    EM_LOG(level, "Gpu", "  Selected device: '%s' (%s.)", capabilities->device_name, emgpu_device_type_string(capabilities->device_type));
}

emgpu_device_config emgpu_device_default() {
    emgpu_device_config config = {};
    config.debug_name       = "EMBER_GPU";
    config.frame_allocator  = em_allocator_default();
    config.backend_api      = EMBER_DEVICE_BACKEND_VULKAN;
    config.app_version      = EMBER_MAKE_VERSION(0, 0, 1);
    config.required_modes   = EMBER_DEVICE_MODE_RASTER;
    config.frames_in_flight = 3; // Standard in low level GAPIs.
    return config;
}

emgpu_buffer_config emgpu_buffer_default() {
    emgpu_buffer_config config = {};
	return config;
}

emgpu_raster_blend_config emgpu_raster_blend_default() {
    emgpu_raster_blend_config config = {};
    config.src_colour = EMBER_BLEND_FACTOR_SRC_ALPHA;
    config.dst_colour = EMBER_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    config.colour_op  = EMBER_BLEND_OP_ADD;
    config.src_alpha = EMBER_BLEND_FACTOR_SRC_ALPHA;
    config.dst_alpha = EMBER_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    config.alpha_op  = EMBER_BLEND_OP_ADD;
    return config;
}

emgpu_raster_vertex_config emgpu_raster_vertex_default() {
    emgpu_raster_vertex_config config = {};
    config.topology = EMBER_PRIMITIVE_TYPE_TRIANGLE_LIST;
    return config;
}

emgpu_raster_pipeline_config emgpu_pipeline_default_raster() {
	emgpu_raster_pipeline_config config = {};
    config.vertex_shader.entry_point = "main";
    config.fragment_shader.entry_point = "main";
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
	return texture->size.x * texture->size.y * EMBER_FORMAT_SIZE(texture->image_format);
}

emgpu_attachment_config emgpu_attachment_from_surface(emgpu_surface* surface) {
    emgpu_attachment_config attachment = {};
    attachment.type = EMBER_ATTACHMENT_TYPE_COLOUR;
    attachment.format = surface->pixel_format;
    attachment.load_op = EMBER_LOAD_OP_LOAD;
    attachment.store_op = EMBER_STORE_OP_STORE;
    attachment.stencil_load_op = EMBER_LOAD_OP_DONT_CARE;
    attachment.stencil_store_op = EMBER_STORE_OP_DONT_CARE;
    attachment.presentable = TRUE;
    
    if (EMBER_FORMAT_FLAGS(surface->pixel_format) & EMBER_FORMAT_FLAG_DEPTH)
        attachment.type = EMBER_ATTACHMENT_TYPE_DEPTH;
    if (EMBER_FORMAT_FLAGS(surface->pixel_format) & EMBER_FORMAT_FLAG_STENCIL)
        attachment.type = EMBER_ATTACHMENT_TYPE_STENCIL;
    if (EMBER_FORMAT_FLAGS(surface->pixel_format) & EMBER_FORMAT_FLAG_DEPTH && EMBER_FORMAT_FLAGS(surface->pixel_format) & EMBER_FORMAT_FLAG_STENCIL)
        attachment.type = EMBER_ATTACHMENT_TYPE_DEPTH_STENCIL;
    return attachment;
}

emgpu_renderpass_config emgpu_renderpass_default() {
    emgpu_renderpass_config config = {};
    return config;
}
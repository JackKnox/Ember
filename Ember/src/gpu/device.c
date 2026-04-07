#include "ember/core.h"
#include "ember/gpu/device.h"

#include "vulkan/vulkan_backend.h"

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

em_result emgpu_device_init(const emgpu_device_config* config, emgpu_device* out_device) {
    EM_ASSERT(config != NULL && out_device != NULL && "Invalid arguments passed to emgpu_device_init");

    if (config->api_type == EMBER_DEVICE_BACKEND_VULKAN) {
        out_device->initialize                  = vulkan_device_initialize;
        out_device->shutdown                    = vulkan_device_shutdown;
        out_device->retreive_capabilities       = vulkan_device_capabilities;
        out_device->submit_frame                = vulkan_device_submit_frame;
        out_device->create_surface              = vulkan_surface_create;
        out_device->destroy_surface             = vulkan_surface_destroy;
        //out_device->create_graphics_pipeline    = vulkan_pipeline_create_graphics;
        //out_device->create_compute_pipeline     = vulkan_pipeline_create_compute;
        //out_device->update_pipeline_descriptors = vulkan_pipeline_update_descriptors;
        //out_device->destroy_pipeline            = vulkan_pipeline_destroy;
        out_device->create_buffer               = vulkan_buffer_create;
        out_device->upload_to_buffer            = vulkan_buffer_upload;
        out_device->destroy_buffer              = vulkan_buffer_destroy;
        out_device->create_texture              = vulkan_texture_create;
        out_device->upload_to_texture           = vulkan_texture_upload;
        out_device->destroy_texture             = vulkan_texture_destroy;
    }
    else {
        EM_ERROR("Gpu", "Unsupported backend type (%i).", config->api_type);
        return EMBER_RESULT_UNAVAILABLE_API;
    }

    EM_INFO("Gpu", "Initializing rendering device: %s", emgpu_backend_type_string(config->api_type));
    EM_TRACE("Gpu", "Application name: %s", config->application_name);
    return out_device->initialize(out_device, config);
}

void emgpu_device_shutdown(emgpu_device* device) {
    EM_ASSERT(device != NULL && "Invalid arguments passed to emgpu_device_shutdown");
    if (device->shutdown) 
        device->shutdown(device);
    
    if (device->capabilities) mem_free(device->capabilities, sizeof(emgpu_device_capabilities), MEMORY_TAG_RENDERER);
    emc_memset(device, 0, sizeof(emgpu_device));
}

em_result emgpu_device_print_capabilities(emgpu_device* device, log_level level) {
    em_result result = device->retreive_capabilities(device, NULL);
    if (result != EMBER_RESULT_OK) return result;

    EM_LOG(level, "Gpu", "Device capabilities:");
    EM_LOG(level, "Gpu", "  Backend: %s %i.%i.%i - %s [%i.%i.%i]", 
        emgpu_backend_type_string(device->capabilities->api_type),
        EM_API_VERSION_MAJOR(device->capabilities->internal_api_version), EM_API_VERSION_MINOR(device->capabilities->internal_api_version), EM_API_VERSION_PATCH(device->capabilities->internal_api_version),
        device->capabilities->vendor_name,
        EM_API_VERSION_MAJOR(device->capabilities->driver_version), EM_API_VERSION_MINOR(device->capabilities->driver_version),  EM_API_VERSION_PATCH(device->capabilities->driver_version));
    EM_LOG(level, "Gpu", "  Selected device: '%s' (%s.)", device->capabilities->device_name, emgpu_device_type_string(device->capabilities->device_type));
}
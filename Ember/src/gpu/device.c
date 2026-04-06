#include "ember/core.h"
#include "ember/gpu/device.h"

em_result emgpu_device_init(emgpu_device_config* config, emgpu_device* out_device) {
    EM_ASSERT(config != NULL && out_device != NULL && "Invalid arguments passed to emgpu_device_init");

    if (config->api_type == EMBER_DEVICE_BACKEND_VULKAN) {
        /** ... */ 
    }
    else {
        EM_ERROR("Unsupported renderer backend type (%i).", config->api_type);
        return EMBER_RESULT_UNAVAILABLE_API;
    }

    return out_device->initialize(out_device, config);
}

void emgpu_device_shutdown(emgpu_device* device) {
    EM_ASSERT(device != NULL && "Invalid arguments passed to emgpu_device_shutdown");
    if (device->shutdown) 
        device->shutdown(device);
    
    bzero_memory(device, sizeof(emgpu_device));
}
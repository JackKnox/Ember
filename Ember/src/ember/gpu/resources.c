#include "defines.h"
#include "resources.h"

emgpu_device_config emgpu_device_default() {
    emgpu_device_config config = {};
    config.enabled_modes = EMBER_DEVICE_MODE_GRAPHICS;
	config.frames_in_flight = 3;

#if EM_ENABLE_VALIDATION
    config.enable_validation = TRUE;
#endif
    return config;
}

emgpu_renderbuffer_config emgpu_renderbuffer_default() {
    emgpu_renderbuffer_config config = {};
	return config;
}

emgpu_graphicstage_config emgpu_renderstage_default_graphic() {
	emgpu_graphicstage_config config = {};
    return config;
}

emgpu_computestage_config emgpu_renderstage_default_compute() {
	emgpu_computestage_config config = {};
    return config;
}

emgpu_texture_config emgpu_texture_default() {
    emgpu_texture_config config = {};
	config.filter_type = EMBER_FILTER_TYPE_NEAREST;
	config.address_mode = EMBER_ADDRESS_MODE_REPEAT;
	config.usage = EMBER_TEXTURE_USAGE_SAMPLED;

	return config;
}

u64 emgpu_texture_get_size_in_bytes(emgpu_texture* texture) {
	EM_ASSERT(texture != NULL && "Invalid arguments passed emgpu_texture_get_size_in_bytes");
	return texture->size.x * texture->size.y * EMBER_FORMAT_SIZE(texture->image_format);
}

emgpu_rendertarget_config emgpu_rendertarget_default() {
    emgpu_rendertarget_config config = {};
    return config;
}
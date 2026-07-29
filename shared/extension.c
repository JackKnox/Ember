#include "ember/core.h"

#include "ember/gpu/ext/emwin_surface.h"

emgpu_extension_desc emgpu_emwin_surface_extension(emwin_desktop* desktop, emgpu_emwin_surface_ext* out_extension) {
    emgpu_extension_desc ext = {};
    ext.name = "EMGPU_EXT_emwin_surface";
    ext.version   = EMBER_VERSION;

    emgpu_emwin_surface_params* params = (emgpu_emwin_surface_params*)ext.user_data;
    params->desktop = desktop;
    params->out_extension = out_extension;
    return ext;
}

emgpu_emwin_surface_config emgpu_emwin_surface_default() {
    emgpu_emwin_surface_config config = {};
    config.preferred_format = EMGPU_FORMAT_BGRA8_UNORM;
    config.force_format     = EMTRUE;
    return config;
}

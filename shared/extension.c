#include "ember/core.h"

#include "ember/gpu/ext/wayland_surface.h"
#include "ember/gpu/ext/emwin_surface.h"

emgpu_extension_desc emgpu_wayland_surface_extension(struct wl_display* display) {
    emgpu_extension_desc desc = {};
    desc.name      = "EMGPU_EXT_wl_surface";
    desc.version   = EMBER_VERSION;
    desc.user_data = (void*)display;
    return desc;
}

emgpu_wayland_surface_config emgpu_wayland_surface_default() {
    emgpu_wayland_surface_config config = {};
    config.preferred_format = EMGPU_FORMAT_BGRA8_UNORM;
    config.force_format     = TRUE;
    return config;
}

emgpu_extension_desc emgpu_emwin_surface_extension(emwin_desktop* desktop) {
    emgpu_extension_desc desc = {};
    desc.name = "EMGPU_EXT_emwin_surface";
    desc.version   = EMBER_VERSION;
    desc.user_data = (void*)desktop;
    return desc;
}

emgpu_emwin_surface_config emgpu_emwin_surface_default() {
    emgpu_emwin_surface_config config = {};
    config.preferred_format = EMGPU_FORMAT_BGRA8_UNORM;
    config.force_format     = TRUE;
    return config;
}
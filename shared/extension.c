#include "ember/core.h"

#include "ember/gpu/ext/wayland_surface.h"
#include "ember/gpu/ext/emwin_surface.h"

emgpu_wayland_surface_ext emgpu_wayland_surface_extension(struct wl_display* display) {
    emgpu_wayland_surface_ext ext = {};
    ext.desc.name      = "EMGPU_EXT_wl_surface";
    ext.desc.version   = EMBER_VERSION;
    ext.desc.user_data = (void*)display;
    return ext;
}

emgpu_wayland_surface_config emgpu_wayland_surface_default() {
    emgpu_wayland_surface_config config = {};
    config.preferred_format = EMGPU_FORMAT_BGRA8_UNORM;
    config.force_format     = EMTRUE;
    return config;
}

emgpu_emwin_surface_ext emgpu_emwin_surface_extension(emwin_desktop* desktop) {
    emgpu_emwin_surface_ext ext = {};
    ext.desc.name = "EMGPU_EXT_emwin_surface";
    ext.desc.version   = EMBER_VERSION;
    ext.desc.user_data = (void*)desktop;
    return ext;
}

emgpu_emwin_surface_config emgpu_emwin_surface_default() {
    emgpu_emwin_surface_config config = {};
    config.preferred_format = EMGPU_FORMAT_BGRA8_UNORM;
    config.force_format     = EMTRUE;
    return config;
}
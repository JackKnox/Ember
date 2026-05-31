#include "ember/core.h"
#include "ember/window/window.h"

#ifdef EM_PLATFORM_LINUX
#include "ember/window/embergpu_surface.h"

#include <ember/gpu/extension.h>
#include <ember/gpu/ext/wayland_surface.h>

emgpu_extension_desc emwin_gpu_surface_extension(emwin_desktop* desktop) {
	return emgpu_wayland_surface_extension(desktop->wayland.display);
}

em_result emwin_gpu_surface_create(emgpu_device* device, em_allocator* allocator, emwin_gpu_surface_ext* extension, emwin_gpu_surface_config* config, emgpu_surface* surface) {
	PFN_create_wayland_surface create_surface = (PFN_create_wayland_surface)extension->create_surface_wsi;

	emgpu_wayland_surface_config wayland_config = emgpu_wayland_surface_default();
    wayland_config.preferred_format = config->preferred_format;
    wayland_config.force_format     = config->force_format;
    wayland_config.size             = config->window->size;
    wayland_config.debug_name       = config->window->title;
	wayland_config.display          = config->window->desktop->wayland.display;
    wayland_config.surface          = config->window->wayland.surface;

	return create_surface(device, allocator, &wayland_config, surface);
}
#endif
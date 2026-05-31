#include "ember/core.h"
#include "ember/window/window.h"

#ifdef EM_PLATFORM_LINUX
#include "ember/window/internal/wayland-protocols/xdg-shell-protocol.c"

#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

em_result em_result_from_errno(int ec) {
	return EMBER_RESULT_UNKNOWN;
}

void registry_global_handler(
    void* data,
    struct wl_registry* registry,
    u32 name,
    const char* interface,
    u32 version) {
	emwin_desktop* desktop = (emwin_desktop*)data;

	if (strcmp(interface, "wl_compositor") == 0) {
		desktop->wayland.compositor 
			= wl_registry_bind(registry, name, &wl_compositor_interface, 3);
	}
	else if (strcmp(interface, "wl_shm") == 0) {
		desktop->wayland.shm 
			= wl_registry_bind(registry, name, &wl_shm_interface, 1);
	}
	else if (strcmp(interface, "xdg_wm_base") == 0) {
		desktop->wayland.xdg_wm_base 
			= wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
	}
}

void registry_global_remove_handler(
    void* data,
    struct wl_registry* registry,
    u32 name) {
}

em_result emwin_window_open(
	const emwin_window_config* config, 
	em_allocator* allocator,
	emwin_window* out_window, 
	emwin_desktop** out_desktop) {

	emwin_desktop* desktop = config->desktop;
	
	if (!desktop) {
		EM_INFO("Platform", "Creating new desktop object; previous reference was not passed");
		desktop = mem_allocate(allocator, sizeof(emwin_desktop), MEMORY_TAG_PLATFORM);

		EM_INFO("Wayland", "Connecting to Wayland display");
		desktop->wayland.display = wl_display_connect(NULL);
		int ec = errno;
		if (!desktop->wayland.display) {
			EM_ERROR("Wayland", "Failed to connect to Wayland display: %s", strerror(ec));
			return em_result_from_errno(ec);
		}
	
		EM_INFO("Wayland", "Retrieving Wayland registry");
		desktop->wayland.registry = wl_display_get_registry(desktop->wayland.display);
		ec = wl_display_get_error(desktop->wayland.display);
		if (ec != 0) {
			EM_ERROR("Wayland", "Failed to retrieve Wayland registry: %s", strerror(ec));
			return em_result_from_errno(ec);
		}

		wl_registry_set_user_data(desktop->wayland.registry, desktop);

		desktop->wayland.registry_listener.global = registry_global_handler;
		desktop->wayland.registry_listener.global_remove = registry_global_remove_handler;
		wl_registry_add_listener(desktop->wayland.registry, &desktop->wayland.registry_listener, (void*)desktop);
		wl_display_roundtrip(desktop->wayland.display);
	}

	out_window->wayland.surface = wl_compositor_create_surface(desktop->wayland.compositor);
	
	out_window->wayland.xdg_surface = xdg_wm_base_get_xdg_surface(
									desktop->wayland.xdg_wm_base, out_window->wayland.surface);

	out_window->wayland.xdg_toplevel = xdg_surface_get_toplevel(out_window->wayland.xdg_surface);
	xdg_toplevel_set_title(out_window->wayland.xdg_toplevel, config->title);
	
	if (config->min_size.x != 0 || config->min_size.y != 0)
		xdg_toplevel_set_min_size(out_window->wayland.xdg_toplevel, config->min_size.x, config->min_size.y);

	if (config->max_size.x != 0 || config->max_size.y != 0)
		xdg_toplevel_set_max_size(out_window->wayland.xdg_toplevel, config->max_size.x, config->max_size.y);

	wl_surface_commit(out_window->wayland.surface);
	wl_display_roundtrip(desktop->wayland.display);

	out_window->size = config->size;
	out_window->title = config->title;
	out_window->desktop = desktop;
	*out_desktop = desktop;
	return EMBER_RESULT_OK;
}

void emwin_window_close(em_allocator* allocator, emwin_window* window) {
}

em_result emwin_desktop_update(emwin_desktop* desktop) {
	wl_display_dispatch(desktop->wayland.display);
	return EMBER_RESULT_OK;
}

b8 emwin_window_should_close(emwin_window* window) {
	return FALSE;
}
#endif
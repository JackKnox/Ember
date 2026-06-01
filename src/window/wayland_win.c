#include "ember/core.h"

#ifdef EM_PLATFORM_LINUX
#include "ember/window/desktop.h"
#include "ember/window/window.h"
#include "ember/window/input.h"
#include "ember/window/dialog.h"

#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "ember/window/internal/wayland-protocols/wayland-protocol.c"
#include "ember/window/internal/wayland-protocols/xdg-shell-protocol.c"

void configure_xdg_surface(void* data,
			  struct xdg_surface* xdg_surface,
			  u32 serial) {
	emwin_window* window = (emwin_window*)data;

	xdg_surface_ack_configure(xdg_surface, serial);
}

void ping_xdg_wm_base(void* data,
		      struct xdg_wm_base* xdg_wm_base,
		      uint32_t serial) {
	emwin_desktop* desktop = (emwin_desktop*)data;

	xdg_wm_base_pong(xdg_wm_base, serial);
}

void registry_global_add(void* data,
			  struct wl_registry* registry,
			  u32 name,
			  const char* interface, u32 version) {
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
		xdg_wm_base_add_listener(desktop->wayland.xdg_wm_base, &desktop->wayland.xdg_wm_base_listener, (void*)desktop);
	}
}

void registry_global_remove(
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

		desktop->wayland.registry_listener.global        = registry_global_add;
		desktop->wayland.registry_listener.global_remove = registry_global_remove;
		desktop->wayland.surface_listener.configure      = configure_xdg_surface;
		desktop->wayland.xdg_wm_base_listener.ping       = ping_xdg_wm_base;

		desktop->wayland.display = wl_display_connect(NULL);
		
		desktop->wayland.registry = wl_display_get_registry(desktop->wayland.display);

		wl_registry_set_user_data(desktop->wayland.registry, desktop);
		
		wl_registry_add_listener(desktop->wayland.registry, &desktop->wayland.registry_listener, (void*)desktop);
		wl_display_roundtrip(desktop->wayland.display);
	}

	u32 string_length = (strlen(config->title) + 1) * sizeof(char);
	out_window->title = mem_allocate(NULL, string_length, MEMORY_TAG_PLATFORM);
	em_memcpy(out_window->title, config->title, string_length);

	out_window->size = config->size;
	out_window->desktop = desktop;

	out_window->wayland.surface = wl_compositor_create_surface(desktop->wayland.compositor);
	
	out_window->wayland.xdg_surface = xdg_wm_base_get_xdg_surface(
									desktop->wayland.xdg_wm_base, out_window->wayland.surface);

	xdg_surface_add_listener(out_window->wayland.xdg_surface, &desktop->wayland.surface_listener, (void*)out_window);

	out_window->wayland.xdg_toplevel = xdg_surface_get_toplevel(out_window->wayland.xdg_surface);
	xdg_toplevel_set_title(out_window->wayland.xdg_toplevel, config->title);
	
	if (config->min_size.x != 0 || config->min_size.y != 0)
		xdg_toplevel_set_min_size(out_window->wayland.xdg_toplevel, config->min_size.x, config->min_size.y);

	if (config->max_size.x != 0 || config->max_size.y != 0)
		xdg_toplevel_set_max_size(out_window->wayland.xdg_toplevel, config->max_size.x, config->max_size.y);

	wl_surface_commit(out_window->wayland.surface);
	wl_display_roundtrip(desktop->wayland.display);

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
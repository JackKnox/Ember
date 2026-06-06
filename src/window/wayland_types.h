#pragma once

#include "ember/core.h"

#include "ember/window/window.h"

#include "ember/window/wayland-protocols/wayland-client.h"
#include "ember/window/wayland-protocols/xdg-shell-client.h"

typedef struct internal_wayland_window {
    struct wl_surface* surface;
    struct xdg_surface* xdg_surface;
    struct xdg_toplevel* xdg_toplevel;
    b8 should_close;

    emwin_window_events events;
} internal_wayland_window;

typedef struct internal_wayland_desktop {
    struct wl_display*    display;
    struct wl_registry*   registry;
    struct wl_compositor* compositor;
    struct wl_shm*        shm;
    struct xdg_wm_base*   xdg_wm_base;
    struct wl_registry_listener registry_listener;
    struct xdg_surface_listener surface_listener;
    struct xdg_wm_base_listener xdg_wm_base_listener;
    struct xdg_toplevel_listener xdg_toplevel_listener;
} internal_wayland_desktop;

em_result emwin_wayland_create_desktop(em_allocator* allocator, struct emwin_desktop** out_desktop);
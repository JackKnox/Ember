#pragma once

#include "ember/core.h"

#include "wayland-protocols/wayland-client.h"
#include "wayland-protocols/xdg-shell-client.h"

typedef struct emwin_wayland_window {
    struct wl_surface* surface;
    struct xdg_surface* xdg_surface;
    struct xdg_toplevel* xdg_toplevel;
} emwin_wayland_window;

typedef struct emwin_wayland_desktop {
    struct wl_display*    display;
    struct wl_registry*   registry;
    struct wl_compositor* compositor;
    struct wl_shm*        shm;
    struct xdg_wm_base*   xdg_wm_base;
    struct wl_registry_listener registry_listener;
    struct xdg_surface_listener surface_listener;
    struct xdg_wm_base_listener xdg_wm_base_listener;
} emwin_wayland_desktop;

#define EMBER_PLATFORM_WINDOW_STATE emwin_wayland_window wayland;

#define EMBER_PLATFORM_DESKTOP_STATE emwin_wayland_desktop wayland;

#define EMBER_PLATFORM_JOYSTICK_STATE u8 _pad;
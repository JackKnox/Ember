#pragma once

#include "ember/core.h"

#include "ember/gpu/ext/wayland_surface.h"

#include "vulkan_types.h"

typedef VkFlags VkWaylandSurfaceCreateFlagsKHR;

typedef struct VkWaylandSurfaceCreateInfoKHR {
    VkStructureType                 sType;
    const void*                     pNext;
    VkWaylandSurfaceCreateFlagsKHR  flags;
    struct wl_display*              display;
    struct wl_surface*              surface;
} VkWaylandSurfaceCreateInfoKHR;

typedef VkResult (*PFN_vkCreateWaylandSurfaceKHR)(VkInstance,const VkWaylandSurfaceCreateInfoKHR*,const VkAllocationCallbacks*,VkSurfaceKHR*);
typedef VkBool32 (*PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)(VkPhysicalDevice,uint32_t,struct wl_display*);

em_result vulkan_wayland_create_surface(
    emgpu_device* device,
    em_allocator* allocator,
    emgpu_wayland_surface_config* config,
    emgpu_surface* out_surface) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR =
        (PFN_vkCreateWaylandSurfaceKHR)vkGetInstanceProcAddr(context->instance, "vkCreateWaylandSurfaceKHR");
    
    out_surface->internal_data = mem_allocate(NULL, sizeof(internal_vulkan_surface), MEMORY_TAG_RENDERER);
    internal_vulkan_surface* internal_surface = (internal_vulkan_surface*)out_surface->internal_data;

    VkWaylandSurfaceCreateInfoKHR create_info = { VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR };
    create_info.display = config->display;
    create_info.surface = config->surface;

    CHECK_VKRESULT(
        vkCreateWaylandSurfaceKHR(context->instance, &create_info, context->allocator, &internal_surface->surface),
        "Failed to create native Vulkan wayland surface");

    EM_INFO("Vulkan", "Creating Vulkan surface on window: '%s'", config->debug_name);

    // Query swapchain support info.
    // --------------------------------------
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->physical_device, internal_surface->surface, &internal_surface->capabilities);

    u32 format_count = 0;

    // Swapchain format count.
    vkGetPhysicalDeviceSurfaceFormatsKHR(context->physical_device, internal_surface->surface, &format_count, NULL);
    VkSurfaceFormatKHR* formats = mem_allocate(NULL, sizeof(VkSurfaceFormatKHR) * format_count, MEMORY_TAG_TEMP);
    vkGetPhysicalDeviceSurfaceFormatsKHR(context->physical_device, internal_surface->surface, &format_count, formats);
    // --------------------------------------

    // Preferred formats;
    // TODO: This could land you on a depth/stencil format, make sure that if `prefered_format` 
    // TODO: is colour it stays as a colour format and if there isn't one error out.
    VkSurfaceFormatKHR found_format = formats[0];
    
    for (u32 i = 0; i < format_count; ++i) {
        if (formats[i].format == format_to_vulkan_type(config->preferred_format) &&
            formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            found_format = formats[i];
            break;
        }
    }

    // Special Vulkan edge case, means no limits on swapchain format.
    if (format_count == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
        found_format.format = format_to_vulkan_type(config->preferred_format);
        found_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }

    if (found_format.format != format_to_vulkan_type(config->preferred_format)) {
        EM_WARN("Vulkan", "Could not find required surface format. (See emgpu_surface_config::force_format)");

        if (config->force_format)
            return EMBER_RESULT_UNSUPPORTED_FORMAT;
    }

    out_surface->pixel_format = vulkan_format_to_engine(found_format.format);
    internal_surface->colour_space = found_format.colorSpace;

    mem_free(NULL, formats, sizeof(VkSurfaceFormatKHR) * format_count, MEMORY_TAG_TEMP);

    return vulkan_surface_recreate(device, out_surface, config->size);
}

// Is not part of extension but is required for use with Vulkan.
b8 vulkan_wayland_presentation_support(emgpu_device* device, void* user_data, VkPhysicalDevice physical_device, u32 queue_family_index) {
    vulkan_context* context = (vulkan_context*)device->internal_context;
    
    PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR vkGetPhysicalDeviceWaylandPresentationSupportKHR =
        (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)vkGetInstanceProcAddr(context->instance, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");

    return vkGetPhysicalDeviceWaylandPresentationSupportKHR(physical_device, queue_family_index, (struct wl_display*)user_data);
}
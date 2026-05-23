#include "ember/core.h"
#include "ember/gpu/ext/wayland_surface.h"

#include "vulkan/vulkan_types.h"

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

em_result vulkan_surface_create_wayland(
    emgpu_device* device,
    em_allocator* allocator,
    emgpu_wayland_surface_config* surface_config,
    emgpu_surface* out_surface) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR =
        (PFN_vkCreateWaylandSurfaceKHR)vkGetInstanceProcAddr(context->instance, "vkCreateWaylandSurfaceKHR");
    
    VkWaylandSurfaceCreateInfoKHR create_info = { VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR };
    create_info.display = surface_config->display;
    create_info.surface = surface_config->surface;

    VkSurfaceKHR surface;

    CHECK_VKRESULT(
        vkCreateWaylandSurfaceKHR(context->instance, &create_info, context->allocator, &surface),
        "Failed to create native Vulkan wayland surface");

    return EMBER_RESULT_OK;
}

// Is not part of extension but is required for use with Vulkan. It must match the function signiture to work the backend so display is void*
b8 vulkan_wayland_presentation_support(emgpu_device* device, void* display, VkPhysicalDevice physical_device, u32 queue_family_index) {
    vulkan_context* context = (vulkan_context*)device->internal_context;
    
    PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR vkGetPhysicalDeviceWaylandPresentationSupportKHR =
        (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)vkGetInstanceProcAddr(context->instance, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");

    return vkGetPhysicalDeviceWaylandPresentationSupportKHR(physical_device, queue_family_index, (struct wl_display*)display);
}
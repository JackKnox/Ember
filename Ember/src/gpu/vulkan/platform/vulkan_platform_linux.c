#include "ember/core.h"

#ifdef EM_PLATFORM_LINUX
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "ember/platform/window.h"
#include "ember/platform/internal.h"
#include "vulkan_platform.h"

VkResult vulkan_platform_create_surface(VkInstance instance, emplat_window* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* out_surface) {
    return glfwCreateWindowSurface(instance, window->glfw.handle, allocator, out_surface);
}

const char** vulkan_platform_get_required_extensions(u32* out_extension_count) {
    return glfwGetRequiredInstanceExtensions(out_extension_count);
}

b8 vulkan_platform_presentation_support(VkInstance instance, VkPhysicalDevice physical_device, u32 queue_family_index) {
    return glfwGetPhysicalDevicePresentationSupport(instance, physical_device, queue_family_index);
}

#endif
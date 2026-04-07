#pragma once

#include "ember/core.h"

#include "ember/platform/window.h"

#include <vulkan/vulkan.h>

/**
 * @brief Creates and assigns a surface to the given platform.
 *
 * @param instance Vulkan instance.
 * @param window Connected window to assign surface to.
 * @param allocator Global Vulkan allocator.
 * @param out_surface Ouputted surface handle.
 * 
 * @return Vulkan return code (VkResult)
 */
VkResult vulkan_platform_create_surface(VkInstance instance, emplat_window* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* out_surface);

/**
 * @brief Returns the Vulkan instance extensions required by the current platform.
 * 
 * @param out_extension_count Output pointer to the number of required extensions.
 * 
 * @return Outputted extension names.
 */
const char** vulkan_platform_get_required_extensions(u32* out_extension_count);

/**
 * @brief Indicates if the given device/queue family index combo supports presentation.
 */
b8 vulkan_platform_presentation_support(VkInstance instance, VkPhysicalDevice physical_device, u32 queue_family_index);
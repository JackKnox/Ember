#include "ember/core.h"
#include "vulkan_surface.h"

#include "vulkan_texture.h"

em_result vulkan_surface_create(
    emgpu_device* device,
    const emgpu_surface_config* config,
    emgpu_surface* out_surface) {
    vulkan_context* context = (vulkan_context*)device->internal_context;
    config->window->internal_renderer_state = out_surface;
    
    out_surface->internal_data = mem_allocate(NULL, sizeof(internal_vulkan_surface), MEMORY_TAG_RENDERER);
    internal_vulkan_surface* internal_surface = (internal_vulkan_surface*)out_surface->internal_data;

    EM_INFO("Vulkan", "Creating Vulkan surface on window: '%s'", config->window->title);

    CHECK_VKRESULT(
        vulkan_platform_create_surface(
            context->instance, 
            config->window, 
            context->allocator, 
            &internal_surface->surface), 
        "Failed to create internal Vulkan surface (VkSurfaceKHR)");

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

    return vulkan_surface_recreate(device, out_surface, config->window->size);
}

void vulkan_surface_destroy(
    emgpu_device* device, 
    emgpu_surface* surface) {
    vulkan_context* context = (vulkan_context*)device->internal_context;
    if (context->logical_device) vkDeviceWaitIdle(context->logical_device);

    internal_vulkan_surface* internal_surface = (internal_vulkan_surface*)surface->internal_data;
    if (!internal_surface) 
        return;

    if (internal_surface->swapchain_images) {
        for (u32 i = 0; i < surface->image_count; ++i)
            vulkan_texture_destroy(device, &internal_surface->swapchain_images[i]);
        
        mem_free(NULL, internal_surface->swapchain_images, sizeof(emgpu_texture) * surface->image_count, MEMORY_TAG_RENDERER);
    }

    if (internal_surface->swapchain)   
        vkDestroySwapchainKHR(context->logical_device, internal_surface->swapchain, context->allocator);

    if (internal_surface->surface)
        vkDestroySurfaceKHR(context->instance, internal_surface->surface, context->allocator);

    mem_free(NULL, internal_surface, sizeof(internal_vulkan_surface), MEMORY_TAG_RENDERER);
    surface->internal_data = NULL;
}

em_result vulkan_surface_recreate(
    emgpu_device* device, 
    emgpu_surface* surface, 
    uvec2 new_size) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    internal_vulkan_surface* internal_surface = (internal_vulkan_surface*)surface->internal_data;

    if (internal_surface->swapchain)
        EM_TRACE("Vulkan", "Recreating Vulkan surface to size: (%i, %i)", new_size.width, new_size.height);

    u32 queueFamilyIndices[] = {
        context->mode_queues[VULKAN_QUEUE_TYPE_GRAPHICS].family_index,
        context->mode_queues[VULKAN_QUEUE_TYPE_PRESENT].family_index };

    u32 image_count = internal_surface->capabilities.minImageCount + 1;
    if (internal_surface->capabilities.maxImageCount > 0 && image_count > internal_surface->capabilities.maxImageCount)
        image_count = internal_surface->capabilities.maxImageCount;

    // Swapchain create info 
    VkSwapchainCreateInfoKHR swapchain_create_info = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    swapchain_create_info.surface = internal_surface->surface;
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.imageFormat = format_to_vulkan_type(surface->pixel_format);
    swapchain_create_info.imageColorSpace = internal_surface->colour_space;
    swapchain_create_info.imageExtent = (VkExtent2D) { new_size.width, new_size.height };
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = internal_surface->capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchain_create_info.clipped = TRUE;
    swapchain_create_info.oldSwapchain = internal_surface->swapchain;

    u32 flags = EMBER_FORMAT_FLAGS(surface->pixel_format);
    if (flags & EMBER_FORMAT_FLAG_DEPTH || flags & EMBER_FORMAT_FLAG_STENCIL)
        swapchain_create_info.imageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    if (context->mode_queues[VULKAN_QUEUE_TYPE_GRAPHICS].family_index != 
        context->mode_queues[VULKAN_QUEUE_TYPE_PRESENT].family_index) {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.pQueueFamilyIndices = queueFamilyIndices;
        swapchain_create_info.queueFamilyIndexCount = 2;
    }

    CHECK_VKRESULT(
        vkCreateSwapchainKHR(
            context->logical_device, 
            &swapchain_create_info, 
            context->allocator,
            &internal_surface->swapchain), 
        "Failed to create internal Vulkan swapchain");
    
    CHECK_VKRESULT(
        vkGetSwapchainImagesKHR(
            context->logical_device, 
            internal_surface->swapchain,
            &surface->image_count,
            NULL), 
        "Failed to retrieve Vulkan swapchain images");
    
    VkImage* images = (VkImage*)mem_allocate(NULL, sizeof(VkImage) * surface->image_count, MEMORY_TAG_TEMP);
    vkGetSwapchainImagesKHR(context->logical_device, internal_surface->swapchain, &surface->image_count, images);

    if (!internal_surface->swapchain_images)
        internal_surface->swapchain_images = (emgpu_texture*)mem_allocate(NULL, sizeof(emgpu_texture) * surface->image_count, MEMORY_TAG_RENDERER);
        
    for (u32 i = 0; i < surface->image_count; ++i) {
        vulkan_texture_destroy(device, &internal_surface->swapchain_images[i]);

        vulkan_texture_ext_config ext_config = {};
        ext_config.existing_image = images[i];
        //ext_config.create_memory = FALSE, ext_config.ownes_provided_image = FALSE;

        emgpu_texture_config swapchain_texture_config = emgpu_texture_default();
        swapchain_texture_config.size = new_size;
        swapchain_texture_config.image_format = surface->pixel_format;
        swapchain_texture_config.usage = EMBER_TEXTURE_USAGE_ATTACHMENT_DST;

        em_result result = vulkan_texture_create(
            device, &swapchain_texture_config, &internal_surface->swapchain_images[i]);
        if (result != EMBER_RESULT_OK) return result;
    }

    // Acquire first image before submitting frames
    // --------------------------------------
    VkFence temp_fence = VK_NULL_HANDLE;
    VkFenceCreateInfo fence_create_info = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };

    CHECK_VKRESULT(
        vkCreateFence(context->logical_device, &fence_create_info, context->allocator, &temp_fence),
        "Failed to temporary Vulkan surface fence");

    CHECK_VKRESULT(
        vulkan_surface_accquire(device, surface, UINT64_MAX, VK_NULL_HANDLE, temp_fence),
        "Failed to acquire first Vulkan swapchain image");

    CHECK_VKRESULT(
        vkWaitForFences(context->logical_device, 1, &temp_fence, TRUE, UINT64_MAX),
        "Failed to acquire first Vulkan swapchain image");

    vkDestroyFence(context->logical_device, temp_fence, context->allocator);
    // --------------------------------------

    mem_free(NULL, images, sizeof(VkImage) * surface->image_count, MEMORY_TAG_TEMP);
    return EMBER_RESULT_OK;
}

VkResult vulkan_surface_accquire(
    emgpu_device* device,
    emgpu_surface* surface,
    u64 timeout,
    VkSemaphore signal_semaphore, VkFence signal_fence) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    internal_vulkan_surface* internal_surface = (internal_vulkan_surface*)surface->internal_data;
    
    return vkAcquireNextImageKHR(
        context->logical_device, 
        internal_surface->swapchain, 
        timeout, 
        signal_semaphore, signal_fence, 
        &internal_surface->image_index);
}

VkResult vulkan_surface_present(
    emgpu_device* device, 
    emgpu_surface* surface, 
    vulkan_queue* present_queue, 
    u32 wait_semaphore_count, VkSemaphore* wait_semaphores) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    internal_vulkan_surface* internal_surface = (internal_vulkan_surface*)surface->internal_data;

}
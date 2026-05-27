#include "ember/core.h"
#include "vulkan_surface.h"

#include "vulkan_texture.h"

em_result vulkan_surface_recreate(
    emgpu_device* device, 
    emgpu_surface* surface, 
    uvec2 new_size) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    internal_vulkan_surface* internal_surface = (internal_vulkan_surface*)surface->internal_data;

    if (internal_surface->swapchain)
        EM_TRACE("Vulkan", "Recreating Vulkan surface to size: (%i, %i)", new_size.width, new_size.height);

    u32 queueFamilyIndices[] = {
        context->mode_queues[VULKAN_QUEUE_TYPE_RASTER].family_index,
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

    if (context->mode_queues[VULKAN_QUEUE_TYPE_RASTER].family_index != 
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
        vulkan_texture_destroy(device, NULL, &internal_surface->swapchain_images[i]);

        vulkan_texture_ext_config ext_config = {};
        ext_config.existing_image = images[i];
        //ext_config.create_memory = FALSE, ext_config.ownes_provided_image = FALSE;

        emgpu_texture_config swapchain_texture_config = emgpu_texture_default();
        swapchain_texture_config.size = new_size;
        swapchain_texture_config.image_format = surface->pixel_format;
        swapchain_texture_config.usage = EMBER_TEXTURE_USAGE_ATTACHMENT_DST;
        swapchain_texture_config.api_next = &ext_config;

        em_result result = vulkan_texture_create(
            device, NULL, &swapchain_texture_config, &internal_surface->swapchain_images[i]);
        if (result != EMBER_RESULT_OK) return result;
    }

    internal_surface->image_available_semaphores = mem_allocate(NULL, sizeof(VkSemaphore) * context->frames_in_flight, MEMORY_TAG_RENDERER);
    internal_surface->render_complete_semaphores = mem_allocate(NULL, sizeof(VkSemaphore) * context->frames_in_flight, MEMORY_TAG_RENDERER);

    for (u32 i = 0; i < context->frames_in_flight; ++i) {
        VkSemaphoreCreateInfo create_info = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        
        CHECK_VKRESULT(
            vkCreateSemaphore(
                context->logical_device, 
                &create_info, 
                context->allocator,
                &internal_surface->image_available_semaphores[i]),
            "Failed to image available semaphore in Vulkan surface");
        
        CHECK_VKRESULT(
            vkCreateSemaphore(
                context->logical_device, 
                &create_info, 
                context->allocator,
                &internal_surface->render_complete_semaphores[i]),
            "Failed to image available semaphore in Vulkan surface");
    }

    mem_free(NULL, images, sizeof(VkImage) * surface->image_count, MEMORY_TAG_TEMP);
    return EMBER_RESULT_OK;
}

void vulkan_surface_destroy(
    emgpu_device* device,
    em_allocator* allocator,
    emgpu_surface* surface) {
    vulkan_context* context = (vulkan_context*)device->internal_context;
    if (context->logical_device) vkDeviceWaitIdle(context->logical_device);

    internal_vulkan_surface* internal_surface = (internal_vulkan_surface*)surface->internal_data;
    if (!internal_surface) 
        return;

    // Image available semaphores
    if (internal_surface->image_available_semaphores) {
        for (u32 i = 0; i < context->frames_in_flight; ++i) {
            if (internal_surface->image_available_semaphores[i]) {
                vkDestroySemaphore(
                    context->logical_device,
                    internal_surface->image_available_semaphores[i],
                    context->allocator);
            }
        }

        mem_free(NULL, internal_surface->image_available_semaphores,
            sizeof(VkSemaphore) * context->frames_in_flight,
            MEMORY_TAG_RENDERER);
    }

    // Render complete semaphores
    if (internal_surface->render_complete_semaphores) {
        for (u32 i = 0; i < context->frames_in_flight; ++i) {
            if (internal_surface->render_complete_semaphores[i]) {
                vkDestroySemaphore(
                    context->logical_device,
                    internal_surface->render_complete_semaphores[i],
                    context->allocator);
            }
        }

        mem_free(NULL, internal_surface->render_complete_semaphores,
            sizeof(VkSemaphore) * context->frames_in_flight,
            MEMORY_TAG_RENDERER);
    }
    
    if (internal_surface->swapchain_images) {
        for (u32 i = 0; i < surface->image_count; ++i)
            vulkan_texture_destroy(device, NULL, &internal_surface->swapchain_images[i]);
        
        mem_free(NULL, internal_surface->swapchain_images, sizeof(emgpu_texture) * surface->image_count, MEMORY_TAG_RENDERER);
    }

    if (internal_surface->swapchain)   
        vkDestroySwapchainKHR(context->logical_device, internal_surface->swapchain, context->allocator);

    if (internal_surface->surface)
        vkDestroySurfaceKHR(context->instance, internal_surface->surface, context->allocator);

    mem_free(NULL, internal_surface, sizeof(internal_vulkan_surface), MEMORY_TAG_RENDERER);
    surface->internal_data = NULL;
}
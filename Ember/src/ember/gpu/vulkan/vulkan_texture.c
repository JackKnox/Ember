#include "defines.h"
#include "vulkan_texture.h"

#include "vulkan_renderbuffer.h"
#include "vulkan_command_buffer.h"

VkImageUsageFlags get_vulkan_texture_usage(
    vulkan_context* context, 
    emgpu_texture_config* config) {

	VkImageUsageFlags image_usage = 0;
    if (config->usage & EMBER_TEXTURE_USAGE_SAMPLED) image_usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (config->usage & EMBER_TEXTURE_USAGE_STORAGE) image_usage |= VK_IMAGE_USAGE_STORAGE_BIT;
	if (context->config.enabled_modes & EMBER_DEVICE_MODE_TRANSFER) image_usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    return image_usage;
}

b8 vulkan_texture_create(
    emgpu_device* device,
    emgpu_texture_config* config,
    emgpu_texture* out_texture) {
    EM_ASSERT(device != NULL && out_texture != NULL && "Invalid arguments passed to vulkan_texture_create");
    vulkan_context* context = (vulkan_context*)device->internal_context;

    VkImage image; 

#if EM_ENABLE_VALIDATION
    if (config->size.width <= 0 || config->size.height <= 0) {
        EM_ERROR("vulkan_texture_create(): Texture size must be greater than zero: Size = (%u, %u)", config->size.width, config->size.height);
        return FALSE;
    }

    if (!context->config.sampler_anisotropy && config->max_anisotropy > 0.0f) {
        EM_ERROR("vulkan_texture_create(): Attempting set anisotropy without enabling sampler anisotropy.");
        return FALSE;
    }

    if (config->max_anisotropy > device->capabilities.max_anisotropy) {
        EM_ERROR("vulkan_texture_create(): Attempting set anisotropy higher than renderer capabilities.");
        return FALSE;
    }
#endif

    VkImageCreateInfo image_create_info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = config->size.width;
    image_create_info.extent.height = config->size.height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.format = format_to_vulkan_type(out_texture->image_format);
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = get_vulkan_texture_usage(context, config);
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    CHECK_VKRESULT(
        vkCreateImage(
            context->device.logical_device, 
            &image_create_info, 
            context->allocator, 
            &image), 
        "Failed to create internal Vulkan image");

    CHECK_VKRESULT(
        vulkan_texture_create_internal(
            context, 
            config, 
            image, 
            TRUE, TRUE, 
            out_texture),
        "Failed to create Vulkan image");

    if (config->usage & EMBER_TEXTURE_USAGE_SAMPLED) {
        internal_vulkan_texture* internal_texture = (internal_vulkan_texture*)out_texture->internal_data;

        VkSamplerCreateInfo sampler_info = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
        sampler_info.magFilter = sampler_info.minFilter 
            = filter_to_vulkan_type(config->filter_type);

        sampler_info.addressModeU = sampler_info.addressModeV = sampler_info.addressModeW 
            = address_mode_to_vulkan_type(config->address_mode);

        sampler_info.anisotropyEnable = (config->max_anisotropy > 0.0f);
        sampler_info.maxAnisotropy = config->max_anisotropy;
        
        sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        sampler_info.unnormalizedCoordinates = VK_FALSE;
        sampler_info.compareEnable = VK_FALSE;
        sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
        sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler_info.mipLodBias = 0.0f;
        sampler_info.minLod = 0.0f;
        sampler_info.maxLod = 0.0f;

        CHECK_VKRESULT(
            vkCreateSampler(
                context->device.logical_device, 
                &sampler_info,
                context->allocator, 
                &internal_texture->sampler),
            "Failed to create internal Vulkan sampler");
    }
    
    return TRUE;
}

VkResult vulkan_texture_create_internal(
    vulkan_context* context,
    emgpu_texture_config* config,
    VkImage existing_image,
    b8 ownes_provided_image,
    b8 create_memory,
    emgpu_texture* out_texture) {
    EM_ASSERT(context != NULL && config != NULL && existing_image != VK_NULL_HANDLE && out_texture != NULL && "Invalid arguments passed to vulkan_texture_create_internal");

    out_texture->internal_data = (internal_vulkan_texture*)ballocate(sizeof(internal_vulkan_texture), MEMORY_TAG_RENDERER);
    internal_vulkan_texture* internal_texture = (internal_vulkan_texture*)out_texture->internal_data;

    internal_texture->ownes_vk_image = ownes_provided_image;

    out_texture->image_format = config->image_format;
    out_texture->size = config->size;
    
    if (create_memory) {
        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(context->device.logical_device, internal_texture->handle, &memory_requirements);

        i32 memory_index = find_memory_index(context, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (memory_index == -1) {
            EM_ERROR("Required memory type not found. Image not valid.");
            return VK_ERROR_OUT_OF_DEVICE_MEMORY;
        }

        // Allocate memory
        VkMemoryAllocateInfo memory_allocate_info = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = memory_index;

        VkResult result = vkAllocateMemory(context->device.logical_device, &memory_allocate_info, context->allocator, &internal_texture->memory);
        if (!vulkan_result_is_success(result)) return result;

        // Bind the memory
        result = vkBindImageMemory(context->device.logical_device, internal_texture->handle, internal_texture->memory, 0);
        if (!vulkan_result_is_success(result)) return result;
    }

    VkImageViewCreateInfo view_create_info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    view_create_info.image = existing_image;
    view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_create_info.format = format_to_vulkan_type(config->image_format);
    
    // TODO: Make configurable
    view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    view_create_info.subresourceRange.baseMipLevel = 0;
    view_create_info.subresourceRange.levelCount = 1;
    view_create_info.subresourceRange.baseArrayLayer = 0;
    view_create_info.subresourceRange.layerCount = 1;

    return vkCreateImageView(
        context->device.logical_device,
        &view_create_info,
        context->allocator,
        &internal_texture->view);
}

void vulkan_texture_transition_layout(
    vulkan_context* context,
    vulkan_command_buffer* command_buffer,
    emgpu_texture* texture,
    VkImageLayout new_layout) {
    internal_vulkan_texture* internal_texture = (internal_vulkan_texture*)texture->internal_data;
    if (new_layout == internal_texture->layout)
        return;

    // TODO: Oh boy this needs some work..
    VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    barrier.oldLayout = internal_texture->layout;
    barrier.newLayout = new_layout;
    barrier.image = internal_texture->handle;
    
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    VkPipelineStageFlags src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

    vkCmdPipelineBarrier(command_buffer->handle,
        src_stage, 
        dst_stage,
        0,
        0, NULL,
        0, NULL,
        1, &barrier);

    internal_texture->layout = new_layout;
}

b8 vulkan_texture_upload_data(
    emgpu_device* device, 
    emgpu_texture* texture, 
    const void* data, 
    uvec2 offset, 
    uvec2 region) {
    EM_ASSERT(device != NULL && texture != NULL && data != NULL && "Invalid arguments passed to vulkan_texture_upload_data");
    vulkan_context* context = (vulkan_context*)device->internal_context;

    internal_vulkan_texture* internal_texture = (internal_vulkan_texture*)texture->internal_data;

#if EM_ENABLE_VALIDATION
	if (!(context->config.enabled_modes & EMBER_DEVICE_MODE_TRANSFER)) {
		EM_ERROR("vulkan_texture_create(): Attempting to upload to texture without enabling transfer mode.");
		return FALSE;
	}

    if (offset.x + region.width > texture->size.width || offset.y + region.height > texture->size.height) {
        EM_ERROR("vulkan_texture_create(): Region size must be within overral texture size: Region = ((%u, %u) -> (%u, %u))", offset.x, offset.y, offset.x + region.width, offset.y + region.height);
        return FALSE;
    }
#endif

    emgpu_renderbuffer_config staging_buffer_config = emgpu_renderbuffer_default();
	staging_buffer_config.usage = EMBER_RENDERBUFFER_USAGE_CPU_VISIBLE;
    staging_buffer_config.buffer_size = emgpu_texture_get_size_in_bytes(texture);

    emgpu_renderbuffer staging_buffer = {};
    if (!vulkan_renderbuffer_create(device, &staging_buffer_config, &staging_buffer) || 
		!vulkan_renderbuffer_map_data(device, &staging_buffer, data, 0, staging_buffer.buffer_size)) {
        EM_ERROR("vulkan_texture_create(): Failed to create staging buffer for uploading data.");
        return FALSE;
    }

    VkImageLayout old_layout = internal_texture->layout;

	vulkan_command_buffer command_buffer;
	vulkan_command_buffer_allocate_and_begin_single_use(context, &context->device.mode_queues[VULKAN_QUEUE_TYPE_TRANSFER], &command_buffer);
    {
        vulkan_texture_transition_layout(context, &command_buffer, texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        internal_vulkan_renderbuffer* internal_staging_buffer = (internal_vulkan_renderbuffer*)staging_buffer.internal_data;

        VkBufferImageCopy image_upload = {};
        image_upload.bufferOffset = 0; // TODO: offset 

        image_upload.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_upload.imageSubresource.mipLevel = 0;
        image_upload.imageSubresource.baseArrayLayer = 0;
        image_upload.imageSubresource.layerCount = 1;

        image_upload.imageExtent.width = region.width;
        image_upload.imageExtent.height = region.height;
        image_upload.imageExtent.depth = 1;

        vkCmdCopyBufferToImage(command_buffer.handle, internal_staging_buffer->handle, internal_texture->handle, internal_texture->layout, 1, &image_upload);

        // TODO: Implicit image transitions.
        vulkan_texture_transition_layout(context, &command_buffer, texture, old_layout);

        CHECK_VKRESULT(
            vulkan_command_buffer_end_single_use(
                context,
                &command_buffer),
            "Failed to transfer Vulkan renderbuffer to GPU");
    }

    vulkan_renderbuffer_destroy(device, &staging_buffer);
    return TRUE;
}

void vulkan_texture_destroy(
    emgpu_device* device, 
    emgpu_texture* texture) {
    EM_ASSERT(device != NULL && texture != NULL && "Invalid arguments passed to vulkan_texture_destroy");
    vulkan_context* context = (vulkan_context*)device->internal_context;
	if (context->device.logical_device) vkDeviceWaitIdle(context->device.logical_device);

    internal_vulkan_texture* internal_texture = (internal_vulkan_texture*)texture->internal_data;
    
    if (internal_texture != NULL) {
        if (internal_texture->sampler)
            vkDestroySampler(context->device.logical_device, internal_texture->sampler, context->allocator);

        if (internal_texture->view)
            vkDestroyImageView(context->device.logical_device, internal_texture->view, context->allocator);
        
        if (internal_texture->memory)
            vkFreeMemory(context->device.logical_device, internal_texture->memory, context->allocator);

        if (internal_texture->ownes_vk_image && internal_texture->handle)
            vkDestroyImage(context->device.logical_device, internal_texture->handle, context->allocator);

        bfree(internal_texture, sizeof(internal_vulkan_texture), MEMORY_TAG_RENDERER);
    }
}
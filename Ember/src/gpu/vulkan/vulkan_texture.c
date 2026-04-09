#include "ember/core.h"
#include "vulkan_texture.h"

#include "vulkan_buffer.h"
#include "vulkan_command_buffer.h"

em_result vulkan_texture_create(
    emgpu_device* device, 
    const emgpu_texture_config* config, 
    emgpu_texture* out_texture) {
    vulkan_context* context = (vulkan_context*)device->internal_context;
    
    out_texture->internal_data = (internal_vulkan_texture*)mem_allocate(sizeof(internal_vulkan_texture), MEMORY_TAG_RENDERER);
    internal_vulkan_texture* internal_texture = (internal_vulkan_texture*)out_texture->internal_data;

    out_texture->size = config->size;
    out_texture->image_format = config->image_format;
    internal_texture->usage = config->usage;
    internal_texture->ownes_image = TRUE;

    if (config->api_next != NULL) {
        vulkan_texture_ext_config* ext_config = (vulkan_texture_ext_config*)config->api_next;
        internal_texture->handle = ext_config->existing_image;
        internal_texture->ownes_image = ext_config->ownes_provided_image;
    }

    if (!internal_texture->handle && !internal_texture->ownes_image) {
        EM_ERROR("Vulkan", "Doesn't own VkImage handle but didn't provide a valid handle");
        return EMBER_RESULT_INVALID_VALUE;
    }

    if (config->usage & EMBER_TEXTURE_USAGE_SAMPLED) {
        VkSamplerCreateInfo sampler_create_info = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
        sampler_create_info.magFilter = sampler_create_info.minFilter = filter_to_vulkan_type(config->filter_type);
        sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler_create_info.addressModeU = sampler_create_info.addressModeV = sampler_create_info.addressModeW = address_mode_to_vulkan_type(config->address_mode);
        
        sampler_create_info.anisotropyEnable = config->max_anisotropy > 0.0f;
        sampler_create_info.maxAnisotropy = config->max_anisotropy;
        sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
        sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    
        CHECK_VKRESULT(
            vkCreateSampler(
                context->logical_device, 
                &sampler_create_info,
                context->allocator, 
                &internal_texture->sampler), 
            "Failed to create texture Vulkan sampler");
    }

    return vulkan_texture_recreate(device, out_texture, config->size);
}

em_result vulkan_texture_recreate(
    emgpu_device* device, 
    emgpu_texture* texture,
    uvec2 new_size) {
    vulkan_context* context = (vulkan_context*)device->internal_context;
    
    internal_vulkan_texture* internal_texture = (internal_vulkan_texture*)texture->internal_data;

    texture->size = new_size;

    if (!internal_texture->handle && internal_texture->ownes_image) {
        // Describe the VkImage (GPU texture resource)
        // TODO: 3D textures, mipmaps, sample count, sharing mode.
        VkImageCreateInfo image_create_info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = format_to_vulkan_type(texture->image_format);
        image_create_info.extent.width = texture->size.width;
        image_create_info.extent.height = texture->size.height;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (internal_texture->usage & EMBER_TEXTURE_USAGE_SAMPLED) image_create_info.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (internal_texture->usage & EMBER_TEXTURE_USAGE_STORAGE) image_create_info.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
        if (internal_texture->usage & EMBER_TEXTURE_USAGE_TRANSFER_SRC)  image_create_info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if (context->config.enabled_modes & EMBER_DEVICE_MODE_TRANSFER) image_create_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        if (internal_texture->usage & EMBER_TEXTURE_USAGE_ATTACHMENT_DST) {
            u32 flags = EMBER_FORMAT_FLAGS(texture->image_format);

            if (flags & EMBER_FORMAT_FLAG_DEPTH || flags & EMBER_FORMAT_FLAG_STENCIL)
                image_create_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            else 
                image_create_info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }

        CHECK_VKRESULT(
            vkCreateImage(
                context->logical_device, 
                &image_create_info, 
                context->allocator, 
                &internal_texture->handle),
            "Failed to create Vulkan image for texture");

        // Allocate memory for space in VkImage, not needed for swapchain images.
        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(context->logical_device, internal_texture->handle, &memory_requirements);

        // Find index of memory compatible for provided image.
        i32 memory_index = find_memory_index(context, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (memory_index == -1) {
            EM_ERROR("Vulkan", "Failed to find required memory type for texture");
            return EMBER_RESULT_OUT_OF_MEMORY_GPU;
        }

        VkMemoryAllocateInfo memory_allocate_info = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = memory_index;

        CHECK_VKRESULT(
            vkAllocateMemory(context->logical_device, &memory_allocate_info, context->allocator, &internal_texture->memory),
            "Failed to allocate Vulkan texture memory");

        CHECK_VKRESULT(
            vkBindImageMemory(context->logical_device, internal_texture->handle, internal_texture->memory, 0),
            "Failed to bind internal Vulkan memory to texture");
    }
    
    VkImageViewCreateInfo view_create_info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    view_create_info.image = internal_texture->handle;
    view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_create_info.format = format_to_vulkan_type(texture->image_format);

    // TODO: Make configurable
    view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    view_create_info.subresourceRange.baseMipLevel = 0;
    view_create_info.subresourceRange.levelCount = 1;
    view_create_info.subresourceRange.baseArrayLayer = 0;
    view_create_info.subresourceRange.layerCount = 1;

    CHECK_VKRESULT(
        vkCreateImageView(context->logical_device, &view_create_info, context->allocator, &internal_texture->view),
        "Failed to create Vulkan texture image view");

    return EMBER_RESULT_OK;
}

em_result vulkan_texture_upload(
    emgpu_device* device, 
    emgpu_texture* texture, 
    const void* data, 
    uvec2 start_offset, uvec2 region) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    internal_vulkan_texture* internal_texture = (internal_vulkan_texture*)texture->internal_data;

    emgpu_buffer_config staging_buffer_config = emgpu_buffer_default();
    staging_buffer_config.buffer_size = region.width * region.height * EMBER_FORMAT_SIZE(texture->image_format);
    staging_buffer_config.usage = EMBER_BUFFER_USAGE_CPU_VISIBLE;

    emgpu_buffer staging_buffer = {};
    em_result result = vulkan_buffer_create(device, &staging_buffer_config, &staging_buffer);
    if (result != EMBER_RESULT_OK) {
        EM_ERROR("Vulkan", "Failed to create staging buffer for texture upload.");
        return result;
    }

    result = vulkan_buffer_upload(device, &staging_buffer, data, 0, staging_buffer_config.buffer_size);
    if (result != EMBER_RESULT_OK) {
        EM_ERROR("Vulkan", "Failed to upload data to texture staging buffer.");
        return result;
    }

    vulkan_command_buffer command_buffer;
	CHECK_VKRESULT(
        vulkan_command_buffer_allocate_and_begin_single_use(
            context, 
            &context->mode_queues[VULKAN_QUEUE_TYPE_TRANSFER],
            &command_buffer), 
        "Failed to allocate internal Vulkan single-use command buffer to transfer texture data");
    {
        internal_vulkan_buffer* internal_staging_buffer = (internal_vulkan_buffer*)staging_buffer.internal_data;

        // vulkan_texture_transition_layout(context, &command_buffer, texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        
        VkBufferImageCopy image_upload = {};
        image_upload.imageOffset.x = start_offset.width;
        image_upload.imageOffset.y = start_offset.height;
        image_upload.imageOffset.z = 0;

        image_upload.imageExtent.width = region.width;
        image_upload.imageExtent.height = region.height;
        image_upload.imageExtent.depth = 1;

        image_upload.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_upload.imageSubresource.mipLevel = 0;
        image_upload.imageSubresource.baseArrayLayer = 0;
        image_upload.imageSubresource.layerCount = 1;

        vkCmdCopyBufferToImage(command_buffer.handles[command_buffer.curr_buffer], internal_staging_buffer->handle, internal_texture->handle, internal_texture->curr_layout, 1, &image_upload);

        CHECK_VKRESULT(
            vulkan_command_buffer_end_single_use(
                context,
                &command_buffer),
            "Failed to transfer Vulkan texture data to GPU");
    }

    vulkan_buffer_destroy(device, &staging_buffer);
    return EMBER_RESULT_OK;
}

void vulkan_texture_destroy(
    emgpu_device* device, 
    emgpu_texture* texture) {

}
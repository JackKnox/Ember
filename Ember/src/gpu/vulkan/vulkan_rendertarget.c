#include "ember/core.h"
#include "vulkan_rendertarget.h"

#include "vulkan_texture.h"

em_result vulkan_rendertarget_create(
    emgpu_device* device, 
    const emgpu_rendertarget_config* config,
    emgpu_rendertarget* out_rendertarget) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    out_rendertarget->internal_data = (internal_vulkan_rendertarget*)mem_allocate(sizeof(internal_vulkan_rendertarget), MEMORY_TAG_RENDERER);
    internal_vulkan_rendertarget* internal_rendertarget = (internal_vulkan_rendertarget*)out_rendertarget->internal_data;

    out_rendertarget->attachment_count = config->attachment_count;
    out_rendertarget->size = config->size;
    internal_rendertarget->frames_in_flight = context->config.frames_in_flight;

    internal_rendertarget->framebuffers = mem_allocate(
        sizeof(VkFramebuffer) * internal_rendertarget->frames_in_flight, 
        MEMORY_TAG_RENDERER);
    
    internal_rendertarget->owned_textures = mem_allocate(
        sizeof(emgpu_texture) * internal_rendertarget->frames_in_flight * out_rendertarget->attachment_count, 
        MEMORY_TAG_RENDERER);

    for (u32 i = 0; i < internal_rendertarget->frames_in_flight; ++i) {
        for (u32 j = 0; j < out_rendertarget->attachment_count; ++j) {
            emgpu_texture* texture = &internal_rendertarget->owned_textures[j + i * out_rendertarget->attachment_count];
            
            if (config->existing_textures != NULL)
                *texture = config->existing_textures[i + (j - 1) * internal_rendertarget->frames_in_flight];
            
            if (!texture->internal_data) {
                emgpu_texture_config attachted_config = emgpu_texture_default();
                attachted_config.image_format = config->attachments[j].format;
                attachted_config.usage = EMBER_TEXTURE_USAGE_ATTACHMENT_DST;
                attachted_config.size = out_rendertarget->size;

                em_result result = vulkan_texture_create(device, &attachted_config, texture);
                if (result != EMBER_RESULT_OK) {
                    EM_ERROR("Vulkan", "Failed to recreate textures owned by rendertarget when resizing");
                    return result;
                }
            }
        }
    }

    VkAttachmentReference* colour_attachments  = NULL;
    VkAttachmentDescription* attachment_descs = darray_reserve(VkAttachmentDescription, out_rendertarget->attachment_count, MEMORY_TAG_RENDERER);

    for (u32 i = 0; i < out_rendertarget->attachment_count; ++i) {
        const emgpu_attachment_config* attachment = &config->attachments[i];

        VkAttachmentDescription* desc = darray_push_empty(attachment_descs);
        desc->format = format_to_vulkan_type(attachment->format);
        desc->samples = VK_SAMPLE_COUNT_1_BIT;
        desc->loadOp = load_op_to_vulkan_type(attachment->load_op);
        desc->storeOp = store_op_to_vulkan_type(attachment->store_op);
        desc->stencilLoadOp = load_op_to_vulkan_type(attachment->stencil_load_op);
        desc->stencilStoreOp = store_op_to_vulkan_type(attachment->stencil_store_op);
        desc->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        desc->finalLayout = attachment_type_to_image_layout(attachment->type);

        switch (attachment->type) {
            case EMBER_ATTACHMENT_TYPE_PRESENT: // TODO: Remove this as all surfaces' may not be colour.
            case EMBER_ATTACHMENT_TYPE_COLOUR:
                if (!colour_attachments)
                    colour_attachments = darray_create(VkAttachmentReference, MEMORY_TAG_RENDERER);

                VkAttachmentReference* reference = darray_push_empty(colour_attachments);
                reference->attachment = i;
                reference->layout     = desc->finalLayout;
                break;
            
            default:
                EM_ASSERT(FALSE && "Unsupported attachment type in rendertarget");
                continue;
        }
    }

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = colour_attachments ? darray_length(colour_attachments) : 0;
    subpass.pColorAttachments    = colour_attachments;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    VkRenderPassCreateInfo pass_create_info = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    pass_create_info.attachmentCount = darray_length(attachment_descs);
    pass_create_info.pAttachments    = attachment_descs;
    pass_create_info.subpassCount    = 1;
    pass_create_info.pSubpasses      = &subpass;
    pass_create_info.dependencyCount = 1;
    pass_create_info.pDependencies   = &dependency;

    CHECK_VKRESULT(
        vkCreateRenderPass(context->logical_device, &pass_create_info, context->allocator, &internal_rendertarget->handle),
        "Failed to create Vulkan renderpass when creating rendertarget");

    return vulkan_rendertarget_resize(device, out_rendertarget, out_rendertarget->size);
}

em_result vulkan_rendertarget_create_present(
    emgpu_device* device, 
    const emgpu_present_target_config* config, 
    emgpu_rendertarget* out_rendertarget) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    out_rendertarget->internal_data = (internal_vulkan_rendertarget*)mem_allocate(sizeof(internal_vulkan_rendertarget), MEMORY_TAG_RENDERER);
    internal_vulkan_rendertarget* internal_rendertarget = (internal_vulkan_rendertarget*)out_rendertarget->internal_data;

    out_rendertarget->attachment_count = config->attachment_count;
    out_rendertarget->size = (uvec2) { 640, 640 };
    out_rendertarget->is_present = TRUE;
    internal_rendertarget->frames_in_flight = context->config.frames_in_flight;
    internal_rendertarget->surface = config->surface;

    internal_rendertarget->framebuffers = mem_allocate(
        sizeof(VkFramebuffer) * internal_rendertarget->frames_in_flight, 
        MEMORY_TAG_RENDERER);

    internal_rendertarget->owned_textures = mem_allocate(
        sizeof(emgpu_texture) * internal_rendertarget->frames_in_flight * out_rendertarget->attachment_count, 
        MEMORY_TAG_RENDERER);

    for (u32 i = 0; i < internal_rendertarget->frames_in_flight; ++i) {
        for (u32 j = 0; j < out_rendertarget->attachment_count; ++j) {
            emgpu_texture* texture = &internal_rendertarget->owned_textures[j + i * out_rendertarget->attachment_count];
            
            if (config->attachments[j].type == EMBER_ATTACHMENT_TYPE_PRESENT) {
                internal_vulkan_surface* internal_surface = (internal_vulkan_surface*)config->surface->internal_data;
                *texture = internal_surface->swapchain_images[i];
            }
            else if (config->existing_textures != NULL) {
                *texture = config->existing_textures[i + (j - 1) * internal_rendertarget->frames_in_flight];
            }
            
            if (!texture->internal_data) {
                emgpu_texture_config attachted_config = emgpu_texture_default();
                attachted_config.image_format = config->attachments[j].format;
                attachted_config.usage = EMBER_TEXTURE_USAGE_ATTACHMENT_DST;
                attachted_config.size = out_rendertarget->size;

                em_result result = vulkan_texture_create(device, &attachted_config, texture);
                if (result != EMBER_RESULT_OK) {
                    EM_ERROR("Vulkan", "Failed to recreate textures owned by rendertarget when resizing");
                    return result;
                }
            }
        }
    }

    VkAttachmentReference* colour_attachments  = NULL;
    VkAttachmentDescription* attachment_descs = darray_reserve(VkAttachmentDescription, out_rendertarget->attachment_count, MEMORY_TAG_RENDERER);

    for (u32 i = 0; i < out_rendertarget->attachment_count; ++i) {
        const emgpu_attachment_config* attachment = &config->attachments[i];

        VkAttachmentDescription* desc = darray_push_empty(attachment_descs);
        desc->format = format_to_vulkan_type(attachment->format);
        desc->samples = VK_SAMPLE_COUNT_1_BIT;
        desc->loadOp = load_op_to_vulkan_type(attachment->load_op);
        desc->storeOp = store_op_to_vulkan_type(attachment->store_op);
        desc->stencilLoadOp = load_op_to_vulkan_type(attachment->stencil_load_op);
        desc->stencilStoreOp = store_op_to_vulkan_type(attachment->stencil_store_op);
        desc->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        desc->finalLayout = attachment_type_to_image_layout(attachment->type);

        switch (attachment->type) {
            case EMBER_ATTACHMENT_TYPE_PRESENT: // TODO: Remove this as all surfaces' may not be colour.
            case EMBER_ATTACHMENT_TYPE_COLOUR:
                if (!colour_attachments)
                    colour_attachments = darray_create(VkAttachmentReference, MEMORY_TAG_RENDERER);

                VkAttachmentReference* reference = darray_push_empty(colour_attachments);
                reference->attachment = i;
                reference->layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                break;
            
            default:
                EM_ASSERT(FALSE && "Unsupported attachment type in rendertarget");
                continue;
        }
    }

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = colour_attachments ? darray_length(colour_attachments) : 0;
    subpass.pColorAttachments    = colour_attachments;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    VkRenderPassCreateInfo pass_create_info = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    pass_create_info.attachmentCount = darray_length(attachment_descs);
    pass_create_info.pAttachments    = attachment_descs;
    pass_create_info.subpassCount    = 1;
    pass_create_info.pSubpasses      = &subpass;
    pass_create_info.dependencyCount = 1;
    pass_create_info.pDependencies   = &dependency;

    CHECK_VKRESULT(
        vkCreateRenderPass(context->logical_device, &pass_create_info, context->allocator, &internal_rendertarget->handle),
        "Failed to create Vulkan renderpass when creating rendertarget");

    return vulkan_rendertarget_resize(device, out_rendertarget, out_rendertarget->size);
}

em_result vulkan_rendertarget_resize(
    emgpu_device* device, 
    emgpu_rendertarget* rendertarget, 
    uvec2 new_size) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    internal_vulkan_rendertarget* internal_rendertarget = (internal_vulkan_rendertarget*)rendertarget->internal_data;

    VkImageView* framebuffer_views = darray_reserve(VkImageView, rendertarget->attachment_count, MEMORY_TAG_RENDERER);

    for (u32 i = 0; i < internal_rendertarget->frames_in_flight; ++i) {
        darray_clear(framebuffer_views);

        for (u32 j = 0; j < rendertarget->attachment_count; ++j) {
            emgpu_texture* texture = &internal_rendertarget->owned_textures[j + i * rendertarget->attachment_count];
            
            em_result result = vulkan_texture_recreate(device, texture, new_size);
            if (result != EMBER_RESULT_OK) {
                EM_ERROR("Vulkan", "Failed to recreate textures owned by rendertarget when resizing");
                return result;
            }

            internal_vulkan_texture* internal_texture = (internal_vulkan_texture*)texture->internal_data;
            darray_push(framebuffer_views, internal_texture->view);
        }

        VkFramebufferCreateInfo framebuffer_create_info = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        framebuffer_create_info.renderPass = internal_rendertarget->handle;
        framebuffer_create_info.attachmentCount = rendertarget->attachment_count;
        framebuffer_create_info.pAttachments = framebuffer_views;
        framebuffer_create_info.width = new_size.width;
        framebuffer_create_info.height = new_size.height;
        framebuffer_create_info.layers = 1;

        CHECK_VKRESULT(
            vkCreateFramebuffer(
                context->logical_device, 
                &framebuffer_create_info,
                context->allocator,
                &internal_rendertarget->framebuffers[i]), 
            "Failed to create framebuffers when resizing rendertarget");
    }

    darray_destroy(framebuffer_views);

    return EMBER_RESULT_OK;
}

void vulkan_rendertarget_destroy(
    emgpu_device* device, 
    emgpu_rendertarget* rendertarget) {

}
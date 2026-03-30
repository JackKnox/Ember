#include "defines.h"
#include "vulkan_rendertarget.h"

b8 vulkan_rendertarget_create(
	emgpu_device* device,
    emgpu_rendertarget_config* config,
    emgpu_rendertarget* out_rendertarget) {
    EM_ASSERT(device != NULL && config != NULL && out_rendertarget != NULL && "Invalid arguments passed to vulkan_rendertarget_create");
    vulkan_context* context = (vulkan_context*)device->internal_context;

#if EM_ENABLE_VALIDATION
    if (config->size.width <= 0 || config->size.height <= 0) {
        EM_ERROR("vulkan_rendertarget_create(): Rendertarget size must be greater than zero: Size = (%u, %u)", config->size.width, config->size.height);
        return FALSE;
    }

    if (config->attachment_count <= 0) {
        EM_ERROR("vulkan_rendertarget_create(): Rendertarget must be created with at least 1 attachment");
        return FALSE;
    }
#endif

    out_rendertarget->internal_data = ballocate(sizeof(internal_vulkan_rendertarget), MEMORY_TAG_RENDERER);
    internal_vulkan_rendertarget* internal_rendertarget = (internal_vulkan_rendertarget*)out_rendertarget->internal_data;

    out_rendertarget->attachment_count = config->attachment_count;
    out_rendertarget->origin = config->origin;
    out_rendertarget->size = config->size;

    //
    // Render pass setup
    //

    VkAttachmentReference* colour_attachments  = NULL;
    VkAttachmentDescription* attachments_descs = darray_reserve(VkAttachmentDescription, out_rendertarget->attachment_count, MEMORY_TAG_RENDERER);

    // Main subpass.
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    for (u32 i = 0; i < out_rendertarget->attachment_count; ++i) {
        const emgpu_rendertarget_attachment* attachment = &config->attachments[i];
        
        VkAttachmentDescription* attachment_desc = darray_push_empty(attachments_descs);
        attachment_desc->format = format_to_vulkan_type(attachment->format);
        attachment_desc->samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_desc->loadOp = load_op_to_vulkan_type(attachment->load_op);
        attachment_desc->storeOp = store_op_to_vulkan_type(attachment->store_op);
        attachment_desc->stencilLoadOp = load_op_to_vulkan_type(attachment->stencil_load_op);
        attachment_desc->stencilStoreOp = store_op_to_vulkan_type(attachment->stencil_store_op);

        attachment_desc->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_desc->finalLayout = attachment_type_to_image_layout(attachment->type);

        switch (attachment->type) {
            case EMBER_ATTACHMENT_TYPE_COLOR:
            case EMBER_ATTACHMENT_TYPE_WINDOW_SURFACE:
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

    subpass.colorAttachmentCount = colour_attachments ? darray_length(colour_attachments) : 0;
    subpass.pColorAttachments = colour_attachments;

    //
    // Subpass dependency
    //

    VkSubpassDependency dependency = {};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    VkRenderPassCreateInfo render_pass_create_info = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    render_pass_create_info.attachmentCount = darray_length(attachments_descs);
    render_pass_create_info.pAttachments    = attachments_descs;
    render_pass_create_info.subpassCount    = 1;
    render_pass_create_info.pSubpasses      = &subpass;
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies   = &dependency;

    CHECK_VKRESULT(
        vkCreateRenderPass(
            context->device.logical_device,
            &render_pass_create_info,
            context->allocator,
            &internal_rendertarget->handle),
        "Failed to create internal Vulkan renderpass");

    // Cleanup temp arrays...
    darray_destroy(attachments_descs);
    if (colour_attachments) darray_destroy(colour_attachments);

    internal_rendertarget->framebuffers = darray_reserve(VkFramebuffer, context->config.frames_in_flight, MEMORY_TAG_RENDERER);
    for (u32 i = 0; i < context->config.frames_in_flight; ++i) {
        VkFramebufferCreateInfo framebuffer_create_info = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        framebuffer_create_info.renderPass = internal_rendertarget->handle;
        framebuffer_create_info.width = out_rendertarget->size.width;
        framebuffer_create_info.height = out_rendertarget->size.height;
        framebuffer_create_info.layers = 1;

        VkImageView* views = darray_reserve(VkImageView, out_rendertarget->attachment_count, MEMORY_TAG_RENDERER);
        for (u32 j = 0; j < out_rendertarget->attachment_count; ++j) {
            internal_vulkan_texture* attachment_texture = (internal_vulkan_texture*)config->attachments[j].textures[i].internal_data;
            darray_push(views, attachment_texture->view);
        }

        framebuffer_create_info.attachmentCount = darray_length(views);
        framebuffer_create_info.pAttachments = views;

        VkFramebuffer* framebuffer = darray_push_empty(internal_rendertarget->framebuffers);
        CHECK_VKRESULT(
            vkCreateFramebuffer(
                context->device.logical_device, 
                &framebuffer_create_info, 
                context->allocator, 
                framebuffer), 
            "Failed to create framebuffer for Vulkan rendertarget");

        darray_destroy(views);
    }

    return TRUE;
}

b8 vulkan_rendertarget_resize(
	emgpu_device* device,
    emgpu_rendertarget* rendertarget,
    uvec2 new_size) {
    EM_ASSERT(device != NULL && rendertarget != NULL && "Invalid arguments passed to vulkan_rendertarget_resize");


}

void vulkan_rendertarget_begin(
    vulkan_context* context,
    vulkan_command_buffer* command_buffer,
    emgpu_rendertarget* rendertarget,
    b8 set_viewport, b8 set_scissor) {
    EM_ASSERT(context != NULL && command_buffer != NULL && rendertarget != NULL && "Invalid arguments passed to vulkan_rendertarget_begin");
    internal_vulkan_rendertarget* internal_rendertarget = (internal_vulkan_rendertarget*)rendertarget->internal_data;
    
    if (set_viewport) {
		VkViewport viewport = {};
        viewport.x = (f32)rendertarget->origin.x;
        viewport.y = (f32)(rendertarget->origin.y + rendertarget->size.height);
        viewport.width  = (f32)rendertarget->size.width;
        viewport.height = -(f32)rendertarget->size.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);
    }

    if (set_scissor) {
        VkRect2D scissor = {};
		scissor.offset.x = rendertarget->origin.x;
        scissor.offset.y = rendertarget->origin.y;
        scissor.extent.width  = rendertarget->size.width;
        scissor.extent.height = rendertarget->size.height;
		vkCmdSetScissor(command_buffer->handle, 0, 1, &scissor);
    }

    VkRenderPassBeginInfo begin_info = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    begin_info.renderPass = internal_rendertarget->handle;
    begin_info.framebuffer = internal_rendertarget->framebuffers[context->image_index];
    begin_info.renderArea.offset.x = rendertarget->origin.x;
    begin_info.renderArea.offset.y = rendertarget->origin.y;
    begin_info.renderArea.extent.width = rendertarget->size.width;
    begin_info.renderArea.extent.height = rendertarget->size.height;

    VkClearValue clear_value = {};
    clear_value.color.float32[0] = ((rendertarget->clear_colour >> 24) & 0xFF) / 255.0f;
    clear_value.color.float32[1] = ((rendertarget->clear_colour >> 16) & 0xFF) / 255.0f;
    clear_value.color.float32[2] = ((rendertarget->clear_colour >> 8)  & 0xFF) / 255.0f;
    clear_value.color.float32[3] = ((rendertarget->clear_colour)       & 0xFF) / 255.0f;

    begin_info.clearValueCount = 1;
    begin_info.pClearValues = &clear_value;

    vkCmdBeginRenderPass(command_buffer->handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void vulkan_rendertarget_end(
    vulkan_context* context,
    vulkan_command_buffer* command_buffer, 
    emgpu_rendertarget* rendertarget) {
    EM_ASSERT(context != NULL && command_buffer != NULL && rendertarget != NULL && "Invalid arguments passed to vulkan_rendertarget_end");
    vkCmdEndRenderPass(command_buffer->handle);
}

void vulkan_rendertarget_destroy(
	emgpu_device* device,
    emgpu_rendertarget* rendertarget) {
    EM_ASSERT(device != NULL && rendertarget != NULL && "Invalid arguments passed to vulkan_rendertarget_destroy");
    vulkan_context* context = (vulkan_context*)device->internal_context;
    if (context->device.logical_device) vkDeviceWaitIdle(context->device.logical_device);

    internal_vulkan_rendertarget* internal_rendertarget = (internal_vulkan_rendertarget*)rendertarget->internal_data;

    if (internal_rendertarget != NULL) {

        if (internal_rendertarget->framebuffers != NULL) {
            for (u32 i = 0; i < darray_length(internal_rendertarget->framebuffers); ++i) {
                if (!internal_rendertarget->framebuffers[i]) continue;

                vkDestroyFramebuffer(
                    context->device.logical_device,
                    internal_rendertarget->framebuffers[i],
                    context->allocator);
            }

            darray_destroy(internal_rendertarget->framebuffers);
        }

        if (internal_rendertarget->handle)
            vkDestroyRenderPass(context->device.logical_device, internal_rendertarget->handle, context->allocator);

        bfree(internal_rendertarget, sizeof(internal_vulkan_rendertarget), MEMORY_TAG_RENDERER);
    }
}
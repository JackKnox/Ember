#include "ember/core.h"
#include "vulkan_renderpass.h"

em_result vulkan_renderpass_create(
    emgpu_device* device,
    const emgpu_renderpass_config* config,
    emgpu_renderpass* out_renderpass) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    out_renderpass->internal_data = (internal_vulkan_renderpass*)mem_allocate(sizeof(internal_vulkan_renderpass), MEMORY_TAG_RENDERER);
    internal_vulkan_renderpass* internal_renderpass = (internal_vulkan_renderpass*)out_renderpass->internal_data;

    out_renderpass->attachment_count = config->attachment_count;

    VkAttachmentReference* colour_attachments  = NULL;
    VkAttachmentDescription* attachment_descs = darray_reserve(VkAttachmentDescription, out_renderpass->attachment_count, MEMORY_TAG_TEMP);

    for (u32 i = 0; i < out_renderpass->attachment_count; ++i) {
        const emgpu_attachment_config* attachment = &config->attachments[i];

        VkAttachmentReference* reference = NULL;
        switch (attachment->type) {
            case EMBER_ATTACHMENT_TYPE_COLOUR:
                if (!colour_attachments) colour_attachments = darray_create(VkAttachmentReference, MEMORY_TAG_TEMP);
                reference = darray_push_empty(colour_attachments);
                break;
            
            default:
                EM_ASSERT(FALSE && "Unsupported attachment type in renderpass");
                continue;
        }

        VkAttachmentDescription* desc = darray_push_empty(attachment_descs);
        desc->format = format_to_vulkan_type(attachment->format);
        desc->samples = VK_SAMPLE_COUNT_1_BIT;
        desc->loadOp = load_op_to_vulkan_type(attachment->load_op);
        desc->storeOp = store_op_to_vulkan_type(attachment->store_op);
        desc->stencilLoadOp = load_op_to_vulkan_type(attachment->stencil_load_op);
        desc->stencilStoreOp = store_op_to_vulkan_type(attachment->stencil_store_op);
        desc->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        desc->finalLayout = attachment_type_to_image_layout(attachment->type);

        reference->attachment = i;
        reference->layout = desc->finalLayout;
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
        vkCreateRenderPass(context->logical_device, &pass_create_info, context->allocator, &internal_renderpass->handle),
        "Failed to create internal Vulkan renderpass");

    darray_destroy(attachment_descs);
    if (colour_attachments) darray_destroy(colour_attachments);
    return EMBER_RESULT_OK;
}

void vulkan_renderpass_destroy(
    emgpu_device* device,
    emgpu_renderpass* renderpass) {
    vulkan_context* context = (vulkan_context*)device->internal_context;
    if (context->logical_device) vkDeviceWaitIdle(context->logical_device);

    internal_vulkan_renderpass* internal_renderpass = (internal_vulkan_renderpass*)renderpass->internal_data;
    if (!internal_renderpass)
        return;
    
    if (internal_renderpass->handle)
        vkDestroyRenderPass(context->logical_device, internal_renderpass->handle, context->allocator);

    mem_free(internal_renderpass, sizeof(internal_vulkan_renderpass), MEMORY_TAG_RENDERER);
    renderpass->internal_data = NULL;
}
#include "ember/core.h"
#include "vulkan_backend.h"

#include "ember/core/darray.h"
#include "ember/core/hashmap.h"

#include "ember/gpu/frame_internal.h"

/*
These functions has a lot of potential improvements.
Here is a list of some stuff to do if you're bored or fancy wrestling with Vulkan for five hours.

. Async model
    Basically this means have one frame in 'flight' while processing another with a wheel of fences and command buffers.
    See this image as an example (docs-assets.developer.apple.com/published/e75ecc44a7f513ac9322a840d37e2995/drawing-a-triangle-with-metal-4-1~dark%402x.png)
. Track resources
    For a simple compute raytracer images need to transition layouts for different use cases.
    Basically have the pipeline store its resources then do stuff to prep them for different ops.
. Pipeline barriers
    Somehow i've implemented the simpler way for making 'memory barries' which is just a call to VkCmdPipelineBarrier.
    If queue family indexes are the same, insert a pipeline barrier instead of a semaphore break.
. Multi-window support
    I haven't tested this yet but this should be reasonably simple to implement.
    Don't allow rendering with a frame-per-window as that just makes it easier.
. Transfer queue ownership
    When using resources over a semaphore break the Vulkan spec says you need to explicitly transfer ownership.
    You just do this with a VkCmdPipelineBarrier.
. Pipeline stage flags
    Each call to submit a command buffer has a list of wait semaphores them the type of op
    to wait for that the semaphore implictly connects to. Compute this somehow as this is very good for performance.

Just a little one here but can someone please put comments everywhere because
next thing I know i'll forget everything. 
*/

em_result vulkan_frame_framebuffer(emgpu_device* device, emgpu_renderpass* renderpass, uvec2 renderarea, VkImageView* attachments, u32 attachment_count, VkFramebuffer* out_framebuffer) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    internal_vulkan_renderpass* internal_renderpass = (internal_vulkan_renderpass*)renderpass->internal_data;

    u64 hash = hash_bytes(attachments, attachment_count);

    if (hashmap_get(internal_renderpass->framebuffers, hash, out_framebuffer) && *out_framebuffer)
        return EMBER_RESULT_OK;

    VkFramebufferCreateInfo create_info = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    create_info.renderPass      = internal_renderpass->handle;
    create_info.attachmentCount = attachment_count;
    create_info.pAttachments    = attachments;
    create_info.width           = renderarea.width;
    create_info.height          = renderarea.height;
    create_info.layers          = 1;

    CHECK_VKRESULT(
        vkCreateFramebuffer(context->logical_device, &create_info, context->allocator, out_framebuffer),
        "Failed to create Vulkan framebuffer during frame submission")
    
    hashmap_set(internal_renderpass->framebuffers, hash, out_framebuffer);
    return EMBER_RESULT_OK;
}

b8 vulkan_frame_process_payload(vulkan_frame_context* frame_context, rendercmd_payload* payload,
    vulkan_frame_submission* curr_submission, vulkan_submission_break* out_break_info, em_result* out_result) {

    switch (payload->hdr.type) {
        case RENDERCMD_NEXT_SURFACE_TEXTURE: {
            break;
        }

        case RENDERCMD_IMPORT_TEXTURE: {
            break;
        }
        
        case RENDERCMD_SET_RENDERAREA: {
            break;
        }

        case RENDERCMD_BEGIN_RENDERPASS: {
            break;
        }

        case RENDERCMD_END_RENDERPASS: {
            break;
        }

        case RENDERCMD_BIND_PIPELINE: {
            break;
        }

        case RENDERCMD_DRAW: {
            break;
        }

        case RENDERCMD_DRAW_INDEXED: {
            break;
        }

        case RENDERCMD_DISPATCH: {
            break;
        }
    }
    return FALSE;
}

em_result vulkan_device_submit_frame(emgpu_device* device, const emgpu_frame* frame) {
    vulkan_context* context = (vulkan_context*)device->internal_context;
    
    vulkan_frame_context frame_context = {};
    frame_context.submissions = darray_create(vulkan_frame_submission, &device->frame_allocator, MEMORY_TAG_FRAME);

    rendercmd_payload* payload = NULL;
    u32 i = 0;
    while ((payload = datastream_get(&frame->commands, i++)) && payload->hdr.type != RENDERCMD_FLUSH) {
        if (darray_length(frame_context.submissions) == 0) {
            // Kick off submissions with this...
        }

        // ------------------
        vulkan_submission_break break_info = {};
        em_result submit_result = EMBER_RESULT_OK;

        b8 broke = vulkan_frame_process_payload(&frame_context, payload, darray_last(frame_context.submissions), &break_info, &submit_result);
        if (submit_result != EMBER_RESULT_OK) {
            EM_ERROR("Vulkan", "Failed to process frame command: %s", em_result_string(submit_result, EMBER_BUILD_DEBUG));
            return submit_result;
        }
        else if (broke) {
            switch (break_info.type) {
                case VULKAN_BREAK_SWITCH_OPS:
                    // Command needs different ops, find new command buffer and return to command next loop.
                    break;

                case VULKAN_BREAK_WAIT_ON_BINARY:
                    // Add binary semaphore wait to current submission struct.
                    // Used for next surface texture.
                    break;
                
                case VULKAN_BREAK_SIGNAL_ON_BINARY:
                    // Add binary semaphore signal to current submission struct.
                    // Used for next surface texture.
                    break;

                //case VULKAN_BREAK_SPLIT_SUBMISSION:
                //    // Split submission 
                //    break;
            }
        }
    }

    // Hit flush command, drain frame context and execute submission calls.
    // --------------------------------------

    // --------------------------------------

    device->current_frame = (device->current_frame + 1) % context->frames_in_flight;
    return EMBER_RESULT_OK;
}
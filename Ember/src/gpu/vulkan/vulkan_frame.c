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

em_result vulkan_frame_framebuffer(emgpu_device* device, vulkan_frame_context* frame_context, emgpu_renderpass* renderpass, uvec2 renderarea, emgpu_frame_texture* attachments, u32 attachment_count, VkFramebuffer* out_framebuffer) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    internal_vulkan_renderpass* internal_renderpass = (internal_vulkan_renderpass*)renderpass->internal_data;

    VkImageView* views = mem_allocate(&device->frame_allocator, sizeof(VkImageView) * attachment_count, MEMORY_TAG_TEMP);

    u64 hash = FNV_OFFSET_BASIS;
    for (u64 i = 0; i < attachment_count; i++) {
        internal_vulkan_texture* internal_texture = (internal_vulkan_texture*)frame_context->frame_textures[attachments[i]]->internal_data;
        darray_push(views, internal_texture->view);

        hash ^= (u64)views[i];
        hash *= FNV_PRIME_NUMBER;
    }

    if (hashmap_get(internal_renderpass->framebuffers, hash, out_framebuffer) && *out_framebuffer)
        goto cleanup;

    VkFramebufferCreateInfo create_info = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    create_info.renderPass      = internal_renderpass->handle;
    create_info.attachmentCount = attachment_count;
    create_info.pAttachments    = views;
    create_info.width           = renderarea.width;
    create_info.height          = renderarea.height;
    create_info.layers          = 1;

    CHECK_VKRESULT(
        vkCreateFramebuffer(context->logical_device, &create_info, context->allocator, out_framebuffer),
        "Failed to create Vulkan framebuffer during frame submission")
    
    hashmap_set(internal_renderpass->framebuffers, hash, out_framebuffer);
cleanup:
    mem_free(&device->frame_allocator, views, sizeof(VkImageView) * attachment_count, MEMORY_TAG_TEMP);
    return EMBER_RESULT_OK;
}

#define ENSURE_OPS(ops_type)                                \
    if (frame_context->curr_ops != ops_type) {              \
            out_break_info->type = VULKAN_BREAK_SWITCH_OPS; \
            out_break_info->switch_ops.new = ops_type;      \
            return TRUE;                                    \
        }

// Compute given render command and convert to command buffers and break info.
b8 vulkan_frame_process_payload(emgpu_device* device, vulkan_frame_context* frame_context, rendercmd_payload* payload,
    vulkan_frame_submission* curr_submission, vulkan_submission_break* out_break_info, em_result* out_result) {
    
    switch (payload->hdr.type) {
        case RENDERCMD_NEXT_SURFACE_TEXTURE: {
            ENSURE_OPS(EMBER_OPER_TYPE_GRAPHICS);

            if (!curr_submission->managed_surfaces)
                curr_submission->managed_surfaces = darray_create(vulkan_managed_surface, &device->frame_allocator, MEMORY_TAG_FRAME);
            vulkan_managed_surface* managed_surface = darray_push_empty(curr_submission->managed_surfaces);

            managed_surface->handle = payload->next_surface_texture.surface;
            frame_context->frame_textures[payload->next_surface_texture.dst_texture] = vulkan_surface_curr_texture(device, managed_surface->handle);

            out_break_info->type = VULKAN_BREAK_BINARY_SEMAPHORES;
            out_break_info->binary_semaphores.wait_semaphores        = vulkan_surface_wait_semaphore(device, managed_surface->handle);
            out_break_info->binary_semaphores.wait_semaphore_count   = 1;
            out_break_info->binary_semaphores.signal_semaphores      = vulkan_surface_signal_semaphore(device, managed_surface->handle);
            out_break_info->binary_semaphores.signal_semaphore_count = 1;
            return TRUE;
        }

        case RENDERCMD_IMPORT_TEXTURE: {
            frame_context->frame_textures[payload->import_texture.dst_texture] = payload->import_texture.texture;
            break;
        }
        
        case RENDERCMD_SET_RENDERAREA: {
            ENSURE_OPS(EMBER_OPER_TYPE_GRAPHICS);
            frame_context->render_origin = payload->set_renderarea.origin;
            frame_context->render_size = payload->set_renderarea.size;

            VkViewport viewport = {};
            viewport.x        =  (f32)frame_context->render_origin.x;
            viewport.y        =  (f32)(frame_context->render_origin.y + frame_context->render_size.height);
            viewport.width    =  (f32)frame_context->render_size.width;
            viewport.height   = -(f32)frame_context->render_size.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(curr_submission->commandbuf, 0, 1, &viewport);

            VkRect2D scissor = {};
            scissor.offset.x      = frame_context->render_origin.x;
            scissor.offset.y      = frame_context->render_origin.y;
            scissor.extent.width  = frame_context->render_size.width;
            scissor.extent.height = frame_context->render_size.height;
            vkCmdSetScissor(curr_submission->commandbuf, 0, 1, &scissor);
            break;
        }

        case RENDERCMD_BEGIN_RENDERPASS: {
            ENSURE_OPS(EMBER_OPER_TYPE_GRAPHICS);
            emgpu_renderpass* renderpass = payload->bind_renderpass.renderpass;
            internal_vulkan_renderpass* internal_renderpass = (internal_vulkan_renderpass*)renderpass->internal_data;

            VkClearValue clear_value = {};
            clear_value.color.float32[0] = ((renderpass->clear_colour >> 24) & 0xFF) / 255.0f;
            clear_value.color.float32[1] = ((renderpass->clear_colour >> 16) & 0xFF) / 255.0f;
            clear_value.color.float32[2] = ((renderpass->clear_colour >> 8)  & 0xFF) / 255.0f;
            clear_value.color.float32[3] = ((renderpass->clear_colour)       & 0xFF) / 255.0f;

            VkRenderPassBeginInfo begin_info = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
            begin_info.renderPass      = internal_renderpass->handle;

            begin_info.renderArea.offset.x      = frame_context->render_origin.x;
            begin_info.renderArea.offset.y      = frame_context->render_origin.y;
            begin_info.renderArea.extent.width  = frame_context->render_size.width;
            begin_info.renderArea.extent.height = frame_context->render_size.height;

            begin_info.clearValueCount = 1;
            begin_info.pClearValues    = &clear_value;

            em_result result = vulkan_frame_framebuffer(
                device, frame_context, renderpass, frame_context->render_size, payload->bind_renderpass.attachments, payload->bind_renderpass.attachment_count, &begin_info.framebuffer);

            if (result != EMBER_RESULT_OK) {
                EM_ERROR("Vulkan", "Failed to retrieve Vulkan framebuffer while beginning renderpass: %s", em_result_string(result, EMBER_BUILD_DEBUG));
                *out_result = result;
                return FALSE;
            }

            vkCmdBeginRenderPass(curr_submission->commandbuf, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
            break;
        }

        case RENDERCMD_END_RENDERPASS: {
            ENSURE_OPS(EMBER_OPER_TYPE_GRAPHICS);
            vkCmdEndRenderPass(curr_submission->commandbuf);
            break;
        }

        case RENDERCMD_BIND_PIPELINE: {
            ENSURE_OPS(payload->bind_pipeline.pipeline->type);
            vulkan_pipeline_bind(curr_submission->commandbuf, 
                payload->bind_pipeline.pipeline,
                device->current_frame);
            break;
        }

        case RENDERCMD_DRAW: {
            ENSURE_OPS(EMBER_OPER_TYPE_GRAPHICS);
            vkCmdDraw(curr_submission->commandbuf,
                payload->draw.vertex_count,
                payload->draw.instance_count,
                0, 0);
            break;
        }

        case RENDERCMD_DRAW_INDEXED: {
            ENSURE_OPS(EMBER_OPER_TYPE_GRAPHICS);
            vkCmdDrawIndexed(curr_submission->commandbuf,
                payload->draw_indexed.index_count,
                payload->draw_indexed.instance_count,
                0, 0, 0);
            break;
        }

        case RENDERCMD_DISPATCH: {
            ENSURE_OPS(EMBER_OPER_TYPE_COMPUTE);
            vkCmdDispatch(curr_submission->commandbuf,
                payload->dispatch.group_size.x,
                payload->dispatch.group_size.y,
                payload->dispatch.group_size.z);
            break;
        }
    }

    return FALSE;
}

void vulkan_frame_finalise_submission(emgpu_device* device, vulkan_frame_context* frame_context, vulkan_frame_submission* curr_submission) {
    vkEndCommandBuffer(curr_submission->commandbuf);
}

em_result vulkan_device_submit_frame(emgpu_device* device, const emgpu_frame* frame) {
    vulkan_context* context = (vulkan_context*)device->internal_context;
    
    vulkan_frame_context frame_context = {};
    frame_context.submissions = darray_create(vulkan_frame_submission, &device->frame_allocator, MEMORY_TAG_FRAME);
    frame_context.frame_textures = mem_allocate(&device->frame_allocator, sizeof(emgpu_texture*) * frame->current_resource_idx, MEMORY_TAG_RENDERER);

    rendercmd_payload* payload = NULL;
    u32 i = 0;
    while ((payload = datastream_get(&frame->commands, i++)) && payload->hdr.type != RENDERCMD_FLUSH) {
        // ------------------
        vulkan_submission_break break_info = {};
        em_result submit_result = EMBER_RESULT_OK;

        b8 broke = vulkan_frame_process_payload(device, &frame_context, payload, darray_last(frame_context.submissions), &break_info, &submit_result);
        if (submit_result != EMBER_RESULT_OK) {
            EM_ERROR("Vulkan", "Failed to process frame command: %s", em_result_string(submit_result, EMBER_BUILD_DEBUG));
            return submit_result;
        }
        else if (broke) {
            switch (break_info.type) {
                case VULKAN_BREAK_SWITCH_OPS:
                    // Command needs different ops, find new command buffer and return to command next loop.
                    if (frame_context.curr_ops == break_info.switch_ops.new) break;

                    if (darray_length(frame_context.submissions) > 1)
                        vulkan_frame_finalise_submission(device, &frame_context, darray_last(frame_context.submissions));
                    
                    vulkan_frame_submission* new_submission = darray_push_empty(frame_context.submissions);
                    new_submission->ops_type = break_info.switch_ops.new;

                    switch (new_submission->ops_type) {
                        case EMBER_OPER_TYPE_GRAPHICS:
                            new_submission->commandbuf = context->graphics_commandbufs[device->current_frame];
                            break;
                        case EMBER_OPER_TYPE_COMPUTE:
                            new_submission->commandbuf = context->compute_commandbufs[device->current_frame];
                            break;
                        default:
                            EM_ASSERT(FALSE && "Unsupported render command ops type");
                            break;
                    }

                    VkCommandBufferBeginInfo begin_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
                    CHECK_VKRESULT(
                        vkBeginCommandBuffer(new_submission->commandbuf, &begin_info),
                        "Failed to begin Vulkan command buffer for new device ops type");

                    --i;
                    break;

                case VULKAN_BREAK_BINARY_SEMAPHORES:
                    // Add wait / binary semaphore signal to current submission struct.
                    // Used for next surface texture.
                    break;
            }
        }
    }

    // Hit flush command, drain frame context and execute submission calls.
    // --------------------------------------
    
    // --------------------------------------

    device->current_frame = (device->current_frame + 1) % context->frames_in_flight;
    return EMBER_RESULT_OK;
}
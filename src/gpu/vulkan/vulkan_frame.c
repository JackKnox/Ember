#include "ember/core.h"
#include "vulkan_backend.h"

#include "ember/core/darray.h"
#include "ember/core/hashmap.h"

#include "ember/gpu/frame_internal.h"

/*
These functions has a lot of potential improvements.
Here is a list of some stuff to do if you're bored or fancy wrestling with Vulkan for five hours.

. Track resources
    For a simple compute raytracer images need to transition layouts for different use cases.
    Basically have the pipeline store its resources then do stuff to prep them for different ops.
. Pipeline barriers
    Somehow i've not implemented the simpler way for making 'memory barries' which is just a call to VkCmdPipelineBarrier.
    If queue family indexes are the same, insert a pipeline barrier instead of a semaphore break.
. Multi-window support
    I haven't tested this yet but this should be reasonably simple to implement.
    Don't allow rendering with a frame-per-window as that just makes it easier.
. Timeline semaphores
    These are a reasonably new feature of Vulkan which basically removes the semaphores and the in-flight fences. 
    Only problem is that they don't work with the WSI, since Vulkan is the main backend for ember_gpu I can't support this.
. Transfer queue ownership
    When using resources over a semaphore break the Vulkan spec says you need to explicitly transfer ownership.
    You just do this with a VkCmdPipelineBarrier.
. Pipeline stage flags
    Each call to submit a command buffer has a list of wait semaphores them the type of op
    to wait for that the semaphore implictly connects to. Compute this somehow as this is very good for performance.
. Allocate more command buffers
    Right now on start the device creates a wheel of command buffers per-ops type
    If two raster submissions are split, maybe due to a memory barrier you need to allocate another command buffer temporally.

Just a little one here but can someone please put comments everywhere because
next thing I know i'll forget everything.
*/

VkResult vulkan_frame_framebuffer(emgpu_device* device, vulkan_frame_context* frame_context, emgpu_renderpass* renderpass, uvec2 renderarea, emgpu_frame_texture* attachments, u32 attachment_count, VkFramebuffer* out_framebuffer) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    internal_vulkan_renderpass* internal_renderpass = (internal_vulkan_renderpass*)renderpass->internal_data;

    // Make a hash to connect to the hashmap while adding to the buffer.
    VkImageView* views = mem_allocate(&device->frame_allocator, sizeof(VkImageView) * attachment_count, MEMORY_TAG_TEMP);

    u64 hash = FNV_OFFSET_BASIS;
    for (u64 i = 0; i < attachment_count; i++) {
        internal_vulkan_texture* internal_texture = (internal_vulkan_texture*)frame_context->frame_textures[attachments[i]]->internal_data;
        views[i] = internal_texture->view;

        hash ^= (u64)views[i];
        hash *= FNV_PRIME_NUMBER;
    }

    VkResult result = VK_SUCCESS;
    if (hashmap_get(internal_renderpass->framebuffers, hash, out_framebuffer) && *out_framebuffer)
        goto cleanup;

    // Framebuffer wasn't found in the hashmap, so create one...
    VkFramebufferCreateInfo create_info = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    create_info.renderPass      = internal_renderpass->handle;
    create_info.attachmentCount = attachment_count;
    create_info.pAttachments    = views;
    create_info.width           = renderarea.width;
    create_info.height          = renderarea.height;
    create_info.layers          = 1;

    result = vkCreateFramebuffer(context->logical_device, &create_info, context->allocator, out_framebuffer);
    if (!vulkan_result_is_success(result)) goto cleanup;

    hashmap_set(internal_renderpass->framebuffers, hash, out_framebuffer);
cleanup:
    mem_free(&device->frame_allocator, views, sizeof(VkImageView) * attachment_count, MEMORY_TAG_TEMP);
    return result;
}

// Switches and breaks ops type in frame if doesn't match.
#define ENSURE_OPS(ops_type)                            \
    if (frame_context->curr_ops != ops_type) {          \
        out_break_info->type = VULKAN_BREAK_SWITCH_OPS; \
        out_break_info->switch_ops.new = ops_type;      \
        return TRUE;                                    \
    }

#define BREAK_VKRESULT(func, message)             \
    {                                             \
        VkResult result = func;                   \
        if (!vulkan_result_is_success(result)) {  \
            EM_ERROR("Vulkan", message ": %s",    \
                     vulkan_result_string(result, EMBER_BUILD_DEBUG)); \
            *out_result = em_result_from_vulkan_result(result);        \
            return TRUE;                          \
        }                                         \
    }

// Compute given render command and convert to command buffers and break info.
b8 vulkan_frame_process_payload(emgpu_device* device, vulkan_frame_context* frame_context, rendercmd_payload* payload,
    vulkan_frame_submission* curr_submission, vulkan_submission_break* out_break_info, em_result* out_result) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    switch (payload->hdr.type) {
        case RENDERCMD_NEXT_SURFACE_TEXTURE: {
            ENSURE_OPS(EMBER_OPER_TYPE_RASTER);

            // Add to a list of 'managed surface's to later present at the end.
            vulkan_managed_surface* managed_surface = darray_push_empty(frame_context->managed_surfaces);
            managed_surface->handle = payload->next_surface_texture.surface;
            managed_surface->submission_index = darray_length(frame_context->submissions) - 1;

            internal_vulkan_surface* internal_surface = (internal_vulkan_surface*)managed_surface->handle->internal_data;

            // Acquire image for the frame, now use the corrosponding resources everywhere.
            BREAK_VKRESULT(
                vkAcquireNextImageKHR(
                    context->logical_device, 
                    internal_surface->swapchain, 
                    UINT64_MAX, 
                    internal_surface->image_available_semaphores[device->current_frame], 
                    VK_NULL_HANDLE, 
                    &internal_surface->image_index),
                "Failed to acquire next internal Vulkan swapchain image");

            managed_surface->usable_image_index = internal_surface->image_index;

            // Add the acquire image to frame textures, remember its purely managed by the surface.
            frame_context->frame_textures[payload->next_surface_texture.dst_texture] = &internal_surface->swapchain_images[internal_surface->image_index];

            // Break and add the semaphores to the current submission.
            out_break_info->type = VULKAN_BREAK_BINARY_SEMAPHORES;
            out_break_info->binary_semaphores.wait_semaphores        = &internal_surface->image_available_semaphores[device->current_frame];
            out_break_info->binary_semaphores.wait_semaphore_count   = 1;
            out_break_info->binary_semaphores.signal_semaphores      = &internal_surface->render_complete_semaphores[device->current_frame];
            out_break_info->binary_semaphores.signal_semaphore_count = 1;
            return TRUE;
        }

        case RENDERCMD_IMPORT_TEXTURE: {
            // Just add the texture to the list, no synchronization required probably
            frame_context->frame_textures[payload->import_texture.dst_texture] = payload->import_texture.texture;
            break;
        }

        case RENDERCMD_SET_RENDERAREA: {
            ENSURE_OPS(EMBER_OPER_TYPE_RASTER);
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
            ENSURE_OPS(EMBER_OPER_TYPE_RASTER);
            emgpu_renderpass* renderpass = payload->begin_renderpass.renderpass;
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

            // Retrieve cached framebuffer or create new one and add it to the hashmap for later.
            BREAK_VKRESULT(
                vulkan_frame_framebuffer(
                    device, frame_context, 
                    renderpass, 
                    frame_context->render_size, 
                    payload->begin_renderpass.attachments, 
                    payload->begin_renderpass.attachment_count, 
                    &begin_info.framebuffer),
                "Failed to create internal Vulkan framebuffer during frame submission");

            vkCmdBeginRenderPass(curr_submission->commandbuf, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
            break;
        }

        case RENDERCMD_END_RENDERPASS: {
            ENSURE_OPS(EMBER_OPER_TYPE_RASTER);
            vkCmdEndRenderPass(curr_submission->commandbuf);
            break;
        }

        case RENDERCMD_BIND_PIPELINE: {
            ENSURE_OPS(payload->bind_pipeline.pipeline->type);
            vulkan_pipeline_bind(curr_submission->commandbuf,
                payload->bind_pipeline.pipeline,
                device->current_frame);
            
            VkDeviceSize offset = 0;

            if (payload->bind_pipeline.vertex_buffer_count > 1) {
                EM_ERROR("Vulkan", "Not yet implemented binding multiple vertex buffers.");
                return EMBER_RESULT_UNIMPLEMENTED;
            }

            if (payload->bind_pipeline.vertex_buffers) {
                internal_vulkan_buffer* internal_buffer = (internal_vulkan_buffer*)payload->bind_pipeline.vertex_buffers->internal_data;
                vkCmdBindVertexBuffers(curr_submission->commandbuf, 0, 1, &internal_buffer->handle, &offset);
            }

            if (payload->bind_pipeline.index_buffer) {
                internal_vulkan_buffer* internal_buffer = (internal_vulkan_buffer*)payload->bind_pipeline.index_buffer->internal_data;
                vkCmdBindIndexBuffer(curr_submission->commandbuf, internal_buffer->handle, offset, VK_INDEX_TYPE_UINT16);
            }
            break;
        }

        case RENDERCMD_DRAW: {
            ENSURE_OPS(EMBER_OPER_TYPE_RASTER);
            vkCmdDraw(curr_submission->commandbuf,
                payload->draw.vertex_count,
                payload->draw.instance_count,
                0, 0);
            break;
        }

        case RENDERCMD_DRAW_INDEXED: {
            ENSURE_OPS(EMBER_OPER_TYPE_RASTER);
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
    frame_context.managed_surfaces = darray_create(vulkan_managed_surface, &device->frame_allocator, MEMORY_TAG_FRAME);
    frame_context.frame_textures = mem_allocate(&device->frame_allocator, sizeof(emgpu_texture*) * (frame->current_resource_idx + 1), MEMORY_TAG_FRAME);

    // Wait for fences here as we use the in flight command buffers in the loop.
    CHECK_VKRESULT(
        vkWaitForFences(context->logical_device, 1, &context->in_flight_fences[device->current_frame], TRUE, UINT64_MAX),
        "Failed to wait for internal Vulkan in flight fences");

    CHECK_VKRESULT(
        vkResetFences(context->logical_device, 1, &context->in_flight_fences[device->current_frame]),
        "Failed to reset internal Vulkan in flight fences");

    u32 block_count = darray_length(frame->commands.block_infos);
    for (u32 i = 0; i < block_count; ++i) {
        rendercmd_payload* payload = (rendercmd_payload*)datastream_get(&frame->commands, i);
        if (payload->hdr.type == RENDERCMD_FLUSH) break;

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

                    if (darray_length(frame_context.submissions) > 0)
                        vulkan_frame_finalise_submission(device, &frame_context, darray_last(frame_context.submissions));

                    vulkan_frame_submission* new_submission = darray_push_empty(frame_context.submissions);
                    new_submission->ops_type = break_info.switch_ops.new;
                    new_submission->binary_waits = darray_create(VkSemaphore, &device->frame_allocator, MEMORY_TAG_FRAME);
                    new_submission->binary_signals = darray_create(VkSemaphore, &device->frame_allocator, MEMORY_TAG_FRAME);

                    switch (new_submission->ops_type) {
                        case EMBER_OPER_TYPE_RASTER:
                            new_submission->commandbuf = context->raster_commandbufs[device->current_frame];
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

                    frame_context.curr_ops = break_info.switch_ops.new;
                    --i;
                    break;

                case VULKAN_BREAK_BINARY_SEMAPHORES:
                    vulkan_frame_submission* curr = darray_last(frame_context.submissions);

                    darray_concat(curr->binary_waits,
                        break_info.binary_semaphores.wait_semaphore_count,
                        break_info.binary_semaphores.wait_semaphores);

                    darray_concat(curr->binary_signals,
                        break_info.binary_semaphores.signal_semaphore_count,
                        break_info.binary_semaphores.signal_semaphores);
                    break;
            }
        }
    }
    if (darray_length(frame_context.submissions) > 0)
        vulkan_frame_finalise_submission(device, &frame_context, darray_last(frame_context.submissions));


    // Hit flush command, drain frame context and execute submission calls.
    // --------------------------------------
    for (u32 i = 0; i < darray_length(frame_context.submissions); ++i) {
        vulkan_frame_submission* submission = &frame_context.submissions[i];
        
        VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };
        
        VkSubmitInfo submit_info = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submit_info.waitSemaphoreCount   = darray_length(submission->binary_waits);
        submit_info.pWaitSemaphores      = submission->binary_waits;
        submit_info.pWaitDstStageMask    = wait_stages;
        submit_info.commandBufferCount   = 1;
        submit_info.pCommandBuffers      = &submission->commandbuf;
        submit_info.signalSemaphoreCount = darray_length(submission->binary_signals);
        submit_info.pSignalSemaphores    = submission->binary_signals;

        CHECK_VKRESULT(
            vkQueueSubmit(
                context->mode_queues[ops_type_to_queue_type(submission->ops_type)].handle,
                1, 
                &submit_info, 
                context->in_flight_fences[device->current_frame]),
            "Failed to submit Vulkan command buffer");
    }
    // --------------------------------------

    for (u32 i = 0; i < darray_length(frame_context.managed_surfaces); ++i) {
        vulkan_managed_surface* managed_surface = &frame_context.managed_surfaces[i];
        internal_vulkan_surface* internal_surface = (internal_vulkan_surface*)managed_surface->handle->internal_data;

        VkPresentInfoKHR present_info = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores    = &internal_surface->render_complete_semaphores[device->current_frame];
        present_info.swapchainCount     = 1;
        present_info.pSwapchains        = &internal_surface->swapchain;
        present_info.pImageIndices      = &managed_surface->usable_image_index;
        // present_info.pResults;

        CHECK_VKRESULT(
            vkQueuePresentKHR(context->mode_queues[VULKAN_QUEUE_TYPE_PRESENT].handle, &present_info),
            "Failed to present Vulkan swapchains");
    }

    for (u32 i = 0; i < darray_length(frame_context.submissions); ++i) {
        const vulkan_frame_submission* submission = &frame_context.submissions[i];
        darray_destroy(submission->binary_signals);
        darray_destroy(submission->binary_waits);
    }
    darray_destroy(frame_context.submissions);
    darray_destroy(frame_context.managed_surfaces); 
    mem_free(&device->frame_allocator, frame_context.frame_textures, sizeof(emgpu_texture*) * (frame->current_resource_idx + 1), MEMORY_TAG_FRAME);

    device->current_frame = (device->current_frame + 1) % context->frames_in_flight;
    return EMBER_RESULT_OK;
}
#include "ember/core.h"
#include "vulkan_backend.h"

#include "ember/gpu/frame_internal.h"

#include "ember/core/darray.h"

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
. Timeline semaphores
    These is a reasonably new features of Vulkan which basically removes the semaphore pool and
    the in-flight fences. Only problem is that they don't work with the WSI, since Vulkan is the main backend for ember_gpu I can't support this.
. Pool allocator
    Basically write a pool allocator using ember_allocator to speed up the frame submission.
    Only problem is what size should the pool be?
. Multi-window support
    I haven't tested this yet but this should be reasonably simple to implement.
    Don't allow rendering with a frame-per-window as that just makes it easier.
. Transfer queue ownership
    When using resources over a semaphore break the Vulkan spec says you need to explicitly transfer ownership.
    You just do this with a VkCmdPipelineBarrier.
. Submission structs in public API
    At the minute the device loops over all the commands and splits them up into queue submissions + command buffer.
    Push this loop to the Frame API to half the time complexity of the renderer.
. Pipeline stage flags
    Each call to submit a command buffer has a list of wait semaphores them the type of op
    to wait for that the semaphore implictly connects to. Compute this somehow as this is very good for performance.

Just a little one here but can someone please put comments everywhere because
next thing I know i'll forget everything. 
*/

em_result vulkan_frame_new_semaphore(emgpu_device* device, vulkan_frame_context* frame_context, VkSemaphore* out_semaphore) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    u32 index = frame_context->semaphore_index;
    if (index >= darray_length(context->semaphore_pool)) {
        u32 target = (index + 1) * DARRAY_RESIZE_FACTOR;
        u32 current = darray_length(context->semaphore_pool);

        // Reserve capacity without changing length
        context->semaphore_pool = darray_resize(context->semaphore_pool, target);

        // Create only the new semaphores needed to reach target length
        for (u32 i = current; i < target; ++i) {
            VkSemaphoreCreateInfo create_info = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
            
            VkResult result = vkCreateSemaphore(context->logical_device, &create_info, context->allocator, &context->semaphore_pool[i]);
            if (result != VK_SUCCESS) return em_result_from_vulkan_result(result);
        }
    }

    frame_context->semaphore_index++;
    *out_semaphore = context->semaphore_pool[index];
    return EMBER_RESULT_OK;
}

em_result vulkan_frame_framebuffer(
    emgpu_device* device, 
    vulkan_frame_context* frame_context,
    emgpu_renderpass* renderpass,
    uvec2 renderarea,
    emgpu_frame_texture* attachments,
    u32 attachment_count, 
    VkFramebuffer* out_framebuffer) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    internal_vulkan_renderpass* internal_renderpass = (internal_vulkan_renderpass*)renderpass->internal_data;

    u32 new_size = internal_renderpass->total_cycle_framebuffers / context->frames_in_flight;
    if (device->current_frame == context->frames_in_flight - 1 && new_size < darray_length(internal_renderpass->framebuffers)) {
        for (u32 i = new_size; i < darray_length(internal_renderpass->framebuffers); ++i) {
            vkDestroyFramebuffer(
                context->logical_device, 
                internal_renderpass->framebuffers[i].framebuffer, 
                context->allocator);
        }

        internal_renderpass->framebuffers = darray_resize(internal_renderpass->framebuffers, new_size);
    }

    VkImageView* views = darray_create(VkImageView, NULL, MEMORY_TAG_TEMP);

    u64 hash = 1469598103934665603ULL; // FNV offset basis
    for (u32 i = 0; i < attachment_count; ++i) {
        const emgpu_texture* texture = frame_context->frame_textures[attachments[i]];
        internal_vulkan_texture* internal_texture = (internal_vulkan_texture*)texture->internal_data;
        darray_push(views, internal_texture->view);

        hash ^= (u64)internal_texture->view;
        hash *= 1099511628211ULL; // FNV prime
    }

    for (u32 i = 0; i < darray_length(internal_renderpass->framebuffers); ++i) {
        const vulkan_renderpass_framebuffer* entry = &internal_renderpass->framebuffers[i];

        if (hash == entry->cache_id) {
            *out_framebuffer = entry->framebuffer;
            return EMBER_RESULT_OK;
        }
    }

    VkFramebufferCreateInfo create_info = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    create_info.renderPass      = internal_renderpass->handle;
    create_info.attachmentCount = attachment_count;
    create_info.pAttachments    = views;
    create_info.width           = renderarea.width;
    create_info.height          = renderarea.height;
    create_info.layers          = 1;

    VkFramebuffer framebuffer;
    CHECK_VKRESULT(
        vkCreateFramebuffer(context->logical_device, &create_info, context->allocator, &framebuffer),
        "Failed to create Vulkan framebuffer during frame submission")

    vulkan_renderpass_framebuffer* entry = darray_push_empty(internal_renderpass->framebuffers);
    entry->cache_id = hash;
    entry->framebuffer = framebuffer;

    *out_framebuffer = entry->framebuffer;
    return EMBER_RESULT_OK;
}

em_result vulkan_frame_context_init(const emgpu_frame* init_frame, vulkan_frame_context* out_context) {
    out_context->present_semaphores = darray_create(VkSemaphore, NULL, MEMORY_TAG_TEMP);
    out_context->vk_submissions     = darray_from_data(vulkan_frame_submission, darray_length(init_frame->submissions), NULL, init_frame->local_allocator, MEMORY_TAG_TEMP);
    out_context->frame_textures     = mem_allocate(init_frame->local_allocator, sizeof(emgpu_texture*) * (init_frame->current_resource_idx + 1), MEMORY_TAG_TEMP);

    return EMBER_RESULT_OK;
}

em_result vulkan_frame_process_submission(emgpu_device* device, const emgpu_frame* frame, vulkan_frame_context* frame_context, u32 submission_index) {
    const emgpu_frame_submission* curr_api_submission = &frame->submissions[submission_index];
    vulkan_frame_submission* curr_submission = &frame_context->vk_submissions[submission_index];

    for (u32 i = 0; i < curr_api_submission->submission_length; ++i) {
        rendercmd_payload* payload = datastream_get(&frame->commands, i + curr_api_submission->start_index);

        switch (payload->hdr.type) {
            case RENDERCMD_SET_RENDERAREA: {
                VkViewport viewport = {};
                viewport.x = (f32)payload->set_renderarea.origin.x;
                viewport.y = (f32)(payload->set_renderarea.origin.y + payload->set_renderarea.size.height);
                viewport.width  = (f32)payload->set_renderarea.size.width;
                viewport.height = -(f32)payload->set_renderarea.size.height;
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
                vkCmdSetViewport(curr_submission->commandbuf, 0, 1, &viewport);

                VkRect2D scissor = {};
                scissor.offset.x = payload->set_renderarea.origin.x;
                scissor.offset.y = payload->set_renderarea.origin.y;
                scissor.extent.width  = payload->set_renderarea.size.width;
                scissor.extent.height = payload->set_renderarea.size.height;
                vkCmdSetScissor(curr_submission->commandbuf, 0, 1, &scissor);
                break;
            }
            
            case RENDERCMD_BIND_NEXT_SURFACE_TEXTURE: {
                if (!curr_submission->signal_semaphore) {
                    em_result result = vulkan_frame_new_semaphore(device, frame_context, &curr_submission->signal_semaphore);
                    if (result != EMBER_RESULT_OK) return result;
                }

                darray_push(frame_context->present_semaphores, curr_submission->signal_semaphore);

                const emgpu_frame_surface* managed_surface = &frame->managed_surfaces[payload->next_surface_texture.surface_index];
                frame_context->frame_textures[payload->next_surface_texture.dst_texture] = vulkan_surface_curr_texture(device, managed_surface->handle);
                break;
            }
            
            case RENDERCMD_BIND_IMPORT_TEXTURE: {
                frame_context->frame_textures[payload->import_texture.dst_texture] = payload->import_texture.texture;
                break;
            }
            
            case RENDERCMD_BEGIN_RENDERPASS: {
                emgpu_renderpass* renderpass = payload->bind_renderpass.renderpass;
                internal_vulkan_renderpass* internal_renderpass = (internal_vulkan_renderpass*)renderpass->internal_data;

                if (device->current_frame == 0) internal_renderpass->total_cycle_framebuffers = 0;
                
                uvec2 renderarea = frame_context->frame_textures[payload->bind_renderpass.attachments[0]]->size;

                VkClearValue clear_value = {};
                clear_value.color.float32[0] = ((renderpass->clear_colour >> 24) & 0xFF) / 255.0f;
                clear_value.color.float32[1] = ((renderpass->clear_colour >> 16) & 0xFF) / 255.0f;
                clear_value.color.float32[2] = ((renderpass->clear_colour >> 8)  & 0xFF) / 255.0f;
                clear_value.color.float32[3] = ((renderpass->clear_colour)       & 0xFF) / 255.0f;

                VkRenderPassBeginInfo begin_info = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
                begin_info.renderPass               = internal_renderpass->handle;
                // Should equal the size of the attachted framebuffers. Viewport / scissor is already defined.
                // TODO: begin_info.renderArea.origin??
                begin_info.renderArea.extent.width  = renderarea.width;
                begin_info.renderArea.extent.height = renderarea.height;
                begin_info.clearValueCount          = 1;
                begin_info.pClearValues             = &clear_value;

                em_result result = vulkan_frame_framebuffer(
                    device, frame_context, renderpass, renderarea, payload->bind_renderpass.attachments, renderpass->attachment_count, &begin_info.framebuffer);
                
                if (result != EMBER_RESULT_OK) {
                    EM_ERROR("Vulkan", "Failed to retrieve Vulkan framebuffer while beginning renderpass: %s", em_result_string(result, EMBER_BUILD_DEBUG));
                    return result;
                }

                vkCmdBeginRenderPass(curr_submission->commandbuf, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
                break;
            }
            
            case RENDERCMD_END_RENDERPASS: {
                vkCmdEndRenderPass(curr_submission->commandbuf);
                break;
            }
            
            case RENDERCMD_MEMORY_BARRIER: {
                EM_ERROR("Vulkan", "Vulkan device backend does not yet implement memory barriers!");
                return EMBER_RESULT_UNIMPLEMENTED;
            }
            
            case RENDERCMD_BIND_PIPELINE: {
                vulkan_pipeline_bind(device, curr_submission->commandbuf, payload->bind_pipeline.pipeline);
                break;
            }
            
            case RENDERCMD_DRAW: {
                vkCmdDraw(curr_submission->commandbuf,
                    payload->draw.vertex_count,
                    payload->draw.instance_count,
                    0, 0);
                break;
            }
            
            case RENDERCMD_DRAW_INDEXED: {
                vkCmdDrawIndexed(curr_submission->commandbuf,
                    payload->draw_indexed.index_count,
                    payload->draw_indexed.instance_count,
                    0, 0, 0);
                break;
            }
            
            case RENDERCMD_DISPATCH: {
                vkCmdDispatch(curr_submission->commandbuf,
                    payload->dispatch.group_size.x,
                    payload->dispatch.group_size.y,
                    payload->dispatch.group_size.z);
                break;
            }
            
        }
    }

    return EMBER_RESULT_OK;
}

em_result vulkan_device_submit_frame(emgpu_device* device, const emgpu_frame* frame) {
    vulkan_context* context = (vulkan_context*)device->internal_context;
    
    vulkan_frame_context frame_context = {};

    em_result result = vulkan_frame_context_init(frame, &frame_context);
    if (result != EMBER_RESULT_OK) {
        EM_ERROR("Vulkan", "Failed to init local Vulkan frame context: %s", em_result_string(result, EMBER_BUILD_DEBUG));
        return result;
    }

    CHECK_VKRESULT(
        vkWaitForFences(context->logical_device, 1, &context->in_flight_fence, TRUE, UINT64_MAX),
        "Failed to wait in flight Vulkan fence");
    
	CHECK_VKRESULT(
		vkResetFences(context->logical_device,  1, &context->in_flight_fence),
		"Failed to reset in flight Vulkan fence");
    
    // Destroy and cleanup unused semaphores, use average of a some num. of frames.
    u32 new_size = context->semaphores_total_refresh / context->frames_in_flight;
    if (device->current_frame == context->frames_in_flight - 1 && new_size < darray_length(context->semaphore_pool)) {
        for (u32 i = new_size; i < darray_length(context->semaphore_pool); ++i) {
            vkDestroySemaphore(
                context->logical_device, 
                context->semaphore_pool[i], 
                context->allocator);
        }

        context->semaphore_pool = darray_resize(context->semaphore_pool, new_size);
    }

    for (u32 i = 0; i < darray_length(frame->managed_surfaces); ++i) {
        const emgpu_frame_surface* managed_surface = &frame->managed_surfaces[i];
        // TODO: Wait semaphore should probably be a list but i'm starting to get annoyed about memory usage.
        VkSemaphore* wait_semaphore =  &frame_context.vk_submissions[managed_surface->owner_submission_index].wait_semaphore;

        em_result result = vulkan_frame_new_semaphore(device, &frame_context, wait_semaphore);
        if (result != EMBER_RESULT_OK) return result;

        CHECK_VKRESULT(
            vulkan_surface_accquire(device, managed_surface->handle, UINT64_MAX, *wait_semaphore, VK_NULL_HANDLE), 
            "Failed to acquire swapchain image from managed surface")
    }

    for (u32 i = 0; i < darray_length(frame->submissions); ++i) {
        const emgpu_frame_submission* curr_api_submission = &frame->submissions[i];
        vulkan_frame_submission* curr_submission = &frame_context.vk_submissions[i];

        switch (curr_api_submission->ops_type) {
            case EMBER_OPER_TYPE_GRAPHICS:
                curr_submission->commandbuf = context->graphics_commandbuf;
                break;
            
            case EMBER_OPER_TYPE_COMPUTE:
                curr_submission->commandbuf = context->compute_commandbuf;
                break;

            default:
                EM_ASSERT(FALSE && "Unsupported device oper mode");
                break;
        }

        VkCommandBufferBeginInfo begin_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        CHECK_VKRESULT(
            vkBeginCommandBuffer(curr_submission->commandbuf, &begin_info),
            "Failed to begin Vulkan command buffer for new device mode");

        em_result result = vulkan_frame_process_submission(device, frame, &frame_context, i);

        vkEndCommandBuffer(curr_submission->commandbuf);
        if (result != EMBER_RESULT_OK) {
            EM_ERROR("Vulkan", "Failed to convert frame commands to Vulkan submissions: %s", em_result_string(result, EMBER_BUILD_DEBUG));
            return result;
        }
        
        // TODO: This is VERY bad for performance.
        VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };

        VkSubmitInfo submit_info = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submit_info.commandBufferCount   = 1;
        submit_info.pCommandBuffers      = &curr_submission->commandbuf;
        if (curr_submission->wait_semaphore) {
            submit_info.waitSemaphoreCount   = 1;
            submit_info.pWaitSemaphores      = &curr_submission->wait_semaphore;
            submit_info.pWaitDstStageMask    = wait_stages;
        }
        if (curr_submission->signal_semaphore) {
            submit_info.signalSemaphoreCount = 1;
            submit_info.pSignalSemaphores    = &curr_submission->signal_semaphore;
        }

        CHECK_VKRESULT(
            vkQueueSubmit(
                context->mode_queues[ops_type_to_queue_type(curr_api_submission->ops_type)].handle, 
                1, 
                &submit_info, 
                i == darray_length(frame_context.vk_submissions) - 1 ? 
                    context->in_flight_fence
                    : VK_NULL_HANDLE),
            "Failed to submit Vulkan command buffer to internal queue");
    }

    // Batch then present all the surface at once.
    // --------------------------------------
    VkSwapchainKHR* swapchains = mem_allocate(NULL, sizeof(VkSwapchainKHR) * darray_length(frame->managed_surfaces), MEMORY_TAG_TEMP);
    u32* image_indices = mem_allocate(NULL, sizeof(u32) * darray_length(frame->managed_surfaces), MEMORY_TAG_TEMP);

    for (u32 i = 0; i < darray_length(frame->managed_surfaces); ++i) {
        internal_vulkan_surface* internal_surface = (internal_vulkan_surface*)frame->managed_surfaces[i].handle->internal_data;
        swapchains[i] = internal_surface->swapchain;
        image_indices[i] = internal_surface->image_index;
    }

    VkPresentInfoKHR present_info = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    present_info.waitSemaphoreCount = darray_length(frame_context.present_semaphores);
    present_info.pWaitSemaphores    = frame_context.present_semaphores;
    present_info.swapchainCount     = darray_length(frame->managed_surfaces);
    present_info.pSwapchains        = swapchains;
    present_info.pImageIndices      = image_indices;
    // present_info.pResults = &present_results;

    CHECK_VKRESULT(
        vkQueuePresentKHR(context->mode_queues[VULKAN_QUEUE_TYPE_PRESENT].handle, &present_info),
        "Failed to present Vulkan surfaces during frame submission");
    // --------------------------------------

    context->semaphores_total_refresh += darray_length(context->semaphore_pool);
    device->current_frame = (device->current_frame + 1) % context->frames_in_flight;
    return EMBER_RESULT_OK;
}
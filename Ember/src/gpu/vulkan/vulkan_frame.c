#include "ember/core.h"
#include "vulkan_backend.h"

#include "ember/core/darray.h"
#include "ember/core/allocators.h"

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
    out_context->submissions = darray_create(vulkan_frame_submission, NULL, MEMORY_TAG_FRAME);
    out_context->managed_surfaces = darray_create(vulkan_frame_surface_entry, NULL, MEMORY_TAG_FRAME);
    out_context->wait_present_semaphores = darray_create(VkSemaphore, NULL, MEMORY_TAG_FRAME);
    out_context->frame_textures = mem_allocate(NULL, sizeof(emgpu_texture*) * (init_frame->current_resource_idx + 1), MEMORY_TAG_FRAME);

    return EMBER_RESULT_OK;
}

em_result vulkan_device_process_frame(emgpu_device* device, const emgpu_frame* frame, vulkan_frame_context* frame_context) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    em_result result = vulkan_frame_context_init(frame, frame_context);
    if (result != EMBER_RESULT_OK) {
        EM_ERROR("Vulkan", "Failed to init local Vulkan frame context: %s", em_result_string(result, EMBER_BUILD_DEBUG));
        return result;
    }

    vulkan_frame_submission* curr_submission = NULL;

    // Read commands in frame objects to submissions
    // --------------------------------------
    for (u32 i = 0; i < frame->curr_command_idx; ++i) {
        rendercmd_payload* payload = datastream_get(&frame->commands, i);

        if (frame_context->curr_mode != payload->hdr.command_mode && payload->hdr.command_mode != 0) {
            // End old state
            if (curr_submission != NULL) {
                vkEndCommandBuffer(curr_submission->handle);
            }

            // Start new state
            curr_submission = darray_push_empty(frame_context->submissions);
            frame_context->curr_mode = payload->hdr.command_mode;

            switch (frame_context->curr_mode) {
                case EMBER_DEVICE_MODE_GRAPHICS: 
                    curr_submission->handle = context->graphics_commandbuf;
                    curr_submission->queue = VULKAN_QUEUE_TYPE_GRAPHICS;
                    break;

                case EMBER_DEVICE_MODE_COMPUTE: 
                    curr_submission->handle = context->compute_commandbuf;
                    curr_submission->queue = VULKAN_QUEUE_TYPE_COMPUTE;
                    break;

                default:
                    EM_ASSERT(FALSE && "Unsupported renderer mode");
                    break;
		    }

            VkCommandBufferBeginInfo begin_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
            CHECK_VKRESULT(
                vkBeginCommandBuffer(curr_submission->handle, &begin_info),
                "Failed to begin Vulkan command buffer for new device mode");
        }

        switch (payload->hdr.type) {
            case RENDERCMD_SET_RENDERAREA: {
                VkViewport viewport = {};
                viewport.x = (f32)payload->set_renderarea.origin.x;
                viewport.y = (f32)(payload->set_renderarea.origin.y + payload->set_renderarea.size.height);
                viewport.width  = (f32)payload->set_renderarea.size.width;
                viewport.height = -(f32)payload->set_renderarea.size.height;
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
                vkCmdSetViewport(curr_submission->handle, 0, 1, &viewport);

                VkRect2D scissor = {};
                scissor.offset.x = payload->set_renderarea.origin.x;
                scissor.offset.y = payload->set_renderarea.origin.y;
                scissor.extent.width  = payload->set_renderarea.size.width;
                scissor.extent.height = payload->set_renderarea.size.height;
                vkCmdSetScissor(curr_submission->handle, 0, 1, &scissor);
                break;
            }

            case RENDERCMD_BIND_NEXT_SURFACE_TEXTURE: {
                vulkan_frame_surface_entry* surface_entry = darray_push_empty(frame_context->managed_surfaces);
                surface_entry->surface = payload->next_surface_texture.surface;
                vulkan_frame_new_semaphore(device, frame_context, &surface_entry->image_available_semaphore);

                // * NOTE: This works because 'next surface' is set as a graphics ops and creates a new submission.
                curr_submission->wait_semaphore = surface_entry->image_available_semaphore;
                vulkan_frame_new_semaphore(device, frame_context, &curr_submission->signal_semaphore);

                darray_push(frame_context->wait_present_semaphores, curr_submission->signal_semaphore);

                internal_vulkan_surface* internal_surface = (internal_vulkan_surface*)surface_entry->surface->internal_data;
                frame_context->frame_textures[payload->next_surface_texture.dst_texture] = &internal_surface->swapchain_images[internal_surface->image_index];   
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

                vkCmdBeginRenderPass(curr_submission->handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
                break;
            }
                
            case RENDERCMD_END_RENDERPASS: {
                vkCmdEndRenderPass(curr_submission->handle);
                break;
            }
            
            case RENDERCMD_MEMORY_BARRIER: {
                EM_ERROR("Vulkan", "Vulkan device backend does not yet implement memory barriers!");
                return EMBER_RESULT_UNIMPLEMENTED;
            }
                
            case RENDERCMD_BIND_PIPELINE: {
                internal_vulkan_pipeline* internal_pipeline = (internal_vulkan_pipeline*)payload->bind_pipeline.pipeline;
                VkPipelineBindPoint bind_point = ops_type_to_bind_point(payload->bind_pipeline.pipeline->type);

                vkCmdBindPipeline(curr_submission->handle, bind_point, internal_pipeline->handle);

                if (internal_pipeline->descriptor_sets)
                    vkCmdBindDescriptorSets(curr_submission->handle, bind_point, internal_pipeline->layout, 0, 1, &internal_pipeline->descriptor_sets[device->current_frame], 0, 0);
                
                // Bind vertex and index buffers.
                if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS && internal_pipeline->graphics.vertex_buffer) {
                    VkDeviceSize offset = 0;

                    internal_vulkan_buffer* vertex_buffer = (internal_vulkan_buffer*)internal_pipeline->graphics.vertex_buffer->internal_data;
                    vkCmdBindVertexBuffers(curr_submission->handle, 0, 1, &vertex_buffer->handle, &offset);

                    if (internal_pipeline->graphics.index_buffer) {
                        internal_vulkan_buffer* index_buffer = (internal_vulkan_buffer*)internal_pipeline->graphics.index_buffer->internal_data;
                        vkCmdBindIndexBuffer(curr_submission->handle, index_buffer->handle, offset, VK_INDEX_TYPE_UINT16); // TODO: Customize index type?
                    }
                }
                break;
            }
                
            case RENDERCMD_DRAW: {
                vkCmdDraw(curr_submission->handle,
                    payload->draw.vertex_count,
                    payload->draw.instance_count,
                    0, 0);
                break;
            }
                
            case RENDERCMD_DRAW_INDEXED: {
                vkCmdDrawIndexed(curr_submission->handle,
                    payload->draw_indexed.index_count,
                    payload->draw_indexed.instance_count,
                    0, 0, 0);
                break;
            }
                
            case RENDERCMD_DISPATCH: {
                vkCmdDispatch(curr_submission->handle,
                    payload->dispatch.group_size.x,
                    payload->dispatch.group_size.y,
                    payload->dispatch.group_size.z);
                break;
            }
        }
    }
    
    vkEndCommandBuffer(curr_submission->handle);
    return EMBER_RESULT_OK;
}

em_result vulkan_device_submit_frame(emgpu_device* device, const emgpu_frame* frame) {
    vulkan_context* context = (vulkan_context*)device->internal_context;
    
    vulkan_frame_context frame_context = {};

    em_result result = vulkan_device_process_frame(device, frame, &frame_context);
    if (result != EMBER_RESULT_OK) {
        EM_ERROR("Vulkan", "Failed to convert frame object to Vulkan submissions");
        return result;
    }

    // Begin frame...
    // --------------------------------------
    CHECK_VKRESULT(
        vkWaitForFences(context->logical_device, 1, &context->in_flight_fence, TRUE, UINT64_MAX),
        "Failed to wait in flight Vulkan fence");
    
	CHECK_VKRESULT(
		vkResetFences(context->logical_device,  1, &context->in_flight_fence),
		"Failed to reset in flight Vulkan fence");
    // --------------------------------------

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

    for (u32 i = 0; i < darray_length(frame_context.managed_surfaces); ++i) {
        vulkan_frame_surface_entry* surface_entry = &frame_context.managed_surfaces[i];

        CHECK_VKRESULT(
            vulkan_surface_accquire(
                device, 
                surface_entry->surface, 
                UINT64_MAX, 
                surface_entry->image_available_semaphore, 
                VK_NULL_HANDLE), 
            "Failed to acquire next surface image");
    }
    // --------------------------------------

    for (u32 i = 0; i < darray_length(frame_context.submissions); ++i) {
        const vulkan_frame_submission* submission = &frame_context.submissions[i];

        // TODO: This is VERY bad for performance.
        VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };

        VkSubmitInfo submit_info = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submit_info.waitSemaphoreCount   = 1;
        submit_info.pWaitSemaphores      = &submission->wait_semaphore;
        submit_info.pWaitDstStageMask    = wait_stages;
        submit_info.commandBufferCount   = 1;
        submit_info.pCommandBuffers      = &submission->handle;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores    = &submission->signal_semaphore;

        CHECK_VKRESULT(
            vkQueueSubmit(
                context->mode_queues[submission->queue].handle, 
                1, 
                &submit_info, 
                i == darray_length(frame_context.submissions) - 1 ? 
                    context->in_flight_fence
                    : VK_NULL_HANDLE),
            "Failed to submit Vulkan command buffer to internal queue");
    }

    // Batch then present all the surface at once.
    // --------------------------------------
    VkSwapchainKHR* swapchains = mem_allocate(NULL, sizeof(VkSwapchainKHR) * darray_length(frame_context.managed_surfaces), MEMORY_TAG_TEMP);
    u32* image_indices = mem_allocate(NULL, sizeof(u32) * darray_length(frame_context.managed_surfaces), MEMORY_TAG_TEMP);

    for (u32 i = 0; i < darray_length(frame_context.managed_surfaces); ++i) {
        internal_vulkan_surface* internal_surface = (internal_vulkan_surface*)frame_context.managed_surfaces[i].surface->internal_data;
        swapchains[i] = internal_surface->swapchain;
        image_indices[i] = internal_surface->image_index;
    }

    VkPresentInfoKHR present_info = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    present_info.waitSemaphoreCount = darray_length(frame_context.wait_present_semaphores);
    present_info.pWaitSemaphores    = frame_context.wait_present_semaphores;
    present_info.swapchainCount     = darray_length(frame_context.managed_surfaces);
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
#include "ember/core.h"
#include "vulkan_command_buffer.h"

VkResult vulkan_command_buffer_allocate(
    vulkan_context* context,
    vulkan_queue* owner,
    u32 command_buffers_count,
    vulkan_command_buffer* out_command_buffers) {
    em_memset(out_command_buffers, 0, sizeof(vulkan_command_buffer));
    out_command_buffers->buffer_count = command_buffers_count;
    out_command_buffers->owner = owner;

    out_command_buffers->handles = mem_allocate(NULL, sizeof(vulkan_command_buffer) * out_command_buffers->buffer_count, MEMORY_TAG_RENDERER);

    VkCommandBufferAllocateInfo allocate_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandPool = out_command_buffers->owner->pool;
    allocate_info.commandBufferCount = out_command_buffers->buffer_count;

    return vkAllocateCommandBuffers(
        context->logical_device, 
        &allocate_info, 
        out_command_buffers->handles);
}

void vulkan_command_buffer_free(
    vulkan_context* context,
    vulkan_command_buffer* command_buffer) {
    if (command_buffer->handles != NULL) {
        vkFreeCommandBuffers(context->logical_device, command_buffer->owner->pool, command_buffer->buffer_count, command_buffer->handles);
        mem_free(NULL, command_buffer->handles, sizeof(vulkan_command_buffer) * command_buffer->buffer_count, MEMORY_TAG_RENDERER);
    }
}

VkResult vulkan_command_buffer_begin(
    vulkan_command_buffer* command_buffer,
    u32 buffer_index,
    b8 is_single_use,
    b8 is_renderpass_continue,
    b8 is_simultaneous_use) {
    
    VkCommandBufferBeginInfo begin_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    if (is_single_use)          begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (is_renderpass_continue) begin_info.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    if (is_simultaneous_use)    begin_info.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    command_buffer->curr_buffer = buffer_index;

    return vkBeginCommandBuffer(
        command_buffer->handles[command_buffer->curr_buffer], 
        &begin_info);
}

VkResult vulkan_command_buffer_end(vulkan_command_buffer* command_buffer) {
    VkResult result = vkEndCommandBuffer(command_buffer->handles[command_buffer->curr_buffer]);
    command_buffer->curr_buffer = 0;

    return result;
}

VkResult vulkan_command_buffer_submit(
    vulkan_command_buffer* command_buffer,
    u32 buffer_index,
    u32 wait_semaphore_count, VkSemaphore* wait_semaphores,
    u32 signal_semaphore_count, VkSemaphore* signal_semaphores,
    VkPipelineStageFlags* wait_stages,
    VkFence fence) {
    VkSubmitInfo submit_info = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submit_info.pCommandBuffers = &command_buffer->handles[buffer_index];
    submit_info.commandBufferCount = 1;
    submit_info.waitSemaphoreCount = wait_semaphore_count;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.signalSemaphoreCount = signal_semaphore_count;
    submit_info.pSignalSemaphores = signal_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    return vkQueueSubmit(command_buffer->owner->handle, 1, &submit_info, fence);
}

void vulkan_command_buffer_reset(vulkan_command_buffer* command_buffer) {
}

VkResult vulkan_command_buffer_allocate_and_begin_single_use(
    vulkan_context* context,
    vulkan_queue* owner,
    vulkan_command_buffer* out_command_buffer) {
    VkResult result = vulkan_command_buffer_allocate(context, owner, 1, out_command_buffer);
    if (!vulkan_result_is_success(result)) return result;
    
    return vulkan_command_buffer_begin(out_command_buffer, 0, TRUE, FALSE, FALSE);
}

VkResult vulkan_command_buffer_end_single_use(
    vulkan_context* context,
    vulkan_command_buffer* command_buffer) {
    // End the command buffer.
    vulkan_command_buffer_end(command_buffer);

    // Submit the queue
    VkResult result = vulkan_command_buffer_submit(command_buffer, 0, 0, NULL, 0, NULL, NULL, VK_NULL_HANDLE);
    if (!vulkan_result_is_success(result)) return result;

    // Wait for it to finish
    result = vkQueueWaitIdle(command_buffer->owner->handle);
    if (!vulkan_result_is_success(result)) return result;

    // Free the command buffer.
    vulkan_command_buffer_free(context, command_buffer);
}
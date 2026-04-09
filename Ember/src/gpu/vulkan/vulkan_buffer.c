#include "ember/core.h"
#include "vulkan_buffer.h"

#include "vulkan_command_buffer.h"

em_result vulkan_buffer_create(
    emgpu_device* device, 
    const emgpu_buffer_config* config, 
    emgpu_buffer* out_buffer) {
    vulkan_context* context = (vulkan_context*)device->internal_context;
    
    out_buffer->internal_data = mem_allocate(sizeof(internal_vulkan_buffer), MEMORY_TAG_RENDERER);
    internal_vulkan_buffer* internal_buffer = (internal_vulkan_buffer*)out_buffer->internal_data;

    out_buffer->buffer_size = config->buffer_size;
    out_buffer->usage = config->usage;

    VkBufferCreateInfo buffer_create_info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    buffer_create_info.size = out_buffer->buffer_size;
    buffer_create_info.usage = vulkan_buffer_usage(context, config);
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    CHECK_VKRESULT(
        vkCreateBuffer(
            context->logical_device, 
            &buffer_create_info, 
            context->allocator, 
            &internal_buffer->handle), 
        "Failed to create Vulkan buffer");

    // Allocate memory for buffer.
    // --------------------------------------
    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(context->logical_device, internal_buffer->handle, &memory_requirements);

    VkMemoryPropertyFlags mem_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (config->usage & EMBER_BUFFER_USAGE_CPU_VISIBLE)
		mem_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    // Find index of memory compatible for provided buffer.
    i32 memory_index = find_memory_index(context, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (memory_index == -1) {
        EM_ERROR("Vulkan", "Failed to find required memory type for buffer");
        return EMBER_RESULT_OUT_OF_MEMORY_GPU;
    }

    VkMemoryAllocateInfo memory_allocate_info = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	memory_allocate_info.allocationSize = memory_requirements.size;
	memory_allocate_info.memoryTypeIndex = memory_index;

    CHECK_VKRESULT(
        vkAllocateMemory(context->logical_device, &memory_allocate_info, context->allocator, &internal_buffer->memory),
        "Failed to allocate Vulkan buffer memory");

    CHECK_VKRESULT(
        vkBindBufferMemory(context->logical_device, internal_buffer->handle, internal_buffer->memory, 0),
        "Failed to bind internal Vulkan memory to buffer");
    // --------------------------------------

    return EMBER_RESULT_OK;
}

em_result vulkan_buffer_upload(
    emgpu_device* device, 
    emgpu_buffer* buffer, 
    const void* data, 
    u64 start_offset, u64 region) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    internal_vulkan_buffer* internal_buffer = (internal_vulkan_buffer*)buffer->internal_data;

    if (buffer->usage & EMBER_BUFFER_USAGE_CPU_VISIBLE) {
        void* mapped_ptr = NULL;
        CHECK_VKRESULT(
            vkMapMemory(context->logical_device, internal_buffer->memory, start_offset, region, 0, &mapped_ptr),
            "Failed to map memory to buffer for upload");
        
        emc_memcpy(mapped_ptr, data, region);
        vkUnmapMemory(context->logical_device, internal_buffer->memory);
        return EMBER_RESULT_OK;
    }

    emgpu_buffer_config staging_buffer_config = emgpu_buffer_default();
    staging_buffer_config.usage = EMBER_BUFFER_USAGE_CPU_VISIBLE;
    staging_buffer_config.buffer_size = region;

    emgpu_buffer staging_buffer = {};
    em_result result = vulkan_buffer_create(device, &staging_buffer_config, &staging_buffer);
    if (result != EMBER_RESULT_OK) {
        EM_ERROR("Vulkan", "Failed to create staging buffer for buffer upload.");
        return result;
    }

    result = vulkan_buffer_upload(device, &staging_buffer, data, 0, staging_buffer_config.buffer_size);
    if (result != EMBER_RESULT_OK) {
        EM_ERROR("Vulkan", "Failed to upload data to buffer upload staging buffer.");
        return result;
    }

    vulkan_command_buffer command_buffer;
	CHECK_VKRESULT(
        vulkan_command_buffer_allocate_and_begin_single_use(
            context, 
            &context->mode_queues[VULKAN_QUEUE_TYPE_TRANSFER],
            &command_buffer), 
        "Failed to allocate internal Vulkan single-use command buffer to transfer buffer data");
    {
        internal_vulkan_buffer* internal_staging_buffer = (internal_vulkan_buffer*)staging_buffer.internal_data;
        
        VkBufferCopy copy_info = {};
		copy_info.dstOffset = start_offset;
		copy_info.size = region;

        vkCmdCopyBuffer(command_buffer.handles[command_buffer.curr_buffer], internal_staging_buffer->handle, internal_buffer->handle, 1, &copy_info);
		
        CHECK_VKRESULT(
            vulkan_command_buffer_end_single_use(
                context,
                &command_buffer),
            "Failed to transfer Vulkan texture data to GPU");
    }

    vulkan_buffer_destroy(device, &staging_buffer);
    return EMBER_RESULT_OK;
}

void vulkan_buffer_destroy(
    emgpu_device* device, 
    emgpu_buffer* buffer) {

}
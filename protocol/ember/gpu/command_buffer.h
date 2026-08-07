#pragma once 

#include "ember/core.h"

#include "ember/gpu/device.h"

/**
 * @brief Framebuffer handle local to a single GPU command buffer.
 *
 * This identifier representes any texture that is correctly controlled to
 * allow rendering directly to it through a renderpass. This allows that is only guaranteed
 * to be valid during the lifetime of the relevent command buffer. This ensures
 * syncronization and lifetime guarantees.
 */
typedef u32 emgpu_local_framebuffer;

/**
 * @brief Opaque resource handle local to a single GPU command buffer.
 *
 * Represents a transient resource binding within a frame. These handles
 * are only valid for the duration of the frame execution.
 */
typedef u32 emgpu_local_resource;

/**
 * @brief Represents a linear sequance of commands relevent to the current GPU device.
 *
 * An emgpu_commandbuffer acts as a transient container for all GPU commands
 * required to render or dispatch work for a single frame. It provides
 * a linear command recording model and manages frame-local resources.
 */
typedef struct emgpu_command_buffer {
    /** @brief Indicates whether the frame was successfully initialized. */
    b8 initialized;

    /** @brief Index used for allocating frame-local resources. */
    u32 current_resource_idx;
    
    /** @brief A reference to the allocator used to manage command buffer memory. */
    const em_allocator* allocator;

    /**
     * @brief Linear command buffer storing recorded GPU commands.
     *
     * Commands are appended during frame recording and later consumed
     * during submission.
     */
    void* commands_buf;
    
    u64 buffer_size, buffer_capacity;
} emgpu_command_buffer;

/**
 * @brief Submits a command buffer for GPU execution.
 *
 * @param device Pointer to the device instance.
 * @param queue Queue of execution to submit command buffer.
 * @param command_buf Pointer to command buffer.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeds.
 */
em_result emgpu_device_submit(const emgpu_device* device, emgpu_queue queue, const emgpu_command_buffer* command_buf);

/**
 * @brief Initializes a GPU command buffer for recording.
 *
 * @param device Pointer to the device instance.
 * @param out_command_buffer Pointer to the buffer to initialize.
 *
 * @return Ember result code; `EMBER_RESULT_OK` if succeds.
 */
em_result emgpu_command_buffer_create(const emgpu_device* device, emgpu_command_buffer* out_command_buffer);

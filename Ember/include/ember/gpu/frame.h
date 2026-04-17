#pragma once 

#include "ember/core.h"

#include "ember/core/allocators.h"

#include "ember/gpu/types.h"
#include "ember/gpu/resources.h"

typedef u32 emgpu_frame_texture;

/**
 * @brief Represents a single GPU frame used to record commands to the rendering frame.
 */
typedef struct emgpu_frame {
    /** @brief Internal command buffer storage. */
    freelist buffer; 

    /** @brief Current index of managed resources within the frame object. */
    u32 current_resource_idx;
} emgpu_frame;

/**
 * @brief Initializes a frame for recording commands.
 *
 * @param frame Pointer to the frame to initialize.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emgpu_frame_init(emgpu_frame* frame);

/**
 * @brief Validates the recorded commands in the frame.
 *
 * @param frame Pointer to the frame to validate.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emgpu_frame_validate(emgpu_frame* frame);

/**
 * @brief Adds a no-op (dummy) command to the frame.
 *
 * This is useful when a frame would otherwise contain no commands, as submitting an empty
 * frame or calling @ref emgpu_frame_validate will result in a validation failure.
 *
 * @param frame Pointer to the frame.
 */
void emgpu_frame_dummy(emgpu_frame* frame);

/**
 * @brief Adds a command to the frame object to retrieve a suitable surface texture to render to.
 * 
 * @param frame Pointer to the frame.
 * @param surface Surface to accquire render texture from.
 * @return A refrence to a texture independent of frame lifetime.
 */
emgpu_frame_texture emgpu_frame_next_surface_texture(emgpu_frame* frame, emgpu_surface* surface);

/**
 * @brief Set the renderable area for subsequent command.
 * 
 * @param frame Pointer to the frame.
 * @param origin Top-left corner of the set renderable area.
 * @param size Size of area to set for subsequent rendering
 * @param set_scissor Whetever to set scissor clip to area as well.
 * 
 * @note The command is not necessary for compute-based operations.
 */
void emgpu_frame_set_renderarea(emgpu_frame* frame, uvec2 origin, uvec2 size, b8 set_scissor);

/**
 * @brief Binds a render pass to the frame.
 *
 * @param frame Pointer to the frame.
 * @param renderpass Pointer to the render pass to bind.
 * @param render_texture Texture to the render the output data to.
 */
void emgpu_frame_bind_renderpass(emgpu_frame* frame, emgpu_renderpass* renderpass, emgpu_frame_texture render_texture);

/**
 * @brief Inserts a memory barrier between pipeline.
 *
 * Ensures proper synchronization between GPU operations by specifying
 * source and destination stages along with access masks.
 *
 * @param frame Pointer to the frame.
 * @param src_pipeline Source pipeline.
 * @param dst_pipeline Destination pipeline.
 * @param src_access Source access flags.
 * @param dst_access Destination access flags.
 */
void emgpu_frame_memory_barrier(
    emgpu_frame* frame,
    emgpu_pipeline* src_pipeline,
    emgpu_pipeline* dst_pipeline,
    emgpu_access_flags src_access,
    emgpu_access_flags dst_access);

/**
 * @brief Binds a render or compute pipeline to the frame.
 *
 * A pipeline defines a sequence of GPU operations on how to draw or dispatch.
 *
 * @param frame Pointer to the frame.
 * @param pipeline Pointer to the pipeline to begin.
 */
void emgpu_frame_bind_pipeline(emgpu_frame* frame, emgpu_pipeline* pipeline);

/**
 * @brief Issues an indexed draw call.
 *
 * @param frame Pointer to the frame.
 * @param index_count Number of indices to draw.
 * @param instance_count Number of instances to draw.
 */
void emgpu_frame_draw_indexed(emgpu_frame* frame, u32 index_count, u32 instance_count);

/**
 * @brief Dispatches a compute workload.
 *
 * @param frame Pointer to the frame.
 * @param group_size Number of workgroups in 3D space.
 */
void emgpu_frame_dispatch(emgpu_frame* frame, uvec3 group_size);
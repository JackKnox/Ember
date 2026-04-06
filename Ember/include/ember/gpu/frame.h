#pragma once 

#include "ember/core.h"

#include "ember/core/allocators.h"

#include "ember/gpu/types.h"
#include "ember/gpu/resources.h"

/**
 * @brief Represents a single GPU frame used to record commands to the rendering frame.
 */
typedef struct emgpu_frame {
    /** @brief Internal command buffer storage. */
    freelist buffer; 
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
 * @param frame Pointer to the frame to which the dummy command will be added.
 */
void emgpu_frame_dummy(emgpu_frame* frame);

/**
 * @brief Binds a render target to the frame.
 *
 * @param frame Pointer to the frame.
 * @param rendertarget Pointer to the render target to bind.
 */
void emgpu_frame_bind_rendertarget(emgpu_frame* frame, emgpu_rendertarget* rendertarget);

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
 * @brief Begins a pipeline within the frame.
 *
 * A pipeline defines a sequence of GPU operations on how to draw or dispatch.
 *
 * @param frame Pointer to the frame.
 * @param pipeline Pointer to the pipeline to begin.
 */
void emgpu_frame_begin_pipeline(emgpu_frame* frame, emgpu_pipeline* pipeline);

/**
 * @brief Issues a non-indexed draw call.
 *
 * @param frame Pointer to the frame.
 * @param vertex_count Number of vertices to draw.
 * @param instance_count Number of instances to draw.
 */
void emgpu_frame_draw(emgpu_frame* frame, u32 vertex_count, u32 instance_count);

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
 * @param group_size_x Number of workgroups in the X dimension.
 * @param group_size_y Number of workgroups in the Y dimension.
 * @param group_size_z Number of workgroups in the Z dimension.
 */
void emgpu_frame_dispatch(
    emgpu_frame* frame,
    u32 group_size_x,
    u32 group_size_y,
    u32 group_size_z);

/**
 * @brief Ends the current pipeline.
 *
 * Must be called after a matching begin_pipeline call.
 *
 * @param frame Pointer to the frame.
 */
void emgpu_frame_end_pipeline(emgpu_frame* frame);
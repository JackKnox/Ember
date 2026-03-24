#pragma once 

#include "defines.h"

#include "ember/core/allocators.h"

#include "types.h"
#include "resources.h"

/**
 * @brief Render command buffer.
 *
 * Collects rendering commands for deferred execution by the device.
 */
typedef struct emgpu_commandbuf {
    /** @brief True if no further commands may be written. */
    b8 finished;

    /** @brief Allocator backing the render command buffer. */
    freelist buffer;
} emgpu_commandbuf;

/**
 * @brief Resets the render command buffer without reallocating memory.
 *
 * Clears all recorded commands but preserves allocated capacity.
 *
 * @param cmd Pointer to the command buffer.
 */
void emgpu_commandbuf_begin(emgpu_commandbuf* cmd);

/**
 * @brief Destroys and frees memory associated with the command buffer.
 *
 * @param cmd Pointer to the command buffer.
 */
void emgpu_commandbuf_destroy(emgpu_commandbuf* cmd);

/**
 * @brief Binds a render target to a render command.
 *
 * This function sets the specified render target as the current target
 * for subsequent rendering operations associated with the given render command.
 *
 * @param cmd Pointer to the command buffer.
 * @param rendertarget Pointer to the render target to bind.
 */
void emgpu_commandbuf_bind_rendertarget(emgpu_commandbuf* cmd, emgpu_rendertarget* rendertarget);

/**
 * @brief Inserts a GPU memory barrier between two render stages.
 *
 * Ensures that memory writes performed in the source render stage
 * become visible to subsequent memory accesses in the destination
 * render stage.
 * 
 * @param cmd Pointer to the command buffer.
 * @param src_renderstage The render stage that performed the writes.
 * @param dst_renderstage The render stage that will perform subsequent reads/writes.
 * @param src_access The type of access performed in the source stage (typically write flags).
 * @param dst_access The type of access that will occur in the destination stage.
 */
void emgpu_commandbuf_memory_barrier(emgpu_commandbuf* cmd, emgpu_renderstage* src_renderstage, emgpu_renderstage* dst_renderstage, emgpu_access_flags src_access, emgpu_access_flags dst_access);

/**
 * @brief Begins a render stage.
 *
 * Subsequent draw or dispatch calls will use this stage.
 *
 * @param cmd Pointer to the command buffer.
 * @param renderstage Pointer to the render stage to bind.
 */
void emgpu_commandbuf_begin_renderstage(emgpu_commandbuf* cmd, emgpu_renderstage* renderstage);

/**
 * @brief Issues a non-indexed draw call.
 *
 * @param cmd Pointer to the command buffer.
 * @param vertex_count Number of vertices to draw.
 * @param instance_count Number of instances to render.
 */
void emgpu_commandbuf_draw(emgpu_commandbuf* cmd, u32 vertex_count, u32 instance_count);

/**
 * @brief Issues an indexed draw call.
 *
 * @param cmd Pointer to the command buffer.
 * @param index_count Number of indices to draw.
 * @param instance_count Number of instances to render.
 */
void emgpu_commandbuf_draw_indexed(emgpu_commandbuf* cmd, u32 index_count, u32 instance_count);

/**
 * @brief Dispatches a compute workload.
 *
 * @param cmd Pointer to the command buffer.
 * @param group_size_x Number of workgroups in X.
 * @param group_size_y Number of workgroups in Y.
 * @param group_size_z Number of workgroups in Z.
 */
void emgpu_commandbuf_dispatch(emgpu_commandbuf* cmd, u32 group_size_x, u32 group_size_y, u32 group_size_z);

/**
 * @brief Ends the current render stage.
 *
 * Finalizes state for the active renderstage.
 *
 * @param cmd Pointer to the command buffer.
 */
void emgpu_commandbuf_end_renderstage(emgpu_commandbuf* cmd);

/**
 * @brief Finalizes the command buffer.
 *
 * Prevents further modification and marks it ready
 * for consumption by the renderer backend.
 *
 * @param cmd Pointer to the command buffer.
 */
void emgpu_commandbuf_end(emgpu_commandbuf* cmd);
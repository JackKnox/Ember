#pragma once 

#include "ember/core.h"

#include "ember/core/datastream.h"

#include "ember/gpu/types.h"
#include "ember/gpu/resources.h"

/**
 * @brief Handle to a texture used within a frame.
 *
 * This is an opaque identifier referencing a texture resource
 * that is valid only within the scope of a single emgpu_frame.
 */
typedef u32 emgpu_frame_texture;

/**
 * @brief Describes a contiguous batch of recorded GPU commands.
 *
 * A submission groups a sequence of commands that share the same
 * execution context or pipeline type. Backends may translate each
 * submission into a single command buffer submission or similar unit.
 */
typedef struct emgpu_frame_submission {
    /** @brief Index of the first command in the command stream. */
    u32 start_index;

    /** @brief Number of commands in this submission. */
    u32 submission_length;

    /** @brief Type of operations contained in this submission (graphics, compute, transfer, etc.). */
    emgpu_ops_type ops_type;
} emgpu_frame_submission;

/**
 * @brief Tracks a surface used during a frame.
 *
 * Surfaces are managed per-frame to ensure proper synchronization
 * and lifetime handling. Each surface is associated with the submission
 * that owns or last used it.
 */
typedef struct emgpu_frame_surface {
    /** @brief Pointer to the underlying surface resource. */
    emgpu_surface* handle;

    /** @brief Index of the submission that owns or last accessed this surface. */
    u32 owner_submission_index;
} emgpu_frame_surface;

/**
 * @brief Represents a single GPU frame for command recording.
 *
 * An emgpu_frame acts as a transient container for all commands,
 * resources, and synchronization data required to build and submit
 * GPU work for one frame.
 *
 * The frame owns:
 * - A linear command stream (datastream)
 * - Submission groupings for execution
 * - Transient resource tracking (e.g., surfaces)
 */
typedef struct emgpu_frame {
    /** @brief Indicates whether emgpu_frame_init() completed successfully. */
    b8 initied;

    /** @brief Current index for allocating frame-local resources. */
    u32 current_resource_idx;

    /** @brief Allocator used for all frame-local allocations. */
    ember_allocator* local_allocator;

    /** @brief Linear buffer storing all recorded commands. */
    datastream commands;

    // TODO: Remove. Two points of truth for datastream.
    /** @brief Current command index within the command stream. */
    u32 curr_command_idx;

    /** @brief Array of recorded submissions for this frame. */
    emgpu_frame_submission* submissions;

    /** @brief Array of surfaces tracked and used during this frame. */
    emgpu_frame_surface* managed_surfaces;
} emgpu_frame;

struct emgpu_device;

/**
 * @brief Initializes a frame for recording commands.
 *
 * @param frame Pointer to the frame to initialize.
 * @param device Rendering device to attach frame to.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emgpu_frame_init(emgpu_frame* frame, struct emgpu_device* device);

/**
 * @brief Validates the recorded commands in the frame.
 *
 * May print warnings messages for common pitfalls.
 * 
 * @param frame Pointer to the frame to validate.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emgpu_frame_validate(const emgpu_frame* frame);

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
 * @brief Imports a texture into the frame to ensure lifetime and assigns a id.
 * 
 * @param frame Pointer to the frame.
 * @param texture Texture to import into frame.
 * @return A refrence to a texture independent of frame lifetime.
 */
emgpu_frame_texture emgpu_frame_import_texture(emgpu_frame* frame, emgpu_texture* texture);

/**
 * @brief Set the renderable area for subsequent command.
 * 
 * @param frame Pointer to the frame.
 * @param origin Top-left corner of the set renderable area.
 * @param size Size of area to set for subsequent rendering
 * 
 * @note The command is not necessary for compute-based operations.
 */
void emgpu_frame_set_renderarea(emgpu_frame* frame, uvec2 origin, uvec2 size);

/**
 * @brief Begins a render pass within the frame.
 *
 * @param frame Pointer to the frame.
 * @param renderpass Pointer to the render pass to bind.
 * @param texture_attachments Array of texture to output render data to.
 * @param attachment_count Number of texture attachments to render to.
 */
void emgpu_frame_begin_renderpass(emgpu_frame* frame, emgpu_renderpass* renderpass, emgpu_frame_texture* texture_attachments, u32 attachment_count);

/**
 * @brief Ends a render pass within the frame.
 * 
 * @param frame Pointer to the frame.
 */
void emgpu_frame_end_renderpass(emgpu_frame* frame);

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
 * @brief Binds a pipeline to the frame.
 *
 * A pipeline defines a sequence of GPU operations on how to draw, dispatch or trace.
 *
 * @param frame Pointer to the frame.
 * @param pipeline Pointer to the pipeline to begin.
 */
void emgpu_frame_bind_pipeline(emgpu_frame* frame, emgpu_pipeline* pipeline);

/**
 * @brief Issues a graphics draw call.
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
 * @param group_size Number of workgroups in 3D space.
 */
void emgpu_frame_dispatch(emgpu_frame* frame, uvec3 group_size);
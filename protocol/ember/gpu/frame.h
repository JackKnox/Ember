#pragma once 

#include "ember/core.h"
#include "ember/core/datastream.h"

#include "ember/gpu/types.h"

#include "ember/gpu/device.h"
#include "ember/gpu/raster.h"
#include "ember/gpu/compute.h"

/**
 * @brief Opaque handle to a texture valid within a single GPU frame.
 *
 * This identifier references a texture resource that is only guaranteed
 * to be valid during the lifetime of the emgpu_frame in which it was created
 * or imported.
 */
typedef u32 emgpu_frame_texture;

/**
 * @brief Opaque handle to a frame-local GPU resource reference.
 *
 * Represents a transient resource binding within a frame. These handles
 * are only valid for the duration of the frame execution.
 */
typedef u32 emgpu_frame_resource;

/**
 * @brief Represents a single GPU frame used for command recording and submission.
 *
 * An emgpu_frame acts as a transient container for all GPU commands
 * required to render or dispatch work for a single frame. It provides
 * a linear command recording model and manages frame-local resources.
 */
typedef struct emgpu_frame {
    /** @brief Indicates whether the frame was successfully initialized. */
    b8 initialized;

    /** @brief Index used for allocating frame-local resources. */
    u32 current_resource_idx;

    /**
     * @brief Linear command buffer storing recorded GPU commands.
     *
     * Commands are appended during frame recording and later consumed
     * during submission.
     */
    datastream commands;
} emgpu_frame;

/**
 * @brief Submits a frame for execution on the GPU.
 *
 * @param device Pointer to the device instance.
 * @param frame Frame containing recorded commands.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emgpu_device_submit_frame(
    emgpu_device* device, 
    const emgpu_frame* frame);

/**
 * @brief Initializes a GPU frame for command recording.
 *
 * @param frame Pointer to the frame to initialize.
 * @param allocator Allocator used for frame-local memory.
 *
 * @return EMBER_RESULT_OK on success, or an error code on failure.
 */
em_result emgpu_frame_init(emgpu_frame* frame, em_allocator* allocator);

/**
 * @brief Validates recorded commands within a frame.
 *
 * Performs sanity checks on recorded GPU commands and may emit warnings
 * for common misuse patterns (e.g., missing render pass, invalid state transitions).
 *
 * @param frame Pointer to the frame to validate.
 *
 * @return EMBER_RESULT_OK if validation succeeds, otherwise an error code.
 */
em_result emgpu_frame_validate(const emgpu_frame* frame);

/**
 * @brief Inserts a no-op command into the frame.
 *
 * Useful for ensuring a frame contains at least one command when required
 * by backend validation rules.
 *
 * @param frame Pointer to the frame.
 */
void emgpu_frame_dummy(emgpu_frame* frame);

/**
 * @brief Acquires the next available surface texture for rendering.
 *
 * Enqueues a presentation acquisition operation and returns a frame-local
 * reference to the acquired surface texture.
 *
 * @param frame Pointer to the frame.
 * @param surface Surface to acquire the next presentation image from.
 *
 * @return A frame-local texture reference valid for the duration of the frame.
 */
emgpu_frame_texture emgpu_frame_next_surface_texture(emgpu_frame* frame, emgpu_surface* surface);

/**
 * @brief Imports a persistent texture into the frame.
 *
 * Registers an external GPU texture for use within the frame and returns
 * a frame-local reference to it.
 *
 * @param frame Pointer to the frame.
 * @param texture Pointer to the GPU texture to import.
 *
 * @return A frame-local texture reference.
 */
emgpu_frame_texture emgpu_frame_import_texture(emgpu_frame* frame, emgpu_texture* texture);

/**
 * @brief Sets the renderable area for subsequent rendering commands.
 *
 * Defines the viewport/render area used by following graphics commands.
 * This does not affect compute operations.
 *
 * @param frame Pointer to the frame.
 * @param origin Top-left coordinate of the render area.
 * @param size Dimensions of the render area.
 */
void emgpu_frame_set_renderarea(emgpu_frame* frame, uvec2 origin, uvec2 size);

/**
 * @brief Begins a render pass.
 *
 * Binds a render pass and attaches output textures for rendering.
 *
 * @param frame Pointer to the frame.
 * @param renderpass Render pass to begin.
 * @param clear_colour Clear colour to current renderarea.
 * @param texture_attachments Array of frame-local texture references.
 * @param attachment_count Number of attachments in the array.
 */
void emgpu_frame_begin_renderpass(emgpu_frame* frame, emgpu_renderpass* renderpass, u32 clear_colour, emgpu_frame_texture* texture_attachments, u32 attachment_count);

/**
 * @brief Ends the currently active render pass.
 *
 * @param frame Pointer to the frame.
 */
void emgpu_frame_end_renderpass(emgpu_frame* frame);

/**
 * @brief Binds a graphics or compute pipeline.
 *
 * Optionally binds vertex and index buffers for graphics pipelines.
 *
 * @param frame Pointer to the frame.
 * @param pipeline Pipeline to bind.
 * @param vertex_buffer_count Optional number of vertex buffers.
 * @param vertex_buffer Optional vertex buffer.
 * @param index_buffer Optional index buffer.
 */
void emgpu_frame_bind_pipeline(emgpu_frame* frame, emgpu_pipeline* pipeline, u32 vertex_buffer_count, emgpu_buffer* vertex_buffer, emgpu_buffer* index_buffer);

/**
 * @brief Issues a non-indexed draw call.
 *
 * @param frame Pointer to the frame.
 * @param vertex_count Number of vertices to draw.
 * @param instance_count Number of instances to draw.
 * @note Whetever a index buffer was bound indicates whetever its a indexed call.
 */
void emgpu_frame_draw(emgpu_frame* frame, u32 vertex_count, u32 instance_count);

/**
 * @brief Dispatches a compute workload.
 *
 * @param frame Pointer to the frame.
 * @param group_size Number of compute workgroups in XYZ dimensions.
 */
void emgpu_frame_dispatch(emgpu_frame* frame, uvec3 group_size);

/**
 * @brief Exports a frame-local resource for use in subsequent commands.
 *
 * Creates a transient GPU resource reference with a defined access mode.
 *
 * @param frame Pointer to the frame.
 * @param descriptor_index Binding or descriptor slot index.
 * @param resource_access Access flags defining usage (read/write/compute/etc).
 *
 * @return Frame-local resource reference.
 */
emgpu_frame_resource emgpu_frame_export_resource(emgpu_frame* frame, u32 descriptor_index, emgpu_access_flags resource_access);

/**
 * @brief Binds a previously exported or imported frame resource.
 *
 * Makes a frame-local resource available at a descriptor binding point
 * with specified access permissions.
 *
 * @param frame Pointer to the frame.
 * @param resource Frame-local resource handle.
 * @param descriptor_index Binding slot index.
 * @param resource_access Required access mode for this binding.
 */
void emgpu_frame_use_resource(emgpu_frame* frame, emgpu_frame_resource resource, u32 descriptor_index, emgpu_access_flags resource_access);

/**
 * @brief Finalizes command recording and submits the frame to the GPU.
 *
 * Flushes recorded commands and schedules execution on the device queue.
 *
 * @param frame Pointer to the frame.
 */
void emgpu_frame_flush(emgpu_frame* frame);
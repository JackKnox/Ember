#pragma once

#include "ember/core.h"

#include "ember/gpu/types.h"

#include "ember/gpu/device.h"
#include "ember/gpu/resources.h"

#include "ember/gpu/command_buffer.h"

/**
 * @brief Describes a single render pass attachment.
 */
typedef struct emgpu_colour_attachment {
    /** @brief Local framebuffer to render all output to. */
    emgpu_local_framebuffer framebuffer;

    /** @brief Load operation for colour or depth aspect. */
    emgpu_load_op load_op;

    /** @brief Store operation for colour or depth aspect. */
    emgpu_store_op store_op;

    /**
     * @brief Load operation for stencil aspect.
     * Only relevant for stencil or depth-stencil attachments.
     */
    emgpu_load_op stencil_load_op;

    /**
     * @brief Store operation for stencil aspect.
     * Only relevant for stencil or depth-stencil attachments.
     */
    emgpu_store_op stencil_store_op;
    
    /** @brief Default colour of the output framebuffer. */
    u32 clear_colour;

    /** @brief Compatible with rendering to a surface object. */
    b8 presentable;
} emgpu_colour_attachment;

/**
 * @brief Configuration for a command buffer renderpass.
 *
 * A renderpass is a context of execution within a command buffer
 * specifically for using the rendering capabilities of the GPU and uses
 * the Graphics Pipeline.
 */
typedef struct emgpu_renderpass_config {
    /** @brief Refrence to extra configuration structure specific to API type. */
    void* api_next;

    /** @brief Attachments for colour data output. */
    const emgpu_colour_attachment* colour_attachments;

    /** @brief Number of colour attachments. */
    u32 colour_attachment_count;
} emgpu_renderpass_config;

#ifdef EMBER_DEFINE_HELPERS

/**
 * @brief Creates a default-initialized renderpass configuration.
 *
 * @return A default-initialized emgpu_renderpass_config.
 */
emgpu_renderpass_config emgpu_renderpass_default();

#endif

/**
 * @brief Begins a renderpass within the given command buffer.
 *
 * @param command_buf Pointer to the command buffer.
 * @param config Renderpass configuration.
 */
void emgpu_cmd_begin_renderpass(emgpu_command_buffer* command_buf, const emgpu_renderpass_config* config);

/**
 * @brief Ends the currently active render pass.
 *
 * @param command_buf Pointer to the command buffer.
 */
void emgpu_cmd_end_renderpass(emgpu_command_buffer* command_buf);

/**
 * @brief Sets the viewport for subsequent rendering commands.
 *
 * Defines the viewport used by following graphics commands.
 * This does not affect compute operations.
 *
 * @param command_buf Pointer to the command buffer.
 * @param origin Top-left coordinate of the viewport.
 * @param size Dimensions of the viewport.
 * @param min_depth Mask for minimum depth displayed by the GPU. 
 * @param max_depth Mask for maximum depth displayed by the GPU.
 */
void emgpu_cmd_set_viewport(emgpu_command_buffer* command_buf, uvec2 origin, uvec2 size, f32 min_depth, f32 max_depth);

/**
 * @brief Set the scissor for subsequent rendering commands.
 *
 * Defines the scissor used by following graphics commands.
 * This does not affect compute operations.
 *
 * @param command_buf Pointer to the command buffer.
 * @param origin Top-left coordinate of the scissor.
 * @param size Dimensions of the scissor.
 */
void emgpu_cmd_set_scissor(emgpu_command_buffer* command_buf, uvec2 origin, uvec2 size);

/**
 * @brief Configuration for rasterization blending state.
 * 
 * Defines how polygons are blended together with existing ones.
 */
typedef struct emgpu_raster_blend_config {
    /** @brief Source blend factors for colour components. */
    emgpu_blend_factor src_colour;

    /** @brief Destination blend factors for colour components. */
    emgpu_blend_factor dst_colour;

    /** @brief Blend operation applied to colour components. */
    emgpu_blend_op colour_op;

    /** @brief Source blend factors for alpha component. */
    emgpu_blend_factor src_alpha;

    /** @brief Destination blend factors for alpha component. */
    emgpu_blend_factor dst_alpha;

    /** @brief Blend operation applied to alpha component. */
    emgpu_blend_op alpha_op;
} emgpu_raster_blend_config;

#ifdef EMBER_DEFINE_HELPERS

/**
 * @brief Creates a default raster blending configuration.
 *
 * @return A default-initialized emgpu_raster_blend_config.
 */
emgpu_raster_blend_config emgpu_raster_blend_default();

#endif

/**
 * @brief Configuration for rasterization vertex input.
 * 
 * Defines how raw vertices are transformed into renderable polygons.
 */
typedef struct emgpu_raster_vertex_config {
    /** @brief Describes how vertices are turned into primitives. */
    emgpu_primitive_type topology;
    
    /** @brief Number of active vertex attributes. */
    u32 attribute_count;
    
    /** @brief Vertex attribute formats in binding order. */
    emgpu_format* attributes;
} emgpu_raster_vertex_config;

#ifdef EMBER_DEFINE_HELPERS

/**
 * @brief Creates a default raster vertex configuration.
 *
 * @return A default-initialized emgpu_raster_vertex_config.
 */
emgpu_raster_vertex_config emgpu_raster_vertex_default();

#endif

/**
 * @brief Configuration for a raster pipeline.
 *
 * Defines the shader layout, vertex input layout, and optional
 * vertex/index buffers used when creating a raster pipeline.
 */
typedef struct emgpu_raster_pipeline_config {
    /** @brief Refrence to extra configuration structure specific to API type. */
    void* api_next;

    /** @brief Shader stage ran for per-vertex operations, must be valid. */
    emgpu_shader_src vertex_shader;

    /** @brief Optional shader stage ran for per-fragment (pixel) operations. */
    emgpu_shader_src fragment_shader;

    /** @brief Number of active descriptor bindings. */
	u32 descriptor_count;

    /** @brief Descriptor binding descriptions used by the pipeline. */
    emgpu_descriptor_desc* descriptors;

    /** @brief Blending configuration, must not be NULL for blending to be enabled. */
    emgpu_raster_blend_config* blend_state;

    /** @brief Vertex input configuration, must not be NULL to enable rasterization. */
    emgpu_raster_vertex_config* vertex_input;
} emgpu_raster_pipeline_config;

/**
 * @brief Info for binding a raster pipeline to the current renderpass.
 */
typedef struct emgpu_raster_bind_info {
    /** @brief Connected pipeline to computepass. */
    const emgpu_pipeline* pipeline;

    /** @brief Resources to export from pipeline. */
    const emgpu_resource_export* export_resources;

    /** @brief Number of export resources. */
    u32 export_resource_count;

    /** @brief Resources to import into pipeline. */
    const emgpu_resource_import* import_resources;

    /** @brief Number of import resources. */
    u32 import_resource_count;
} emgpu_raster_bind_info;

#ifdef EMBER_DEFINE_HELPERS

/**
 * @brief Creates a default raster stage configuration.
 *
 * @return A default-initialized emgpu_raster_pipeline_config.
 */
emgpu_raster_pipeline_config emgpu_pipeline_default_raster();

#endif

/**
 * @brief Creates a raster pipeline.
 *
 * @param device Pointer to the device instance.
 * @param allocator Allocator used to manage device memory.
 * @param config Pipeline configuration.
 * @param out_pipeline Output pipeline.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emgpu_raster_pipeline_create(
    const emgpu_device* device, 
    const em_allocator* allocator, 
    const emgpu_raster_pipeline_config* config, 
    emgpu_pipeline* out_pipeline);

/**
 * @brief Binds a raster pipeline.
 *
 * @param command_buf Pointer to the command buffer.
 * @param pipeline Pipeline to bind.
 */
void emgpu_cmd_bind_raster_pipeline(emgpu_command_buffer* command_buf, emgpu_raster_bind_info* bind_info);

/**
 * @brief Issues a draw call.
 *
 * @param command_buf Pointer to the command buffer.
 * @param vertex_count Number of vertices to draw.
 * @param instance_count Number of instances to draw.
 *
 * @note Whetever a index buffer was bound
 *       indicates whetever its a indexed call.
 */
void emgpu_cmd_draw(emgpu_command_buffer* command_buf, u32 vertex_count, u32 instance_count);


#pragma once

#include "ember/core.h"

#include "ember/gpu/types.h"

#include "ember/gpu/device.h"
#include "ember/gpu/resources.h"

#include "ember/gpu/surface.h"

/**
 * @brief Describes a single render pass attachment.
 */
typedef struct emgpu_attachment_config {
    /** @brief Logical attachment type (colour, depth, stencil, etc.). */
    emgpu_attachment_type type;

    /** @brief Library-defined pixel format. Must be compatible with the attachment type. */
    emgpu_format format;

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

    /** @brief Compatible with rendering to a surface object. */
    b8 presentable;
} emgpu_attachment_config;

/**
 * @brief Returns a basic attachment guaranteed to work on a surface.
 */
emgpu_attachment_config emgpu_attachment_from_surface(emgpu_surface* surface);

/**
 * @brief Configuration for a renderpass.
 *
 * Contains attachments, image layout transitions
 * and attachment images.
 */
typedef struct emgpu_renderpass_config {
    /** @brief Refrence to extra configuration structure specific to API type. */
    void* api_next;

    /** @brief Number of attachments attached to the renderpass. */
    u32 attachment_count;

    /** @brief Attachments created within the renderpass. */
    emgpu_attachment_config* attachments;
} emgpu_renderpass_config;

/**
 * @brief Represents a render pass used by the renderer backend.
 *
 * A render pass is the blueprint for rendering operations. It can either
 * represent a window-backed surface (such as a swapchain image) or an
 * offscreen render pass.
 */
typedef struct emgpu_renderpass {
    /** @brief Backend-specific internal data. */
    void* internal_data;

    /** @brief Clear colour value used when beginning a render pass. */
    u32 clear_colour;

    /** @brief Number of attachments attached to the renderpass. */
    u32 attachment_count;
} emgpu_renderpass;

/**
 * @brief Creates a default-initialized renderpass configuration.
 *
 * @return A default-initialized emgpu_renderpass_config.
 */
emgpu_renderpass_config emgpu_renderpass_default();

/**
 * @brief Creates a render pass.
 *
 * @param device Pointer to the device instance.
 * @param allocator Allocator used to manage device memory.
 * @param config Render pass configuration.
 * @param out_renderpass Output render pass.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emgpu_device_create_renderpass(
    emgpu_device* device, 
    em_allocator* allocator, 
    const emgpu_renderpass_config* config, 
    emgpu_renderpass* out_renderpass);

/**
 * @brief Destroys a render pass.
 *
 * @param device Pointer to the device instance.
 * @param allocator Allocator used to manage device memory.
 * @param renderpass Pass to destroy.
 */
void emgpu_device_destroy_renderpass(
    emgpu_device* device, 
    em_allocator* allocator, 
    emgpu_renderpass* renderpass);

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

/**
 * @brief Creates a default raster blending configuration.
 *
 * @return A default-initialized emgpu_raster_blend_config.
 */
emgpu_raster_blend_config emgpu_raster_blend_default();

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

/**
 * @brief Creates a default raster vertex configuration.
 *
 * @return A default-initialized emgpu_raster_vertex_config.
 */
emgpu_raster_vertex_config emgpu_raster_vertex_default();

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
 * @brief Creates a default raster stage configuration.
 *
 * @return A default-initialized emgpu_raster_pipeline_config.
 */
emgpu_raster_pipeline_config emgpu_pipeline_default_raster();

/**
 * @brief Creates a raster pipeline.
 *
 * @param device Pointer to the device instance.
 * @param allocator Allocator used to manage device memory.
 * @param config Pipeline configuration.
 * @param bound_renderpass Render pass the pipeline is compatible with.
 * @param out_pipeline Output pipeline.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emgpu_device_create_raster_pipeline(
    emgpu_device* device, 
    em_allocator* allocator, 
    const emgpu_raster_pipeline_config* config, 
    emgpu_renderpass* bound_renderpass, 
    emgpu_pipeline* out_pipeline);
#pragma once

#include "ember/core.h"

#include "ember/gpu/types.h"

#include <ember/platform/window.h>

/**
 * @brief Configuration for a surface that connects to 
 * a platform window.
 * 
 * Defines a refrence to a window and a prefered/required 
 * format for the surface.
 */
typedef struct emgpu_surface_config {
    // TODO: Change to some sort of platform_data struct to decouple from emplat_*
    /** @brief Window to attach render surface to. */
    emplat_window* window;

    /** @brief Requested format of the surface texture. */
    emgpu_format preferred_format;

    /** @brief Whetever to exit if exact preferred format isn't found. */
    b8 force_format;
} emgpu_surface_config;

/**
 * @brief Creates a default surface configuration.
 *
 * @return A default-initialized emgpu_surface_config.
 */
emgpu_surface_config emgpu_surface_default();

/**
 * @brief Backend-agnostic GPU surface objects.
 *
 * Represents a backend-agnsotic object that connectes a platform surface.
 */
typedef struct emgpu_surface {
    /** @brief Backend-specific internal data. */
    void* internal_data;

    /** @brief Format of the pixel(s) attachted to the platform surface. */
    emgpu_format pixel_format;

    /** @brief Number of owned images used for concurrent rendering. */
    u32 image_count;
} emgpu_surface;

/**
 * @brief Configuration for a render buffer.
 *
 * Defines a GPU buffer such as a vertex, index, 
 * uniform, or storage buffer.
 */
typedef struct emgpu_buffer_config {
    /** @brief Intended usage of the buffer (vertex, index, uniform, etc.). */
    emgpu_buffer_usage usage;

    /** @brief Total size of the buffer in bytes. */
    u64 buffer_size;
} emgpu_buffer_config;

/**
 * @brief Backend-agnostic GPU buffer handle.
 *
 * Represents a GPU-resident buffer resource.
 */
typedef struct emgpu_buffer {
    /** @brief Total size of the buffer in bytes. */
    u64 buffer_size;

    /** @brief Current usage of the buffer. */
    emgpu_buffer_usage usage;

    /** @brief Backend-specific buffer state/handle. */
    void* internal_data;
} emgpu_buffer;

/**
 * @brief Creates a default render buffer configuration.
 *
 * @return A default-initialized emgpu_buffer_config.
 */
emgpu_buffer_config emgpu_buffer_default();

/**
 * @brief Configuration for a graphics pipeline.
 *
 * Defines the shader layout, vertex input layout, and optional
 * vertex/index buffers used when creating a graphics pipeline.
 */
typedef struct emgpu_graphics_pipeline_config {
    /** @brief Describes how vertices are turned into primitives. */
    emgpu_primitive_type vertex_topology;

    /** @brief Number of active vertex attributes. */
    u32 vertex_attribute_count;
    
    /** @brief Vertex attribute formats in binding order. */
    emgpu_format* vertex_attributes;

    /** @brief Optional vertex buffer bound to this pipeline. */
    emgpu_buffer* vertex_buffer;

    /** @brief Optional index buffer bound to this pipeline. */
    emgpu_buffer* index_buffer;

    /** @brief Number of active descriptor bindings. */
	u32 descriptor_count;

    /** @brief Descriptor binding descriptions used by the pipeline. */
    emgpu_descriptor_desc* descriptors;

    /** @brief Shader stage ran for per-vertex operations, must be valid. */
    emgpu_shader_src vertex_shader;

    /** @brief Optional shader stage ran for per-fragment (pixel) operations. */
    emgpu_shader_src fragment_shader;
} emgpu_graphics_pipeline_config;

/**
 * @brief Configuration for a compute pipeline.
 */
typedef struct emgpu_compute_pipeline_config {
    /** @brief Number of active descriptor bindings. */
	u32 descriptor_count;

    /** @brief Descriptor binding descriptions used by the pipeline. */
    emgpu_descriptor_desc* descriptors;

    /** @brief Shader stage ran per-compute cell, must be valid. */
    emgpu_shader_src compute_shader;
} emgpu_compute_pipeline_config;

/**
 * @brief Backend-agnostic GPU pipeline handle.
 *
 * Defines a GPU pipeline internally coupled with
 * descriptors or a vertex/index buffer.
 */
typedef struct emgpu_pipeline {
    // TODO: Remove this as emgpu_device_mode is technically a bitmask
    /** @brief Type / supported mode of the pipeline. */
    emgpu_device_mode pipeline_type;

    /** @brief Backend-specific pipeline or program data. */
    void* internal_data;
} emgpu_pipeline;

/**
 * @brief Creates a default graphics stage configuration.
 *
 * @return A default-initialized emgpu_graphics_pipeline_config.
 */
emgpu_graphics_pipeline_config emgpu_pipeline_default_graphics();

/**
 * @brief Creates a default compute stage configuration.
 *
 * @return A default-initialized emgpu_compute_pipeline_config.
 */
emgpu_compute_pipeline_config emgpu_pipeline_default_compute();

/**
 * @brief Configuration for a texture.
 *
 * Defines a sampled image stored on the GPU.
 */
typedef struct emgpu_texture_config {
    /** @brief Image format of the texture data. */
    emgpu_format image_format;

    /** @brief Texture filtering mode. */
    emgpu_filter_type filter_type;

    /** @brief Texture address (wrap) mode. */
    emgpu_address_mode address_mode;

    /** @brief Intended usage of texture after creation. */
    emgpu_texture_usage usage;

    /** @brief Dimensions of the texture in pixels. */
    uvec2 size;

    /** @brief Refrence to extra configuration structure specific to API type. */
    void* api_next;

    /** 
     * @brief Max anisotropy of attached sampler in backend.
     * 
     * @note Ignored if texture is not sampled or equals zero. 
     */
    f32 max_anisotropy;
} emgpu_texture_config;

/**
 * @brief Backend-agnostic GPU texture handle.
 *
 * Represents GPU image data along with sampling configuration.
 * Converted internally into API-specific image and sampler objects.
 */
typedef struct emgpu_texture {
    /** @brief Image format of the texture data. */
    emgpu_format image_format;

    /** @brief Backend-specific image and sampler state. */
    void* internal_data;

    /** @brief Dimensions of the texture in pixels. */
    uvec2 size;
} emgpu_texture;

/**
 * @brief Creates a default-initialized texture configuration.
 *
 * @return A default texture configuration.
 */
emgpu_texture_config emgpu_texture_default();

/**
 * @brief Calculates the total memory size of a texture in bytes.
 *
 * @return The total size of the texture in bytes.
 */
u64 emgpu_texture_get_size_in_bytes(emgpu_texture* texture);

/**
 * @brief Describes a single render pass attachment.
 */
typedef struct emgpu_attachment_config {
    /** @brief Logical attachment type (colour, depth, stencil, etc.). */
    emgpu_attachment_type type;

    /** @brief Engine-defined pixel format. Must be compatible with the attachment type. */
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
 * @brief Configuration for a renderpass.
 *
 * Contains attachments, image layout transitions
 * and attachment images.
 */
typedef struct emgpu_renderpass_config {
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
    /** @brief Clear colour value used when beginning a render pass. */
    u32 clear_colour;

    /** @brief Number of attachments attached to the renderpass. */
    u32 attachment_count;

    /** @brief Backend-specific internal data. */
    void* internal_data;
} emgpu_renderpass;

emgpu_renderpass_config emgpu_renderpass_default();

/**
 * @brief Describes a single descriptor update for a pipeline.
 *
 * This structure is used to update a single descriptor bindings within a
 * emgpu_pipeline. The descriptor type determines which union member
 * is expected to be valid.
 *
 * @note Only one union member must be set, according to the value of @ref type.
 */
typedef struct emgpu_update_descriptors {
    /** @brief Binding index as declared in the shader. */
    u32 binding;

    /** @brief Type of descriptor being updated (buffer, texture, etc.). */
    emgpu_descriptor_type type;

    union {
        /** @brief Render buffer resource (uniform or storage). */
        emgpu_buffer* buffer;

        /** @brief Texture resource (sampled or storage texture). */
        emgpu_texture* texture;
    };
} emgpu_update_descriptors;
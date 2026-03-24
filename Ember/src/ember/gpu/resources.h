#pragma once

#include "defines.h"

#include "types.h"

/**
 * @brief Configuration for a render buffer.
 *
 * Defines a GPU buffer such as a vertex, index, 
 * uniform, or storage buffer.
 */
typedef struct emgpu_renderbuffer_config {
    /** @brief Intended usage of the buffer (vertex, index, uniform, etc.). */
    emgpu_renderbuffer_usage usage;

    /** @brief Total size of the buffer in bytes. */
    u64 buffer_size;
} emgpu_renderbuffer_config;

/**
 * @brief Backend-agnostic GPU buffer handle.
 *
 * Represents a GPU-resident buffer resource.
 */
typedef struct emgpu_renderbuffer {
    /** @brief Total size of the buffer in bytes. */
    u64 buffer_size;

    /** @brief Backend-specific buffer state/handle. */
    void* internal_data;
} emgpu_renderbuffer;

/**
 * @brief Creates a default render buffer configuration.
 *
 * @return A default-initialized emgpu_renderbuffer_config.
 */
emgpu_renderbuffer_config emgpu_renderbuffer_default();

/**
 * @brief Describes the shader layout used by a render stage.
 *
 * Contains the shader sources and descriptor bindings required
 * to build a pipeline layout. This layout is shared between
 * graphics and compute stages.
 */
typedef struct emgpu_renderstage_layout {
    /** @brief Number of active descriptor bindings. */
	u32 descriptor_count;

    /** @brief Descriptor binding descriptions used by the renderstage. */
    emgpu_descriptor_desc* descriptors;

    /** @brief Shader stages indexed by emgpu_shader_stage_type. */
    emgpu_shader_src stages[EMBER_SHADER_STAGE_TYPE_MAX];
} emgpu_renderstage_layout;

/**
 * @brief Configuration for a graphics render stage.
 *
 * Defines the shader layout, vertex input layout, and optional
 * vertex/index buffers used when creating a graphics renderstage.
 */
typedef struct emgpu_graphicstage_config {
    /** @brief Number of active vertex attributes. */
    u32 vertex_attribute_count;
    
    /** @brief Vertex attribute formats in binding order. */
    emgpu_format* vertex_attributes;

    /** @brief Optional vertex buffer bound to this render stage. */
    emgpu_renderbuffer* vertex_buffer;

    /** @brief Optional index buffer bound to this render stage. */
    emgpu_renderbuffer* index_buffer;

    /** @brief Shader layout used by the stage. */
    emgpu_renderstage_layout layout;
} emgpu_graphicstage_config;

/**
 * @brief Configuration for a compute render stage.
 */
typedef struct emgpu_computestage_config {
    /** @brief Shader layout used by the stage. */
    emgpu_renderstage_layout layout;
} emgpu_computestage_config;

/**
 * @brief Backend-agnostic GPU pipeline handle.
 *
 * Defines a GPU pipeline internally coupled with
 * descriptors or a vertex/index buffer.
 */
typedef struct emgpu_renderstage {
    // TODO: Remove this as emgpu_device_mode is technically a bitmask
    /** @brief Type / supported mode of the renderstage. */
    emgpu_device_mode pipeline_type;

    /** @brief Descriptor binding descriptions used by the renderstage. */
    emgpu_descriptor_desc* descriptors;

    /** @brief Backend-specific pipeline or program data. */
    void* internal_data;
} emgpu_renderstage;

/**
 * @brief Creates a default graphics stage configuration.
 *
 * @return A default-initialized emgpu_graphicstage_config.
 */
emgpu_graphicstage_config emgpu_renderstage_default_graphic();

/**
 * @brief Creates a default compute stage configuration.
 *
 * @return A default-initialized emgpu_computestage_config.
 */
emgpu_computestage_config emgpu_renderstage_default_compute();

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

    /** @brief Max anisotropy of attached sampler in backend, ignored if texture is not sampled. */
    f32 max_anisotropy;

    /** @brief Dimensions of the texture in pixels. */
    uvec2 size;
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
 * @brief Configuration for a rendertarget.
 *
 * Contains attachments, image layout transitions
 * and attachment images.
 */
typedef struct emgpu_rendertarget_config {
    /** @brief Number of attachments attached to the rendertarget. */
    u32 attachment_count;

    /** @brief Attachments created within the rendertarget. */
    emgpu_rendertarget_attachment* attachments;

    /** @brief Render area origin. */
    uvec2 origin;

    /** @brief Render area size. */
    uvec2 size;
} emgpu_rendertarget_config;

/**
 * @brief Represents a render target used by the renderer backend.
 *
 * A render target is the destination for rendering operations. It can either
 * represent a window-backed surface (such as a swapchain image) or an
 * offscreen render target (such as a framebuffer or texture).
 */
typedef struct emgpu_rendertarget {
    /** @brief Indicates whetever rendertarget connects to a platform surface. */
    b8 window_dest;

    /** @brief Clear colour value used when beginning a render pass. */
    u32 clear_colour;

    /** @brief Number of attachments attached to the rendertarget. */
    u32 attachment_count;

    /** @brief Backend-specific internal data. */
    void* internal_data;

    /** @brief Render area origin. */
    uvec2 origin;
    
    /** @brief Render area size. */
    uvec2 size;
} emgpu_rendertarget;

emgpu_rendertarget_config emgpu_rendertarget_default();

/**
 * @brief Describes a single descriptor update for a renderstage.
 *
 * This structure is used to update a single descriptor bindings within a
 * emgpu_renderstage. The descriptor type determines which union member
 * is expected to be valid.
 *
 * @note Only one union member must be set, according to the value of @ref type.
 */
typedef struct emgpu_update_descriptors {
    /** @brief Binding index as declared in the shader. */
    u32 binding;

    /** @brief Renderstage to update descriptor to. */
    emgpu_renderstage* renderstage;

    /** @brief Type of descriptor being updated (buffer, texture, etc.). */
    emgpu_descriptor_type type;

    union {
        /** @brief Render buffer resource (uniform or storage). */
        emgpu_renderbuffer* buffer;

        /** @brief Texture resource (sampled or storage texture). */
        emgpu_texture* texture;
    };
} emgpu_update_descriptors;
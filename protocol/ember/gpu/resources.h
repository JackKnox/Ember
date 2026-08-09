#pragma once

#include "ember/core.h"

#include "ember/gpu/types.h"
#include "ember/gpu/device.h"

#include "ember/gpu/command_buffer.h"

/**
 * @brief Configuration for a render buffer.
 *
 * Defines a GPU buffer such as a vertex, index, 
 * uniform, or storage buffer.
 */
typedef struct emgpu_buffer_config {
    /** @brief Refrence to extra configuration structure specific to API type. */
    void* api_next;

    /** @brief Total size of the buffer in bytes. */
    u64 buffer_size;

    /** @brief Intended usage of the buffer (vertex, index, uniform, etc.). */
    emgpu_buffer_usage usage;
} emgpu_buffer_config;

/**
 * @brief Backend-agnostic GPU buffer handle.
 *
 * Represents a GPU-resident buffer resource.
 */
typedef struct emgpu_buffer {
    /** @brief Backend-specific buffer state/handle. */
    void* internal_data;

    /** @brief Current usage of the buffer. */
    emgpu_buffer_usage usage;

    /** @brief Total size of the buffer in bytes. */
    u64 buffer_size;
} emgpu_buffer;

#ifdef EMBER_DEFINE_HELPERS

/**
 * @brief Creates a default render buffer configuration.
 *
 * @return A default-initialized emgpu_buffer_config.
 */
emgpu_buffer_config emgpu_buffer_default();

#endif

/**
 * @brief Creates a GPU buffer.
 *
 * @param device Pointer to the device instance.
 * @param allocator Allocator used to manage device memory.
 * @param config Buffer configuration.
 * @param out_buffer Output buffer.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emgpu_buffer_create(
    emgpu_device* device, 
    em_allocator* allocator, 
    const emgpu_buffer_config* config, 
    emgpu_buffer* out_buffer);

/**
 * @brief Destroys a buffer.
 *
 * @param device Pointer to the device instance.
 * @param allocator Allocator used to manage device memory.
 * @param buffer Buffer to destroy.
 */
void emgpu_buffer_destroy(
    emgpu_device* device, 
    em_allocator* allocator, 
    emgpu_buffer* buffer);

/**
 * @brief Configuration for a texture.
 *
 * Defines a sampled image stored on the GPU.
 */
typedef struct emgpu_texture_config {
    /** @brief Refrence to extra configuration structure specific to API type. */
    void* api_next;

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
    /** @brief Backend-specific image and sampler state. */
    void* internal_data;

    /** @brief Dimensions of the texture in pixels. */
    uvec2 size;

    /** @brief Image format of the texture data. */
    emgpu_format image_format;
} emgpu_texture;

#ifdef EMBER_DEFINE_HELPERS

/**
 * @brief Creates a default-initialized texture configuration.
 *
 * @return A default-initialized emgpu_texture_config.
 */
emgpu_texture_config emgpu_texture_default();

/**
 * @brief Calculates the total memory size of a texture in bytes.
 *
 * @return The total size of the texture in bytes.
 */
u64 emgpu_texture_get_size_in_bytes(emgpu_texture* texture);

#endif

/**
 * @brief Creates a texture.
 *
 * @param device Pointer to the device instance.
 * @param allocator Allocator used to manage device memory.
 * @param config Texture configuration.
 * @param out_texture Output texture.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emgpu_texture_create(
    emgpu_device* device, 
    em_allocator* allocator, 
    const emgpu_texture_config* config, 
    emgpu_texture* out_texture);

/**
 * @brief Destroys a textures.
 *
 * @param device Pointer to the device instance.
 * @param allocator Allocator used to manage device memory.
 * @param texture Texture to destroy.
 */
void emgpu_texture_destroy(
    emgpu_device* device,
    em_allocator* allocator,
    emgpu_texture* texture);

/**
 * @brief Imports a persistent texture into the command buffer.
 *
 * Registers an external GPU texture for use within the command buffer
 * and returns a valid framebuffer handle to it.
 *
 * @param command_buf Pointer to the command buffer.
 * @param texture Pointer to the GPU texture to import.
 *
 * @return A local framebuffer handle.
 */
emgpu_local_framebuffer emgpu_cmd_import_texture(emgpu_command_buffer* command_buf, emgpu_texture* texture);

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
    /** @brief Refrence to extra configuration structure specific to API type. */
    void* api_next;

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

/**
 * @brief Descriptor for importing a resource into a live pipeline execution 
 * for use in descriptors.
 */
typedef struct emgpu_resource_import {
    /** @brief Bind index to import the resource. */
    u32 dst_binding;

    /** @brief Local reference to the resource. */
    emgpu_local_resource resource;

    /** @brief Allowed access for the revelent pipeline. */
    emgpu_access_flags access_flags;
} emgpu_resource_import;

/**
 * @brief Descriptor for relasing resource control from a live pipeline
 * execution to another consumer.
 */
typedef struct emgpu_resource_export {
    /** @brief Bind index of the resource to export. */
    u32 src_binding;

    /** @brief Destination local resource handle. */
    emgpu_local_resource resource;

    /** @brief Authorized access of other consumers for this resource. */
    emgpu_access_flags access_flags;
} emgpu_resource_export;

/**
 * @brief Backend-agnostic GPU pipeline handle.
 *
 * Defines a GPU pipeline internally coupled with
 * descriptors or a vertex/index buffer.
 */
typedef struct emgpu_pipeline {
    /** @brief Backend-specific pipeline or program data. */
    void* internal_data;

    /** @brief Type / supported mode of the pipeline. */
    emgpu_ops_type type;
} emgpu_pipeline;

/**
 * @brief Updates descriptor bindings for a pipeline.
 *
 * @param device Pointer to the device instance.
 * @param allocator Allocator used to manage device memory.
 * @param pipeline Pipeline to update.
 * @param descriptors Descriptor update data.
 * @param descriptor_count Number of descriptors.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emgpu_pipeline_upload_descriptors(
    emgpu_device* device, 
    emgpu_pipeline* pipeline, 
    emgpu_update_descriptors* descriptors, 
    u32 descriptor_count);
    
/**
 * @brief Destroys a pipeline.
 *
 * @param device Pointer to the device instance.
 * @param allocator Allocator used to manage device memory.
 * @param pipeline Pipeline to destroy.
 */
void emgpu_pipeline_destroy(
    emgpu_device* device, 
    em_allocator* allocator, 
    emgpu_pipeline* pipeline);


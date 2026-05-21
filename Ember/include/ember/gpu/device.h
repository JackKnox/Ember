#pragma once

#include "ember/core.h"

#include "ember/gpu/resources.h"
#include "ember/gpu/frame.h"

/**
 * @brief Renderer device interface.
 *
 * Provides a backend-agnostic abstraction over graphics APIs.
 * Implementations translate commands into API-specific calls.
 */
typedef struct emgpu_device {
    /** @brief Backend-specific device context or handle. */
    void* internal_context;

    /** @brief Index of the current frame in flight. */
    u32 current_frame;

    /** @brief Allocator used to manage per-frame resources. */
    em_allocator frame_allocator;

    /**
     * @brief Initializes the GPU device.
     *
     * @param device Pointer to the device instance.
     * @param config Configuration parameters for initialization.
     * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
     */
    em_result (*initialize)(struct emgpu_device* device, em_allocator* allocator, const emgpu_device_config* config);

    /**
     * @brief Shuts down the GPU device and releases all associated resources.
     *
     * @param device Pointer to the device instance.
     */
    void (*shutdown)(struct emgpu_device* device, em_allocator* allocator);

    /**
     * @brief Retreives capabilties of the rendering device.
     * 
     * @param device Pointer to the device instance.
     * @param out_capabilities Output capabilties structure.
     * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
     */
    em_result (*retreive_capabilities)(struct emgpu_device* device, emgpu_device_capabilities* out_capabilities);

    /**
     * @brief Submits a frame for execution on the GPU.
     *
     * @param device Pointer to the device instance.
     * @param frame Frame containing recorded commands.
     * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
     */
    em_result (*submit_frame)(struct emgpu_device* device, const emgpu_frame* frame);

    /**
     * @brief Destroys a rendering surface.
     *
     * @param device Pointer to the device instance.
     * @param surface Surface to destroy.
     */
    void (*destroy_surface)(struct emgpu_device* device, em_allocator* allocator, emgpu_surface* surface);

    /**
     * @brief Creates a render pass.
     *
     * @param device Pointer to the device instance.
     * @param config Render pass configuration.
     * @param out_renderpass Output render pass.
     * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
     */
    em_result (*create_renderpass)(struct emgpu_device* device, em_allocator* allocator, const emgpu_renderpass_config* config, emgpu_renderpass* out_renderpass);

    /**
     * @brief Destroys a render pass.
     *
     * @param device Pointer to the device instance.
     * @param renderpass Pass to destroy.
     */
    void (*destroy_renderpass)(struct emgpu_device* device, em_allocator* allocator, emgpu_renderpass* renderpass);

    /**
     * @brief Creates a graphics pipeline.
     *
     * @param device Pointer to the device instance.
     * @param config Pipeline configuration.
     * @param bound_renderpass Render pass the pipeline is compatible with.
     * @param out_graphics_pipeline Output pipeline.
     * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
     */
    em_result (*create_graphics_pipeline)(struct emgpu_device* device, em_allocator* allocator, const emgpu_graphics_pipeline_config* config, emgpu_renderpass* bound_renderpass, emgpu_pipeline* out_graphics_pipeline);

    /**
     * @brief Creates a compute pipeline.
     *
     * @param device Pointer to the device instance.
     * @param config Pipeline configuration.
     * @param out_compute_pipeline Output pipeline.
     * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
     */
    em_result (*create_compute_pipeline)(struct emgpu_device* device, em_allocator* allocator, const emgpu_compute_pipeline_config* config, emgpu_pipeline* out_compute_pipeline);

    /**
     * @brief Updates descriptor bindings for a pipeline.
     *
     * @param device Pointer to the device instance.
     * @param pipeline Pipeline to update.
     * @param descriptors Descriptor update data.
     * @param descriptor_count Number of descriptors.
     * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
     */
    em_result (*update_pipeline_descriptors)(struct emgpu_device* device, emgpu_pipeline* pipeline, emgpu_update_descriptors* descriptors, u32 descriptor_count);

    /**
     * @brief Destroys a pipeline.
     *
     * @param device Pointer to the device instance.
     * @param pipeline Pipeline to destroy.
     */
    void (*destroy_pipeline)(struct emgpu_device* device, em_allocator* allocator, emgpu_pipeline* pipeline);

    /**
     * @brief Creates a GPU buffer.
     *
     * @param device Pointer to the device instance.
     * @param config Buffer configuration.
     * @param out_buffer Output buffer.
     * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
     */
    em_result (*create_buffer)(struct emgpu_device* device, em_allocator* allocator, const emgpu_buffer_config* config, emgpu_buffer* out_buffer);

    /**
     * @brief Uploads data to a buffer.
     *
     * @param device Pointer to the device instance.
     * @param buffer Target buffer.
     * @param data Pointer to source data.
     * @param start_offset Offset into the buffer.
     * @param region Size of the data to upload.
     * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
     */
    em_result (*upload_to_buffer)(struct emgpu_device* device, emgpu_buffer* buffer, const void* data, u64 start_offset, u64 region);

    /**
     * @brief Destroys a buffer.
     *
     * @param device Pointer to the device instance.
     * @param buffer Buffer to destroy.
     */
    void (*destroy_buffer)(struct emgpu_device* device, em_allocator* allocator, emgpu_buffer* buffer);

    /**
     * @brief Creates a texture.
     *
     * @param device Pointer to the device instance.
     * @param config Texture configuration.
     * @param out_texture Output texture.
     * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
     */
    em_result (*create_texture)(struct emgpu_device* device, em_allocator* allocator, const emgpu_texture_config* config, emgpu_texture* out_texture);

    /**
     * @brief Uploads data to a texture.
     *
     * @param device Pointer to the device instance.
     * @param texture Target texture.
     * @param data Pointer to source data.
     * @param start_offset Starting texel position.
     * @param region Size of the region to update.
     * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
     */
    em_result (*upload_to_texture)(struct emgpu_device* device, emgpu_texture* texture, const void* data, uvec2 start_offset, uvec2 region);

    /**
     * @brief Destroys a texture.
     *
     * @param device Pointer to the device instance.
     * @param texture Texture to destroy.
     */
    void (*destroy_texture)(struct emgpu_device* device, em_allocator* allocator, emgpu_texture* texture);
} emgpu_device;

/**
 * @brief Initializes a GPU device instance.
 *
 * This function sets up the device and assigns the appropriate backend
 * implementation based on the provided configuration.
 *
 * @param config Device configuration parameters.
 * @param out_device Output device instance.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emgpu_device_init(const emgpu_device_config* config, em_allocator* allocator, emgpu_device* out_device);

/**
 * @brief Shuts down a GPU device.
 *
 * Releases all resources associated with the device and performs
 * backend-specific cleanup.
 *
 * @param device Pointer to the device instance.
 */
void emgpu_device_shutdown(em_allocator* allocator, emgpu_device* device);

/**
 * @brief Print rendering device info to Ember standard log.
 * 
 * Retreives and caches capabilities from the rendering device, 
 * fairly slow function.
 * 
 * @param device Pointer to the device instance.
 * @param level Log level when printing info.
 */
void emgpu_device_print_capabilities(emgpu_device* device, emgpu_device_capabilities* capabilities, log_level level);
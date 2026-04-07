#pragma once

#include "ember/core.h"

#include "ember/gpu/resources.h"
#include "ember/gpu/frame.h"

#include <ember/platform/window.h>

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
 * @brief Configuration for a rendertarget that implictly 
 * connects to a platform surface.
 *
 * Defines a rendertarget to a platform surface, omitting
 * some fields from base configuration that aren't applicable.
 */
typedef struct emgpu_present_target_config {
    /** @brief Surface that the created rendertarget renders to. */
    emgpu_surface* surface;

    /** @brief Number of attachments attached to the rendertarget. */
    u32 attachment_count;

    /** @brief Attachments created within the rendertarget. */

    emgpu_attachment_config* attachments;

    /** 
     * @brief Textures that are connected to the rendertarget.
     * 
     * These textures are now owned by the rendertarget, automatically recreates when resizing.
     * 
     * @note The texture array must be in the format of `[attachment][frame] (f0 -> a0, a1, f1 -> a0, a1 ...)`
     *       and the size must be `surface.image_count * attachment_count`.
     */
    emgpu_texture* existing_textures;
} emgpu_present_target_config;

/**
 * @brief Creates a default rendertarget configuration with presentation.
 *
 * @return A default-initialized emgpu_present_target_config.
 */
emgpu_present_target_config emgpu_rendertarget_default_present();

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

    /** 
     * @brief Pointer to dynamicaly allocated capabilities struct, may be NULL.
     * 
     * Pointer becomes valid when polling `capabilities()` on the device.
     */
    emgpu_device_capabilities* capabilities;

    /**
     * @brief Initializes the GPU device.
     *
     * @param device Pointer to the device instance.
     * @param config Configuration parameters for initialization.
     * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
     */
    em_result (*initialize)(struct emgpu_device* device, const emgpu_device_config* config);

    /**
     * @brief Shuts down the GPU device and releases all associated resources.
     *
     * @param device Pointer to the device instance.
     */
    void (*shutdown)(struct emgpu_device* device);

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
     * @brief Creates a rendering surface for a window.
     *
     * @param device Pointer to the device instance.
     * @param window Platform window handle.
     * @param out_surface Output surface.
     * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
     */
    em_result (*create_surface)(struct emgpu_device* device, emplat_window* window, emgpu_surface* out_surface);

    /**
     * @brief Destroys a rendering surface.
     *
     * @param device Pointer to the device instance.
     * @param surface Surface to destroy.
     */
    void (*destroy_surface)(struct emgpu_device* device, emgpu_surface* surface);

    /**
     * @brief Creates a render target.
     *
     * @param device Pointer to the device instance.
     * @param config Render target configuration.
     * @param out_rendertarget Output render target.
     * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
     */
    em_result (*create_rendertarget)(struct emgpu_device* device, const emgpu_rendertarget_config* config, emgpu_rendertarget* out_rendertarget);

    /**
     * @brief Creates a presentable render target (swapchain-backed).
     *
     * @param device Pointer to the device instance.
     * @param config Present target configuration.
     * @param out_rendertarget Output render target.
     * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
     */
    em_result (*create_present_target)(struct emgpu_device* device, const emgpu_present_target_config* config, emgpu_rendertarget* out_rendertarget);

    /**
     * @brief Resizes a render target.
     *
     * @param device Pointer to the device instance.
     * @param rendertarget Target to resize.
     * @param new_size New dimensions.
     * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
     */
    em_result (*resize_rendertarget)(struct emgpu_device* device, emgpu_rendertarget* rendertarget, uvec2 new_size);

    /**
     * @brief Destroys a render target.
     *
     * @param device Pointer to the device instance.
     * @param rendertarget Target to destroy.
     */
    void (*destroy_rendertarget)(struct emgpu_device* device, emgpu_rendertarget* rendertarget);

    /**
     * @brief Creates a graphics pipeline.
     *
     * @param device Pointer to the device instance.
     * @param config Pipeline configuration.
     * @param bound_rendertarget Render target the pipeline is compatible with.
     * @param out_graphics_pipeline Output pipeline.
     * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
     */
    em_result (*create_graphics_pipeline)(struct emgpu_device* device, const emgpu_graphics_pipeline_config* config, emgpu_rendertarget* bound_rendertarget, emgpu_pipeline* out_graphics_pipeline);

    /**
     * @brief Creates a compute pipeline.
     *
     * @param device Pointer to the device instance.
     * @param config Pipeline configuration.
     * @param out_compute_pipeline Output pipeline.
     * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
     */
    em_result (*create_compute_pipeline)(struct emgpu_device* device, const emgpu_compute_pipeline_config* config, emgpu_pipeline* out_compute_pipeline);

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
    void (*destroy_pipeline)(struct emgpu_device* device, emgpu_pipeline* pipeline);

    /**
     * @brief Creates a GPU buffer.
     *
     * @param device Pointer to the device instance.
     * @param config Buffer configuration.
     * @param out_buffer Output buffer.
     * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
     */
    em_result (*create_buffer)(struct emgpu_device* device, const emgpu_buffer_config* config, emgpu_buffer* out_buffer);

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
    void (*destroy_buffer)(struct emgpu_device* device, emgpu_buffer* buffer);

    /**
     * @brief Creates a texture.
     *
     * @param device Pointer to the device instance.
     * @param config Texture configuration.
     * @param out_texture Output texture.
     * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
     */
    em_result (*create_texture)(struct emgpu_device* device, const emgpu_texture_config* config, emgpu_texture* out_texture);

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
    void (*destroy_texture)(struct emgpu_device* device, emgpu_texture* texture);
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
em_result emgpu_device_init(const emgpu_device_config* config, emgpu_device* out_device);

/**
 * @brief Shuts down a GPU device.
 *
 * Releases all resources associated with the device and performs
 * backend-specific cleanup.
 *
 * @param device Pointer to the device instance.
 */
void emgpu_device_shutdown(emgpu_device* device);

/**
 * @brief Print rendering device info to Ember standard log.
 * 
 * Retreives and caches capabilities from the rendering device, 
 * fairly slow function.
 * 
 * @param device Pointer to the device instance.
 * @param level Log level when printing info.
 */
em_result emgpu_device_print_capabilities(emgpu_device* device, log_level level);
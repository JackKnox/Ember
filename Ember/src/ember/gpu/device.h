#pragma once

#include "defines.h"

#include "ember/platform/window.h"

#include "types.h"
#include "resources.h"
#include "commandbuf.h"

/**
 * @brief Render command types.
 *
 * Represent high-level rendering operations that are translated
 * into backend-specific API calls (OpenGL, Vulkan, etc.).
 */
enum {
    RENDERCMD_BIND_RENDERTARGET,
    RENDERCMD_MEMORY_BARRIER,
    RENDERCMD_BEGIN_RENDERSTAGE,
    RENDERCMD_END_RENDERSTAGE,
    RENDERCMD_DRAW,
    RENDERCMD_DRAW_INDEXED,
    RENDERCMD_DISPATCH,

    /** @brief Internal command used to finalize command buffers. */
    RENDERCMD_END,
};

/** @brief Type used to identify a render command payload. */
typedef u32 rendercmd_payload_type;

#pragma pack(push, 1)

/**
 * @brief Header prepended to every render command payload.
 *
 * Describes the command type and the renderer mode it supports.
 */
typedef struct rendercmd_header {
    /** @brief Type of render command. */
    rendercmd_payload_type type;

    /** @brief Renderer mode this command is valid for (graphics or compute). */
    emgpu_device_mode supported_mode;
} rendercmd_header;

#pragma pack(pop)

#pragma pack(push, 1)

/**
 * @brief Payload data for all supported render commands.
 *
 * The active member is determined by rendercmd_payload_type.
 */
typedef union rendercmd_payload {
    /**
     * @brief Clear color command payload.
     */
    struct {
        /** @brief Render target to bind for subsequent operations. */
        emgpu_rendertarget* rendertarget;
    } bind_rendertarget;

    struct {
        emgpu_renderstage* src_renderstage, * dst_renderstage;
        emgpu_access_flags src_access, dst_access;
    } memory_barrier;

    /**
     * @brief Begin render stage command payload.
     */
    struct {
        /** @brief Render stage to bind for subsequent operations. */
        emgpu_renderstage* renderstage;
    } begin_renderstage;

    /**
     * @brief Non-indexed draw command payload.
     */
    struct {
        /** @brief Number of vertices to draw. */
        u32 vertex_count;

        /** @brief Number of instances to draw. */
        u32 instance_count;
    } draw;

    /**
     * @brief Indexed draw command payload.
     */
    struct {
        /** @brief Number of indices to draw. */
        u32 index_count;

        /** @brief Number of instances to draw. */
        u32 instance_count;
    } draw_indexed;

    /**
     * @brief Compute dispatch command payload.
     */
    struct {
        /** @brief Compute workgroup dimensions. */
        uvec3 group_size;
    } dispatch;

} rendercmd_payload;

#pragma pack(pop)

/**
 * @brief Context used during render command playback.
 *
 * Tracks currently active render state while commands are executed.
 */
typedef struct emgpu_playback_context {
    /** @brief Current renderer mode (graphics or compute). */
    emgpu_device_mode current_mode;

    /** @brief Currently bound render target. */
    emgpu_rendertarget* current_target;

    /** @brief Currently bound render stage. */
    emgpu_renderstage* current_shader;
} emgpu_playback_context;

/**
 * @brief Renderer device interface.
 *
 * Provides a backend-agnostic abstraction over graphics APIs.
 * Implementations translate commands into API-specific calls.
 */
typedef struct emgpu_device {
    /** @brief Backend-specific internal state. */
    void* internal_context;

    /** @brief Platform state attachted to rendering device, can be null */
    emplat_window* window_context;

    /** @brief Renderer capabilities supported by this device. */
    emgpu_device_capabilities capabilities;

    /** @brief Main rendertarget created automattically on init, exists regardless of attachted window. */
    emgpu_rendertarget main_rendertarget;

    /**
     * @brief Initializes the rendering device.
     *
     * @param device Pointer to the backend instance.
     * @param config Backend configuration.
     * @param starting_size Initial framebuffer size.
     * @param application_name Name of the application.
     * @return True if initialization succeeded.
     */
    b8 (*initialize)(struct emgpu_device* device, emgpu_device_config* config);

    /**
     * @brief Shuts down the device and releases resources.
     */
    void (*shutdown)(struct emgpu_device* device);

    /**
     * @brief Notifies the device of a framebuffer resize.
     * 
     * Not guranteed to resize the main framebuffer immediately.
     *
     * @param device Pointer to the backend instance.
     * @param new_size New framebuffer dimensions.
     * 
     * @note This would be used instead of calling `device->resize_rendertarget(main_rendertarget)`
     */
    void (*resized)(struct emgpu_device* device, uvec2 new_size);
    /**
     * @brief Retrieves frames-in-flight attachted to internal window surface.
     *
     * @param device Pointer to the backend instance.
     * @param out_textures Refrence to output pointer of a managed array.
     * @return True if initialization succeeded.
     */
    b8 (*window_textures)(struct emgpu_device* device, emgpu_texture** out_textures);

    /**
     * @brief Updates one or more descriptor bindings for a renderstage.
     *
     * @param device Pointer to the renderer backend.
     * @param descriptors Array of descriptor update descriptions.
     * @param descriptor_count Number of elements in @p descriptors.
     */
    b8 (*update_descriptors) (struct emgpu_device* device, emgpu_update_descriptors* descriptors, u32 descriptor_count);

    /**
     * @brief Begins a new frame.
     *
     * @param device Pointer to the backend instance.
     * @param delta_time Time elapsed since last frame.
     * @return True if frame preparation succeeded.
     */
    b8 (*begin_frame)(struct emgpu_device* device, f64 delta_time);

     /**
     * @brief Executes a single render command.
     *
     * @param device Pointer to the backend instance.
     * @param context Playback state context.
     * @param header Command header.
     * @param payload Command payload.
     */
    void (*execute_command)(struct emgpu_device* device, emgpu_playback_context* context, rendercmd_header* hdr, rendercmd_payload* payload);

    /**
     * @brief Ends the current frame.
     *
     * @return True if frame submission succeeded.
     */
    b8 (*end_frame)(struct emgpu_device* device);

    /** @name Resource Management */
    /** @{ */

    /** @brief Creates a render stage with graphics configuration. */
    b8 (*create_graphicstage)(struct emgpu_device* device, emgpu_graphicstage_config* config, emgpu_rendertarget* bound_rendertarget, emgpu_renderstage* out_graphicstage);

    /** @brief Creates a render stage with compute configuration. */
    b8 (*create_computestage)(struct emgpu_device* device, emgpu_computestage_config* config, emgpu_renderstage* out_computestage);

    /** @brief Destroys a render stage. */
    void (*destroy_renderstage)(struct emgpu_device* device, emgpu_renderstage* stage);
    
    /** @brief Creates a render buffer. */
    b8 (*create_renderbuffer)(struct emgpu_device* device, emgpu_renderbuffer_config* config, emgpu_renderbuffer* out_buffer);

    /**
     * @brief Uploads data into a render buffer.
     *
     * @param backend Pointer to the backend instance.
     * @param buffer Target buffer.
     * @param data Source data pointer.
     * @param start_offset Offset into target buffer.
     * @param region Size of region within data in bytes.
     */
    b8 (*upload_to_renderbuffer)(struct emgpu_device* device, emgpu_renderbuffer* buffer, const void* data, u64 start_offset, u64 region);

    /** @brief Destroys a render buffer. */
    void (*destroy_renderbuffer)(struct emgpu_device* device, emgpu_renderbuffer* buffer);

    /** @brief Creates a texture resource. */
    b8 (*create_texture)(struct emgpu_device* device, emgpu_texture_config* config, emgpu_texture* out_texture);

    /**
     * @brief Uploads data into a texture.
     *
     * @param backend Pointer to the backend instance.
     * @param buffer Target texture.
     * @param data Source data pointer.
     * @param start_offset Offset into target texture.
     * @param region Size of region within data in width & height.
     */
    b8 (*upload_to_texture)(struct emgpu_device* device, emgpu_texture* texture, const void* data, uvec2 start_offset, uvec2 region);

    /** @brief Destroys a texture resource. */
    void (*destroy_texture)(struct emgpu_device* device, emgpu_texture* texture);

    /** @brief Creates a rendertarget. */
    b8 (*create_rendertarget)(struct emgpu_device* device, emgpu_rendertarget_config* config, emgpu_rendertarget* out_rendertarget);

    /** @brief Resize a rendertarget lazyilly to `new_size` */
    b8 (*resize_rendertarget)(struct emgpu_device* device, emgpu_rendertarget* rendertarget, uvec2 new_size);

    /** @brief Destroys a rendertarget. */
    void (*destroy_rendertarget)(struct emgpu_device* device, emgpu_rendertarget* rendertarget);

    /** @} */
} emgpu_device;

/**
 * @brief Creates and initializes a rendering device.
 *
 * @param device Output device instance.
 * @param config Device configuration.
 * @param window Attached window context (can be null)
 * @return True if creation succeeded.
 */
b8 emgpu_device_init(emgpu_device_config* config, emplat_window* window, emgpu_device* out_device);

/**
 * @brief Destroys and fully shuts down a device instance.
 */
void emgpu_device_shutdown(emgpu_device* device);

/**
 * @brief Executes a single command buffer on a rendering device.
 *
 * Performs validation based on device configuration.
 * 
 * @param device Device instance.
 * @param cmd Command buffer to execute.
 * @return True if execution succeeded.
 */
b8 emgpu_device_submit_commandbuf(emgpu_device* device, emgpu_commandbuf* cmd);
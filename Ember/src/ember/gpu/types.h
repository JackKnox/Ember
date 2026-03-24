#pragma once

#include "defines.h"

/**
 * @brief Supported rendering APIs.
 */
typedef enum emgpu_device_backend {
    EMBER_DEVICE_BACKEND_VULKAN,   /**< Vulkan backend */ 
    EMBER_DEVICE_BACKEND_OPENGL,   /**< OpenGL backend */
    EMBER_DEVICE_BACKEND_DIRECTX,  /**< DirectX backend */
} emgpu_device_backend;

/**
 * @brief Type of logical rendering device.
 */
typedef enum emgpu_device_type {
    EMBER_DEVICE_TYPE_OTHER,           /**< Unknown or generic device */
    EMBER_DEVICE_TYPE_INTEGRATED_GPU,  /**< Integrated GPU (shared memory) */
    EMBER_DEVICE_TYPE_DISCRETE_GPU,    /**< Dedicated graphics card */
    EMBER_DEVICE_TYPE_VIRTUAL_GPU,     /**< Virtualized GPU (e.g. VM) */
    EMBER_DEVICE_TYPE_CPU,             /**< CPU-based rendering device */
} emgpu_device_type;

/**
 * @brief Operating modes supported by the renderer.
 *
 * Modes may be combined as bit flags.
 */
typedef enum emgpu_device_mode {
    EMBER_DEVICE_MODE_GRAPHICS = 1 << 0,  /**< Graphics pipeline operations */
    EMBER_DEVICE_MODE_COMPUTE  = 1 << 1,  /**< Compute shader operations */
    EMBER_DEVICE_MODE_TRANSFER = 1 << 2,  /**< Data transfer operations */
} emgpu_device_mode;

#define EMGPU_FORMAT_FLAG_SRGB       1 << 1
//#define EMGPU_FORMAT_FLAG_DEPTH    1 << 2
//#define EMGPU_FORMAT_FLAG_STENCIL  1 << 3

/**
 * @brief Describes a data format used by the ember_gpu subsystem.
 * Any other format values outside this enum is not supported by 
 * the subsystem.
 *
 * Used for vertex attributes, textures, and general GPU data.
 */
typedef enum emgpu_format {
    EMGPU_FORMAT_UNDEFINED = 0,

    /**
     * @brief 8-bit integer formats
     */
    EMGPU_FORMAT_R8_UINT    = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_UINT, 8, 1, 0),
    EMGPU_FORMAT_RG8_UINT   = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_UINT, 8, 2, 0),
    EMGPU_FORMAT_RGB8_UINT  = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_UINT, 8, 3, 0),
    EMGPU_FORMAT_RGBA8_UINT = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_UINT, 8, 4, 0),

    EMGPU_FORMAT_R8_SINT    = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_SINT, 8, 1, 0),
    EMGPU_FORMAT_RG8_SINT   = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_SINT, 8, 2, 0),
    EMGPU_FORMAT_RGB8_SINT  = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_SINT, 8, 3, 0),
    EMGPU_FORMAT_RGBA8_SINT = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_SINT, 8, 4, 0),

    /**
     * @brief 8-bit normalized formats
     */
    EMGPU_FORMAT_R8_UNORM    = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_UINT, 8, 1, EMBER_FORMAT_FLAG_NORMALIZED),
    EMGPU_FORMAT_RG8_UNORM   = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_UINT, 8, 2, EMBER_FORMAT_FLAG_NORMALIZED),
    EMGPU_FORMAT_RGB8_UNORM  = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_UINT, 8, 3, EMBER_FORMAT_FLAG_NORMALIZED),
    EMGPU_FORMAT_RGBA8_UNORM = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_UINT, 8, 4, EMBER_FORMAT_FLAG_NORMALIZED),

    EMGPU_FORMAT_R8_SNORM    = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_SINT, 8, 1, EMBER_FORMAT_FLAG_NORMALIZED),
    EMGPU_FORMAT_RG8_SNORM   = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_SINT, 8, 2, EMBER_FORMAT_FLAG_NORMALIZED),
    EMGPU_FORMAT_RGB8_SNORM  = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_SINT, 8, 3, EMBER_FORMAT_FLAG_NORMALIZED),
    EMGPU_FORMAT_RGBA8_SNORM = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_SINT, 8, 4, EMBER_FORMAT_FLAG_NORMALIZED),

    /**
     * @brief 16-bit formats
     */
    EMGPU_FORMAT_R16_UINT    = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_UINT, 16, 1, 0),
    EMGPU_FORMAT_RG16_UINT   = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_UINT, 16, 2, 0),
    EMGPU_FORMAT_RGB16_UINT  = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_UINT, 16, 3, 0),
    EMGPU_FORMAT_RGBA16_UINT = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_UINT, 16, 4, 0),

    EMGPU_FORMAT_R16_SINT    = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_SINT, 16, 1, 0),
    EMGPU_FORMAT_RG16_SINT   = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_SINT, 16, 2, 0),
    EMGPU_FORMAT_RGB16_SINT  = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_SINT, 16, 3, 0),
    EMGPU_FORMAT_RGBA16_SINT = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_SINT, 16, 4, 0),

    EMGPU_FORMAT_R16_FLOAT   = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_FLOAT, 16, 1, 0),
    EMGPU_FORMAT_RG16_FLOAT  = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_FLOAT, 16, 2, 0),
    EMGPU_FORMAT_RGB16_FLOAT = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_FLOAT, 16, 3, 0),
    EMGPU_FORMAT_RGBA16_FLOAT = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_FLOAT, 16, 4, 0),

    /**
     * @brief 32-bit formats
     */
    EMGPU_FORMAT_R32_UINT    = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_UINT,  32, 1, 0),
    EMGPU_FORMAT_RG32_UINT   = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_UINT,  32, 2, 0),
    EMGPU_FORMAT_RGB32_UINT  = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_UINT,  32, 3, 0),
    EMGPU_FORMAT_RGBA32_UINT = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_UINT,  32, 4, 0),

    EMGPU_FORMAT_R32_SINT    = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_SINT,  32, 1, 0),
    EMGPU_FORMAT_RG32_SINT   = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_SINT,  32, 2, 0),
    EMGPU_FORMAT_RGB32_SINT  = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_SINT,  32, 3, 0),
    EMGPU_FORMAT_RGBA32_SINT = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_SINT,  32, 4, 0),

    EMGPU_FORMAT_R32_FLOAT   = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_FLOAT, 32, 1, 0),
    EMGPU_FORMAT_RG32_FLOAT  = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_FLOAT, 32, 2, 0),
    EMGPU_FORMAT_RGB32_FLOAT = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_FLOAT, 32, 3, 0),
    EMGPU_FORMAT_RGBA32_FLOAT = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_FLOAT, 32, 4, 0),

    /**
     * @brief sRGB formats
     */
    EMGPU_FORMAT_R8_SRGB    = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_UINT, 8, 1, EMGPU_FORMAT_FLAG_SRGB),
    EMGPU_FORMAT_RG8_SRGB   = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_UINT, 8, 2, EMGPU_FORMAT_FLAG_SRGB),
    EMGPU_FORMAT_RGB8_SRGB  = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_UINT, 8, 3, EMGPU_FORMAT_FLAG_SRGB),
    EMGPU_FORMAT_RGBA8_SRGB = EMBER_FORMAT_MAKE(EMBER_DATA_TYPE_UINT, 8, 4, EMGPU_FORMAT_FLAG_SRGB),
} emgpu_format;

/**
 * @brief Intended usage of a render buffer.
 *
 * Usage flags may be combined as bit flags.
 */
typedef enum emgpu_renderbuffer_usage {
    EMBER_RENDERBUFFER_USAGE_VERTEX  = 1 << 0, /**< Vertex buffer */
    EMBER_RENDERBUFFER_USAGE_INDEX   = 1 << 1, /**< Index buffer */
    EMBER_RENDERBUFFER_USAGE_STORAGE = 1 << 2, /**< Storage buffer */
    EMBER_RENDERBUFFER_USAGE_CPU_VISIBLE = 1 << 3, /**< CPU-coherent buffer */
} emgpu_renderbuffer_usage;

/**
 * @brief Intended usage of a texture.
 *
 * Usage flags may be combined as bit flags.
 */
typedef enum emgpu_texture_usage {
    EMBER_TEXTURE_USAGE_STORAGE = 1 << 0, /**< Storage image */
    EMBER_TEXTURE_USAGE_SAMPLED = 1 << 1, /**< Texture created with sampler */
} emgpu_texture_usage;

/**
 * @brief Type of render target attachment.
 *
 * Defines the logical purpose of an attachment within a render target.
 */
typedef enum emgpu_attachment_type {
    EMBER_ATTACHMENT_TYPE_COLOR,   /**< Color render target (RGBA output) */
    EMBER_ATTACHMENT_TYPE_DEPTH,   /**< Depth-only attachment */
    EMBER_ATTACHMENT_TYPE_STENCIL, /**< Stencil-only attachment */
    EMBER_ATTACHMENT_TYPE_DEPTH_STENCIL, /**< Combined depth-stencil attachment */
} emgpu_attachment_type;

/**
 * @brief Attachment load operation at the start of a render target.
 *
 * Determines how existing attachment contents are treated
 * when the render pass begins.
 */
typedef enum emgpu_load_op {
    EMBER_LOAD_OP_LOAD,     /**< Preserve existing contents of the attachment */
    EMBER_LOAD_OP_CLEAR,    /**< Clear the attachment at the start of the target */
    EMBER_LOAD_OP_DONT_CARE, /**< Contents are undefined at the start of the target */
} emgpu_load_op;

/**
 * @brief Attachment store operation at the end of a render pass.
 *
 * Determines whether attachment contents are preserved
 * after the render pass completes.
 */
typedef enum emgpu_store_op {
    EMBER_STORE_OP_STORE,      /**< Store the results for later use */
    EMBER_STORE_OP_DONT_CARE,  /**< Contents become undefined after the pass completes */
} emgpu_store_op;

/**
 * @brief Describes memory access types used for synchronization between render stages.
 *
 * These flags represent how a resource is accessed within a render stage.
 * They are used to establish proper memory visibility and ordering guarantees 
 * between GPU operations.
 * 
 * Usage flags may be combined.
 */
typedef enum emgpu_access_flags {
    EMBER_ACCESS_FLAGS_SHADER_WRITE            = 1 << 0,  /**< Written by a shader */
    EMBER_ACCESS_FLAGS_SHADER_READ             = 1 << 1,  /**< Resource read by a shader */
    EMBER_ACCESS_FLAGS_VERTEX_ATTRIBUTE_READ   = 1 << 2,  /**< Read as a vertex attribute during vertex input */
    EMBER_ACCESS_FLAGS_COLOUR_ATTACHMENT_WRITE = 1 << 3,  /**< Written as a colour attachment during rendering */
} emgpu_access_flags;

/**
 * @brief Descriptor types supported by shaders.
 */
typedef enum emgpu_descriptor_type {
    EMBER_DESCRIPTOR_TYPE_STORAGE_BUFFER,  /**< Storage buffer (SSBO) */
    EMBER_DESCRIPTOR_TYPE_STORAGE_IMAGE,   /**< Storage image, no sampler */
    EMBER_DESCRIPTOR_TYPE_IMAGE_SAMPLER,   /**< Combined image + sampler */
} emgpu_descriptor_type;

/**
 * @brief Texture filtering modes.
 */
typedef enum emgpu_filter_type {
    EMBER_FILTER_TYPE_NEAREST, /**< Nearest-neighbor filtering */
    EMBER_FILTER_TYPE_LINEAR,  /**< Linear interpolation filtering */
} emgpu_filter_type;

/**
 * @brief Shader stage types.
 */
typedef enum emgpu_shader_stage_type {
    EMBER_SHADER_STAGE_TYPE_VERTEX,   /**< Vertex shader */
    EMBER_SHADER_STAGE_TYPE_FRAGMENT, /**< Fragment/pixel shader */
    EMBER_SHADER_STAGE_TYPE_COMPUTE,  /**< Compute shader */
    EMBER_SHADER_STAGE_TYPE_MAX,      /**< Sentinel (max stages) */
} emgpu_shader_stage_type;

/**
 * @brief Texture address (wrap) modes.
 */
typedef enum emgpu_address_mode {
    EMBER_ADDRESS_MODE_REPEAT,               /**< Repeat texture coordinates */
    EMBER_ADDRESS_MODE_MIRRORED_REPEAT,      /**< Mirrored repeat */
    EMBER_ADDRESS_MODE_CLAMP_TO_EDGE,        /**< Clamp to edge */
    EMBER_ADDRESS_MODE_CLAMP_TO_BORDER,      /**< Clamp to border color */
    EMBER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE, /**< Mirror then clamp to edge */
} emgpu_address_mode;

/**
 * @brief Describes a single render target attachment.
 */
typedef struct emgpu_rendertarget_attachment {
    /** Logical attachment type (color, depth, stencil, etc.). */
    emgpu_attachment_type type;

    /** Engine-defined pixel format. Must be compatible with the attachment type. */
    emgpu_format format;

    /** Load operation for color or depth aspect. */
    emgpu_load_op load_op;

    /** Store operation for color or depth aspect. */
    emgpu_store_op store_op;

    /**
     * Load operation for stencil aspect.
     * Only relevant for stencil or depth-stencil attachments.
     */
    emgpu_load_op stencil_load_op;

    /**
     * Store operation for stencil aspect.
     * Only relevant for stencil or depth-stencil attachments.
     */
    emgpu_store_op stencil_store_op;
} emgpu_rendertarget_attachment;

/**
 * @brief Describes a single descriptor binding.
 */
typedef struct emgpu_descriptor_desc {
    /** @brief Binding index within the shader. */
    u32 binding;

    /** @brief Descriptor resource type. */
    emgpu_descriptor_type descriptor_type;

    /** @brief Shader stage this descriptor is visible to. */
    emgpu_shader_stage_type stage_type;
} emgpu_descriptor_desc;

/**
 * @brief Raw shader stage data.
 *
 * Contains compiled bytecode loaded from disk or memory.
 */
typedef struct emgpu_shader_src {
    /** @brief Pointer to compiled bytecode. */
    const void* data;

    /** @brief Size of the shader data in bytes. */
    u64 size;
} emgpu_shader_src;

/**
 * @brief Describes the capabilities of the active relevent device.
 */
typedef struct emgpu_device_capabilities {
    /** @brief Maximum supported anisotropic filtering level. */
    f32 max_anisotropy;

    /** @brief Device classification. */
    emgpu_device_type device_type;

    /** @brief Human-readable device name. */
    char* device_name;
} emgpu_device_capabilities;

/**
 * @brief Configuration for creating a renderer backend.
 */
typedef struct emgpu_device_config {
    /** @brief Enable validation and debug messages. */
    b8 enable_validation;

    /** @brief Enable sampler anisotropy if supported. */
    b8 sampler_anisotropy;

    /** @brief Prefer or require a discrete GPU. */
    b8 discrete_gpu;

    /** @brief Number of attachments in @ref main_attachments. */
    u32 main_attachment_count;

    /**
     * @brief Number of frames processed concurrently.
     *
     * Must be greater than 1.
     */
    u32 frames_in_flight;

    /** @brief Enabled renderer modes (bitmask). */
    emgpu_device_mode enabled_modes;

    /** @brief Selected backend API type. */
    emgpu_device_backend api_type;

    /** @brief Initial size of the main rendering surface (in pixels). */
    uvec2 size;

    /** @brief Name of the application. */
    const char* application_name;

    /** @brief Array of attachments for the main render target. */
    emgpu_rendertarget_attachment* main_attachments;
} emgpu_device_config;

/**
 * @brief Returns a default-initialized renderer backend configuration.
 *
 * @return Default configuration values.
 */
emgpu_device_config emgpu_device_default();
#pragma once

#include "ember/core.h"

#include "ember/gpu/format.h"

/**
 * @brief Supported rendering APIs.
 */
typedef enum emgpu_device_backend {
    EMBER_DEVICE_BACKEND_VULKAN,   /**< Vulkan device backend */ 
    EMBER_DEVICE_BACKEND_OPENGL,   /**< OpenGL device backend */
    EMBER_DEVICE_BACKEND_DIRECTX,  /**< DirectX device backend */
} emgpu_device_backend;

/**
 * @brief Util function for converting `emgpu_device_backend` to a string representation.
 * 
 * @param device_backend Enum type to convert.
 */
const char* emgpu_backend_type_string(emgpu_device_backend device_backend);

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

const char* emgpu_device_type_string(emgpu_device_type device_type);

/**
 * @brief Operating modes supported by the renderer.
 *
 * Modes may be combined as bit flags.
 */
typedef enum emgpu_device_mode {
    EMBER_DEVICE_MODE_GRAPHICS = 1 << 0,  /**< Graphics pipeline operations */
    EMBER_DEVICE_MODE_COMPUTE  = 1 << 1,  /**< Compute shader operations */
    EMBER_DEVICE_MODE_TRANSFER = 1 << 2,  /**< Data transfer operations */
    EMBER_DEVICE_MODE_PRESENT  = 1 << 3,  /**< Presentation to a platform surface */
} emgpu_device_mode;

/**
 * @brief Intended usage of a GPU buffer.
 *
 * Usage flags may be combined as bit flags.
 */
typedef enum emgpu_buffer_usage {
    EMBER_BUFFER_USAGE_VERTEX  = 1 << 0, /**< Vertex buffer */
    EMBER_BUFFER_USAGE_INDEX   = 1 << 1, /**< Index buffer */
    EMBER_BUFFER_USAGE_STORAGE = 1 << 2, /**< Storage buffer */
    EMBER_BUFFER_USAGE_TRANSFER_SRC = 1 << 3, /**< Transfer from this buffer */
    EMBER_BUFFER_USAGE_CPU_VISIBLE = 1 << 4,  /**< CPU-coherent buffer */
} emgpu_buffer_usage;

/**
 * @brief Intended usage of a texture.
 *
 * Usage flags may be combined as bit flags.
 */
typedef enum emgpu_texture_usage {
    EMBER_TEXTURE_USAGE_STORAGE = 1 << 0,      /**< Storage image */
    EMBER_TEXTURE_USAGE_SAMPLED = 1 << 1,      /**< Texture created with sampler */
    EMBER_TEXTURE_USAGE_TRANSFER_SRC = 1 << 2, /**< Transfer from this texture */
    EMBER_TEXTURE_USAGE_ATTACHMENT_DST = 1 << 4, /**< Used as a texture in a rendertarget */
} emgpu_texture_usage;

/**
 * @brief Describes how vertex data is interpreted and assembled into primitives.
 */
typedef enum emgpu_primitive_type {
    /**
     * @brief Each vertex represents a single point.
     * 
     * No connectivity between vertices. Useful for point rendering.
     */
    EMBER_PRIMITIVE_TYPE_POINT_LIST, 

    /**
     * @brief Each pair of vertices forms an independent line.
     * 
     * Vertices are grouped in pairs with no sharing.
     */
    EMBER_PRIMITIVE_TYPE_LINE_LIST,

    /**
     * @brief Vertices are connected in a continuous line strip.
     * 
     * Each vertex (after the first) forms a line with the previous vertex.
     */
    EMBER_PRIMITIVE_TYPE_LINE_STRIP,

    /**
     * @brief Each group of three vertices forms an independent triangle.
     * 
     * No vertex sharing between triangles.
     */
    EMBER_PRIMITIVE_TYPE_TRIANGLE_LIST,

    /**
     * @brief Forms a connected strip of triangles sharing vertices.
     * 
     * Each new vertex (after the first two) creates a triangle with the previous two.
     * Winding order typically alternates.
     */
    EMBER_PRIMITIVE_TYPE_TRIANGLE_STRIP,
} emgpu_primitive_type;

/**
 * @brief Type of render target attachment.
 *
 * Defines the logical purpose of an attachment within a render target.
 */
typedef enum emgpu_attachment_type {
    // TODO: Remove this attachment type as a window surface may not be colour.
    EMBER_ATTACHMENT_TYPE_PRESENT,  /**< Surface attachment (implictly colour) */
    EMBER_ATTACHMENT_TYPE_COLOUR,   /**< Colour render target (RGBA output) */
    EMBER_ATTACHMENT_TYPE_DEPTH,    /**< Depth-only attachment */
    EMBER_ATTACHMENT_TYPE_STENCIL,  /**< Stencil-only attachment */
    EMBER_ATTACHMENT_TYPE_DEPTH_STENCIL, /**< Combined depth-stencil attachment */
} emgpu_attachment_type;

/**
 * @brief Attachment load operation at the start of a render target.
 *
 * Determines how existing attachment contents are treated
 * when the render target begins.
 */
typedef enum emgpu_load_op {
    EMBER_LOAD_OP_LOAD,     /**< Preserve existing contents of the attachment */
    EMBER_LOAD_OP_CLEAR,    /**< Clear the attachment at the start of the target */
    EMBER_LOAD_OP_DONT_CARE, /**< Contents are undefined at the start of the target */
} emgpu_load_op;

/**
 * @brief Attachment store operation at the end of a render target.
 *
 * Determines whether attachment contents are preserved
 * after the render target completes.
 */
typedef enum emgpu_store_op {
    EMBER_STORE_OP_STORE,      /**< Store the results for later use */
    EMBER_STORE_OP_DONT_CARE,  /**< Contents become undefined after the target completes */
} emgpu_store_op;

/**
 * @brief Describes memory access types used for synchronization between pipelines.
 *
 * These flags represent how a resource is accessed within a pipeline.
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
    EMBER_ADDRESS_MODE_CLAMP_TO_BORDER,      /**< Clamp to border colour */
    EMBER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE, /**< Mirror then clamp to edge */
} emgpu_address_mode;

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

    /** @brief Name of entrypoint in shader ran by the rendering device. */
    const char* entry_point;
} emgpu_shader_src;

/**
 * @brief Describes the capabilities of the active relevent device.
 */
typedef struct emgpu_device_capabilities {
    /** @brief Internal graphics API used by the device. */
    emgpu_device_backend api_type;

    /** @brief Device classification. */
    emgpu_device_type device_type;

    /** @brief Version of the internal API used by the device. */
    em_version internal_api_version;

    /** @brief Version of the OS driver used by the device. */
    em_version driver_version;

    /** @brief Maximum supported anisotropic filtering level. */
    f32 max_anisotropy;

    /** @brief Human-readable name for driver vendor. */
    const char* vendor_name;

    /** @brief Human-readable device name. */
    char device_name[32];
} emgpu_device_capabilities;

/**
 * @brief Configuration for creating a renderer backend.
 */
typedef struct emgpu_device_config {
    /** @brief Enable validation and debug messages. */
    b8 enable_validation;

    /** @brief Enable sampler anisotropy if supported. */
    b8 sampler_anisotropy;

    /**
     * @brief Number of frames processed concurrently.
     *
     * Must be greater than 1.
     */
    u32 frames_in_flight;

    /** @brief Version of the cretaed app in `em_version` format. */
    em_version application_version;

    /** @brief Enabled renderer modes (bitmask). */
    emgpu_device_mode enabled_modes;

    /** @brief Selected backend API type. */
    emgpu_device_backend api_type;

    /** @brief Name of the application. */
    const char* application_name;
} emgpu_device_config;

/**
 * @brief Creates a default rendering device configuration.
 *
 * @return A default-initialized emgpu_device_config.
 */
emgpu_device_config emgpu_device_default();
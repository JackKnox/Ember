#pragma once

#include "ember/core.h"

#include "ember/gpu/device.h"

#include "platform/vulkan_platform.h"

// Checks the given Vulkan expression for success and fatally aborts on failure.
// Intended for calls that must never fail in a valid engine state.
#define VK_CHECK(expr)                                    \
    {                                                     \
        VkResult r = expr;                                \
        if (!vulkan_result_is_success(r))                 \
            EM_FATAL("VK_CHECK failed: (Line = %i) " __FILE__ "  (Error code: %s) ", \
                     __LINE__, vulkan_result_string(r, 1)); \
    }

// Checks a Vulkan call and logs an error on failure.
// Intended for recoverable errors during initialization or runtime.
#define CHECK_VKRESULT(func, message)                     \
    {                                                     \
        VkResult result = func;                           \
        if (!vulkan_result_is_success(result)) {          \
            EM_ERROR("Vulkan", message ": %s",             \
                     vulkan_result_string(result, EM_ENABLE_VALIDATION)); \
            return em_result_from_vulkan_result(result);  \
        }                                                 \
    }

typedef enum vulkan_queue_type {
    VULKAN_QUEUE_TYPE_GRAPHICS,
    VULKAN_QUEUE_TYPE_COMPUTE,
    VULKAN_QUEUE_TYPE_TRANSFER,
    VULKAN_QUEUE_TYPE_PRESENT,
    VULKAN_QUEUE_TYPE_MAX,
} vulkan_queue_type;

// Represents a queue handle together with the command pool used to allocate command buffers for that queue family.
typedef struct vulkan_queue {
    VkQueue handle;
    VkCommandPool pool;
    emgpu_device_mode supported_modes;
    i32 family_index;
} vulkan_queue;

// Represents a ring Vulkan command buffers and its current state.
typedef struct vulkan_command_buffer {
    VkCommandBuffer* handles;
    vulkan_queue* owner;
    u32 buffer_count;
    u32 curr_buffer;
} vulkan_command_buffer;

// Internal Vulkan implementation of a emgpu_surface.
typedef struct internal_vulkan_surface {
    VkSurfaceKHR surface;
    VkSurfaceCapabilitiesKHR capabilities;
    VkColorSpaceKHR colour_space;

    VkSwapchainKHR swapchain;
    emgpu_texture* swapchain_images;
} internal_vulkan_surface;

typedef struct internal_vulkan_renderpass {
    VkRenderPass handle;
} internal_vulkan_renderpass;

// Internal Vulkan implementation of a emgpu_pipeline.
typedef struct internal_vulkan_pipeline {
    VkPipeline handle;
    VkPipelineLayout layout;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet* descriptor_sets;
    VkDescriptorSetLayout descriptor;

    union {
        struct {
            emgpu_buffer* vertex_buffer, * index_buffer;
        } graphics;
    };
} internal_vulkan_pipeline;

// Internal Vulkan implementation of a emgpu_buffer.
typedef struct internal_vulkan_buffer {
    VkBuffer handle;
    VkDeviceMemory memory;
} internal_vulkan_buffer;

// Structure for extra texture configuration specific to this backend.
typedef struct vulkan_texture_ext_config {
    VkImage existing_image;
    b8 ownes_provided_image;
} vulkan_texture_ext_config;

// Internal Vulkan implementation of a emgpu_texture.
typedef struct internal_vulkan_texture {
    VkImage handle;
    emgpu_texture_usage usage;
    VkDeviceMemory memory;
    
    VkImageView view;
    VkSampler sampler;

    VkImageLayout curr_layout;
    b8 ownes_image;
} internal_vulkan_texture;

// Represents the global Vulkan backend context.
// Owns the Vulkan instance, device, swapchain, synchronization primitives, and per-frame resources.
typedef struct vulkan_context {
    emgpu_device_config config;

    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkDebugUtilsMessengerEXT debug_messenger;

    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    vulkan_queue mode_queues[VULKAN_QUEUE_TYPE_MAX];
    
    vulkan_command_buffer graphics_command_ring, compute_command_ring;
} vulkan_context;

// Finds a compatible memory type index on the physical device.
i32 find_memory_index(vulkan_context* context, u32 type_filter, VkMemoryPropertyFlags property_flags);

// Finds the necessary usage flags for a texture config.
VkImageUsageFlags vulkan_texture_usage(vulkan_context* context, const emgpu_texture_config* config);

// Finds the necessary usage flags for a buffer config.
VkBufferUsageFlags vulkan_buffer_usage(vulkan_context* context, const emgpu_buffer_config* config);

// Converts engine shader stage flags to Vulkan shader stage flags.
VkShaderStageFlags shader_type_to_vulkan_type(emgpu_shader_stage_type type);

// Converts engine descriptor type to a Vulkan descriptor type.
VkDescriptorType descriptor_type_to_vulkan_type(emgpu_descriptor_type descriptor_type);

// Converts engine filter mode to a Vulkan filter.
VkFilter filter_to_vulkan_type(emgpu_filter_type filter_type);

// Converts engine address mode to a Vulkan sampler address mode.
VkSamplerAddressMode address_mode_to_vulkan_type(emgpu_address_mode address);

// Converts engine primitive type to a Vulkan enum.
VkPrimitiveTopology primitive_to_vulkan_type(emgpu_primitive_type type);

// Converts engine render format to a Vulkan format.
VkFormat format_to_vulkan_type(emgpu_format format);

// Converts Vulkan format to engine render format, use sparingly.
emgpu_format vulkan_format_to_engine(VkFormat format);

// Converts engine load op format to a Vulkan format.
VkAttachmentLoadOp load_op_to_vulkan_type(emgpu_load_op load_op);

// Converts engine store op format to a Vulkan format.
VkAttachmentStoreOp store_op_to_vulkan_type(emgpu_store_op store_op);

// Converts engine attachment to a final image layout of a renderpass attachment.
VkImageLayout attachment_type_to_image_layout(emgpu_attachment_type type);

// Converts Vulkan error code to engine result code.
em_result em_result_from_vulkan_result(VkResult result);

// Returns a human-readable string for a Vulkan result code.
const char* vulkan_result_string(VkResult result, b8 get_extended);

// Determines whether a Vulkan result represents a success code.
b8 vulkan_result_is_success(VkResult result);
#pragma once

#include "ember/core.h"

#include "ember/gpu/types.h"

/**
 * @brief Describes the capabilities of the relevent device.
 */
typedef struct emgpu_device_capabilities {
    /** @brief Device classification. */
    emgpu_device_type device_type;

    /** @brief Enabled modes in device. */
    emgpu_device_mode enabled_modes;

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
 * @brief Describes an extension to the rendering device.
 *
 * Extensions are optional or required modules provided by the GPU backend
 * and requested by the user during device creation.
 *
 * They may:
 * - Enable additional GPU features
 * - Provide platform-specific integration (WSI)
 * - Expose extra API entry points
 *
 * Most extensions may require additional platform or library dependencies
 * to be linked. See offical documentation for more.
 */ 
typedef struct emgpu_extension_desc {
    /**
     * @brief Internal name of the extension.
     *
     * This name is defined by the system and is used for matching
     * against backend-supported extensions.
     *
     * @note This value is not intended to be used for application logic.
     */
    const char* name;

    /**
     * @brief Version of the extension.
     *
     * Used to ensure compatibility between the requested extension and
     * the backend implementation.
     */
    em_version version;

    /** @brief Type-erased create info for the extension. */
    void* user_data;

    /**
     * @brief Indicates whether the extension is optional.
     *
     * If set to FALSE, device initialization will fail if the extension
     * is not supported by the selected backend.
     */
    b8 optional;
} emgpu_extension_desc;

/**
 * @brief Configuration used for creating a GPU device.
 */
typedef struct emgpu_device_config {
    /** @brief Refrence to extra configuration structure specific to API type. */
    void* api_next;
    
    /** @brief Name of the application (used for debugging/profiling). */
    const char* debug_name;

    /** @brief Allocator used for per-frame allocations. */
    em_allocator frame_allocator;

    /** @brief Application version encoded as em_version. */
    em_version app_version;

    /**
     * @brief Required device modes.
     *
     * The device must support all modes in this bitmask or initialization fails.
     */
    emgpu_device_mode required_modes;

    /**
     * @brief Optional device modes.
     *
     * These will be enabled if supported. Unsupported modes are ignored.
     *
     * @note Final enabled modes are exposed via the device capabilities.
     */
    emgpu_device_mode optional_modes;

    /**
     * @brief Number of frames that can be processed concurrently.
     *
     * Must be greater than 1 for proper buffering and synchronization.
     */
    u32 frames_in_flight;

    /**
     * @brief Array of requested device extensions.
     *
     * Extensions may enable additional features or platform support.
     */
    emgpu_extension_desc* extensions;

    /** @brief Number of requested extensions. */
    u32 extension_count;

    /**
     * @brief Optional output array for resolved extension instances.
     *
     * If non-null, will be populated with pointers to active extension
     * instances matching the requested extensions.
     */
    void* out_extensions;
} emgpu_device_config;

#ifdef EMBER_DEFINE_HELPERS

/**
 * @brief Creates a default rendering device configuration.
 *
 * @return A default-initialized emgpu_device_config.
 */
emgpu_device_config emgpu_device_default();

#endif

/**
 * @brief Renderer device interface.
 *
 * Provides a backend-agnostic abstraction over GAPIs.
 * Implementations translate commands into API-specific calls.
 */
typedef struct emgpu_device {
    /** @brief Backend-specific device context or handle. */
    void* internal_context;

    /** @brief Index of the current frame in flight. */
    u32 current_frame;

    /** @brief Allocator used to manage per-frame resources. */
    em_allocator frame_allocator;
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
 * @brief Retreives capabilties of the rendering device.
 * 
 * @param device Pointer to the device instance.
 * @param out_capabilities Output capabilties structure.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emgpu_device_get_capabilities(
    emgpu_device* device, 
    emgpu_device_capabilities* out_capabilities);

#ifdef EMBER_DEFINE_HELPERS

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

#endif
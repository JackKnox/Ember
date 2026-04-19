#include "ember/core.h"
#include "vulkan_backend.h"

#include "ember/core/string_utils.h"

#include "frame_internal.h"

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	VkDebugUtilsMessageTypeFlagsEXT message_types,
	const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
	void* user_data) {
	switch (message_severity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		EM_ERROR("Vulkan", callback_data->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		EM_WARN("Vulkan", callback_data->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		EM_INFO("Vulkan", callback_data->pMessage);
		break;

	default:
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		EM_TRACE("Vulkan", callback_data->pMessage);
		break;
	}

	return VK_FALSE;
}

em_result vulkan_device_initialize(emgpu_device* device, const emgpu_device_config* config) {
    device->internal_context = mem_allocate(sizeof(vulkan_context), MEMORY_TAG_RENDERER);
    vulkan_context* context = (vulkan_context*)device->internal_context;
    context->config = *config;

    // * NOTE: This value is kind of a lie as each frame waits for the next one
    // *       in the future this should act as a trade between memory and performance.
    // *       (docs-assets.developer.apple.com/published/e75ecc44a7f513ac9322a840d37e2995/drawing-a-triangle-with-metal-4-1~dark%402x.png)
    /** config->frames_in_flight ... */

    // Global Vulkan init code
    // --------------------------------------
    u32 platform_extension_count = 0;
    const char** platform_extensions = vulkan_platform_get_required_extensions(&platform_extension_count);

	// Obtain a list of required extensions
	const char** required_extensions = darray_from_data(const char*, platform_extension_count, platform_extensions, MEMORY_TAG_RENDERER);
	const char** required_validation_layers = darray_create(const char*, MEMORY_TAG_RENDERER);

	// Add debug extensions/layers if enabled
	if (config->enable_validation) {
		darray_push(required_extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		darray_push(required_validation_layers, "VK_LAYER_KHRONOS_validation");
	}

    // Verify exsistence of validation layers
	u32 supported_layer_count = 0;
	vkEnumerateInstanceLayerProperties(&supported_layer_count, NULL);

	VkLayerProperties* supported_layers = darray_from_data(VkLayerProperties, supported_layer_count, NULL, MEMORY_TAG_RENDERER);
	vkEnumerateInstanceLayerProperties(&supported_layer_count, supported_layers);

	for (u32 i = 0; i < darray_length(required_validation_layers); ++i) {
		b8 found = FALSE;
		for (u32 j = 0; j < darray_length(supported_layers); ++j) {
			if (strings_equal(required_validation_layers[i], supported_layers[j].layerName)) {
				found = TRUE;
				break;
			}
		}

		if (!found) {
			EM_ERROR("Vulkan", "Required Vulkan validation layer is missing: %s.", required_validation_layers[i]);
			return FALSE;
		}
	}

	// Fill create info
	VkApplicationInfo app_info = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	app_info.apiVersion       = VK_API_VERSION_1_3;
    app_info.pApplicationName = config->application_name;
	app_info.pEngineName      = "ember_gpu";

    app_info.applicationVersion = VK_MAKE_API_VERSION(0, 
        EM_API_VERSION_MAJOR(config->application_version), 
        EM_API_VERSION_MINOR(config->application_version), 
        EM_API_VERSION_PATCH(config->application_version));

	app_info.engineVersion = VK_MAKE_API_VERSION(0, 
        EM_API_VERSION_MAJOR(EMBER_VERSION), 
        EM_API_VERSION_MINOR(EMBER_VERSION), 
        EM_API_VERSION_PATCH(EMBER_VERSION));

	VkInstanceCreateInfo create_info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	create_info.pApplicationInfo        = &app_info;
	create_info.enabledLayerCount       = darray_length(required_validation_layers);
	create_info.ppEnabledLayerNames     = required_validation_layers;
	create_info.enabledExtensionCount   = darray_length(required_extensions);
	create_info.ppEnabledExtensionNames = required_extensions;

    EM_INFO("Vulkan", "Creating Vulkan instance.");

	CHECK_VKRESULT(
		vkCreateInstance(
			&create_info,
			context->allocator,
			&context->instance),
		"Failed to create Vulkan instance");

	// Clean up temp arrays
	darray_destroy(supported_layers);
	darray_destroy(required_extensions);
	darray_destroy(required_validation_layers);

    // Setup debug messenger if validation enabled
	if (config->enable_validation) {
		VkDebugUtilsMessengerCreateInfoEXT debug_create_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
		debug_create_info.messageSeverity = 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		debug_create_info.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		debug_create_info.pfnUserCallback = vk_debug_callback;

		PFN_vkCreateDebugUtilsMessengerEXT create_debug_messenger = 
			(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context->instance, "vkCreateDebugUtilsMessengerEXT");

        EM_INFO("Vulkan", "Creating Vulkan validation layers.");

		CHECK_VKRESULT(
			create_debug_messenger(
				context->instance, 
				&debug_create_info, 
				context->allocator, 
				&context->debug_messenger),
			"Failed to create internal Vulkan debug messenger");
	}

    // Query the number of available Vulkan physical devices (GPUs)
    u32 physical_device_count = 0;
    CHECK_VKRESULT(
        vkEnumeratePhysicalDevices(context->instance, &physical_device_count, NULL), 
        "Failed to enumerate Vulkan devices");

    if (physical_device_count == 0) {
        EM_ERROR("Vulkan", "No devices which support Vulkan were found.");
        return EMBER_RESULT_UNAVAILABLE_API;
    }

    EM_TRACE("Vulkan", "Enumerated %i physical device(s).", physical_device_count);

    VkPhysicalDevice* physical_devices = darray_from_data(VkPhysicalDevice, physical_device_count, NULL, MEMORY_TAG_RENDERER);
    vkEnumeratePhysicalDevices(context->instance, &physical_device_count, physical_devices);

    // Build a list of required device extensions based on configuration
    const char** required_device_extensions = darray_create(const char*, MEMORY_TAG_RENDERER);
    if (config->enabled_modes & EMBER_DEVICE_MODE_PRESENT) darray_push(required_device_extensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    
    vulkan_queue queue_support[VULKAN_QUEUE_TYPE_MAX] = {};
    // Iterate over all available physical devices to find the most suitable one
    for (u32 i = 0; i < physical_device_count; ++i) {
        VkPhysicalDevice physical_device = physical_devices[i];
        for (u32 i = 0; i < EM_ARRAYSIZE(queue_support); ++i) {
            queue_support[i].family_index = -1;
            queue_support[i].supported_modes = 0;
        }

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physical_device, &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(physical_device, &features);

        EM_INFO("Vulkan", "Checking device %s", properties.deviceName);

        u32 queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, 0);
        VkQueueFamilyProperties* queue_families = darray_from_data(VkQueueFamilyProperties, queue_family_count, NULL, MEMORY_TAG_RENDERER);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);

        // Look at each queue and see what queues it supports
        u8 min_transfer_score = 255;
        for (u32 i = 0; i < queue_family_count; ++i) {
            u8 current_transfer_score = 0;

            // Graphics queue?
            if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                queue_support[VULKAN_QUEUE_TYPE_GRAPHICS].family_index = i;
                queue_support[VULKAN_QUEUE_TYPE_GRAPHICS].supported_modes |= EMBER_DEVICE_MODE_GRAPHICS;
                ++current_transfer_score;
            }

            // Compute queue?
            if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
                queue_support[VULKAN_QUEUE_TYPE_COMPUTE].family_index = i;
                queue_support[VULKAN_QUEUE_TYPE_COMPUTE].supported_modes |= EMBER_DEVICE_MODE_COMPUTE;
                ++current_transfer_score;
            }

            // Transfer queue?
            if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
                // Take the index if it is the current lowest. This increases the
                // liklihood that it is a dedicated transfer queue.
                if (current_transfer_score <= min_transfer_score) {
                    min_transfer_score = current_transfer_score;
                    queue_support[VULKAN_QUEUE_TYPE_TRANSFER].family_index = i;
                    queue_support[VULKAN_QUEUE_TYPE_TRANSFER].supported_modes |= EMBER_DEVICE_MODE_TRANSFER;
                }
            }

            // Present queue?
            if (vulkan_platform_presentation_support(context->instance, physical_device, i)) {
                queue_support[VULKAN_QUEUE_TYPE_PRESENT].family_index = i;
                queue_support[VULKAN_QUEUE_TYPE_PRESENT].supported_modes |= EMBER_DEVICE_MODE_PRESENT;
            }
        }

        if (((config->enabled_modes & EMBER_DEVICE_MODE_GRAPHICS) && queue_support[VULKAN_QUEUE_TYPE_GRAPHICS].family_index == -1) ||
            ((config->enabled_modes & EMBER_DEVICE_MODE_COMPUTE)  && queue_support[VULKAN_QUEUE_TYPE_COMPUTE].family_index  == -1) ||
            ((config->enabled_modes & EMBER_DEVICE_MODE_TRANSFER) && queue_support[VULKAN_QUEUE_TYPE_TRANSFER].family_index == -1) ||
            ((config->enabled_modes & EMBER_DEVICE_MODE_PRESENT)  && queue_support[VULKAN_QUEUE_TYPE_PRESENT].family_index  == -1)) {
            continue;
        }

        EM_TRACE("Vulkan", "  Graphics queue family: %i", queue_support[VULKAN_QUEUE_TYPE_GRAPHICS].family_index);
        EM_TRACE("Vulkan", "  Compute queue family:  %i", queue_support[VULKAN_QUEUE_TYPE_COMPUTE].family_index); 
        EM_TRACE("Vulkan", "  Transfer queue family: %i", queue_support[VULKAN_QUEUE_TYPE_TRANSFER].family_index);

        u32 supported_extension_count = 0;
        vkEnumerateDeviceExtensionProperties(physical_device, NULL, &supported_extension_count, NULL);

        VkExtensionProperties* supported_extensions = darray_from_data(VkExtensionProperties, supported_extension_count, NULL, MEMORY_TAG_RENDERER);
        vkEnumerateDeviceExtensionProperties(physical_device, NULL, &supported_extension_count, supported_extensions);

        for (u32 i = 0; i < darray_length(required_device_extensions); ++i) {
            b8 found = FALSE;
            for (u32 j = 0; j < darray_length(supported_extensions); ++j) {
                if (strings_equal(required_device_extensions[i], supported_extensions[j].extensionName)) {
                    found = TRUE;
                    break;
                }
            }

            if (!found) {
                EM_INFO("Vulkan", "Device does not support required extensions: %s.", required_device_extensions[i]);
                continue;
            }
        }

        darray_destroy(queue_families);
        darray_destroy(supported_extensions);

        EM_TRACE("Vulkan", "  Present support: %s", queue_support[VULKAN_QUEUE_TYPE_PRESENT].family_index != -1 ? "yes" : "no");

        // Sampler anisotropy
        if (config->sampler_anisotropy && !features.samplerAnisotropy) {
            EM_INFO("Vulkan", "Device does not support sampler anisotropy.");
            continue;
        }

        // Device meets all requirements.
        EM_INFO("Vulkan", "Selected device '%s'", properties.deviceName);
        context->physical_device = physical_devices[i];
        break;
    }

    // Clean up temporary arrays
    darray_destroy(physical_devices);

    // Ensure that a suitable physical device was selected
    if (!context->physical_device) {
        EM_ERROR("Vulkan", "No physical devices were found which meet the requirements.");
        return EMBER_RESULT_UNAVAILABLE_API;
    }

    EM_INFO("Vulkan", "Creating logical device.");

    f32 queue_priority = 1.0f;
    VkDeviceQueueCreateInfo* queue_create_info = darray_create(VkDeviceQueueCreateInfo, MEMORY_TAG_RENDERER);

    for (u32 i = 0; i < VULKAN_QUEUE_TYPE_MAX; ++i) {
        u32 family = queue_support[i].family_index;
        if (family == -1) continue;

        b8 exists = FALSE;
        for (u32 j = 0; j < darray_length(queue_create_info); ++j) {
            if (queue_create_info[j].queueFamilyIndex == family) {
                exists = TRUE;
                break;
            }
        }

        if (!exists) {
            VkDeviceQueueCreateInfo* create_info = darray_push_empty(queue_create_info);
            create_info->sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            create_info->queueFamilyIndex = family;
            create_info->queueCount       = 1;
            create_info->pQueuePriorities = &queue_priority;
        }
    }

    EM_TRACE("Vulkan", "Requesting %i unique queue families.", darray_length(queue_create_info));

    // Request device features.
    VkPhysicalDeviceFeatures device_features = {};
    device_features.samplerAnisotropy = context->config.sampler_anisotropy;  // Request anisotropy

    VkDeviceCreateInfo device_create_info = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    device_create_info.queueCreateInfoCount = darray_length(queue_create_info);
    device_create_info.pQueueCreateInfos = queue_create_info;
    device_create_info.enabledExtensionCount = darray_length(required_device_extensions);
    device_create_info.ppEnabledExtensionNames = required_device_extensions;
    device_create_info.pEnabledFeatures = &device_features;

    // Create the device.
    VkResult result = vkCreateDevice(context->physical_device, &device_create_info, context->allocator, &context->logical_device);
    if (!vulkan_result_is_success(result)) return result;

    darray_destroy(required_device_extensions);
    darray_destroy(queue_create_info);

    EM_TRACE("Vulkan", "Creating queue command pools");

    // Create command pool for necessary queue.
    for (u32 i = 0; i < EM_ARRAYSIZE(context->mode_queues); ++i) {
        if (!(context->config.enabled_modes & queue_support[i].supported_modes)) continue;
        vulkan_queue* mode = &context->mode_queues[i];

        mode->family_index = queue_support[i].family_index;
        mode->supported_modes = queue_support[i].supported_modes;

        vkGetDeviceQueue(
            context->logical_device,
            mode->family_index, 
            0, 
            &mode->handle);

        if (i == VULKAN_QUEUE_TYPE_PRESENT) {
            // Present doesn't need a command pool, managed by swapchain and extension.
            continue;
        }

        VkCommandPoolCreateInfo pool_create_info = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        pool_create_info.queueFamilyIndex = mode->family_index;
        pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VkResult result = vkCreateCommandPool(context->logical_device, &pool_create_info, context->allocator, &mode->pool);
        if (!vulkan_result_is_success(result)) return result;
    }
    // --------------------------------------

    EM_INFO("Vulkan", "Creating intermediate render objects");

    if (config->enabled_modes & EMBER_DEVICE_MODE_GRAPHICS) {
		vulkan_queue_type queue_type = VULKAN_QUEUE_TYPE_GRAPHICS;
        EM_TRACE("Vulkan", "Creating global Vulkan graphics command ring");

        VkCommandBufferAllocateInfo allocate_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        allocate_info.commandPool = context->mode_queues[queue_type].pool;
        allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocate_info.commandBufferCount = context->config.frames_in_flight;

        context->graphics_command_ring = mem_allocate(sizeof(VkCommandBuffer) * allocate_info.commandBufferCount, MEMORY_TAG_RENDERER);

        CHECK_VKRESULT(
            vkAllocateCommandBuffers(
                context->logical_device, 
                &allocate_info, 
                context->graphics_command_ring),
            "Failed to create Vulkan graphics command buffers");
	}

	if (config->enabled_modes & EMBER_DEVICE_MODE_COMPUTE) {
		vulkan_queue_type queue_type = VULKAN_QUEUE_TYPE_COMPUTE;
        EM_TRACE("Vulkan", "Creating global Vulkan compute command ring");

        VkCommandBufferAllocateInfo allocate_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        allocate_info.commandPool = context->mode_queues[queue_type].pool;
        allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocate_info.commandBufferCount = context->config.frames_in_flight;

        context->compute_command_ring = mem_allocate(sizeof(VkCommandBuffer) * allocate_info.commandBufferCount, MEMORY_TAG_RENDERER);

        CHECK_VKRESULT(
            vkAllocateCommandBuffers(
                context->logical_device, 
                &allocate_info, 
                context->compute_command_ring),
            "Failed to create Vulkan compute command buffers");
	}

    context->semaphore_pool = darray_create(VkSemaphore, MEMORY_TAG_RENDERER);
    context->in_flight_fences = darray_reserve(VkFence, context->config.frames_in_flight, MEMORY_TAG_RENDERER);

    for (u32 i = 0; i < context->config.frames_in_flight; ++i) {
		VkFenceCreateInfo fence_create_info = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		CHECK_VKRESULT(
			vkCreateFence(
				context->logical_device,
				&fence_create_info,
				context->allocator,
				darray_push_empty(context->in_flight_fences)),
			"Failed to create Vulkan sync objects");
    }

    EM_INFO("Vulkan", "Rendering device fully initiailized.");
    return EMBER_RESULT_OK;
}

void vulkan_device_shutdown(emgpu_device* device) {
    vulkan_context* context = (vulkan_context*)device->internal_context;
    if (context->logical_device) vkDeviceWaitIdle(context->logical_device);

    for (u32 i = 0; i < context->config.frames_in_flight; ++i) {
        if (context->in_flight_fences[i]) {
            vkDestroyFence(
                context->logical_device, 
                context->in_flight_fences[i],
                context->allocator);
        }
    }

    for (u32 i = 0; i < darray_length(context->semaphore_pool); ++i) {
        if (context->semaphore_pool[i]) {
            vkDestroySemaphore(
                context->logical_device, 
                context->semaphore_pool[i], 
                context->allocator);
        }
    }

    darray_destroy(context->in_flight_fences);
    darray_destroy(context->semaphore_pool);

    if (context->compute_command_ring) {
		vulkan_queue_type queue_type = VULKAN_QUEUE_TYPE_COMPUTE;
        vkFreeCommandBuffers(
            context->logical_device, 
            context->mode_queues[queue_type].pool, 
            context->config.frames_in_flight, 
            context->compute_command_ring);

        mem_free(context->compute_command_ring, sizeof(VkCommandBuffer) * context->config.frames_in_flight, MEMORY_TAG_RENDERER);
    }

    if (context->graphics_command_ring) {
		vulkan_queue_type queue_type = VULKAN_QUEUE_TYPE_GRAPHICS;
        vkFreeCommandBuffers(
            context->logical_device, 
            context->mode_queues[queue_type].pool, 
            context->config.frames_in_flight, 
            context->graphics_command_ring);
        
        mem_free(context->graphics_command_ring, sizeof(VkCommandBuffer) * context->config.frames_in_flight, MEMORY_TAG_RENDERER);
    }

    for (u32 i = 0; i < EM_ARRAYSIZE(context->mode_queues); ++i) {
        if (context->mode_queues[i].pool) {
            vkDestroyCommandPool(
                context->logical_device, 
                context->mode_queues[i].pool, 
                context->allocator);
        }
    }

    if (context->logical_device)
        vkDestroyDevice(context->logical_device, context->allocator);

    if (context->debug_messenger) {
        PFN_vkDestroyDebugUtilsMessengerEXT func =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context->instance, "vkDestroyDebugUtilsMessengerEXT");
        
        func(context->instance, context->debug_messenger, context->allocator);
    }

    if (context->instance)
        vkDestroyInstance(context->instance, context->allocator);

    mem_free(context, sizeof(vulkan_context), MEMORY_TAG_RENDERER);
    device->internal_context = NULL;
}

emgpu_device_capabilities* vulkan_device_capabilities(emgpu_device* device) {
    vulkan_context* context = (vulkan_context*)device->internal_context;

    if (device->capabilities == NULL) {
        // Query physical device properties from Vulkan
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(context->physical_device, &properties);

        // --- Extended driver properties (Vulkan 1.1+)
        VkPhysicalDeviceDriverProperties driver_props = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES };

        VkPhysicalDeviceProperties2 extended_props = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
        extended_props.pNext = &driver_props;
        vkGetPhysicalDeviceProperties2(context->physical_device, &extended_props);

        device->capabilities = (emgpu_device_capabilities*)mem_allocate(sizeof(emgpu_device_capabilities), MEMORY_TAG_RENDERER);
        emc_memcpy(device->capabilities->device_name, properties.deviceName, sizeof(device->capabilities->device_name));

        switch (driver_props.driverID) {
            case VK_DRIVER_ID_AMD_PROPRIETARY:
            case VK_DRIVER_ID_AMD_OPEN_SOURCE:
                device->capabilities->vendor_name = "AMD";
                break;
            case VK_DRIVER_ID_NVIDIA_PROPRIETARY:
                device->capabilities->vendor_name = "NVIDIA";
                break;
            case VK_DRIVER_ID_INTEL_PROPRIETARY_WINDOWS:
            case VK_DRIVER_ID_INTEL_OPEN_SOURCE_MESA:
                device->capabilities->vendor_name = "Intel";
                break;
            case VK_DRIVER_ID_IMAGINATION_PROPRIETARY:
                device->capabilities->vendor_name = "Imagination Technologies";
                break;
            case VK_DRIVER_ID_QUALCOMM_PROPRIETARY:
                device->capabilities->vendor_name = "Qualcomm";
                break;
            case VK_DRIVER_ID_ARM_PROPRIETARY:
                device->capabilities->vendor_name = "ARM";
                break;
            case VK_DRIVER_ID_MOLTENVK:
                device->capabilities->vendor_name = "MoltenVK";
                break;
            case VK_DRIVER_ID_SAMSUNG_PROPRIETARY:
                device->capabilities->vendor_name = "Samsung";
                break;
        }

        // Populate capability fields
        device->capabilities->api_type = EMBER_DEVICE_BACKEND_VULKAN;
        device->capabilities->device_type = (emgpu_device_type)properties.deviceType;
        device->capabilities->max_anisotropy = properties.limits.maxSamplerAnisotropy;

        // Convert Vulkan API version into engine-specific version format
        device->capabilities->internal_api_version = EM_API_MAKE_VERSION(
            VK_API_VERSION_MAJOR(properties.apiVersion),
            VK_API_VERSION_MINOR(properties.apiVersion),
            VK_API_VERSION_PATCH(properties.apiVersion));

        // Convert Vulkan driver version into engine-specific version format
        device->capabilities->driver_version = EM_API_MAKE_VERSION(
            VK_API_VERSION_MAJOR(properties.driverVersion),
            VK_API_VERSION_MINOR(properties.driverVersion),
            VK_API_VERSION_PATCH(properties.driverVersion));
    }

    return device->capabilities;
}

em_result vulkan_device_submit_frame(emgpu_device* device, const emgpu_frame* frame) {
    return EMBER_RESULT_UNIMPLEMENTED;
}
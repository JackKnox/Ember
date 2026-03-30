#include "defines.h"
#include "vulkan_backend.h"

#include "ember/core/string_utils.h"

#include "vulkan_command_buffer.h"
#include "vulkan_device.h"
#include "vulkan_renderbuffer.h"
#include "vulkan_renderstage.h"
#include "vulkan_rendertarget.h"
#include "vulkan_texture.h"
#include "vulkan_window_system.h"

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	VkDebugUtilsMessageTypeFlagsEXT message_types,
	const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
	void* user_data) {
	switch (message_severity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		EM_ERROR(callback_data->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		EM_WARN(callback_data->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		EM_INFO(callback_data->pMessage);
		break;

	default:
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		EM_TRACE(callback_data->pMessage);
		break;
	}

	return VK_FALSE;
}

b8 vulkan_device_initialize(emgpu_device* device, emgpu_device_config* config) {
	EM_ASSERT(device != NULL && config != NULL && "Invalid arguments passed to vulkan_device_initialize");
	device->internal_context = ballocate(sizeof(vulkan_context), MEMORY_TAG_RENDERER);
	vulkan_context* context = (vulkan_context*)device->internal_context;

	context->config = *config;

	if (device->window_context == NULL) {
		EM_ERROR("Vulkan device: Offscreen renderering is not supported by the Vulkan backend");
		return FALSE;
	}

    // Global Vulkan init code (can technically span across multiple rendering devices)
    // --------------------------------------
	u32 platform_extensions_count = 0;
	const char** platform_extensions = vulkan_platform_get_required_extensions(&platform_extensions_count);

	// Obtain a list of required extensions
	const char** required_extensions = darray_from_data(const char*, platform_extensions_count, platform_extensions, MEMORY_TAG_RENDERER);
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
			EM_ERROR("Required Vulkan validation layer is missing: %s.", required_validation_layers[i]);
			return FALSE;
		}
	}

	// Fill create info
	VkApplicationInfo app_info = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	app_info.apiVersion = VK_API_VERSION_1_2;
	app_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	app_info.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
	app_info.pApplicationName = config->application_name;
	app_info.pEngineName = "Ember-GPU";

	VkInstanceCreateInfo create_info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	create_info.pApplicationInfo = &app_info;
	create_info.enabledExtensionCount = darray_length(required_extensions);
	create_info.ppEnabledExtensionNames = required_extensions;
	create_info.enabledLayerCount = darray_length(required_validation_layers);
	create_info.ppEnabledLayerNames = required_validation_layers;

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
		u32 log_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

		VkDebugUtilsMessengerCreateInfoEXT debug_create_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
		debug_create_info.messageSeverity = log_severity;
		debug_create_info.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		debug_create_info.pfnUserCallback = vk_debug_callback;

		PFN_vkCreateDebugUtilsMessengerEXT create_debug_messenger = 
			(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context->instance, "vkCreateDebugUtilsMessengerEXT");

		CHECK_VKRESULT(
			create_debug_messenger(
				context->instance, 
				&debug_create_info, 
				context->allocator, 
				&context->debug_messenger),
			"Failed to create internal Vulkan debug messenger");
	}

	// Create the Vulkan device
	CHECK_VKRESULT(
		vulkan_device_create(device),
		"Failed to create Vulkan device");
    // --------------------------------------

	// Vulkan window system code.
    // --------------------------------------
	if (device->window_context != NULL) {
		EM_ASSERT(device->window_context->internal_renderer_state == NULL && "Invalid state reached: Platform state already has renderer attached!");
	
		device->window_context->internal_renderer_state = ballocate(sizeof(vulkan_window_system), MEMORY_TAG_RENDERER);
		vulkan_window_system* window_system = (vulkan_window_system*)device->window_context->internal_renderer_state;

		CHECK_VKRESULT(
			vulkan_window_system_create(device, window_system),
			"Failed to create internal Vulkan window surface / swapchain");
	}
    // --------------------------------------

	context->in_flight_fences = darray_reserve(VkFence, config->frames_in_flight, MEMORY_TAG_RENDERER);

	for (u32 i = 0; i < config->frames_in_flight; ++i) {
		VkFence* fence = darray_push_empty(context->in_flight_fences);

		VkFenceCreateInfo fence_create_info = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		CHECK_VKRESULT(
			vkCreateFence(
				context->device.logical_device,
				&fence_create_info,
				context->allocator,
				fence),
			"Failed to create Vulkan sync objects");
	}

	if (config->enabled_modes & EMBER_DEVICE_MODE_GRAPHICS) {
		vulkan_queue_type queue_type = VULKAN_QUEUE_TYPE_GRAPHICS;
		context->graphics_command_ring = darray_reserve(vulkan_command_buffer, config->frames_in_flight, MEMORY_TAG_RENDERER);

		for (u32 i = 0; i < config->frames_in_flight; ++i) {
			// TODO: Allocate command buffer all at once.
			CHECK_VKRESULT(
				vulkan_command_buffer_allocate(
					context,
					&context->device.mode_queues[queue_type],
					TRUE,
					&context->graphics_command_ring[i]),
				"Failed to create Vulkan command buffers");
		}
	}

	if (config->enabled_modes & EMBER_DEVICE_MODE_COMPUTE) {
		vulkan_queue_type queue_type = VULKAN_QUEUE_TYPE_COMPUTE;
		context->compute_command_ring = darray_reserve(vulkan_command_buffer, config->frames_in_flight, MEMORY_TAG_RENDERER);

		for (u32 i = 0; i < config->frames_in_flight; ++i) {
			// TODO: Allocate command buffer all at once.
			CHECK_VKRESULT(
				vulkan_command_buffer_allocate(
					context,
					&context->device.mode_queues[queue_type],
					TRUE,
					&context->compute_command_ring[i]),
				"Failed to create Vulkan command buffers");
		}
	}

    // Per frame structures (needs BIG improvements soon)
    // --------------------------------------
	context->memory_barriers = darray_create(memory_barrier, MEMORY_TAG_RENDERER);
	context->queued_submissions = darray_create(vulkan_queue_submission, MEMORY_TAG_RENDERER);
	context->semaphore_pool = darray_create(VkSemaphore, MEMORY_TAG_RENDERER);

	VkSemaphoreCreateInfo semaphore_create_info = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	for (u32 i = 0; i < VULKAN_QUEUE_TYPE_MAX * config->frames_in_flight; ++i) {
		VkSemaphore* semaphore = darray_push_empty(context->semaphore_pool);

		CHECK_VKRESULT(
			vkCreateSemaphore(
				context->device.logical_device,
				&semaphore_create_info,
				context->allocator,
				semaphore),
			"Failed to create Vulkan semaphore in pool");
	}
    // --------------------------------------

	return TRUE;
}

void vulkan_device_shutdown(emgpu_device* device) {
	EM_ASSERT(device != NULL && "Invalid arguments passed to vulkan_device_shutdown");
	vulkan_context* context = (vulkan_context*)device->internal_context;
	if (context->device.logical_device) vkDeviceWaitIdle(context->device.logical_device);

	// Destroy in the opposite order of creation.

    // Per frame structures
    // --------------------------------------
	if (context->semaphore_pool) {
		for (u32 i = 0; i < darray_length(context->semaphore_pool); ++i) {
			if (!context->semaphore_pool[i]) continue;

			vkDestroySemaphore(
				context->device.logical_device, 
				context->semaphore_pool[i], 
				context->allocator);
		}

		darray_destroy(context->semaphore_pool);
	}

	if (context->memory_barriers) darray_destroy(context->memory_barriers);

	if (context->queued_submissions) darray_destroy(context->queued_submissions);
    // --------------------------------------

	if (context->graphics_command_ring) {
		for (u32 i = 0; i < darray_length(context->graphics_command_ring); ++i) {
			vulkan_command_buffer_free(
				context, 
				&context->graphics_command_ring[i]);
		}

		darray_destroy(context->graphics_command_ring);
	}

	if (context->compute_command_ring) {
		for (u32 i = 0; i < darray_length(context->compute_command_ring); ++i) {
			vulkan_command_buffer_free(
				context, 
				&context->compute_command_ring[i]);
		}

		darray_destroy(context->compute_command_ring);
	}

	if (context->in_flight_fences) {
		for (u32 i = 0; i < darray_length(context->in_flight_fences); ++i) {
			if (!context->in_flight_fences[i]) continue;

			vkDestroyFence(
				context->device.logical_device,
				context->in_flight_fences[i],
				context->allocator);
		}

		darray_destroy(context->in_flight_fences);
	}

	if (device->window_context && device->window_context->internal_renderer_state) {
		vulkan_window_system* window_system = (vulkan_window_system*)device->window_context->internal_renderer_state;
		vulkan_window_system_destroy(device, window_system);
		bfree(window_system, sizeof(vulkan_window_system), MEMORY_TAG_RENDERER);
	}

	vulkan_device_destroy(device);

	if (context->instance) {
		if (context->debug_messenger) {
			PFN_vkDestroyDebugUtilsMessengerEXT func =
				(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context->instance, "vkDestroyDebugUtilsMessengerEXT");
			
			func(context->instance, context->debug_messenger, context->allocator);
		}

		vkDestroyInstance(context->instance, context->allocator);
	}

	bfree(context, sizeof(vulkan_context), MEMORY_TAG_RENDERER);
	device->internal_context = NULL;
}

void vulkan_device_resized(emgpu_device* device, uvec2 new_size) {
	EM_ASSERT(device != NULL && "Invalid arguments passed to vulkan_device_resized");
	// TODO: Swapchain recreation.
}

b8 vulkan_device_window_textures(emgpu_device* device, emgpu_texture** out_textures) {
	if (!device->window_context || !device->window_context->internal_renderer_state) {
		EM_ERROR("Rendering device has no window context attached.");
		return FALSE;
	}

	vulkan_window_system* window_system = (vulkan_window_system*)device->window_context->internal_renderer_state;
	*out_textures = window_system->swapchain_images;
	return TRUE;
}

b8 vulkan_device_update_descriptors(
	emgpu_device* device,
    emgpu_update_descriptors* descriptors,
    u32 descriptor_count) {
    EM_ASSERT(device != NULL && descriptors && "Invalid arguments passed to vulkan_renderstage_update_descriptors");
    vulkan_context* context = (vulkan_context*)device->internal_context;

    u32 image_count = context->config.frames_in_flight;
    u32 max_writes  = descriptor_count * image_count;

    VkWriteDescriptorSet* write_commands = darray_reserve(VkWriteDescriptorSet, max_writes, MEMORY_TAG_RENDERER);
    VkDescriptorBufferInfo* buffer_infos = darray_reserve(VkDescriptorBufferInfo, max_writes, MEMORY_TAG_RENDERER);
    VkDescriptorImageInfo* image_infos = darray_reserve(VkDescriptorImageInfo, max_writes, MEMORY_TAG_RENDERER);

    for (u32 i = 0; i < descriptor_count; ++i) {
        emgpu_update_descriptors* write = &descriptors[i];
        const emgpu_descriptor_desc* layout_desc = &write->renderstage->descriptors[write->binding];

        if (write->type != layout_desc->descriptor_type) {
            EM_ERROR("Descriptor type mismatch at binding %u (write=%u, layout=%u)", write->binding, write->type, layout_desc->descriptor_type);
            continue;
        }

        EM_ASSERT(write->renderstage != NULL && "vulkan_renderstage_update_descriptors(): Malformed data when updating descriptors");
        internal_vulkan_renderstage* internal_renderstage = (internal_vulkan_renderstage*)write->renderstage->internal_data;

        for (u32 j = 0; j < image_count; ++j) {
            VkWriteDescriptorSet* descriptor_write = darray_push_empty(write_commands);
			descriptor_write->sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_write->dstSet          = internal_renderstage->descriptor_sets[j];
			descriptor_write->dstBinding      = write->binding;
			descriptor_write->dstArrayElement = 0;
			descriptor_write->descriptorType  = descriptor_type_to_vulkan_type(write->type);
			descriptor_write->descriptorCount = 1;

            switch (write->type) {
                case EMBER_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                    internal_vulkan_renderbuffer* renderbuffer = (internal_vulkan_renderbuffer*)write->buffer->internal_data;

                    VkDescriptorBufferInfo* buffer_info = darray_push_empty(buffer_infos);
                    buffer_info->buffer = renderbuffer->handle;
                    buffer_info->range  = write->buffer->buffer_size;
                    buffer_info->offset = 0;
                    descriptor_write->pBufferInfo = buffer_info;
					break;

                case EMBER_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                case EMBER_DESCRIPTOR_TYPE_IMAGE_SAMPLER:
                    internal_vulkan_texture* texture = (internal_vulkan_texture*)write->texture->internal_data;

                    VkDescriptorImageInfo* image_info = darray_push_empty(image_infos);
                    image_info->sampler     = texture->sampler;
                    image_info->imageView   = texture->view;
                    image_info->imageLayout = texture->layout;
                    descriptor_write->pImageInfo = image_info;
					break;

                default:
                    EM_ASSERT(FALSE && "Unsupported descriptor type");
                    continue;
            }
        }
    }

    if (darray_length(write_commands) > 0)
        vkUpdateDescriptorSets(context->device.logical_device, darray_length(write_commands), write_commands, 0, NULL);

    darray_destroy(buffer_infos);
    darray_destroy(image_infos);
    darray_destroy(write_commands);

    return TRUE;
}

b8 vulkan_device_begin_frame(emgpu_device* device, f64 delta_time) {
	EM_ASSERT(device != NULL && "Invalid arguments passed to vulkan_device_begin_frame");
    vulkan_context* context = (vulkan_context*)device->internal_context;

	CHECK_VKRESULT(
		vkWaitForFences(
			context->device.logical_device,
			1, &context->in_flight_fences[context->current_frame],
			TRUE, 
			UINT64_MAX),
		"Failed to wait or reset internal Vulkan fence");

	CHECK_VKRESULT(
		vkResetFences(
			context->device.logical_device, 
			1, &context->in_flight_fences[context->current_frame]),
		"Failed to wait or reset Vulkan fence");

	if (device->window_context != NULL) {
		vulkan_window_system* window_system = (vulkan_window_system*)device->window_context->internal_renderer_state;

		CHECK_VKRESULT(
			vulkan_window_system_acquire(
				device, 
				window_system, 
				UINT64_MAX,
				VK_NULL_HANDLE),
			"Failed to accquire next Vulkan swapchain image");
	}
	
	darray_clear(context->memory_barriers);
	darray_clear(context->queued_submissions);
	context->last_mode = 0;
    return TRUE;
}

void vulkan_device_execute_command(emgpu_device* device, emgpu_playback_context* playback_context, rendercmd_header* hdr, rendercmd_payload* payload) {
	EM_ASSERT(device != NULL && playback_context != NULL && hdr != NULL && payload != NULL && "Invalid arguments passed to vulkan_device_execute_command");
    vulkan_context* context = (vulkan_context*)device->internal_context;

	if (playback_context->current_mode != context->last_mode) {
		vulkan_queue_submission* new = darray_push_empty(context->queued_submissions);

		switch (playback_context->current_mode) {
			case EMBER_DEVICE_MODE_GRAPHICS: 
				new->command_buffer = &context->graphics_command_ring[context->current_frame];
				break;

			case EMBER_DEVICE_MODE_COMPUTE: 
				new->command_buffer = &context->compute_command_ring[context->current_frame];
				break;

			default:
				EM_ASSERT(FALSE && "Unsupported renderer mode");
				break;
		}

		new->signal_semaphore = context->semaphore_pool[context->semaphore_next_index];
		context->semaphore_next_index = (context->semaphore_next_index + 1) % darray_length(context->semaphore_pool);

		new->wait_semaphores = darray_create(VkSemaphore, MEMORY_TAG_RENDERER);

        vulkan_command_buffer_reset(new->command_buffer);
		vulkan_command_buffer_begin(new->command_buffer, FALSE, FALSE, FALSE);
	}

	context->last_mode = playback_context->current_mode;
	
	vulkan_queue_submission* curr_submission = &context->queued_submissions[darray_length(context->queued_submissions) - 1];

    switch (hdr->type) {
    case RENDERCMD_BIND_RENDERTARGET:
        vulkan_rendertarget_begin(
			context, curr_submission->command_buffer,
            playback_context->current_target,
            TRUE, TRUE);
        break;

	case RENDERCMD_MEMORY_BARRIER:
		memory_barrier* barrier = darray_push_empty(context->memory_barriers);
		barrier->created_on_submission = darray_length(context->queued_submissions) - 1;
		barrier->src_renderstage = payload->memory_barrier.src_renderstage;
		barrier->dst_renderstage = payload->memory_barrier.dst_renderstage;
		barrier->src_access = payload->memory_barrier.src_access;
		barrier->dst_access = payload->memory_barrier.dst_access;
		break;

    case RENDERCMD_BEGIN_RENDERSTAGE:
		for (u32 i = 0; i < darray_length(context->memory_barriers); ++i) {
			if (context->memory_barriers[i].dst_renderstage != payload->begin_renderstage.renderstage)
				continue;
			
			memory_barrier barrier = {};
			darray_pop_at(context->memory_barriers, i, &barrier);

			darray_push(curr_submission->wait_semaphores, 
						context->queued_submissions[barrier.created_on_submission].signal_semaphore);
			--i;
		}

        vulkan_renderstage_bind(
            context, curr_submission->command_buffer,
            playback_context->current_shader);
        break;

    case RENDERCMD_DRAW:
        vkCmdDraw(curr_submission->command_buffer->handle,
                  payload->draw.vertex_count,
                  payload->draw.instance_count,
                  0, 0);
        break;

    case RENDERCMD_DRAW_INDEXED:
        vkCmdDrawIndexed(curr_submission->command_buffer->handle,
                         payload->draw_indexed.index_count,
                         payload->draw_indexed.instance_count,
                         0, 0, 0);
        break;

    case RENDERCMD_DISPATCH:
        vkCmdDispatch(curr_submission->command_buffer->handle,
                      payload->dispatch.group_size.x,
                      payload->dispatch.group_size.y,
                      payload->dispatch.group_size.z);
        break;

    case RENDERCMD_END:
        if (playback_context->current_target)
            vulkan_rendertarget_end(
                context, curr_submission->command_buffer,
                playback_context->current_target);
        break;
    }
}

b8 vulkan_device_end_frame(emgpu_device* device) {
	EM_ASSERT(device != NULL && "Invalid arguments passed to vulkan_device_end_frame");
    vulkan_context* context = (vulkan_context*)device->internal_context;

	VkSemaphore render_complete_semaphore;

	if (device->window_context != NULL) {
		vulkan_window_system* window_system = (vulkan_window_system*)device->window_context->internal_renderer_state;
		darray_push(context->queued_submissions[0].wait_semaphores, window_system->image_available_semaphores[context->current_frame]);
	}

	for (u32 i = 0; i < darray_length(context->queued_submissions); ++i) {
		vulkan_queue_submission* submission = &context->queued_submissions[i];
		VkFence signal_fence = VK_NULL_HANDLE;

		CHECK_VKRESULT(
			vulkan_command_buffer_end(submission->command_buffer), 
			"Failed to end Vulkan command buffer");

		// TODO: Propbably change this at some point...
		VkPipelineStageFlags wait_flags[] = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };

		if (i == darray_length(context->queued_submissions) - 1)
			signal_fence = context->in_flight_fences[context->current_frame];
		
		CHECK_VKRESULT(
			vulkan_command_buffer_submit(
				submission->command_buffer,
				darray_length(submission->wait_semaphores), 
				submission->wait_semaphores, 
				1, 
				&submission->signal_semaphore,
				wait_flags,
				signal_fence),
			"Failed to submit Vulkan command buffer");
		
		render_complete_semaphore = submission->signal_semaphore;
		darray_destroy(submission->wait_semaphores);
	}

	if (device->window_context != NULL) {
		vulkan_window_system* window_system = (vulkan_window_system*)device->window_context->internal_renderer_state;
	
		CHECK_VKRESULT(
			vulkan_window_system_present(
				device, 
				window_system, 
				&context->device.mode_queues[VULKAN_QUEUE_TYPE_PRESENT], 
				1, 
				&render_complete_semaphore),
			"Failed to present Vulkan swapchain image");
	}	

	// Advance to next frame
    context->current_frame = (context->current_frame + 1) % context->config.frames_in_flight;
    return TRUE;
}
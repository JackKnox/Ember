#include "defines.h"
#include "device.h"

#include "vulkan/vulkan_backend.h"

b8 emgpu_device_init(emgpu_device_config* config, emplat_window* window, emgpu_device* out_device) {
    EM_ASSERT(config != NULL && out_device != NULL && "Invalid arguments passed to emgpu_device_init");
    EM_ASSERT(config->application_name != NULL && "Malformed data in emgpu_device_config");

#if EM_ENABLE_VALIDATION
    if (window->internal_renderer_state != NULL) {
		EM_ERROR("emgpu_device_init(): Cannot attach more than 1 renderer backend to the same platform");
		return FALSE;
	}

	if (config->frames_in_flight < 1) {
		EM_ERROR("emgpu_device_init(): Must create renderer backend with at least 1 frame(s) in flight: %u", config->frames_in_flight);
		return FALSE;
	}
#endif

    out_device->window_context = window;

    if (config->api_type == EMBER_DEVICE_BACKEND_VULKAN) {
        out_device->initialize      = vulkan_device_initialize;
        out_device->shutdown        = vulkan_device_shutdown;

        out_device->resized         = vulkan_device_resized;
		out_device->window_textures = vulkan_device_window_textures;
        out_device->update_descriptors = vulkan_device_update_descriptors;

        out_device->begin_frame     = vulkan_device_begin_frame;
        out_device->execute_command = vulkan_device_execute_command;
        out_device->end_frame       = vulkan_device_end_frame;

        out_device->create_graphicstage    = vulkan_renderstage_create_graphic;
		out_device->create_computestage    = vulkan_renderstage_create_compute;
        out_device->destroy_renderstage    = vulkan_renderstage_destroy;

        out_device->create_renderbuffer    = vulkan_renderbuffer_create;
        out_device->upload_to_renderbuffer = vulkan_renderbuffer_upload_data;
        out_device->destroy_renderbuffer   = vulkan_renderbuffer_destroy;

        out_device->create_texture       = vulkan_texture_create;
		out_device->upload_to_texture    = vulkan_texture_upload_data;
        out_device->destroy_texture      = vulkan_texture_destroy;

   	 	out_device->create_rendertarget  = vulkan_rendertarget_create;
        out_device->resize_rendertarget  = vulkan_rendertarget_resize;
    	out_device->destroy_rendertarget = vulkan_rendertarget_destroy;
    }
    else {
        EM_ERROR("Unsupported renderer backend type (%i).", config->api_type);
        return FALSE;
    }

    if (!out_device->initialize(out_device, config))
        return FALSE;

    emgpu_texture* window_textures = NULL;
    if (!out_device->window_textures(out_device, &window_textures))
        return FALSE;

    emgpu_rendertarget_attachment attachments[] = {
        {
            .type = EMBER_ATTACHMENT_TYPE_WINDOW_SURFACE,
            .format = window_textures->image_format,
            .textures = window_textures,
			.load_op = EMBER_LOAD_OP_CLEAR,
			.store_op = EMBER_STORE_OP_STORE,
			.stencil_load_op = EMBER_LOAD_OP_DONT_CARE,
			.stencil_store_op = EMBER_STORE_OP_DONT_CARE,
        }
    };
    
    emgpu_rendertarget_config main_rendertarget_config = emgpu_rendertarget_default();
    main_rendertarget_config.origin = (uvec2) { 0, 0 };
    main_rendertarget_config.size = config->size;
    main_rendertarget_config.attachment_count = EM_ARRAYSIZE(attachments);
    main_rendertarget_config.attachments = attachments;
    return out_device->create_rendertarget(out_device, &main_rendertarget_config, &out_device->main_rendertarget);
}

void emgpu_device_shutdown(emgpu_device* device) {
    EM_ASSERT(device != NULL && "Invalid arguments passed to emgpu_device_shutdown");
    if (device->shutdown) {
        device->destroy_rendertarget(device, &device->main_rendertarget);

        device->shutdown(device);
    }
    
    bzero_memory(device, sizeof(emgpu_device));
}

b8 emgpu_device_submit_commandbuf(emgpu_device* device, emgpu_commandbuf* cmd) {
    EM_ASSERT(device != NULL && cmd != NULL && "Invalid arguments passed to emgpu_device_submit_commandbuf");

#if EM_ENABLE_VALIDATION
	if (!cmd->finished) {
	    EM_ERROR("emgpu_device_submit_commandbuf(): Tried to submit rendercmd before ending commands.");
		return FALSE;
	}
#endif

    emgpu_playback_context context = {};

    u8* cursor = 0;
    while (freelist_next_block(&cmd->buffer, &cursor)) {
        rendercmd_header* hdr = (rendercmd_header*)cursor;
        rendercmd_payload* payload = (rendercmd_payload*)(cursor + sizeof(rendercmd_header));
        
        EM_ASSERT(hdr->type <= RENDERCMD_END && "Data is corrupted on command buffer");
        if (hdr->supported_mode)
            context.current_mode = hdr->supported_mode;

        switch (hdr->type) {
            case RENDERCMD_BIND_RENDERTARGET:
                context.current_target = payload->bind_rendertarget.rendertarget;
                break;
            
            case RENDERCMD_BEGIN_RENDERSTAGE:
                context.current_shader = payload->begin_renderstage.renderstage;
                break;

            case RENDERCMD_END_RENDERSTAGE:
                context.current_shader = 0;
                break;
        }

        device->execute_command(device, &context, hdr, payload);
    }

    return TRUE;
}

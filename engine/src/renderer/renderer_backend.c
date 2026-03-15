#include "defines.h"
#include "renderer_backend.h"

#include "vulkan/vulkan_backend.h"
#include "vulkan/vulkan_renderbuffer.h"
#include "vulkan/vulkan_renderstage.h"
#include "vulkan/vulkan_rendertarget.h"
#include "vulkan/vulkan_texture.h"
#include "renderer_types.h"

u64 box_render_format_size(box_render_format format) {
    u32 bits_index = (format >> 16) & 0xF;
    u32 channels   = (format >> 12) & 0xF;

	return (u64)((bits_index + 1) * 8 * channels);
}

b8 box_render_format_normalized(box_render_format format) {
    u32 flags = (format >> 24) & 0xFF;
    return (flags & BOX_FORMAT_FLAG_NORMALIZED) != 0;
}

u32 box_render_format_channel_count(box_render_format format) {
    return (format >> 12) & 0xF;
}

box_rendersurface_config box_rendersurface_default_config() {
	box_rendersurface_config config = {};
	return config;
}

box_renderer_backend_config box_renderer_backend_default_config() {
    box_renderer_backend_config config = {};
    config.modes = RENDERER_MODE_GRAPHICS;
    config.enable_validation = FALSE;
	config.frames_in_flight = 3;

#if BOX_ENABLE_VALIDATION
    config.enable_validation = TRUE;
#endif
    return config;
}

b8 box_renderer_backend_create(const char* application_name, box_renderer_backend_config* config, box_renderer_backend* out_renderer_backend) {

    if (config->api_type == RENDERER_BACKEND_TYPE_VULKAN) {
        out_renderer_backend->initialize      = vulkan_renderer_backend_initialize;
		out_renderer_backend->connect_rendersurface = vulkan_renderer_backend_connect_rendersurface;
        out_renderer_backend->shutdown        = vulkan_renderer_backend_shutdown;
        out_renderer_backend->wait_until_idle = vulkan_renderer_backend_wait_until_idle;
        out_renderer_backend->resized         = vulkan_renderer_backend_on_resized;

        out_renderer_backend->begin_frame     = vulkan_renderer_backend_begin_frame;
        out_renderer_backend->execute_command = vulkan_renderer_execute_command;
        out_renderer_backend->end_frame       = vulkan_renderer_backend_end_frame;

        out_renderer_backend->create_graphicstage            = vulkan_renderstage_create_graphic;
		out_renderer_backend->create_computestage            = vulkan_renderstage_create_compute;
		out_renderer_backend->update_renderstage_descriptors = vulkan_renderstage_update_descriptors;
        out_renderer_backend->destroy_renderstage            = vulkan_renderstage_destroy;

        out_renderer_backend->create_renderbuffer            = vulkan_renderbuffer_create;
        out_renderer_backend->upload_to_renderbuffer         = vulkan_renderbuffer_upload_data;
        out_renderer_backend->destroy_renderbuffer           = vulkan_renderbuffer_destroy;

        out_renderer_backend->create_texture       = vulkan_texture_create;
		out_renderer_backend->upload_to_texture    = vulkan_texture_upload_data;
        out_renderer_backend->destroy_texture      = vulkan_texture_destroy;

   	 	out_renderer_backend->create_rendertarget  = vulkan_rendertarget_create;
    	out_renderer_backend->destroy_rendertarget = vulkan_rendertarget_destroy;
    }
    else {
        BX_ERROR("Unsupported renderer backend type (%i).", config->api_type);
        return FALSE;
    }
	
    return out_renderer_backend->initialize(out_renderer_backend, application_name, config);
}

void box_renderer_backend_destroy(box_renderer_backend* renderer_backend) {
	renderer_backend->wait_until_idle(renderer_backend, UINT64_MAX);

    if (renderer_backend->shutdown != NULL) 
        renderer_backend->shutdown(renderer_backend);
    
    bzero_memory(renderer_backend, sizeof(box_renderer_backend));
}

b8 box_renderer_backend_submit_rendercmd(box_renderer_backend* renderer_backend, box_rendercmd_context* playback_context, box_rendercmd* rendercmd) {
	BX_ASSERT(renderer_backend != NULL && playback_context != NULL && rendercmd != NULL && "Invalid arguments passed to box_renderer_backend_submit_rendercmd");

	u8* cursor = 0;
	while (freelist_next_block(&rendercmd->buffer, &cursor)) {
		rendercmd_header* hdr = (rendercmd_header*)cursor;
		rendercmd_payload* payload = (rendercmd_payload*)(cursor + sizeof(rendercmd_header));

		if (hdr->supported_mode)
			playback_context->current_mode = hdr->supported_mode;

		switch (hdr->type) {
		case RENDERCMD_BIND_RENDERTARGET:
#if BOX_ENABLE_VALIDATION
			if (playback_context->current_target != NULL) {
				BX_ERROR("Tried to bind rendertarget twice in box_rendercmd.");
				return FALSE;
			}
#endif

			playback_context->current_target = payload->bind_rendertarget.rendertarget;
			break;

		case RENDERCMD_BEGIN_RENDERSTAGE:
#if BOX_ENABLE_VALIDATION
			if (playback_context->current_shader != NULL) {
				BX_ERROR("Tried to begin renderstage twice in box_rendercmd.");
				return FALSE;
			}
#endif

			playback_context->current_shader = payload->begin_renderstage.renderstage;
			break;

		case RENDERCMD_END_RENDERSTAGE:
#if BOX_ENABLE_VALIDATION
			if (playback_context->current_shader == NULL) {
				BX_ERROR("Tried to end renderstage twice in box_rendercmd.");
				return FALSE;
			}
#endif

			playback_context->current_shader = NULL;
			break;

		case RENDERCMD_DRAW:
		case RENDERCMD_DRAW_INDEXED:
		case RENDERCMD_DISPATCH:
#if BOX_ENABLE_VALIDATION
			if (playback_context->current_shader == NULL) {
				BX_ERROR("Tried to dispatch draw call without a renderstage in box_rendercmd.");
				return FALSE;
			}
#endif
			break;
		}

		renderer_backend->execute_command(renderer_backend, playback_context, hdr, payload);
	}

	return TRUE;
}

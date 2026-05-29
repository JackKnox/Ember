#include "ember/core.h"
#include "ember/gpu/frame.h"

#include "ember/gpu/frame_internal.h"

#include "ember/core/darray.h"

rendercmd_payload* add_command(emgpu_frame* frame, rendercmd_payload_type type, u64 payload_size) {
    rendercmd_payload* payload;
    payload = (rendercmd_payload*)datastream_push(&frame->commands, sizeof(payload->hdr) + payload_size, EM_ALIGNOF(rendercmd_payload));
    payload->hdr.type = type;

    return payload;
}

em_result emgpu_frame_init(emgpu_frame* frame, em_allocator* allocator) {
    if (frame->initialized) {
        EM_WARN("Gpu", "Already initied frame object but called emgpu_frame_init twice");
        return EMBER_RESULT_OK;
    }

    frame->commands = datastream_create(allocator, MEMORY_TAG_FRAME);
    frame->initialized = TRUE;
    return EMBER_RESULT_OK;
}

em_result emgpu_frame_validate(const emgpu_frame* frame) {
    return EMBER_RESULT_OK;
}

void emgpu_frame_dummy(emgpu_frame* frame) {
    add_command(frame, RENDERCMD_DUMMY, 0);
}

emgpu_frame_texture emgpu_frame_next_surface_texture(emgpu_frame* frame, emgpu_surface* surface) {
    rendercmd_payload* payload;
    payload = add_command(frame, RENDERCMD_NEXT_SURFACE_TEXTURE, sizeof(payload->next_surface_texture));
    payload->next_surface_texture.surface = surface;
    payload->next_surface_texture.dst_texture = frame->current_resource_idx++;

    return payload->next_surface_texture.dst_texture;
}

emgpu_frame_texture emgpu_frame_import_texture(emgpu_frame* frame, emgpu_texture* texture) {
    rendercmd_payload* payload;
    payload = add_command(frame, RENDERCMD_IMPORT_TEXTURE, sizeof(payload->import_texture));
    payload->import_texture.texture = texture;
    payload->import_texture.dst_texture = frame->current_resource_idx++;

    return payload->import_texture.dst_texture;
}

void emgpu_frame_set_renderarea(emgpu_frame* frame, uvec2 origin, uvec2 size) {
    rendercmd_payload* payload;
    payload = add_command(frame, RENDERCMD_SET_RENDERAREA, sizeof(payload->set_renderarea));
    payload->set_renderarea.origin = origin;
    payload->set_renderarea.size = size;
}

void emgpu_frame_begin_renderpass(emgpu_frame* frame, emgpu_renderpass* renderpass, emgpu_frame_texture* texture_attachments, u32 attachment_count) {
    rendercmd_payload* payload;
    u64 attachments_offset = EM_OFFSETOF(rendercmd_payload, begin_renderpass.attachments);
    u64 payload_size       = attachments_offset 
                            + sizeof(emgpu_frame_texture) * attachment_count
                            - sizeof(payload->hdr);

    payload = add_command(frame, RENDERCMD_BEGIN_RENDERPASS, payload_size);
    payload->begin_renderpass.renderpass       = renderpass;
    payload->begin_renderpass.attachment_count = attachment_count;
    em_memcpy(payload->begin_renderpass.attachments,
              texture_attachments,
              sizeof(emgpu_frame_texture) * attachment_count);
}

void emgpu_frame_end_renderpass(emgpu_frame* frame) {
    add_command(frame, RENDERCMD_END_RENDERPASS, 0);
}

void emgpu_frame_bind_pipeline(emgpu_frame* frame, emgpu_pipeline* pipeline, u32 vertex_buffer_count, emgpu_buffer* vertex_buffers, emgpu_buffer* index_buffer) {
    rendercmd_payload* payload;
    payload = add_command(frame, RENDERCMD_BIND_PIPELINE, sizeof(payload->bind_pipeline));
    payload->bind_pipeline.pipeline            = pipeline;
    payload->bind_pipeline.vertex_buffer_count = vertex_buffer_count;
    payload->bind_pipeline.vertex_buffers      = vertex_buffers;
    payload->bind_pipeline.index_buffer        = index_buffer;
}

void emgpu_frame_draw(emgpu_frame* frame, u32 vertex_count, u32 instance_count) {
    rendercmd_payload* payload;
    payload = add_command(frame, RENDERCMD_DRAW, sizeof(payload->draw));
    payload->draw.vertex_count = vertex_count;
    payload->draw.instance_count = instance_count;
}

void emgpu_frame_draw_indexed(emgpu_frame* frame, u32 index_count, u32 instance_count) {
    rendercmd_payload* payload;
    payload = add_command(frame, RENDERCMD_DRAW_INDEXED, sizeof(payload->draw_indexed));
    payload->draw_indexed.index_count = index_count;
    payload->draw_indexed.instance_count = instance_count;
}

void emgpu_frame_dispatch(emgpu_frame* frame, uvec3 group_size) {
    rendercmd_payload* payload;
    payload = add_command(frame, RENDERCMD_DISPATCH, sizeof(payload->dispatch));
    payload->dispatch.group_size = group_size;
}

emgpu_frame_resource emgpu_frame_export_resource(emgpu_frame* frame, u32 descriptor_index, emgpu_access_flags resource_access) {
    rendercmd_payload* payload;
    payload = add_command(frame, RENDERCMD_EXPORT_RESOURCE, sizeof(payload->export_resource));
    payload->export_resource.descriptor_index = descriptor_index;
    payload->export_resource.resource_access  = resource_access;
    payload->export_resource.dst_resource     = frame->current_resource_idx++; 
    return payload->export_resource.dst_resource;
}

void emgpu_frame_use_resource(emgpu_frame* frame, emgpu_frame_resource resource, u32 descriptor_index, emgpu_access_flags resource_access) {
    rendercmd_payload* payload;
    payload = add_command(frame, RENDERCMD_USE_RESOURCE, sizeof(payload->use_resource));
    payload->use_resource.src_resource     = resource;
    payload->use_resource.descriptor_index = descriptor_index;
    payload->use_resource.resource_access  = resource_access;
}

void emgpu_frame_flush(emgpu_frame* frame) {
    rendercmd_payload* payload;
    payload = add_command(frame, RENDERCMD_FLUSH, 0);
}
#include "ember/core.h"
#include "ember/gpu/frame.h"

#include "ember/gpu/frame_internal.h"

#include "ember/core/darray.h"

rendercmd_payload* add_command(emgpu_frame* frame, emgpu_ops_type mode, rendercmd_payload_type type, u64 payload_size) {
    emgpu_frame_submission* submission = NULL;

    if (darray_length(frame->submissions) == 0 || darray_last(frame->submissions)->ops_type != mode) {
        submission = darray_push_empty(frame->submissions);
        submission->ops_type = mode;
        submission->start_index = frame->curr_command_idx++;
    }
    else {
        submission = darray_last(frame->submissions);
    }

    rendercmd_payload* payload;
    payload  = (rendercmd_payload*)datastream_push(&frame->commands, sizeof(payload->hdr) + payload_size);

    payload->hdr.type = type;
    payload->hdr.command_mode = mode;

    ++submission->submission_length;
    return payload_size > 0 ? payload : NULL;
}

em_result emgpu_frame_init(emgpu_frame* frame, emgpu_device* device) {
    if (frame->initied) {
        EM_WARN("Gpu", "Already initied frame object but called emgpu_frame_init twice");
        return EMBER_RESULT_OK;
    }

    frame->initied = TRUE;
    frame->local_allocator = &device->frame_allocator;
    frame->commands = datastream_create(&device->frame_allocator, MEMORY_TAG_RENDERER);
    frame->submissions = darray_create(emgpu_frame_submission, frame->local_allocator, MEMORY_TAG_RENDERER);
    frame->managed_surfaces = darray_create(emgpu_frame_surface, frame->local_allocator, MEMORY_TAG_RENDERER);
    return EMBER_RESULT_OK;
}

em_result emgpu_frame_validate(const emgpu_frame* frame) {
    return EMBER_RESULT_OK;
}

void emgpu_frame_dummy(emgpu_frame* frame) {
    add_command(frame, EMBER_OPER_TYPE_UNIVERSAL, RENDERCMD_DUMMY, 0);
}

emgpu_frame_texture emgpu_frame_next_surface_texture(emgpu_frame* frame, emgpu_surface* surface) {
    rendercmd_payload* payload; // * DO NOT CHANGE MODE.
    payload = add_command(frame, EMBER_OPER_TYPE_GRAPHICS, RENDERCMD_BIND_NEXT_SURFACE_TEXTURE, sizeof(payload->next_surface_texture));
    payload->next_surface_texture.surface_index = darray_length(frame->managed_surfaces);
    payload->next_surface_texture.dst_texture = frame->current_resource_idx++;

    emgpu_frame_surface* managed_surface = darray_push_empty(frame->managed_surfaces);
    managed_surface->handle = surface;
    managed_surface->owner_submission_index = (darray_length(frame->submissions) - 1);

    return payload->next_surface_texture.dst_texture;
}

emgpu_frame_texture emgpu_frame_import_texture(emgpu_frame* frame, emgpu_texture* texture) {
    rendercmd_payload* payload;
    payload = add_command(frame, EMBER_OPER_TYPE_UNIVERSAL, RENDERCMD_BIND_IMPORT_TEXTURE, sizeof(payload->import_texture));
    payload->import_texture.texture = texture;
    payload->import_texture.dst_texture = frame->current_resource_idx++;
    return payload->import_texture.dst_texture;
}

void emgpu_frame_set_renderarea(emgpu_frame* frame, uvec2 origin, uvec2 size) {
    rendercmd_payload* payload;
    payload = add_command(frame, EMBER_OPER_TYPE_GRAPHICS, RENDERCMD_SET_RENDERAREA, sizeof(payload->set_renderarea));
    payload->set_renderarea.origin = origin;
    payload->set_renderarea.size = size;
}

void emgpu_frame_begin_renderpass(emgpu_frame* frame, emgpu_renderpass* renderpass, emgpu_frame_texture* texture_attachments, u32 attachment_count) {
    rendercmd_payload* payload;
    payload = add_command(frame, EMBER_OPER_TYPE_GRAPHICS, RENDERCMD_BEGIN_RENDERPASS, sizeof(payload->bind_renderpass) + sizeof(emgpu_frame_texture) * attachment_count);
    payload->bind_renderpass.renderpass = renderpass;
    payload->bind_renderpass.attachments = (emgpu_frame_texture*)((u8*)payload + sizeof(payload->bind_renderpass));
    payload->bind_renderpass.attachment_count = attachment_count;

    em_memcpy(payload->bind_renderpass.attachments, texture_attachments, sizeof(emgpu_frame_texture) * attachment_count);
}

void emgpu_frame_end_renderpass(emgpu_frame* frame) {
    add_command(frame, EMBER_OPER_TYPE_GRAPHICS, RENDERCMD_END_RENDERPASS, 0);
}

void emgpu_frame_memory_barrier(emgpu_frame* frame, emgpu_pipeline* src_pipeline, emgpu_pipeline* dst_pipeline, emgpu_access_flags src_access, emgpu_access_flags dst_access) {
    rendercmd_payload* payload;
    payload = add_command(frame, EMBER_OPER_TYPE_UNIVERSAL, RENDERCMD_MEMORY_BARRIER, sizeof(payload->memory_barrier));
    payload->memory_barrier.src_pipeline = src_pipeline;
    payload->memory_barrier.dst_pipeline = dst_pipeline;
    payload->memory_barrier.src_access = src_access;
    payload->memory_barrier.dst_access = dst_access;
}

void emgpu_frame_bind_pipeline(emgpu_frame* frame, emgpu_pipeline* pipeline) {
    rendercmd_payload* payload;
    payload = add_command(frame, pipeline->type, RENDERCMD_BIND_PIPELINE, sizeof(payload->bind_pipeline));
    payload->bind_pipeline.pipeline = pipeline;
}

void emgpu_frame_draw(emgpu_frame* frame, u32 vertex_count, u32 instance_count) {
    rendercmd_payload* payload;
    payload = add_command(frame, EMBER_OPER_TYPE_GRAPHICS, RENDERCMD_DRAW, sizeof(payload->draw));
    payload->draw.vertex_count = vertex_count;
    payload->draw.instance_count = instance_count;
}

void emgpu_frame_draw_indexed(emgpu_frame* frame, u32 index_count, u32 instance_count) {
    rendercmd_payload* payload;
    payload = add_command(frame, EMBER_OPER_TYPE_GRAPHICS, RENDERCMD_DRAW_INDEXED, sizeof(payload->draw_indexed));
    payload->draw_indexed.index_count = index_count;
    payload->draw_indexed.instance_count = instance_count;
}

void emgpu_frame_dispatch(emgpu_frame* frame, uvec3 group_size) {
    rendercmd_payload* payload;
    payload = add_command(frame, EMBER_OPER_TYPE_COMPUTE, RENDERCMD_DISPATCH, sizeof(payload->dispatch));
    payload->dispatch.group_size = group_size;
}
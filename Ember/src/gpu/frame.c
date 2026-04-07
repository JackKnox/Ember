#include "ember/core.h"
#include "ember/gpu/frame.h"

#include "frame_internal.h"

rendercmd_payload* add_command(emgpu_frame* cmd, rendercmd_payload_type type, u64 payload_size) {
    EM_ASSERT(cmd != NULL && "Invalid arguments passed to add_command");

    void* user_memory = freelist_push(&cmd->buffer, payload_size, NULL);
    if (!user_memory) {
        EM_ERROR("gpu", "Frame object allocation failed.");
        return NULL;
    }

    rendercmd_payload* payload = (rendercmd_payload*)user_memory;
    payload->type = type;
    return payload_size > 0 ? payload : NULL;
}

em_result emgpu_frame_init(emgpu_frame* frame) {
    if (!frame->buffer.memory || !freelist_capacity(&frame->buffer))
        freelist_create(0, MEMORY_TAG_RENDERER, &frame->buffer);
    
    return EMBER_RESULT_OK;
}

void emgpu_frame_dummy(emgpu_frame* frame) {
    add_command(frame, RENDERCMD_DUMMY, 0);
}

void emgpu_frame_bind_rendertarget(emgpu_frame* frame, emgpu_rendertarget* rendertarget) {
    rendercmd_payload* payload;
    payload = add_command(frame, RENDERCMD_BIND_RENDERTARGET, sizeof(payload->bind_rendertarget));
    payload->bind_rendertarget.rendertarget = rendertarget;
}

void emgpu_frame_memory_barrier(emgpu_frame* frame, emgpu_pipeline* src_pipeline, emgpu_pipeline* dst_pipeline, emgpu_access_flags src_access, emgpu_access_flags dst_access) {
    rendercmd_payload* payload;
    payload = add_command(frame, RENDERCMD_MEMORY_BARRIER, sizeof(payload->memory_barrier));
    payload->memory_barrier.src_pipeline = src_pipeline;
    payload->memory_barrier.dst_pipeline = dst_pipeline;
    payload->memory_barrier.src_access = src_access;
    payload->memory_barrier.dst_access = dst_access;
}

void emgpu_frame_begin_pipeline(emgpu_frame* frame, emgpu_pipeline* pipeline) {
    rendercmd_payload* payload;
    payload = add_command(frame, RENDERCMD_BEGIN_PIPELINE, sizeof(payload->begin_pipeline));
    payload->begin_pipeline.pipeline = pipeline;
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

void emgpu_frame_dispatch(emgpu_frame* frame, u32 group_size_x, u32 group_size_y, u32 group_size_z) {
    rendercmd_payload* payload;
    payload = add_command(frame, RENDERCMD_DISPATCH, sizeof(payload->draw));
    payload->dispatch.group_size.x = group_size_x;
    payload->dispatch.group_size.y = group_size_y;
    payload->dispatch.group_size.z = group_size_z;
}

void emgpu_frame_end_pipeline(emgpu_frame* frame) {
    add_command(frame, RENDERCMD_END_PIPELINE, 0);
}
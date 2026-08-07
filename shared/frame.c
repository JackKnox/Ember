#include "ember/core.h"
#include "ember/gpu/command_buffer.h"

#include "commands_internal.h"

#include <string.h>

// A C implementation of the emgpu_frame format, not required by every Ember usage to use this
// way of managing a datastream. e.g bindings to other languages could write an implementation in that language.

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L 
// C11 and later
#   define EM_ALIGNOF(type) _Alignof(type)
#elif defined(_MSC_VER) 
// MSVC (works in both C and C++)
#   define EM_ALIGNOF(type) __alignof(type)
#elif defined(__GNUC__) || defined(__clang__) 
// GCC / Clang (works in both C and C++)
#   define EM_ALIGNOF(type) __alignof__(type)
#else 
// Pure fallback (standard C89, no extensions)
#   include <stddef.h>
#   define EM_ALIGNOF(type) offsetof(struct { char c; type member; }, member)
#endif

u64 align_up_command(u64 value, u64 alignment) {
    // alignment must be a power of two
    return (value + alignment - 1) & ~(alignment - 1);
}

rendercmd_payload* add_command(emgpu_frame* frame, rendercmd_payload_type type, u64 payload_size) {
    rendercmd_payload* payload;

    u64 offset = align_up_command(frame->buffer_size, 16);
    u64 total_size = sizeof(payload->hdr) + payload_size;

    if (frame->buffer_capacity < offset + total_size) {
        u64 new_capacity = (frame->buffer_capacity == 0 ? 4 : frame->buffer_capacity);
        while (new_capacity < offset + total_size) new_capacity *= 2;
        
        frame->commands_buf = mem_reallocate(frame->allocator, frame->commands_buf, frame->buffer_capacity, new_capacity);
        frame->buffer_capacity = new_capacity;
    }

    frame->buffer_size = offset + total_size;

    payload = (rendercmd_payload*)((u8*)frame->commands_buf + offset);
    payload->hdr.type = type;
    payload->hdr.size = total_size;
    return payload;
}

em_result emgpu_frame_init(emgpu_device* device, emgpu_frame* out_frame) {
    if (out_frame->initialized) {
        return EMBER_RESULT_OK;
    }

    out_frame->initialized = EMTRUE;
    return EMBER_RESULT_OK;
}

emgpu_frame_texture emgpu_frame_accquire_surface(emgpu_frame* frame, emgpu_surface* surface) {
    rendercmd_payload* payload;
    payload = add_command(frame, RENDERCMD_ACCQUIRE_SURFACE, sizeof(payload->accquire_surface));
    payload->accquire_surface.surface = surface;
    payload->accquire_surface.dst_texture = frame->current_resource_idx++;

    return payload->accquire_surface.dst_texture;
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

void emgpu_frame_begin_renderpass(emgpu_frame* frame, emgpu_renderpass* renderpass, u32 clear_colour, emgpu_frame_texture* texture_attachments, u32 attachment_count) {
    rendercmd_payload* payload;
    u64 attachments_offset = EM_OFFSETOF(rendercmd_payload, begin_renderpass.attachments);
    u64 payload_size       = attachments_offset 
                            + sizeof(emgpu_frame_texture) * attachment_count
                            - sizeof(payload->hdr);

    payload = add_command(frame, RENDERCMD_BEGIN_RENDERPASS, payload_size);
    payload->begin_renderpass.renderpass       = renderpass;
    payload->begin_renderpass.attachment_count = attachment_count;
    payload->begin_renderpass.clear_colour     = clear_colour;
    memcpy(payload->begin_renderpass.attachments,
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

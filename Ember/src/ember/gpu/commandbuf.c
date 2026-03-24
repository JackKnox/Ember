#include "defines.h"
#include "commandbuf.h"

#include "device.h"

#if EM_ENABLE_VALIDATION
#   define CHECK_FINISHED() if (!cmd) return; if (cmd->finished) { EM_ERROR("Tried to record command into emgpu_commandbuf after ending."); return; }
#else
#   define CHECK_FINISHED()
#endif

rendercmd_payload* add_command(emgpu_commandbuf* cmd, emgpu_device_mode mode, rendercmd_payload_type type, u64 payload_size) {
    EM_ASSERT(cmd != NULL && "Invalid arguments passed to add_command");

    u64 user_block_size = sizeof(rendercmd_header) + payload_size;

    if (!cmd->buffer.memory || !freelist_capacity(&cmd->buffer))
        freelist_create(user_block_size, MEMORY_TAG_RENDERER, &cmd->buffer);

    void* user_memory = freelist_push(&cmd->buffer, user_block_size, NULL);
    if (!user_memory) {
        EM_ERROR("Render command buffer out of memory.");
        return NULL;
    }

    rendercmd_header* header = (rendercmd_header*)user_memory;
    header->type = type;
    header->supported_mode = mode;

    // user_memory points at rendercmd_header
    return payload_size > 0 ? (rendercmd_payload*)((u8*)user_memory + sizeof(rendercmd_header)) : NULL;
}

void emgpu_commandbuf_begin(emgpu_commandbuf* cmd) {
    EM_ASSERT(cmd != NULL && "Invalid arguments passed to emgpu_commandbuf_begin");
    if (cmd->buffer.memory && freelist_capacity(&cmd->buffer))
        freelist_reset(&cmd->buffer, FALSE, FALSE);
    cmd->finished = FALSE;
}

void emgpu_commandbuf_destroy(emgpu_commandbuf* cmd) {
    EM_ASSERT(cmd != NULL && "Invalid arguments passed to emgpu_commandbuf_end");
    freelist_destroy(&cmd->buffer);
    bzero_memory(cmd, sizeof(emgpu_commandbuf));
}

void emgpu_commandbuf_bind_rendertarget(emgpu_commandbuf* cmd, emgpu_rendertarget* rendertarget) {
    CHECK_FINISHED();

    rendercmd_payload* payload;
    payload = add_command(cmd, EMBER_DEVICE_MODE_GRAPHICS, RENDERCMD_BIND_RENDERTARGET, sizeof(payload->bind_rendertarget));
    payload->bind_rendertarget.rendertarget = rendertarget;
}

void emgpu_commandbuf_memory_barrier(emgpu_commandbuf* cmd, emgpu_renderstage* src_renderstage, emgpu_renderstage* dst_renderstage, emgpu_access_flags src_access, emgpu_access_flags dst_access) {
    CHECK_FINISHED();

    rendercmd_payload* payload;
    payload = add_command(cmd, 0, RENDERCMD_MEMORY_BARRIER, sizeof(payload->memory_barrier));
    payload->memory_barrier.src_renderstage = src_renderstage;
    payload->memory_barrier.dst_renderstage = dst_renderstage;
    payload->memory_barrier.src_access = src_access;
    payload->memory_barrier.dst_access = dst_access;
}

void emgpu_commandbuf_begin_renderstage(emgpu_commandbuf* cmd, emgpu_renderstage* renderstage) {
    CHECK_FINISHED();

    rendercmd_payload* payload;
    payload = add_command(cmd, renderstage->pipeline_type, RENDERCMD_BEGIN_RENDERSTAGE, sizeof(payload->begin_renderstage));
    payload->begin_renderstage.renderstage = renderstage;
}

void emgpu_commandbuf_draw(emgpu_commandbuf* cmd, u32 vertex_count, u32 instance_count) {
    CHECK_FINISHED();

    rendercmd_payload* payload;
    payload = add_command(cmd, EMBER_DEVICE_MODE_GRAPHICS, RENDERCMD_DRAW, sizeof(payload->draw));
    payload->draw.vertex_count = vertex_count;
    payload->draw.instance_count = instance_count;
}

void emgpu_commandbuf_draw_indexed(emgpu_commandbuf* cmd, u32 index_count, u32 instance_count) {
    CHECK_FINISHED();

    rendercmd_payload* payload;
    payload = add_command(cmd, EMBER_DEVICE_MODE_GRAPHICS, RENDERCMD_DRAW_INDEXED, sizeof(payload->draw_indexed));
    payload->draw_indexed.index_count = index_count;
    payload->draw_indexed.instance_count = instance_count;
}

void emgpu_commandbuf_dispatch(emgpu_commandbuf* cmd, u32 group_size_x, u32 group_size_y, u32 group_size_z) {
    CHECK_FINISHED();

    rendercmd_payload* payload;
    payload = add_command(cmd, EMBER_DEVICE_MODE_COMPUTE, RENDERCMD_DISPATCH, sizeof(payload->draw));
    payload->dispatch.group_size.x = group_size_x;
    payload->dispatch.group_size.y = group_size_y;
    payload->dispatch.group_size.z = group_size_z;
}

void emgpu_commandbuf_end_renderstage(emgpu_commandbuf* cmd) {
    CHECK_FINISHED();

    add_command(cmd, 0, RENDERCMD_END_RENDERSTAGE, 0);
}

void emgpu_commandbuf_end(emgpu_commandbuf* cmd) {
    CHECK_FINISHED();

    add_command(cmd, 0, RENDERCMD_END, 0);
    cmd->finished = TRUE;
}
#include "ember/core.h"
#include "commands_internal.h"

#include <string.h>

#include "ember/gpu/compute.h"
#include "ember/gpu/raster.h"

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L 
// C11 and later
#   define EM_ALIGNOF(type) _Alignof(type)
#elif defined(_MSC_VER) 
// MSVC (works in both C and C++)
#   define EM_ALIGNOF(type) __alignof(type)
#elif defined(__GNUC__) || defined(__clang__) 
// GCC / Clang (works in both C and C++)
#   define EM_ALIGNOF(type) __alignof__(type)
#endif


u64 align_up_command(u64 value, u64 alignment) {
    // alignment must be a power of two
    return (value + alignment - 1) & ~(alignment - 1);
}

void* cmdalloc(emgpu_command_buffer* command_buf, cmd_payload_type type, u64 payload_size) {
    u64 offset = align_up_command(command_buf->buffer_size, 16);
    u64 total_size = sizeof(cmd_header) + payload_size;

    if (command_buf->buffer_capacity < offset + total_size) {
        u64 new_capacity = (command_buf->buffer_capacity == 0 ? 4 : command_buf->buffer_capacity);
        while (new_capacity < offset + total_size) new_capacity *= 2;
        
        command_buf->commands_buf = mem_reallocate(command_buf->allocator, command_buf->commands_buf, command_buf->buffer_capacity, new_capacity);
        command_buf->buffer_capacity = new_capacity;
    }

    command_buf->buffer_size = offset + total_size;

    cmd_header* hdr = (cmd_header*)((u8*)command_buf->commands_buf + offset);
    hdr->type = type;
    hdr->size = total_size;
    return (void*)((u8*)hdr + sizeof(cmd_header));
}

em_result emgpu_command_buffer_create(emgpu_device* device, emgpu_command_buffer* out_command_buffer) {
    out_command_buffer->initialized = EMTRUE;
    out_command_buffer->allocator = &device->frame_allocator;
    return EMBER_RESULT_OK;
}

void emgpu_cmd_begin_computepass(emgpu_command_buffer* command_buf, const emgpu_computepass_config* config) {
    cmd_payload* payload;
    payload = (cmd_payload*)cmdalloc(command_buf, COMMAND_BEGIN_COMPUTEPASS, sizeof(payload->begin_computepass));
    payload->begin_computepass.pipeline = config->pipeline;
    
    // payload->begin_computepass.exports
    emgpu_resource_export* exports;
    exports = (emgpu_resource_export*)cmdalloc(command_buf, COMMAND_IMPORT_RESOURCES, sizeof(*exports) * config->export_resource_count);
    memcpy(exports, 
           config->export_resources, 
           sizeof(*exports) * config->export_resource_count);

    // payload->begin_computepass.imports
    emgpu_resource_import* imports;
    imports = (emgpu_resource_import*)cmdalloc(command_buf, COMMAND_EXPORT_RESOURCES, sizeof(*imports) * config->import_resource_count);
    memcpy(imports,
           config->import_resources,
           sizeof(*imports) * config->import_resource_count);
}

void emgpu_cmd_dispatch(emgpu_command_buffer* command_buf, uvec3 group_size) {
    cmd_payload* payload;
    payload = (cmd_payload*)cmdalloc(command_buf, COMMAND_DISPATCH, sizeof(payload->dispatch));
    payload->dispatch.group_size = group_size;
}

void emgpu_cmd_end_computepass(emgpu_command_buffer* command_buf) {
    cmdalloc(command_buf, COMMAND_EMPTY_RESOURCE, 0);
}

void emgpu_cmd_begin_renderpass(emgpu_command_buffer* command_buf, const emgpu_renderpass_config* config) {
    cmd_payload* payload;
    payload = (cmd_payload*)cmdalloc(command_buf, COMMAND_BEGIN_RENDERPASS, sizeof(payload->begin_renderpass));
    payload->begin_renderpass.render_origin = config->render_origin;
    payload->begin_renderpass.render_size   = config->render_size;

    // payload->begin_renderpass.colours
    emgpu_colour_attachment* colours;
    colours = (emgpu_colour_attachment*)cmdalloc(command_buf, COMMAND_COLOUR_ATTACHMENTS, sizeof(*colours) * config->colour_attachment_count);
    memcpy(colours,
           config->colour_attachments,
           sizeof(*colours) * config->colour_attachment_count);
}

void emgpu_cmd_end_renderpass(emgpu_command_buffer* command_buf) {
    cmdalloc(command_buf, COMMAND_END_RENDERPASS, 0);
}

void emgpu_cmd_set_viewport(emgpu_command_buffer* command_buf, uvec2 origin, uvec2 size, f32 min_depth, f32 max_depth) {
    cmd_payload* payload;
    payload = (cmd_payload*)cmdalloc(command_buf, COMMAND_SET_VIEWPORT, sizeof(payload->set_viewport));
    payload->set_viewport.origin = origin;
    payload->set_viewport.size = size;
    payload->set_viewport.min_depth = min_depth;
    payload->set_viewport.max_depth = max_depth;
}

void emgpu_cmd_set_scissor(emgpu_command_buffer* command_buf, uvec2 origin, uvec2 size) {
    cmd_payload* payload;
    payload = (cmd_payload*)cmdalloc(command_buf, COMMAND_SET_SCISSOR, sizeof(payload->set_scissor));
    payload->set_scissor.origin = origin;
    payload->set_scissor.size = size;
}

void emgpu_cmd_bind_raster_pipeline(emgpu_command_buffer* command_buf, emgpu_raster_bind_info* bind_info) {
    cmd_payload* payload;
    payload = (cmd_payload*)cmdalloc(command_buf, COMMAND_BIND_RASTER_PIPELINE, sizeof(payload->bind_raster_pipeline));
    payload->bind_raster_pipeline.pipeline = bind_info->pipeline;
    
    // payload->bind_raster_pipeline.exports
    emgpu_resource_export* exports;
    exports = (emgpu_resource_export*)cmdalloc(command_buf, COMMAND_IMPORT_RESOURCES, sizeof(*exports) * bind_info->export_resource_count);
    memcpy(exports, 
           bind_info->export_resources, 
           sizeof(*exports) * bind_info->export_resource_count);

    // payload->bind_raster_pipeline.imports
    emgpu_resource_import* imports;
    imports = (emgpu_resource_import*)cmdalloc(command_buf, COMMAND_EXPORT_RESOURCES, sizeof(*imports) * bind_info->import_resource_count);
    memcpy(imports,
           bind_info->import_resources,
           sizeof(*imports) * bind_info->import_resource_count);
}

void emgpu_cmd_bind_vertex_buffers(emgpu_command_buffer* command_buf, u32 vertex_buffer_count, emgpu_buffer* vertex_buffers) {
    emgpu_buffer* buffers;
    buffers = (emgpu_buffer*)cmdalloc(command_buf, COMMAND_VERTEX_BUFFERS, sizeof(*buffers) * vertex_buffer_count);

    // payload->bind_vertex_buffers.vertex_buffers
    memcpy(buffers,
           vertex_buffers,
           sizeof(*buffers) * vertex_buffer_count);
}

void emgpu_cmd_bind_index_buffer(emgpu_command_buffer* command_buf, emgpu_buffer* index_buffer) {
    cmd_payload* payload;
    payload = (cmd_payload*)cmdalloc(command_buf, COMMAND_BIND_INDEX_BUFFER, sizeof(payload->bind_index_buffer));
    payload->bind_index_buffer.index_buffer = index_buffer;
}

void emgpu_cmd_draw(emgpu_command_buffer* command_buf, u32 vertex_count, u32 instance_count) {
    cmd_payload* payload;
    payload = (cmd_payload*)cmdalloc(command_buf, COMMAND_DRAW, sizeof(payload->draw));
    payload->draw.vertex_count = vertex_count;
    payload->draw.instance_count = instance_count;
}

emgpu_local_resource emgpu_cmd_empty_resource(emgpu_command_buffer* command_buf) {
    cmd_payload* payload;
    payload = (cmd_payload*)cmdalloc(command_buf, COMMAND_EMPTY_RESOURCE, sizeof(payload->empty_resource));
    payload->empty_resource.dst_resource = command_buf->current_resource_idx++;
    return payload->empty_resource.dst_resource;
}

emgpu_local_framebuffer emgpu_cmd_import_texture(emgpu_command_buffer* command_buf, emgpu_texture* texture) {
    cmd_payload* payload;
    payload = (cmd_payload*)cmdalloc(command_buf, COMMAND_IMPORT_TEXTURE, sizeof(payload->import_texture));
    payload->import_texture.texture = texture;
    payload->import_texture.dst_framebuffer = command_buf->current_resource_idx++;
    return payload->import_texture.dst_framebuffer;
}

emgpu_local_framebuffer emgpu_cmd_acquire_surface(emgpu_command_buffer* command_buf, emgpu_surface* surface) {
    cmd_payload* payload;
    payload = (cmd_payload*)cmdalloc(command_buf, COMMAND_ACQUIRE_SURFACE, sizeof(payload->acquire_surface));
    payload->acquire_surface.surface = surface;
    payload->acquire_surface.dst_framebuffer = command_buf->current_resource_idx++;
    return payload->acquire_surface.dst_framebuffer;
}

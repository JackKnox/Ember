#pragma once

#include "ember/core.h"

#include "ember/gpu/command_buffer.h"

#include "ember/gpu/resources.h"
#include "ember/gpu/surface.h"


typedef enum cmd_payload_type {
    COMMAND_EMPTY, // Prevents against corrupted memory.
    
    // Compute mode.
    // -------------------------
    COMMAND_BEGIN_COMPUTEPASS,
    COMMAND_DISPATCH,
    COMMAND_END_COMPUTEPASS,

    // Graphics work.
    // -------------------------
    COMMAND_BEGIN_RENDERPASS,
    COMMAND_END_RENDERPASS,
    COMMAND_SET_VIEWPORT,
    COMMAND_SET_SCISSOR,

    // Raster mode.
    // -------------------------
    COMMAND_BIND_RASTER_PIPELINE,
    COMMAND_BIND_INDEX_BUFFER,
    COMMAND_DRAW,
    
    COMMAND_EMPTY_RESOURCE,

    COMMAND_IMPORT_TEXTURE,

    COMMAND_ACQUIRE_SURFACE,

    // Array payloads
    // -------------------------
    COMMAND_EXPORT_RESOURCES,
    COMMAND_IMPORT_RESOURCES,
    COMMAND_COLOUR_ATTACHMENTS,
    COMMAND_VERTEX_BUFFERS,
} cmd_payload_type;

typedef struct cmd_header {
    cmd_payload_type type;
    u64 size;
} cmd_header;

typedef union cmd_payload {
    struct {
        const emgpu_pipeline* pipeline;
        // export_resources, import_resources.
    } begin_computepass;

    struct {
        uvec3 group_size;
    } dispatch;

    struct {
        uvec2 render_origin, render_size;
        // colour_attachments
    } begin_renderpass;

    struct {
        uvec2 origin, size;
        f32 min_depth, max_depth;
    } set_viewport;

    struct {
        uvec2 origin, size;
    } set_scissor;

    struct {
        const emgpu_pipeline* pipeline;
        // export_resources, import_resources.
    } bind_raster_pipeline;

    // vertex_buffers.

    struct {
        emgpu_buffer* index_buffer;
    } bind_index_buffer;

    struct {
        u32 vertex_count, instance_count;
    } draw;
    
    struct {
        emgpu_local_resource dst_resource;
    } empty_resource;

    struct {
        emgpu_texture* texture;
        emgpu_local_framebuffer dst_framebuffer;
    } import_texture;

    struct {
        emgpu_surface* surface;
        emgpu_local_framebuffer dst_framebuffer;
    } acquire_surface;
} cmd_payload;

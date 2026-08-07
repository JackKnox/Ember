#pragma once

#include "ember/core.h"

#include "ember/gpu/command_buffer.h"

#include "ember/gpu/compute.h"
#include "ember/gpu/raster.h"
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
    COMMAND_BIND_VERTEX_BUFFERS,
    COMMAND_BIND_INDEX_BUFFER,
    COMMAND_DRAW,

    COMMAND_IMPORT_TEXTURE,

    COMMAND_ACQUIRE_SURFACE,
} rendercmd_payload_type;

typedef struct rendercmd_payload {
    struct {
        rendercmd_payload_type type;
        u64 size;
    } hdr;

    union {
        struct {
            const emgpu_pipeline* pipeline;
            emgpu_resource_export* exports;
            emgpu_resource_import* imports;
            uvec3 local_size;
            u32 export_resource_count;
            u32 import_resource_count;
        } begin_computepass;

        struct {
            uvec3 group_size;
        } dispatch;

        struct {
            b8 _p;
        } end_computepass;

        struct {
            emgpu_colour_attachment* colours;
            u32 colour_attachment_count;
        } begin_renderpass;

        struct {
            b8 _p;
        } end_renderpass;

        struct {
            uvec2 origin, size;
            f32 min_depth, max_depth;
        } set_viewport;

        struct {
            uvec2 origin, size;
        } set_scissor;

        struct {
            const emgpu_pipeline* pipeline;
            emgpu_resource_export* exports;
            emgpu_resource_import* imports;
            u32 export_resource_count;
            u32 import_resource_count;
        } bind_raster_pipeline;

        struct {
            emgpu_buffer* vertex_buffers;
            u32 vertex_buffer_count;
        } bind_vertex_buffers;

        struct {
            emgpu_buffer* index_buffer;
        } bind_index_buffer;

        struct {
            u32 vertex_count, instance_count;
        } draw;

        struct {
            emgpu_texture* texture;
            emgpu_local_framebuffer dst_framebuffer;
        } import_texture;

        struct {
            emgpu_surface* surface;
            emgpu_local_framebuffer dst_framebuffer;
        } acquire_surface;
    };
} rendercmd_payload;

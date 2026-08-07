#pragma once

#include "ember/core.h"

#include "ember/gpu/command_buffer.h"

typedef enum rendercmd_payload_type {
    RENDERCMD_ACCQUIRE_SURFACE,
    RENDERCMD_IMPORT_TEXTURE,
    RENDERCMD_SET_RENDERAREA,
    RENDERCMD_BEGIN_RENDERPASS,
    RENDERCMD_END_RENDERPASS,
    RENDERCMD_BIND_PIPELINE,
    RENDERCMD_DRAW,
    RENDERCMD_DRAW_INDEXED,
    RENDERCMD_DISPATCH,
} rendercmd_payload_type;

typedef struct rendercmd_payload {
    struct {
        rendercmd_payload_type type;
        u64 size;
    } hdr;

    union {
        struct {
            emgpu_surface* surface;
            emgpu_frame_texture dst_texture;
        } accquire_surface;

        struct {
            emgpu_texture* texture;
            emgpu_frame_texture dst_texture;
        } import_texture;

        struct {
            uvec2 origin, size;
            u64 _p;
        } set_renderarea;

        struct {
            emgpu_renderpass* renderpass;
            u32 attachment_count;
            u32 clear_colour;
            emgpu_frame_texture attachments[];
        } begin_renderpass;

        struct {
            emgpu_pipeline* pipeline;
            emgpu_buffer* vertex_buffers, * index_buffer;
            u32 vertex_buffer_count;
        } bind_pipeline;

        struct {
            u32 vertex_count, instance_count;
        } draw;

        struct {
            u32 index_count, instance_count;
        } draw_indexed;

        struct {
            uvec3 group_size;
        } dispatch;
    };
} rendercmd_payload;

#pragma once

#include "ember/core.h"

#include "ember/gpu/device.h"

/**
 * @brief Render command types.
 *
 * Represent high-level rendering operations that are translated
 * into backend-specific API calls (OpenGL, Vulkan, etc.).
 */
typedef enum rendercmd_payload_type {
    RENDERCMD_SET_RENDERAREA,
    RENDERCMD_BIND_NEXT_SURFACE_TEXTURE,
    RENDERCMD_BIND_IMPORT_TEXTURE,
    RENDERCMD_BEGIN_RENDERPASS,
    RENDERCMD_END_RENDERPASS,
    RENDERCMD_MEMORY_BARRIER,
    RENDERCMD_BIND_PIPELINE,
    RENDERCMD_DRAW,
    RENDERCMD_DRAW_INDEXED,
    RENDERCMD_DISPATCH,

    RENDERCMD_DUMMY,
} rendercmd_payload_type;

typedef struct rendercmd_payload {
    struct {
        rendercmd_payload_type type;
        emgpu_ops_type command_mode;
    } hdr;

    union {
        struct {
            uvec2 origin, size;
        } set_renderarea;

        struct {
            emgpu_frame_texture dst_texture;
            u32 surface_index;
        } next_surface_texture;

        struct {
            emgpu_texture* texture;
            emgpu_frame_texture dst_texture;
        } import_texture;

        struct {
            emgpu_renderpass* renderpass;
            emgpu_frame_texture* attachments;
            u32 attachment_count;
        } bind_renderpass;

        struct {
            emgpu_pipeline* src_pipeline, * dst_pipeline;
            emgpu_access_flags src_access, dst_access;
        } memory_barrier;

        struct {
            emgpu_pipeline* pipeline;
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
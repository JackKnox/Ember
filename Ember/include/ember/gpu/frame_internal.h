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
    RENDERCMD_NEXT_SURFACE_TEXTURE,
    RENDERCMD_IMPORT_TEXTURE,
    RENDERCMD_SET_RENDERAREA,
    RENDERCMD_BEGIN_RENDERPASS,
    RENDERCMD_END_RENDERPASS,
    RENDERCMD_BIND_PIPELINE,
    RENDERCMD_DRAW,
    RENDERCMD_DRAW_INDEXED,
    RENDERCMD_DISPATCH,
    RENDERCMD_EXPORT_RESOURCE,
    RENDERCMD_USE_RESOURCE,

    RENDERCMD_FLUSH,
    RENDERCMD_DUMMY,
} rendercmd_payload_type;

typedef struct rendercmd_payload {
    struct {
        rendercmd_payload_type type;
    } hdr;

    union {
        struct {
            emgpu_surface* surface;
            emgpu_frame_texture dst_texture;
        } next_surface_texture;

        struct {
            emgpu_texture* texture;
            emgpu_frame_texture dst_texture;
        } import_texture;

        struct {
            uvec2 origin, size;
        } set_renderarea;

        struct {
            emgpu_renderpass* renderpass;
            emgpu_frame_texture* attachments;
            u32 attachment_count;
        } begin_renderpass;

        struct {
            emgpu_pipeline* pipeline;
            emgpu_buffer* vertex_buffer, * index_buffer;
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

        struct {
            emgpu_access_flags resource_access;
            u32 descriptor_index;
        } export_resource;

        struct {
            emgpu_access_flags resource_access;
            u32 descriptor_index;
        } use_resource;
    };
} rendercmd_payload;
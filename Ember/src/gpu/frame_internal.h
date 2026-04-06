#pragma once

#include "ember/core.h"

#include "ember/gpu/device.h"

/**
 * @brief Render command types.
 *
 * Represent high-level rendering operations that are translated
 * into backend-specific API calls (OpenGL, Vulkan, etc.).
 */
enum {
    RENDERCMD_BIND_RENDERTARGET,
    RENDERCMD_MEMORY_BARRIER,
    RENDERCMD_BEGIN_PIPELINE,
    RENDERCMD_END_PIPELINE,
    RENDERCMD_DRAW,
    RENDERCMD_DRAW_INDEXED,
    RENDERCMD_DISPATCH,

    RENDERCMD_DUMMY,
};

/** @brief Type used to identify a render command payload. */
typedef u32 rendercmd_payload_type;

#pragma pack(push, 1)

typedef struct rendercmd_payload {
    rendercmd_payload_type type;

    union {
        struct {
            emgpu_rendertarget* rendertarget;
        } bind_rendertarget;

        struct {
            emgpu_pipeline* src_pipeline, * dst_pipeline;
            emgpu_access_flags src_access, dst_access;
        } memory_barrier;

        struct {
            emgpu_pipeline* pipeline;
        } begin_pipeline;

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

#pragma pack(pop)
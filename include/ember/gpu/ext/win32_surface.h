#pragma once

#include "ember/core.h"

#include "ember/gpu/device.h"

#include <Windows.h>

/**
 * @brief Returns the Win32 surface extension descriptor.
 */
emgpu_extension_desc emgpu_win32_surface_extension();

typedef struct emgpu_win32_surface_config {
    /** @brief Requested format of the surface texture. */
    emgpu_format preferred_format;

    /** @brief Whetever to exit if exact preferred format isn't found. */
    b8 force_format;

    /** @brief Size of the Win32 surface. */
    uvec2 size;

    /** @brief Title of the surface, used in validation. */
    const char* debug_name;

    /** @brief HINSANCE to create window surface with. */
    HINSTANCE hinstance;

    /** @brief HWND to create window surface with. */
    HWND hwnd;
} emgpu_win32_surface_config;

/**
 * @brief Creates a default Win32 surface configuration.
 *
 * @return A default-initialized emgpu_win32_surface_config.
 */
emgpu_win32_surface_config emgpu_win32_surface_default();

/**
 * @brief Function pointer type for creating a Win32 surface.
 *
 * @param device GPU device handle.
 * @param allocator Memory allocator used for internal allocations.
 * @param config Win32 surface creation parameters.
 * @param out_surface Output GPU surface object.
 *
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
typedef em_result (*PFN_create_win32_surface)(
    emgpu_device* device,
    em_allocator* allocator,
    emgpu_win32_surface_config* config,
    emgpu_surface* out_surface);

/**
 * @brief Win32 surface extension API interface.
 *
 * Contains function pointers and internal state required to create
 * and manage Win32-presentable GPU surfaces.
 */
typedef struct emgpu_win32_surface_ext {
    /** @brief If this value is FALSE, the rest of the structure is uninitalized. */
    b8 enabled;

    /** @brief Creates a Win32 surface backed by the GPU device. */
    PFN_create_win32_surface create_surface;
} emgpu_win32_surface_ext;

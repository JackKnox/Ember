#pragma once

#include "ember/core.h"

#include "ember/gpu/types.h"

#include "ember/gpu/device.h"
#include "ember/gpu/resources.h"

/**
 * @brief Configuration for a compute pipeline.
 */
typedef struct emgpu_compute_pipeline_config {
    /** @brief Refrence to extra configuration structure specific to API type. */
    void* api_next;

    /** @brief Shader stage ran per-compute cell, must be valid. */
    emgpu_shader_src shader;

    /** @brief Number of active descriptor bindings. */
	u32 descriptor_count;

    /** @brief Descriptor binding descriptions used by the pipeline. */
    emgpu_descriptor_desc* descriptors;
} emgpu_compute_pipeline_config;

#ifdef EMBER_DEFINE_HELPERS

/**
 * @brief Creates a default compute stage configuration.
 *
 * @return A default-initialized emgpu_compute_pipeline_config.
 */
emgpu_compute_pipeline_config emgpu_pipeline_default_compute();

#endif

/**
 * @brief Creates a compute pipeline.
 *
 * @param device Pointer to the device instance.
 * @param allocator Allocator used to manage device memory.
 * @param config Pipeline configuration.
 * @param out_compute_pipeline Output pipeline.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emgpu_device_create_compute_pipeline(
    emgpu_device* device, 
    em_allocator* allocator, 
    const emgpu_compute_pipeline_config* config, 
    emgpu_pipeline* out_compute_pipeline);
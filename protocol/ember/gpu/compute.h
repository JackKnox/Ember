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
 * @brief Configuration for a compute pass.
 *
 * A compute pass is a live context of a compute pipeline
 * within a command buffer. It requires info about local group size,
 * resource imports and resource exports.
 */
typedef struct emgpu_computepass_config {
    /** @brief Connected pipeline to computepass. */
    const emgpu_pipeline* pipeline;

    /** @brief Resources to export from pipeline. */
    const emgpu_resource_export* export_resources;

    /** @brief Number of export resources. */
    u32 export_resource_count;

    /** @brief Resources to import into pipeline. */
    const emgpu_resource_import* import_resources;

    /** @brief Number of import resources. */
    u32 import_resource_count;
} emgpu_computepass_config;

#ifdef EMBER_DEFINE_HELPERS

/**
 * @brief Creates a default-initialized compute pass configuration.
 *
 * @return A default-initialized emgpu_computepass_config.
 */
emgpu_computepass_config emgpu_computepass_default();

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
em_result emgpu_compute_pipeline_create(
    emgpu_device* device, 
    em_allocator* allocator, 
    const emgpu_compute_pipeline_config* config, 
    emgpu_pipeline* out_compute_pipeline);

/**
 * @brief Begins a compute pass.
 *
 * @param command_buf Pointer to the command buffer.
 * @param config Compute pass configuration.
 */
void emgpu_cmd_begin_computepass(emgpu_command_buffer* command_buf, const emgpu_computepass_config* config);

/**
 * @brief Dispatches a compute workload.
 *
 * @param command_buf Pointer to the command buffer.
 * @param group_size Number of compute workgroups in XYZ dimensions.
 */
void emgpu_cmd_dispatch(emgpu_command_buffer* command_buf, uvec3 group_size);

/**
 * @brief Ends current compute pass.
 *
 * @param command_buf Pointer to the command buffer.
 */
void emgpu_cmd_end_computepass(emgpu_command_buffer* command_buf);

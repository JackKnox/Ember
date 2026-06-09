#pragma once

#include "ember/core.h"

/**
 * @brief Platform-specific shared memory state.
 */
typedef void* emplat_shm_state;

/**
 * Creates or opens a named shared memory region and maps it into the process.
 *
 * @param name Name of the shared memory region.
 * @param size Size of the region in bytes.
 * @param out_state Receives the mapped memory address.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 * 
 * @note The returned handle must be released using
 *       emplat_close_shm().
 */
em_result emplat_open_shm(const char* name, u64 size, emplat_shm_state* out_state);

/**
 * @brief Returns pointer to mapped memory region.
 */
void* emplat_shm_pointer(emplat_shm_state* state);


/**
 * Unmaps a shared memory region from the process.
 *
 * @param name Name of the shared memory region.
 * @param size Size of the region in bytes.
 * @param state Mapped memory state returned by emplat_open_shm().
 * 
 * @note After unloading, the handle becomes invalid and must not be used.
 */
void emplat_close_shm(emplat_shm_state* state);
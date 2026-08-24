#pragma once

#include "ember/core.h"

#include "ember/window/format.h"
#include "ember/window/desktop.h"

/**
 * @brief Configuration for a shared-memory pool.
 */
typedef struct emwin_shm_pool_config {
    /** @brief Size of the shared-memory pool in bytes. */
    u64 size;
} emwin_shm_pool_config;

#ifdef EMBER_DEFINE_HELPERS

/**
 * @brief Creates a default shared memory pool configuration
 *
 * @return A deafault initailized emwin_shm_pool_config.
 */
emwin_shm_pool_config emwin_shm_pool_default();

#endif

/**
 * @brief Represents a shared-memory pool.
 *
 * A shared-memory pool owns the backing storage from which one or more
 * shared-memory buffers can be created.
 */
typedef struct emwin_shm_pool {
    /** @brief Internal platform-specific pool data. */
    void* internal_data;
} emwin_shm_pool;

/**
 * @brief Creates a shared-memory pool.
 *
 * @param desktop Desktop connection the pool belongs to.
 * @param allocator Allocator used to create the pool.
 * @param config Pool configuration.
 * @param out_pool Receives the created pool.
 *
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeds.
 */
em_result emwin_shm_pool_create(emwin_desktop* desktop, em_allocator* allocator, const emwin_shm_pool_config* config, emwin_shm_pool* out_pool);

/**
 * @brief Destroys a shared-memory pool.
 *
 * All buffers created from the pool must be freed before destroying it.
 *
 * @param allocator Allocator used to create the pool.
 * @param pool Pool to destroy.
 */
void emwin_shm_pool_destroy(em_allocator* allocator, emwin_shm_pool* pool);

/**
 * @brief Configuration for a shared-memory buffer.
 */
typedef struct emwin_shm_buffer_config {
    /** @brief Pixel format of the image stored in the buffer. */
    emwin_format image_format;

    /** @brief Byte offset of the buffer within its shared-memory pool. */
    u64 offset;

    /** @brief Size of the buffer in bytes. */
    uvec2 size;

    /** @brief Number of bytes between the start of consecutive image rows. */
    u64 stride;
} emwin_shm_buffer_config;

#ifdef EMBER_DEFINE_HELPERS

/**
 * @brief Creates a default shared memory buffer configuration
 *
 * @return A deafault initailized emwin_shm_buffer_config.
 */
emwin_shm_buffer_config emwin_shm_buffer_default();

#endif

/**
 * @brief Represents a shared-memory image buffer.
 *
 * A buffer describes a region of a shared-memory pool that can be used
 * as pixel storage for a window surface.
 */
typedef struct emwin_shm_buffer {
    /** @brief Internal platform-specific buffer data. */
    void* internal_data;

    /** @brief Pointer to the buffer's CPU-accessible pixel data. */
    void* buffer;

    /** @brief Width and height of the image in pixels. */
    uvec2 size;
} emwin_shm_buffer;

/**
 * @brief Allocates a shared-memory buffer from a pool.
 *
 * @param pool Pool from which the buffer is allocated.
 * @param allocator Allocator used to create the buffer.
 * @param config Buffer configuration.
 * @param out_buffer Receives the allocated buffer.
 *
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeds.
 */
em_result emwin_shm_buffer_alloc(emwin_shm_pool* pool, em_allocator* allocator, const emwin_shm_buffer_config* config, emwin_shm_buffer* out_buffer);

/**
 * @brief Frees a shared-memory buffer.
 *
 * @param pool Pool containing the buffer.
 * @param allocator Allocator used to create the buffer.
 * @param buffer Buffer to free.
 */
void emwin_shm_buffer_free(emwin_shm_pool* pool, em_allocator* allocator, emwin_shm_buffer* buffer);

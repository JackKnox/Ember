#pragma once

#include "ember/core.h"

#include "ember/core/datastream.h"

/**
 * @brief Platform-specific shared memory state.
 */
typedef void* emplat_shm_state;

/**
 * @brief Creates or opens a named shared memory region and maps it into the process.
 *
 * @param name Name of the shared memory region.
 * @param size Size of the region in bytes.
 * @param out_state Receives the mapped memory address.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 * 
 * @note The returned handle must be released using
 *       emplat_shm_close().
 */
em_result emplat_shm_open(const char* name, u64 size, emplat_shm_state* out_state);

/**
 * @brief Returns pointer to mapped memory region.
 */
void* emplat_shm_pointer(emplat_shm_state* state);


/**
 * @brief Unmaps a shared memory region from the process.
 *
 * @param name Name of the shared memory region.
 * @param size Size of the region in bytes.
 * @param state Mapped memory state returned by emplat_open_shm().
 * 
 * @note After unloading, the handle becomes invalid and must not be used.
 */
void emplat_shm_close(emplat_shm_state* state);

/**
 * @brief Flags controlling how a message queue is opened.
 */
typedef enum emplat_mqueue_flags {
    EMBER_MQUEUE_FLAG_CREATE   = 1 << 0, /**< Create the message queue if it does not already exist. */
    EMBER_MQUEUE_FLAG_OPEN     = 1 << 1, /**< Open an existing message queue. */
    EMBER_MQUEUE_FLAG_READ     = 1 << 2, /**< Allow messages to be received from the queue. */
    EMBER_MQUEUE_FLAG_WRITE    = 1 << 3, /**< Allow messages to be sent to the queue. */
    EMBER_MQUEUE_FLAG_NONBLOCK = 1 << 4, /**< Perform send and receive operations without blocking. */
} emplat_mqueue_flags;

/**
 * @brief Opens or creates a platform message queue endpoint.
 *
 * @param name Null-terminated UTF-8 name identifying the queue.
 * @param capacity Maximum number of messages the queue can hold.
 * @param message_size Size of each message, in bytes.
 * @param flags Message queue flags. 
 *
 * @note If @ref EMBER_MQUEUE_FLAG_CREATE is specified, @p capacity 
 * and @p message_size define the queue configuration. When opening an existing
 * queue, these values must be compatible with the existing queue.
 *
 * @return A valid endpoint on success, or EM_ENDPOINT_INVALID on failure.
 */
em_endpoint emplat_mqueue_open(const char* name, u64 capacity, u64 message_size, emplat_mqueue_flags flags);

/**
 * @brief Closes a platform message queue endpoint.
 *
 * Releases all resources associated with the endpoint. 
 *
 * @note Closing an endpoint does not necessarily destroy the underlying 
 *       named message queue; other processes or endpoints may still be using it.
 */
void emplat_mqueue_close(em_endpoint endpoint);

#pragma once

#include "ember/core.h"

/**
 * @brief Opaque handle to a message endpoint.
 *
 * Endpoints provide zero-copy message passing through a unified send and
 * receive interface. An endpoint may represent a peer, an OS connection,
 * a callback, or any other message source or sink.
 */
typedef u64 em_endpoint;

#define EM_ENDPOINT_INVALID ((em_endpoint)0)

/**
 * @brief Borrows the next available message from an endpoint.
 *
 * If a message is available, @p recv_ptr is set to point to the message payload
 * owned by the endpoint and @p out_size receives its size in bytes.
 *
 * The returned pointer remains valid until @ref em_endpoint_consume() is called.
 * The caller must not free or modify the returned memory unless the endpoint
 * explicitly permits it.
 *
 * @param endpoint Endpoint to receive from.
 * @param out_size Receives the size of the message in bytes.
 * @param recv_ptr Receives a pointer to the message payload.
 *
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result em_endpoint_recv(em_endpoint endpoint, u64* out_size, void** recv_ptr);

/**
 * @brief Consumes the currently borrowed message.
 *
 * Marks the current message as processed and releases it back to the endpoint,
 * allowing the next queued message to become available.
 *
 * Every successful call to @ref em_endpoint_recv() must eventually be followed
 * by exactly one call to em_endpoint_consume().
 *
 * @param endpoint Endpoint previously passed to em_endpoint_recv().
 */
void em_endpoint_consume(em_endpoint endpoint);

/**
 * @brief Borrows storage for a new outgoing message.
 *
 * Requests writable storage for a message of @p size bytes. On success,
 * @p send_ptr points to memory owned by the endpoint into which the caller
 * writes the message contents.
 *
 * The borrowed storage must be returned by calling
 * @ref em_endpoint_release().
 *
 * @param endpoint Endpoint to send to.
 * @param size Size of the message in bytes.
 * @param[out] send_ptr Receives writable message storage.
 *
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result em_endpoint_send(em_endpoint endpoint, u64 size, void** send_ptr);

/**
 * @brief Publishes the currently borrowed outgoing message.
 *
 * Makes the message previously acquired with @ref em_endpoint_send() available
 * to receivers.
 *
 * Every successful call to @ref em_endpoint_send() must eventually be followed
 * by exactly one call to em_endpoint_release().
 *
 * @param endpoint Endpoint previously passed to em_endpoint_send().
 */
void em_endpoint_release(em_endpoint endpoint);

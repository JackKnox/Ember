#pragma once

#include "ember/core.h"

#include "ember/platform/internal.h"

/**
 * @brief Platform-specific thread handle.
 */
typedef EMBER_PLATFORM_THREAD_STATE emplat_thread;

/**
 * @typedef PFN_thread_start
 * @brief Thread entry point function type.
 *
 * @param arg User-provided argument.
 * @return Non-descriptive code for any system.
 */
typedef void* (*PFN_thread_start)(void* arg);

/**
 * @brief Creates a new thread.
 *
 * @param thr Pointer to store the created thread handle.
 * @param func Entry function for the thread.
 * @param arg Argument passed to the thread function.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_thread_create(emplat_thread* thr, PFN_thread_start func, void* arg);

/**
 * @brief Retrieves the calling thread identifier.
 *
 * @return Handle representing the current thread.
 */
emplat_thread emplat_thread_current();

/**
 * @brief Compares two thread identifiers.
 *
 * @param thr0 First thread.
 * @param thr1 Second thread.
 * @return True if equal, false if not equal.
 */
b8 emplat_thread_equal(emplat_thread thr0, emplat_thread thr1);

/**
 * @brief Waits for a thread to terminate.
 *
 * Blocks the calling thread until the specified thread exits.
 *
 * @param thr Thread to join.
 * @param res Optional pointer to store the thread's return value.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_thread_join(emplat_thread thr, u32* res);

/**
 * @brief Defines the type of mutex to be created.
 */
typedef enum emplat_mutex_type {
    EMBER_MUTEX_TYPE_PLAIN,     /**< Standard non-recursive mutex */
    EMBER_MUTEX_TYPE_TIMED,     /**< Mutex supporting timed locking operations */
    EMBER_MUTEX_TYPE_RECURSIVE, /**< Recursive mutex allowing the same thread to lock multiple times */
} emplat_mutex_type;

/**
 * @brief Platform-specific mutex object.
 */
typedef EMBER_PLATFORM_MUTEX_STATE emplat_mutex;

/**
 * @brief Initializes a mutex.
 *
 * @param type The type of mutex to create.
 * @param mtx Pointer to the mutex to initialize.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_mutex_init(emplat_mutex_type type, emplat_mutex* mtx);

/**
 * @brief Destroys a mutex and releases its resources.
 *
 * @param mtx Pointer to the mutex to destroy.
 */
void emplat_mutex_destroy(emplat_mutex* mtx);

/**
 * @brief Locks a mutex.
 *
 * Blocks the calling thread until the mutex becomes available.
 *
 * @param mtx Pointer to the mutex to lock.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_mutex_lock(emplat_mutex* mtx);

/**
 * @brief Locks a mutex with a timeout.
 *
 * Blocks until the mutex is acquired or the specified time is reached.
 *
 * @param mtx Pointer to the mutex to lock.
 * @param ts Absolute timeout in milliseconds.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_mutex_timedlock(emplat_mutex* mtx, f64 ms);

/**
 * @brief Attempts to lock a mutex without blocking.
 *
 * @param mtx Pointer to the mutex to lock.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_mutex_trylock(emplat_mutex* mtx);

/**
 * @brief Unlocks a mutex.
 *
 * @param mtx Pointer to the mutex to unlock.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_mutex_unlock(emplat_mutex* mtx);

/**
 * @typedef emplat_cond
 * @brief Platform-specific condition variable.
 */
typedef EMBER_PLATFORM_COND_STATE emplat_cond;

/**
 * @brief Initializes a condition variable.
 *
 * @param cond Pointer to the condition variable to initialize.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_cond_init(emplat_cond* cond);

/**
 * @brief Destroys a condition variable.
 *
 * @param cond Pointer to the condition variable to destroy.
 */
void emplat_cond_destroy(emplat_cond* cond);

/**
 * @brief Signals one waiting thread.
 *
 * Unblocks one thread waiting on the condition variable.
 *
 * @param cond Pointer to the condition variable.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_cond_signal(emplat_cond* cond);

/**
 * @brief Signals all waiting threads.
 *
 * Unblocks all threads waiting on the condition variable.
 *
 * @param cond Pointer to the condition variable.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_cond_broadcast(emplat_cond* cond);

/**
 * @brief Waits for a condition variable.
 *
 * Atomically unlocks the mutex and blocks until the condition is signaled.
 * Upon wake-up, the mutex is re-acquired before returning.
 *
 * @param cond Pointer to the condition variable.
 * @param mtx Pointer to the associated mutex.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_cond_wait(emplat_cond* cond, emplat_mutex* mtx);

/**
 * @brief Waits for a condition variable with a timeout.
 *
 * Atomically unlocks the mutex and blocks until:
 * - The condition variable is signaled, or
 * - The specified timeout is reached.
 *
 * The mutex is re-acquired before returning.
 *
 * @param cond Pointer to the condition variable.
 * @param mtx Pointer to the associated mutex.
 * @param ts Absolute timeout in milliseconds.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_cond_timedwait(emplat_cond* cond, emplat_mutex* mtx, f64 ms);
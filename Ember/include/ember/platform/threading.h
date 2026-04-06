#pragma once

#include "ember/core.h"

// Generic includes
#include <time.h>

// Platform-specific includes
#if defined(EM_PLATFORM_POSIX)
#   include <pthread.h>
#elif defined(EM_PLATFORM_WINDOWS)
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN
#   endif
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   include <windows.h>
#endif

// If TIME_UTC is missing, provide it and provide a wrapper for timespec_get.
#ifndef TIME_UTC
#define TIME_UTC 1
#define EM_EMULATE_TIMESPEC_GET

#if defined(EM_PLATFORM_WINDOWS)
struct _tthread_timespec {
    time_t tv_sec;
    long   tv_nsec;
};
#define timespec _tthread_timespec
#endif

int _tthread_timespec_get(struct timespec* ts, int base);
#define timespec_get _tthread_timespec_get
#endif

// Mutex types
typedef enum emplat_mutex_type {
    EMBER_MUTEX_TYPE_PLAIN,
    EMBER_MUTEX_TYPE_TIMED,
    EMBER_MUTEX_TYPE_RECURSIVE,
} emplat_mutex_type;

// Mutexs
#if defined(EM_PLATFORM_WINDOWS)
typedef struct emplat_mutex {
    union {
        CRITICAL_SECTION cs;
        HANDLE mut;         
    } mHandle;
    emplat_mutex_type mType;
    int mAlreadyLocked;
} emplat_mutex;
#else
typedef pthread_mutex_t emplat_mutex;
#endif

// Create a mutex object.
b8 emplat_mutex_init(emplat_mutex* mtx, emplat_mutex_type type);

//  Release any resources used by the given mutex.
void emplat_mutex_destroy(emplat_mutex* mtx);

// Lock the given mutex. Blocks until the given mutex can be locked.
b8 emplat_mutex_lock(emplat_mutex* mtx);

// Lock the given mutex, or block until a specific point in time.
b8 emplat_mutex_timedlock(emplat_mutex* mtx, const struct timespec* ts);

// Try to lock the given mutex.
b8 emplat_mutex_trylock(emplat_mutex* mtx);

// Unlock the given mutex.
b8 emplat_mutex_unlock(emplat_mutex* mtx);

// Condition variable
#if defined(EM_PLATFORM_WINDOWS)
typedef struct emplat_cond {
    HANDLE mEvents[2];
    unsigned int mWaitersCount;
    CRITICAL_SECTION mWaitersCountLock;
} emplat_cond;
#else
typedef pthread_cond_t emplat_cond;
#endif

// Create a condition variable object.
b8 emplat_cond_init(emplat_cond* cond);

// Release any resources used by the given condition variable.
void emplat_cond_destroy(emplat_cond* cond);

// Signal a condition variable.
b8 emplat_cond_signal(emplat_cond* cond);

// Broadcast a condition variable.
b8 emplat_cond_broadcast(emplat_cond* cond);

// Wait for a condition variable to become signaled.
b8 emplat_cond_wait(emplat_cond* cond, emplat_mutex* mtx);

// Wait for a condition variable to become signaled.
// The function atomically unlocks the given mutex and endeavors to block until
// the given condition variable is signaled by a call to cnd_signal or to
// cnd_broadcast, or until after the specified time. When the calling thread
// becomes unblocked it locks the mutex before it returns.
b8 emplat_cond_timedwait(emplat_cond* cond, emplat_mutex* mtx, const struct timespec* ts);

// Thread
#if defined(EM_PLATFORM_WINDOWS)
typedef HANDLE emplat_thread;
#else
typedef pthread_t emplat_thread;
#endif

// Thread start function.
typedef b8 (*PFN_thread_start)(void* arg);

// Create a new thread.
b8 emplat_thread_create(emplat_thread* thr, PFN_thread_start func, void* arg);

// Identify the calling thread.
emplat_thread emplat_thread_current();

// Dispose of any resources allocated to the thread when that thread exits.
b8 emplat_thread_detach(emplat_thread thr);

// Compare two thread identifiers.
b8 emplat_thread_equal(emplat_thread thr0, emplat_thread thr1);

// Terminate execution of the calling thread.
EM_NORETURN void emplat_thread_exit(int res);

// Wait for a thread to terminate. The function joins the given thread with the current thread by blocking until the other thread has terminated.
b8 emplat_thread_join(emplat_thread thr, int* res);

// Yield execution to another thread. Permit other threads to run, even if the current thread would ordinarily continue to run.
void emplat_thread_yield();

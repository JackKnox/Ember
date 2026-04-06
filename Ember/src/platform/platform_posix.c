#include "ember/core.h"

#ifdef EM_PLATFORM_POSIX
#include "ember/platform/threading.h"

#include <unistd.h>

#include <sched.h>
#include <sys/time.h>
#include <errno.h>

void emplat_sleep_ms(f64 ms) {
    struct timespec ts;
    ts.tv_sec  = ms / 1000;

    while (nanosleep(&ts, &ts) == -1)
        continue;
}

b8 emplat_mutex_init(emplat_mutex* mtx, emplat_mutex_type type) {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    if (type & EMBER_MUTEX_TYPE_RECURSIVE)
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    int ret = pthread_mutex_init(mtx, &attr);
    pthread_mutexattr_destroy(&attr);
    return ret == 0 ? TRUE : FALSE;
}

void emplat_mutex_destroy(emplat_mutex* mtx) {
    pthread_mutex_destroy(mtx);
}

b8 emplat_mutex_lock(emplat_mutex* mtx) {
    return pthread_mutex_lock(mtx) == 0 ? TRUE : FALSE;
}

b8 emplat_mutex_timedlock(emplat_mutex* mtx, const struct timespec* ts) {
#if defined(_POSIX_TIMEOUTS) && (_POSIX_TIMEOUTS >= 200112L) && defined(_POSIX_THREADS) && (_POSIX_THREADS >= 200112L)
    switch (pthread_mutex_timedlock(mtx, ts)) {
    case 0:
        return TRUE;
    case ETIMEDOUT:
    default:
        return FALSE;
    }
#else
    int rc;
    struct timespec cur, dur;

    // Try to acquire the lock and, if we fail, sleep for 5ms.
    while ((rc = pthread_mutex_trylock(mtx)) == EBUSY) {
        timespec_get(&cur, TIME_UTC);

        if ((cur.tv_sec > ts->tv_sec) || ((cur.tv_sec == ts->tv_sec) && (cur.tv_nsec >= ts->tv_nsec)))
            break;

        dur.tv_sec = ts->tv_sec - cur.tv_sec;
        dur.tv_nsec = ts->tv_nsec - cur.tv_nsec;
        if (dur.tv_nsec < 0) {
            dur.tv_sec--;
            dur.tv_nsec += 1000000000;
        }

        if ((dur.tv_sec != 0) || (dur.tv_nsec > 5000000)) {
            dur.tv_sec = 0;
            dur.tv_nsec = 5000000;
        }

        nanosleep(&dur, NULL);
    }

    switch (rc) {
    case 0:
        return TRUE;
    case ETIMEDOUT:
    case EBUSY:
    default:
        return FALSE;
    }
#endif
}

b8 emplat_mutex_trylock(emplat_mutex* mtx) {
    return (pthread_mutex_trylock(mtx) == 0) ? TRUE : FALSE;
}

b8 emplat_mutex_unlock(emplat_mutex* mtx) {
    return pthread_mutex_unlock(mtx) == 0 ? TRUE : FALSE;
}

b8 emplat_cond_init(emplat_cond* cond) {
    return pthread_cond_init(cond, NULL) == 0 ? TRUE : FALSE;
}

void emplat_cond_destroy(emplat_cond* cond) {
    pthread_cond_destroy(cond);
}

b8 emplat_cond_signal(emplat_cond* cond) {
    return pthread_cond_signal(cond) == 0 ? TRUE : FALSE;
}

b8 emplat_cond_broadcast(emplat_cond* cond) {
    return pthread_cond_broadcast(cond) == 0 ? TRUE : FALSE;
}

b8 emplat_cond_wait(emplat_cond* cond, emplat_mutex* mtx) {
    return pthread_cond_wait(cond, mtx) == 0 ? TRUE : FALSE;
}

b8 emplat_cond_timedwait(emplat_cond* cond, emplat_mutex* mtx, const struct timespec* ts) {
    int ret = pthread_cond_timedwait(cond, mtx, ts);
    if (ret == ETIMEDOUT) return FALSE;

    return ret == 0 ? TRUE : FALSE;
}

// Information to pass to the new thread (what to run).
typedef struct _thread_start_info {
    PFN_thread_start mFunction; // Pointer to the function to be executed.
    void* mArg;             // Function argument for the thread function.
} _thread_start_info;

// Thread wrapper function.
static void* _thrd_wrapper_function(void* aArg) {
    PFN_thread_start fun;
    void* arg;
    b8  res;

    // Get thread startup information
    _thread_start_info* ti = (_thread_start_info*)aArg;
    fun = ti->mFunction;
    arg = ti->mArg;

    // The thread is responsible for freeing the startup information
    bfree(ti, sizeof(ti), MEMORY_TAG_PLATFORM);

    // Call the actual client thread function
    res = fun(arg);

    return (void*)(intptr_t)res;
}

b8 emplat_thread_create(emplat_thread* thr, PFN_thread_start func, void* arg) {
    // Fill out the thread startup information (passed to the thread wrapper, which will eventually free it)
    _thread_start_info* ti = (_thread_start_info*)ballocate(sizeof(_thread_start_info), MEMORY_TAG_PLATFORM);
    ti->mFunction = func;
    ti->mArg = arg;

    // Create the thread
    if (pthread_create(thr, NULL, _thrd_wrapper_function, (void*)ti) != 0)
        *thr = 0;

    // Did we fail to create the thread?
    if (!*thr) {
        bfree(ti, sizeof(ti), MEMORY_TAG_PLATFORM);
        return FALSE;
    }

    return TRUE;
}

emplat_thread emplat_thread_current(void) {
    return pthread_self();
}

b8 emplat_thread_detach(emplat_thread thr) {
    return pthread_detach(thr) == 0 ? TRUE : FALSE;
}

b8 emplat_thread_equal(emplat_thread thr0, emplat_thread thr1) {
    return pthread_equal(thr0, thr1);
}

void emplat_thread_exit(int res) {
    pthread_exit((void*)(intptr_t)res);
}

b8 emplat_thread_join(emplat_thread thr, int* res) {
    void* pres;
    if (pthread_join(thr, &pres) != 0)
        return FALSE;

    if (res != NULL)
        *res = (int)(intptr_t)pres;

    return TRUE;
}

void emplat_thread_yield(void) {
    sched_yield();
}
#endif
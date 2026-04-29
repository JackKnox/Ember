#include "ember/core.h"

#ifdef EM_PLATFORM_POSIX
#include "ember/platform/system.h"
#include "ember/platform/threading.h"

#include <unistd.h>
#include <time.h>

#include <sched.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <errno.h>

const char* emplat_system_get_env(const char* name) {
    return getenv(name);
}

em_result emplat_system_set_env(const char* name, const char* value) {
    return setenv(name, value, 1) == 0 ? EMBER_RESULT_OK : EMBER_RESULT_UNKNOWN;
}

u32 emplat_system_get_pid() {
    return getpid();
}

em_result emplat_system_execute(const char* command) {
	pid_t new_pid = fork();
	if (new_pid < 0) return EMBER_RESULT_UNKNOWN;

	if (new_pid == 0) {
        execl("/bin/sh", "sh", "-c", command, (char*)0);
        _exit(127);
    }

    int status;
    if (waitpid(new_pid, &status, 0) < 0) {
        return EMBER_RESULT_UNKNOWN;
    }

    return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? EMBER_RESULT_OK : EMBER_RESULT_UNKNOWN;
}

emplat_library emplat_system_library_load(const char* path) {
	return dlopen(path, RTLD_LAZY | RTLD_LOCAL);
}

void* emplat_system_library_symbol(emplat_library lib, const char* name) {
    return dlsym(lib, name);
}

void emplat_system_library_unload(emplat_library lib) {
	dlclose(lib);
}

void emplat_sleep_ms(f64 ms) {
    struct timespec ts;
    ts.tv_sec  = ms / 1000;

    while (nanosleep(&ts, &ts) == -1)
        continue;
}

em_result emplat_thread_create(emplat_thread* thr, PFN_thread_start func, void* arg) {
    switch (pthread_create(thr, NULL, func, arg)) {
        case 0: return EMBER_RESULT_OK;
        case EAGAIN:  return EMBER_RESULT_OUT_OF_MEMORY_CPU;
        case EINVAL:  return EMBER_RESULT_INVALID_VALUE;
        case EPERM:   return EMBER_RESULT_PERMISSION_DENIED;
    }

    return EMBER_RESULT_UNKNOWN;
}

emplat_thread emplat_thread_current() {
    return pthread_self();
}

b8 emplat_thread_equal(emplat_thread thr0, emplat_thread thr1) {
    return pthread_equal(thr0, thr1);
}

em_result emplat_thread_join(emplat_thread thr, u32* res) {
    void* pres;
    switch (pthread_join(thr, &pres)) {
        case 0: /* ... */ break;
        case ESRCH:   return EMBER_RESULT_INVALID_VALUE;
        case EINVAL:  return EMBER_RESULT_INVALID_VALUE;
        case EDEADLK: return EMBER_RESULT_IN_USE;

        default: 
            return EMBER_RESULT_UNKNOWN;
    }
    
    if (res) *res = (u32)(intptr_t)pres;
    return EMBER_RESULT_OK;
}

em_result emplat_mutex_init(emplat_mutex_type type, emplat_mutex* mtx) {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    if (type & EMBER_MUTEX_TYPE_RECURSIVE)
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    int ret = pthread_mutex_init(mtx, &attr);
    pthread_mutexattr_destroy(&attr);
    return ret == 0 ? EMBER_RESULT_OK : EMBER_RESULT_UNKNOWN; // Man pages say `pthread_mutex_init` will always return 0 anyway.
}

void emplat_mutex_destroy(emplat_mutex* mtx) {
    pthread_mutex_destroy(mtx);
}

em_result emplat_mutex_lock(emplat_mutex* mtx) {
    switch (pthread_mutex_lock(mtx)) {
        case 0: return EMBER_RESULT_OK;
        case EINVAL:  return EMBER_RESULT_UNINITIALIZED;
        case EDEADLK: return EMBER_RESULT_IN_USE;
    }

    return EMBER_RESULT_UNKNOWN;
}

em_result emplat_mutex_timedlock(emplat_mutex* mtx, f64 ms) {
    struct timespec ts;
    ts.tv_sec  = ms / 1000;

    switch (pthread_mutex_timedlock(mtx, &ts)) {
        case 0: return EMBER_RESULT_OK;
        case EINVAL:    return EMBER_RESULT_UNINITIALIZED;
        case EDEADLK:   return EMBER_RESULT_IN_USE;
        case ETIMEDOUT: return EMBER_RESULT_TIMEOUT;
    }

    return EMBER_RESULT_UNKNOWN;
}

em_result emplat_mutex_trylock(emplat_mutex* mtx) {
    switch (pthread_mutex_trylock(mtx)) {
        case 0: return EMBER_RESULT_OK;
        case EINVAL: return EMBER_RESULT_UNINITIALIZED;
        case EBUSY:  return EMBER_RESULT_IN_USE;
    }

    return EMBER_RESULT_UNKNOWN;
}

em_result emplat_mutex_unlock(emplat_mutex* mtx) {
    switch (pthread_mutex_unlock(mtx)) {
        case 0: return EMBER_RESULT_OK;
        case EINVAL: return EMBER_RESULT_UNINITIALIZED;
        case EPERM:  return EMBER_RESULT_IN_USE;
    }

    return EMBER_RESULT_UNKNOWN;
}

em_result emplat_cond_init(emplat_cond* cond) {
    switch (pthread_cond_init(cond, NULL)) {
        case 0: return EMBER_RESULT_OK;
    }

    return EMBER_RESULT_UNKNOWN;
}

void emplat_cond_destroy(emplat_cond* cond) {
    pthread_cond_destroy(cond);
}

em_result emplat_cond_signal(emplat_cond* cond) {
    switch (pthread_cond_signal(cond)) {
        case 0: return EMBER_RESULT_OK;
    }

    return EMBER_RESULT_UNKNOWN;
}

em_result emplat_cond_broadcast(emplat_cond* cond) {
     switch (pthread_cond_broadcast(cond)) {
        case 0: return EMBER_RESULT_OK;
    }

    return EMBER_RESULT_UNKNOWN;
}

em_result emplat_cond_wait(emplat_cond* cond, emplat_mutex* mtx) {
    switch (pthread_cond_wait(cond, NULL)) {
        case 0: return EMBER_RESULT_OK;
    }

    return EMBER_RESULT_UNKNOWN;
}

em_result emplat_cond_timedwait(emplat_cond* cond, emplat_mutex* mtx, f64 ms) {    
    struct timespec ts;
    ts.tv_sec  = ms / 1000;

    switch (pthread_cond_timedwait(cond, mtx, &ts)) {
        case 0: return EMBER_RESULT_OK;
        case ETIMEDOUT: return EMBER_RESULT_TIMEOUT;
    }

    return EMBER_RESULT_UNKNOWN;
}
#endif
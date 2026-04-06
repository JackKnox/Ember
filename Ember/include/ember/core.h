#pragma once

// Unsigned int types.
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// Signed int types.
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

// Floating point types
typedef float f32;
typedef double f64;

// Boolean types
typedef int b32;
typedef _Bool b8;

#define INT8_MAX         127
#define INT16_MAX        32767
#define INT32_MAX        2147483647
#define INT64_MAX        9223372036854775807
#define UINT8_MAX        0xffu
#define UINT16_MAX       0xffffu
#define UINT32_MAX       0xffffffffu
#define UINT64_MAX       0xffffffffffffffffu

// Properly define static assertions.
#if defined(__clang__) || defined(__GNUC__)
#   define STATIC_ASSERT _Static_assert
#else
#   define STATIC_ASSERT static_assert
#endif

#ifndef emc_console_write
#   include <stdio.h>
#   define emc_console_write(format_string, ...) printf((const char*)(format_string) __VA_OPT__(,) __VA_ARGS__)
#endif

#ifndef emc_malloc
#   include <stdlib.h>
#   define emc_malloc(size, aligned) malloc((size_t)(size))
#endif

#ifndef emc_free
#   include <stdlib.h>
#   define emc_free(block, aligned) free((void*)(block))
#endif

#ifndef emc_memcpy
#   include <string.h>
#   define emc_memcpy(dest, source, size) memcpy((void*)(dest), (const void*)(source), (size_t)(size))
#endif

#ifndef emc_memset
#   include <string.h>
#   define emc_memset(dest, value, size) memset((void*)(dest), (int)(value), (size_t)(size))
#endif

#ifndef emc_memcmp
#   include <string.h>
#   define emc_memcmp(buf1, buf2, size) memcmp((const void*)(buf1), (const void*)(buf2), (size_t)(size))
#endif

// Ensure all types are of the correct size.
STATIC_ASSERT(sizeof(u8) == 1, "Expected u8 to be 1 byte.");
STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");

STATIC_ASSERT(sizeof(i8) == 1, "Expected i8 to be 1 byte.");
STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");
STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");
STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");

STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");

#define TRUE 1
#define FALSE 0
#define NULL ((void *)0)

// Always define emdebug_break in case it is ever needed outside assertions (i.e fatal log errors)
// Try via __has_builtin first.
#if defined(__has_builtin) && !defined(__ibmxl__)
#    if __has_builtin(__builtin_debugtrap)
#        define emdebug_break() __builtin_debugtrap()
#    elif __has_builtin(__debugbreak)
#        define emdebug_break() __debugbreak()
#    endif
#endif

// If not setup, try the old way.
#if !defined(emdebug_break)
#    if defined(__clang__) || defined(__GNUC__)
#        define emdebug_break() __builtin_trap()
#    elif defined(_MSC_VER)
#        include <intrin.h>
#        define emdebug_break() __debugbreak()
#    else
// Fall back to x86/x86_64
#        define emdebug_break() asm { int 3 }
#    endif
#endif

// Compiler-specific stuff
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#   define EM_NORETURN _Noreturn
#   elif defined(__GNUC__)
#       define EM_NORETURN __attribute__((__noreturn__))
#   else
#       define EM_NORETURN
#endif

#if !(defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201102L)) && !defined(_Thread_local)
#   if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__SUNPRO_CC) || defined(__IBMCPP__)
#       define _Thread_local __thread
#   else
#       define _Thread_local __declspec(thread)
#   endif
#elif defined(__GNUC__) && defined(__GNUC_MINOR__) && (((__GNUC__ << 8) | __GNUC_MINOR__) < ((4 << 8) | 9))
#   define _Thread_local __thread
#endif

// Platform detection
#if defined(_WIN32)
#define EM_PLATFORM_WINDOWS 1
#if !defined(_WIN64)
#error "64-bit Windows is required!"
#endif
#elif defined(__APPLE__) && defined(__MACH__)
#define EM_PLATFORM_APPLE 1
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
    #define EM_PLATFORM_IOS 1
    #define EM_PLATFORM_IOS_SIMULATOR 1
#elif TARGET_OS_IPHONE
    #define EM_PLATFORM_IOS 1
#elif TARGET_OS_MAC
    #define EM_PLATFORM_MACOS 1
#else
    #error "Unknown Apple platform"
#endif
#elif defined(__ANDROID__)
#define EM_PLATFORM_ANDROID 1
#define EM_PLATFORM_LINUX 1 
#define EM_PLATFORM_POSIX 1
#elif defined(__linux__)
#define EM_PLATFORM_LINUX 1
#define EM_PLATFORM_POSIX 1
#elif defined(__unix__) || defined(__unix)
#define EM_PLATFORM_UNIX 1
#define EM_PLATFORM_POSIX 1
#else
#error "Unknown platform!"
#endif

#if defined(EMBER_BUILD_DEV)
#   define EM_ENABLE_ASSERTS 1
#   define EM_ENABLE_LOGGING 1
#   define EM_ENABLE_DIAGNOSTICS 1
#   define EM_ENABLE_VALIDATION 1
#elif defined(EMBER_BUILD_RELEASE)
#   define EM_ENABLE_ASSERTS 1
#   define EM_ENABLE_LOGGING 1
#   define EM_ENABLE_DIAGNOSTICS 0
#   define EM_ENABLE_VALIDATION 1
#elif defined(EMBER_BUILD_DIST)
#   define EM_ENABLE_ASSERTS 0
#   define EM_ENABLE_LOGGING 0
#   define EM_ENABLE_DIAGNOSTICS 0
#   define EM_ENABLE_VALIDATION 0
#endif

/**
 * @brief Result codes returned by Ember functions.
 * 
 * Indicates the outcome of an operation. Functions typically return
 * `EMBER_RESULT_OK` on success or another value indicating the type
 * of failure or status.
 */
typedef enum em_result {
    EMBER_RESULT_OK = 0,             /**< Operation completed successfully. */
    EMBER_RESULT_TIMEOUT,            /**< Operation timed out before completion. */
    EMBER_RESULT_UNINITIALIZED,      /**< The system, device, or resource was not initialized. */
    EMBER_RESULT_INVALID_ENUM,       /**< An invalid enum value was provided. */
    EMBER_RESULT_INVALID_VALUE,      /**< An invalid value was provided (out of expected range). */
    EMBER_RESULT_UNSUPPORTED_FORMAT, /**< The requested format or type is not supported. */
    EMBER_RESULT_OUT_OF_MEMORY_CPU,  /**< CPU memory allocation failed. */
    EMBER_RESULT_OUT_OF_MEMORY_GPU,  /**< GPU memory allocation failed. */
    EMBER_RESULT_UNAVAILABLE_API,    /**< The requested API is not available on this device. */
    EMBER_RESULT_UNIMPLEMENTED,      /**< The requested feature or function is not implemented. */
    EMBER_RESULT_VALIDATION_FAILED,  /**< Input or operation validation failed. */
    EMBER_RESULT_IN_USE,             /**< The resource is currently in use and cannot be accessed. */
    EMBER_RESULT_UNKNOWN             /**< An unknown error has occurred; either the application has provided invalid input, or an implementation failure has occurred. */
} em_result;

const char* em_result_string(em_result result, b8 get_extended);

#define EMBER_DATA_TYPE_UINT  0
#define EMBER_DATA_TYPE_SINT  1
#define EMBER_DATA_TYPE_FLOAT 2
#define EMBER_DATA_TYPE_BOOL  3

#define EMBER_FORMAT_FLAG_NORMALIZED 1 << 0
#define EMBER_FORMAT_FLAG_BGRA       1 << 1
#define EMBER_FORMAT_FLAG_SRGB       1 << 2

#define EMBER_FORMAT_MAKE(type, bytes, channels, flags) \
    (((flags)              << 24)  | \
     ((type)               << 20)  | \
     (((bytes) / 8)        << 16)  | \
     ((channels)           << 12))

#define EMBER_FORMAT_FLAGS(format)     (((format)  >> 24)  & 0xFF)
#define EMBER_FORMAT_DATA_TYPE(format) (((format)  >> 20)  & 0xF)
#define EMBER_FORMAT_BYTES(format)     ((((format) >> 16)  & 0xF) * 8)
#define EMBER_FORMAT_CHANNELS(format)  (((format)  >> 12)  & 0xF)
#define EMBER_FORMAT_SIZE(format)      (EMBER_FORMAT_BYTES(format) * EMBER_FORMAT_CHANNELS(format))

#define EM_OFFSETOF(s, m) (&(((s*)0)->m))

#define EM_CLAMP(value, min, max) ((value <= min) ? min : (value >= max) ? max : value)

#define EM_MIN(x, y) (x < y ? x : y)
#define EM_MAX(x, y) (x > y ? x : y)

#define EM_ARRAYSIZE(arr) (sizeof(arr) / sizeof(*arr))

inline static u64 alignment(u64 v, u64 align) {
    return (v + (align - 1)) & ~(align - 1);
}

#include "ember/core/logger.h"
#include "ember/core/memory.h"
#include "ember/core/math_types.h"
#include "ember/core/allocators.h"
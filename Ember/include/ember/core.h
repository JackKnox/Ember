#pragma once

#include <stdint.h>

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

#define TRUE 1
#define FALSE 0

#ifndef emwrite_console
#   include <stdio.h>
#   define emwrite_console(format_string, ...) printf((const char*)(format_string) __VA_OPT__(,) __VA_ARGS__)
#endif

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


#if defined(__cplusplus) && __cplusplus >= 201103L // C++11 and later
#   define EM_ALIGNOF(type) alignof(type)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L // C11 and later
#   define EM_ALIGNOF(type) _Alignof(type)
#elif defined(_MSC_VER) // MSVC (works in both C and C++)
#   define EM_ALIGNOF(type) __alignof(type)
#elif defined(__GNUC__) || defined(__clang__) // GCC / Clang (works in both C and C++)
#   define EM_ALIGNOF(type) __alignof__(type)
#else // Pure fallback (standard C89, no extensions)
#   include <stddef.h>
#   define EM_ALIGNOF(type) offsetof(struct { char c; type member; }, member)
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
#   define EM_PLATFORM_WINDOWS 1
#   if !defined(_WIN64)
#       error "64-bit Windows is required!"
#   endif
#elif defined(__APPLE__) && defined(__MACH__)
#   define EM_PLATFORM_APPLE 1
#   include <TargetConditionals.h>
#   if TARGET_IPHONE_SIMULATOR
#       define EM_PLATFORM_IOS 1
#       define EM_PLATFORM_IOS_SIMULATOR 1
#   elif TARGET_OS_IPHONE
#       define EM_PLATFORM_IOS 1
#   elif TARGET_OS_MAC
#       define EM_PLATFORM_MACOS 1
#   else
#       error "Unknown Apple platform"
#endif
#elif defined(__ANDROID__)
#   define EM_PLATFORM_ANDROID 1
#   define EM_PLATFORM_LINUX 1 
#   define EM_PLATFORM_POSIX 1
#elif defined(__linux__)
#   define EM_PLATFORM_LINUX 1
#   define EM_PLATFORM_POSIX 1
#elif defined(__unix__) || defined(__unix)
#   define EM_PLATFORM_UNIX 1
#   define EM_PLATFORM_POSIX 1
#else
#   error "Unknown platform!"
#endif

#define EM_ARRAYSIZE(arr) (sizeof(arr) / sizeof(*arr))
#define EM_OFFSETOF(s, m) (&(((s*)0)->m))

typedef u32 em_version;

#define EMBER_MAKE_VERSION(major, minor, patch) \
    ((((u32)(major)) << 22U) | (((u32)(minor)) << 12U) | ((u32)(patch)))

#define EMBER_VERSION_MAJOR(version) ((u32)(version) >> 22U)
#define EMBER_VERSION_MINOR(version) (((u32)(version) >> 12U) & 0x3FFU)
#define EMBER_VERSION_PATCH(version) ((u32)(version) & 0xFFFU)

#include "ember/core/logger.h"
#include "ember/core/memory.h"
#include "ember/core/math_types.h"
#include "ember/core/format.h"
#include "ember/core/result.h"

#include "ember/version.h"
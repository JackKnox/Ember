#pragma once

#include <stdint.h>

// Unsigned int types.
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long int u64;

// Signed int types.
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long int i64;

// Floating point types
typedef float f32;
typedef double f64;

// Boolean types
typedef int b32;
typedef _Bool b8;

#define EMTRUE 1
#define EMFALSE 0
#define EMNULL ((void*)0)

#if defined(__clang__) || defined(__GNUC__)
#	define STATIC_ASSERT _Static_assert
#else
#	define STATIC_ASSERT static_assert
#endif


STATIC_ASSERT(sizeof(u8) == 1,  "Expected u8 to be 1 byte.");
STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");
STATIC_ASSERT(sizeof(i8) == 1,  "Expected i8 to be 1 byte.");
STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");
STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");
STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");
STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");

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

#if defined(__cplusplus) && __cplusplus >= 201703L
#   define EM_NODISCARD [[nodiscard]]
#elif defined(__GNUC__) || defined(__clang__)
#   define EM_NODISCARD __attribute__((warn_unused_result))
#else
#   define EM_NODISCARD
#endif

#if defined(_MSC_VER)
#   define EM_FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#   define EM_FORCE_INLINE inline __attribute__((always_inline))
#else
#   define EM_FORCE_INLINE inline
#endif

#if defined(_MSC_VER)
#   define EM_NO_INLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#   define EM_NO_INLINE __attribute__((noinline))
#else
#   define EM_NO_INLINE
#endif

#if defined(__cplusplus) && __cplusplus >= 201103L 
// C++11 and later
#   define EM_ALIGNOF(type) alignof(type)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L 
// C11 and later
#   define EM_ALIGNOF(type) _Alignof(type)
#elif defined(_MSC_VER) 
// MSVC (works in both C and C++)
#   define EM_ALIGNOF(type) __alignof(type)
#elif defined(__GNUC__) || defined(__clang__) 
// GCC / Clang (works in both C and C++)
#   define EM_ALIGNOF(type) __alignof__(type)
#else 
// Pure fallback (standard C89, no extensions)
#   include <stddef.h>
#   define EM_ALIGNOF(type) offsetof(struct { char c; type member; }, member)
#endif

#if defined(__GNUC__) || defined(__clang__)
#   define EM_LIKELY(x)   __builtin_expect(!!(x), 1)
#   define EM_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#   define EM_LIKELY(x)   (x)
#   define EM_UNLIKELY(x) (x)
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

#if defined(_MSC_VER)
#   define EM_ASSUME(x) __assume(x)
#elif defined(__clang__)
#   define EM_ASSUME(x) __builtin_assume(x)
#elif defined(__GNUC__)
#   define EM_ASSUME(x) do { if (!(x)) __builtin_unreachable(); } while (0)
#else
#   define EM_ASSUME(x)
#endif

#if defined(_MSC_VER)
#   define EM_UNREACHABLE() __assume(0)
#elif defined(__GNUC__) || defined(__clang__)
#   define EM_UNREACHABLE() __builtin_unreachable()
#else
#   define EM_UNREACHABLE() ((void)0)
#endif

#if defined(__cplusplus)
#   if defined(_MSC_VER)
#       define EM_RESTRICT __restrict
#   else
#       define EM_RESTRICT __restrict__
#   endif
#else
#   define EM_RESTRICT restrict
#endif

#define EM_ARRAYSIZE(arr) (sizeof(arr) / sizeof(*arr))
#define EM_OFFSETOF(s, m) ((u64)(&(((s*)0)->m)))

#include "ember/core/logger.h"
#include "ember/core/memory.h"
#include "ember/core/math_types.h"
#include "ember/core/format.h"
#include "ember/core/result.h"
#include "ember/core/version_types.h"

#include "ember/version.h"
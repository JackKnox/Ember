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

#if defined(__clang__) || defined(__GNUC__)
#	define EM_STATIC_ASSERT _Static_assert
#else
#	define EM_STATIC_ASSERT static_assert
#endif


EM_STATIC_ASSERT(sizeof(u8) == 1,  "Expected u8 to be 1 byte.");
EM_STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
EM_STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
EM_STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");
EM_STATIC_ASSERT(sizeof(i8) == 1,  "Expected i8 to be 1 byte.");
EM_STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");
EM_STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");
EM_STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");
EM_STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
EM_STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");

#define EMTRUE 1
#define EMFALSE 0
#define EMNULL ((void*)0)

#define EM_ARRAYSIZE(arr) (sizeof(arr) / sizeof(*arr))
#define EM_OFFSETOF(s, m) ((u64)(&(((s*)0)->m)))

#include "ember/core/logger.h"
#include "ember/core/memory.h"
#include "ember/core/math_types.h"
#include "ember/core/format.h"
#include "ember/core/result.h"
#include "ember/core/version_types.h"

#include "ember/version.h"
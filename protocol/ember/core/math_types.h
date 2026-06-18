#pragma once

#include "ember/core.h"

// A 2-element vector.
typedef union vec2 {
    f32 x, y;
} vec2;

// A 3-element vector.
typedef union vec3 {
    f32 x, y, z;
} vec3;

// A 4-element vector.
typedef union vec4 {
    float x, y, z, w;
} vec4;

// A unsigned integer 2-element vector.
typedef union uvec2 {
    u32 x, y;
} uvec2;

// A unsigned interger 3-element vector.
typedef union uvec3 {
    u32 x, y, z;
} uvec3;
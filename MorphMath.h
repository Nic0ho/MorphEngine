#pragma once

#include "MorphTypes.h"

//column-major 4x4 matrix
typedef struct
{
    f32 m[4][4];
} Mat4;

static inline Mat4 mat4Identity(void)
{
    Mat4 m = {0};
    m.m[0][0] = 1.0f;
    m.m[1][1] = 1.0f;
    m.m[2][2] = 1.0f;
    m.m[3][3] = 1.0f;

    return m;
}

static inline Mat4 mat4Translate(f32 x, f32 y)
{
    Mat4 m = mat4Identity();
    m.m[3][0] = x;
    m.m[3][1] = y;

    return m;
}

static inline Mat4 mat4Scale(f32 x, f32 y)
{
    Mat4 m = mat4Identity();
    m.m[0][0] = x;
    m.m[1][1] = y;

    return m;
}
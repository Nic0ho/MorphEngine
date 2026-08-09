#pragma once

#include "MorphTypes.h"
#include "math.h"

//VECTORS
typedef struct { f32 x, y; } Vec2;
typedef struct { f32 x, y, z; } Vec3;
typedef struct { f32 x, y, z, w; } Vec4;

static inline Vec2 vec2(f32 x, f32 y)
{ return (Vec2){x, y}; }

static inline Vec3 vec3(f32 x, f32 y, f32 z)
{ return (Vec3){x, y, z}; }

static inline Vec4 vec4(f32 x, f32 y, f32 z, f32 w)
{ return (Vec4){x, y, z, w}; }

//VEC2 OPERATIONS ------------
//add
static inline Vec2 vec2Add(Vec2 a, Vec2 b)
{ return (Vec2){a.x + b.x, a.y + b.y}; }

//substact
static inline Vec2 vec2Sub(Vec2 a, Vec2 b)
{ return (Vec2){a.x - b.x, a.y - b.y}; }

//multiply
static inline Vec2 vec2Scale(Vec2 v, f32 s)
{ return (Vec2){v.x * s, v.y * s}; }

//VEC3 OPERATIONS ------------
//add
static inline Vec3 vec3Add(Vec3 a, Vec3 b)
{ return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z}; }

//substract
static inline Vec3 vec3Sub(Vec3 a, Vec3 b)
{ return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z}; }

//multily
static inline Vec3 vec3Scale(Vec3 v, f32 s)
{ return (Vec3){v.x * s, v.y * s, v.z * s}; }

//negate
static inline Vec3 vec3Neg(Vec3 v)
{ return (Vec3){-v.x, -v.y, -v.z}; }

//dot
static inline f32 vec3Dot(Vec3 a, Vec3 b)
{ return a.x * b.x + a.y * b.y + a.z * b.z; }

//cross
static inline Vec3 vec3Cross(Vec3 a, Vec3 b)
{
    return (Vec3)
    {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

//length
static inline f32 vec3Len(Vec3 v)
{ return sqrt(vec3Dot(v, v)); }

//normalize
static inline Vec3 vec3Normalize(Vec3 v)
{
    f32 len = vec3Len(v);

    if (len < 0.0001f) return (Vec3) {0, 0, 0};
    return vec3Scale(v, 1.0f/len);
}

//column-major 4x4 matrix
typedef struct
{ f32 m[4][4]; } Mat4;

//identity
static inline Mat4 mat4Identity(void)
{
    Mat4 m = {0};
    m.m[0][0] = 1.0f;
    m.m[1][1] = 1.0f;
    m.m[2][2] = 1.0f;
    m.m[3][3] = 1.0f;

    return m;
}

//muiltiply
static inline Mat4 mat4Mul(Mat4 a, Mat4 b)
{
    Mat4 result = {0};
    
    for (int col = 0; col < 4; col++)
        for (int row = 0; row < 4; row++)
            for (int k = 0; k < 4; k++)
                result.m[col][row] += a.m[k][row] * b.m[col][k];
    
    return result;
}

//translate
static inline Mat4 mat4Translate(f32 x, f32 y, f32 z)
{
    Mat4 m = mat4Identity();
    m.m[3][0] = x;
    m.m[3][1] = y;
    m.m[3][2] = z;

    return m;
}

//scale
static inline Mat4 mat4Scale(f32 x, f32 y, f32 z)
{
    Mat4 m = mat4Identity();
    m.m[0][0] = x;
    m.m[1][1] = y;
    m.m[2][2] = z;

    return m;
}

//rotate (angle in radians)
static inline Mat4 mat4Rotate(Vec3 axis, f32 angle)
{
    axis = vec3Normalize(axis);
    f32 c = cosf(angle);
    f32 s= sinf(angle);
    f32 t = 1.0f - c;
    f32 x = axis.x, y = axis.y, z = axis.z;

    Mat4 m = {0};
    m.m[0][0] = t * x * x + c;
    m.m[0][1] = t * x * y + s * z;
    m.m[0][2] = t * x * z - s * y;
    
    m.m[1][0] = t * x * y - s * z;
    m.m[1][1]  = t * y * y + c;
    m.m[1][2] = t * y * z + s * x;

    m.m[2][0] = t * x * z + s * y;
    m.m[2][1] = t * y * z - s * x;
    m.m[2][2] = t * z * z + c;

    m.m[3][3] = 1.0f;

    return m;
}

// perspective projection (fovY in radians)
static inline Mat4 mat4Perspective(f32 fovY, f32 aspect, f32 nearZ, f32 farZ)
{
    f32 tanHalfFov = tanf(fovY * 0.5);

    Mat4  m = {0};
    m.m[0][0] = 1.0f / (aspect * tanHalfFov);
    m.m[1][1] = -1.0f / tanHalfFov; //flip for Vulkan
    m.m[2][2] = farZ / (nearZ - farZ);
    m.m[2][3] = -1.0f;
    m.m[3][2]  = (nearZ * farZ) / (nearZ - farZ);

    return m;
}

//orthographic projection
static inline Mat4 mat4Ortho(f32 left, f32 right, f32 bottom, f32 top, f32 nearZ, f32 farZ)
{
    Mat4 m = {0};
    m.m[0][0] = 2.0f / (right - left);
    m.m[1][1] = 2.0f / (top - bottom); //without flip for Vulkan
    m.m[2][2] = 1.0f / (nearZ - farZ);
    m.m[3][0] = -(right + left) / (right - left);
    m.m[3][1] = (top + bottom) / (top - bottom);
    m.m[3][2] = nearZ / (nearZ - farZ);
    m.m[3][3] = 1.0f;

    return m;
}

//view matrix - positions camera in world
static inline Mat4 mat4LookAt(Vec3 eye, Vec3 center, Vec3 up)
{
    Vec3 f = vec3Normalize(vec3Sub(center, eye)); //forward
    Vec3 r = vec3Normalize(vec3Cross(f, up)); //right
    Vec3 u = vec3Cross(r, f); //up (recomputed)

    Mat4 m = {0};
    m.m[0][0] = r.x;
    m.m[1][0] = r.y;
    m.m[2][0] = r.z;

    m.m[0][1] = u.x;
    m.m[1][1] = u.y;
    m.m[2][1] = u.z;

    m.m[0][2] = -f.x;
    m.m[1][2] = -f.y;
    m.m[2][2] = -f.z;

    m.m[3][0] = -vec3Dot(r, eye);
    m.m[3][1] = -vec3Dot(u, eye);
    m.m[3][2] = vec3Dot(f, eye);
    m.m[3][3] = 1.0f;

    return m;
}
#pragma once

#include <stdint.h>
#include <stdbool.h>

//--
//--CUSTOM VARS
//--

//signed int
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

//unsigned int
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

//float
typedef float f32;
typedef double f64;

//pointers
typedef uintptr_t uptr;
typedef intptr_t iptr;
typedef size_t usize;

//--
//--MACRO
//--

//sizing
#define KB(x) ((usize)(x) * 1024)
#define MB(x) ((usize)(x) * 1024 * 1024)
#define GB(x) ((usize)(x) * 1024 * 1024 * 1024)

//LEN
#define LEN(arr) (sizeof(arr) / sizeof((arr)[0]))
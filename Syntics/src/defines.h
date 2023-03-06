#pragma once
#include <stdint.h>

#define for_range(i, n) for (u32 i = 0; i < n; i++)
#define b_switch(val) val = val ? false : true

#define KILOBYTE(n) n * 1024
#define MEGABYTE(n) KILOBYTE(n) * 1024
#define GIGABYTE(n) MEGABYTE(n) * 1024

#define INIT_ARR0(type, name, size)                                                 \
    type name[size];                                                                \
    memset(name, 0, sizeof(name));

#define INIT_0(type, obj)                                                           \
    type obj;                                                                       \
    memset(&obj, 0, sizeof(obj));

#define SET_0(obj) memset(&obj, 0, sizeof(obj));

#define ARR_0(obj) memset(obj, 0, sizeof(obj));

#define sy_SIZE(array) sizeof(array) / sizeof(array[0])

#define sy(...) __VA_ARGS__

#define true 1
#define false 0

typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

typedef int64_t b64;
typedef int32_t b32;
typedef int16_t b16;
typedef int8_t b8;

typedef double f64;
typedef float f32;

#pragma once
#include <stdint.h>

#define for_range(i, n) for (u32 i = 0; i < n; i++)
#define b_switch(val) (val) = (val) ? false : true
#define closed_interval(low, val, high) ((val) >= (low) && (val) <= (high))
#define open_interval(low, val, high) ((val) > (low) && (val) < (high))

#define KILOBYTE(n) n * 1024
#define MEGABYTE(n) KILOBYTE(n) * 1024
#define GIGABYTE(n) MEGABYTE(n) * 1024

#define sy_SIZE(array) sizeof(array) / sizeof(array[0])

#define sy(...) __VA_ARGS__

#define true 1
#define false 0

// Syntics related
typedef struct Queue_Family_Indices Queue_Family_Indices;
typedef struct Queues Queues;
typedef struct Buffer Buffer;
typedef struct Vertex_Buffer Vertex_Buffer;
typedef struct Index_Buffer Index_Buffer;
typedef struct Uniform_Buffer Uniform_Buffer;
typedef struct Image Image;
typedef struct Texture Texture;
typedef struct Descriptors Descriptors;
typedef struct Graphic_Pipline Graphic_Pipline;
typedef struct Swap_Chain_attrib Swap_Chain_attrib;
typedef struct Application_State Application_State;

typedef struct Region_Alloc Region_Alloc;
typedef struct Camera Camera;
typedef struct Events Events;
typedef struct Vec4 V4;
typedef struct Vec3 V3;
typedef struct Vec2 V2;
typedef struct Point4 P4;
typedef struct Point3 P3;
typedef struct Point2 P2;
typedef struct Rect3D Rect3D;
typedef struct Rect2D Rect2D;
typedef struct Quad2D Quad2D;
typedef struct Polygon2D Polygon2D;
typedef struct Vertex Vertex;
typedef struct Dynamic_Entity_2D Dynamic_Entity_2D;

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

Region_Alloc* get_stack();
void reset_stack();

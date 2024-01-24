#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#endif

#define V2_FMT(v) "(x: %f, y: %f)\n", (v).x, (v).y
#define V3_FMT(v) "(x: %f, y: %f, z: %f)\n", (v).x, (v).y, (v).z
#define V4_FMT(v) "(x: %f, y: %f, z: %f, w: %f)\n", (v).x, (v).y, (v).z, (v).w

#define M3_FMT(m)                                                              \
    "|%f,%f,%f|\n|%f,%f,%f|\n|%f,%f,%f|\n\n", (m).data[0][0], (m).data[1][0],  \
        (m).data[2][0], (m).data[0][1], (m).data[1][1], (m).data[2][1],        \
        (m).data[0][2], (m).data[1][2], (m).data[2][2]

#define M4_FMT(m)                                                              \
    "|%f,%f,%f,%f|\n|%f,%f,%f,%f|\n|%f,%f,%f,%f|\n|%f,%f,%f,%f|\n\n",          \
        (m).data[0][0], (m).data[1][0], (m).data[2][0], (m).data[3][0],        \
        (m).data[0][1], (m).data[1][1], (m).data[2][1], (m).data[3][1],        \
        (m).data[0][2], (m).data[1][2], (m).data[2][2], (m).data[3][2],        \
        (m).data[0][3], (m).data[1][3], (m).data[2][3], (m).data[3][3]

typedef struct V2 V2;
struct V2
{
    union
    {
        struct
        {
            f32 x;
            f32 y;
        };

        struct
        {
            f32 width;
            f32 height;
        };

        struct
        {
            f32 min;
            f32 max;
        };

        struct
        {
            f32 u;
            f32 v;
        };

        struct
        {
            f32 l;
            f32 r;
        };
    };
};

typedef struct V3 V3;
struct V3
{
    union
    {
        struct
        {
            f32 x;
            f32 y;
            f32 z;
        };
        struct
        {
            f32 r;
            f32 g;
            f32 b;
        };
        struct
        {
            f32 data[3];
        };
    };
};

typedef struct V4 V4;
struct V4
{
    union
    {
        struct
        {
            f32 x;
            f32 y;
            f32 z;
            f32 w;
        };

        struct
        {
            f32 r;
            f32 g;
            f32 b;
            f32 a;
        };
    };
};

typedef struct P2
{
    f32 x;
    f32 y;
} P2;

typedef struct P3
{
    f32 x;
    f32 y;
    f32 z;
} P3;

typedef struct P4
{
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} P4;

typedef struct Mat2f
{
    f32 data[2][2];
} Mat2f, M2;

typedef struct Mat3f
{
    f32 data[3][3];
} Mat3f, M3;

typedef struct Mat4f
{
    f32 data[4][4];
} Mat4f, M4;

typedef struct Vertex
{
    V3 pos;
    V3 normal;
    V2 tex_coords;
    V4 color;
    f32 tex_index;
} Vertex;

#define v2_array_create(region, array, array_capacity)                         \
    array_create(region, array, array_capacity, V2)
#define v3_array_create(region, array, array_capacity)                         \
    array_create(region, array, array_capacity, V3)
#define vertex_array_create(region, array, array_capacity)                     \
    array_create(region, array, array_capacity, Vertex)
#define u32_array_create(region, array, array_capacity)                        \
    array_create(region, array, array_capacity, u32)

#define array_create(region, array, array_capacity, data_type)                 \
    do                                                                         \
    {                                                                          \
        (array)->size = 0;                                                     \
        (array)->capacity = (array_capacity);                                  \
        (array)->data =                                                        \
            (region) ? region_calloc(region, array_capacity, data_type)        \
                     : (data_type*)calloc(array_capacity, sizeof(data_type));  \
    } while (0)

#define array_push(array, value)                                               \
    do                                                                         \
    {                                                                          \
        assert((array)->size < (array)->capacity && "array_push");             \
        (array)->data[(array)->size++] = (value);                              \
    } while (0)

#define array_value_ptr(array, index)                                          \
    ((array)->data + array_index_out_of_bounds_check(index, (array)->capacity))

#define array_value(array, index) (*array_value_ptr(array, index))

#define array_pop(array) (array)->data[(array)->size ? --(array)->size : 0]

int array_index_out_of_bounds_check(u32 index, u32 capacity);


typedef struct V2_Array
{
    u32 size;
    u32 capacity;
    V2* data;
} V2_Array;

typedef struct V3_Array
{
    u32 size;
    u32 capacity;
    V3* data;
} V3_Array;

typedef struct Vertex_Array
{
    u32 size;
    u32 capacity;
    Vertex* data;
} Vertex_Array;

typedef struct U32_Array
{
    u32 size;
    u32 capacity;
    u32* data;
} U32_Array;

typedef struct VP
{
    M4 view;
    M4 proj;
} VP;

typedef struct Push_Constant
{
    M4 model;
    M4 normal;
} Push_Constant;

typedef struct Polygon2D
{
    V2 pos;
    V2 vel;
    V2* points;
    V2* normals;
    u32 n_sides;
    u32 id;
} Polygon2D;

typedef struct Quad2D
{
    V2 pos;
    V2 vel;
    V2 points[4];
    V2 normals[4];
    u32 id;
} Quad2D;

typedef struct Quad3D
{
    V2 pos;
    V2 vel;
    P2 points[4];
    V2 normals[8];
    u32 id;
} Quad3D;

typedef struct Rect2D
{
    V2 pos;
    V2 size;
    V4 color;
    V2 vel;
    u32 id;
} Rect2D;

typedef struct Rect3D
{
    V3 pos;
    V3 size;
    u32 id;
    f32 misc; // Filling out padding
} Rect3D;

typedef struct Plane
{
    V3 n;
    float d;
} Plane;

typedef enum Axis
{
    X,
    Y,
    Z,
} Axis;

Vertex_Array vertex_array_ref_at_size_offset(Vertex_Array* array,
                                             u32 ref_capacity);
U32_Array u32_array_ref_at_size_offset(U32_Array* array, u32 ref_capacity);
u32* u32_array_back(U32_Array* array);

V2 v2d(void);
V2 v2i(f32 i);
V2 v2f(f32 x, f32 y);
V2 v2_v3(V3 v3);
V2 v2_v4(V4 v4);
V3 v3d(void);
V3 v3i(f32 i);
V3 v3f(f32 x, f32 y, f32 z);
V2 v2_random(u32 seed, f32 min, f32 max);
V3 v3_random(u32 seed, f32 min, f32 max);
V3 v3_v2(V2 v2);
V3 v3_v2f(V2 v2, f32 z);
V3 v3_v4(V4 v4);
V4 v4d(void);
V4 v4i(f32 i);
V4 v4ic(f32 i);
V4 v4f(f32 x, f32 y, f32 z, f32 w);
V4 v4_v2(V2 v2);
V4 v4_v2f(V2 v2, f32 z, f32 w);
V4 v4_v3(V3 v3);
V4 v4_v3f(V3 v3, f32 w);
f32 v2_sum(V2 v);
f32 v3_sum(V3 v);
f32 v4_sum(V4 v);
V2 v2_add(V2 v1, V2 v2);
V3 v3_add(V3 v1, V3 v2);
V4 v4_add(V4 v1, V4 v2);
V2 v2_sub(V2 v1, V2 v2);
V3 v3_sub(V3 v1, V3 v2);
V4 v4_sub(V4 v1, V4 v2);
V2 v2_s_add(V2 v1, f32 s);
V3 v3_s_add(V3 v1, f32 s);
V4 v4_s_add(V4 v1, f32 s);
V2 v2_s_sub(V2 v1, f32 s);
V3 v3_s_sub(V3 v1, f32 s);
V4 v4_s_sub(V4 v1, f32 s);
V2 v2_s_multi(V2 v1, f32 s);
V3 v3_s_multi(V3 v1, f32 s);
V4 v4_s_multi(V4 v1, f32 s);
V2 v2_neg(V2 v);
V3 v3_neg(V3 v);
V4 v4_neg(V4 v);
V2 v2_multi(V2 v1, V2 v2);
V3 v3_multi(V3 v1, V3 v2);
V4 v4_multi(V4 v1, V4 v2);
V2 v2_s_div(V2 v1, f32 s);
V3 v3_s_div(V3 v1, f32 s);
V4 v4_s_div(V4 v1, f32 s);
void v2_add_equal(V2* v1, V2 v2);
void v3_add_equal(V3* v1, V3 v2);
void v4_add_equal(V4* v1, V4 v2);
void v2_sub_equal(V2* v1, V2 v2);
void v3_sub_equal(V3* v1, V3 v2);
void v4_sub_equal(V4* v1, V4 v2);
void v2_s_add_equal(V2* v1, f32 s);
void v3_s_add_equal(V3* v1, f32 s);
void v4_s_add_equal(V4* v1, f32 s);
void v2_s_sub_equal(V2* v1, f32 s);
void v3_s_sub_equal(V3* v1, f32 s);
void v4_s_sub_equal(V4* v1, f32 s);
void v2_s_multi_equal(V2* v1, f32 s);
void v3_s_multi_equal(V3* v1, f32 s);
void v4_s_multi_equal(V4* v1, f32 s);
void v2_s_div_equal(V2* v1, f32 s);
void v3_s_div_equal(V3* v1, f32 s);
void v4_s_div_equal(V4* v1, f32 s);
b8 v2_equal(V2 v1, V2 v2);
b8 v3_equal(V3 v1, V3 v2);
b8 v4_equal(V4 v1, V4 v2);
b8 v2_less(V2 v1, V2 v2);
b8 v3_less(V3 v1, V3 v2);
b8 v4_less(V4 v1, V4 v2);
b8 v2_more(V2 v1, V2 v2);
b8 v3_more(V3 v1, V3 v2);
b8 v4_more(V4 v1, V4 v2);
P2 p2d(void);
P2 p2i(f32 i);
P2 p2f(f32 x, f32 y);
P2 p2_p3(P3 p3);
P2 p2_p4(P4 p4);
P3 p3d(void);
P3 p3i(f32 i);
P3 p3f(f32 x, f32 y, f32 z);
P3 p3_p2(P2 p2);
P3 v3_p2f(P2 p2, f32 z);
P3 p3_p4(P4 p4);
P4 p4d(void);
P4 p4i(f32 i);
P4 p4f(f32 x, f32 y, f32 z, f32 w);
P4 p4_p2(P2 p2);
P4 p4_p2f(P2 p2, f32 z, f32 w);
P4 p4_p3(P3 p3);
f32 p2_sum(P2 p);
f32 p3_sum(P3 p);
f32 p4_sum(P4 p);
P2 p2_add(P2 p1, P2 p2);
P3 p3_add(P3 p1, P3 p2);
P4 p4_add(P4 p1, P4 p2);
V2 p2_sub(P2 p1, P2 p2);
V3 p3_sub(P3 p1, P3 p2);
V4 p4_sub(P4 p1, P4 p2);
P2 p2_s_multi(P2 p1, f32 s);
P3 p3_s_multi(P3 p1, f32 s);
P4 p4_s_multi(P4 p1, f32 s);
M2 m2i(f32 i);
M2 m2d(void);
M3 m3i(f32 i);
M3 m3d(void);
M3 m3f(f32 f0, f32 f1, f32 f2, f32 f3, f32 f4, f32 f5, f32 f6, f32 f7, f32 f8);
M3 m3_m4(M4 matrix);
M4 m4i(f32 i);
M4 m4d(void);
M4 m4f(f32 f0, f32 f1, f32 f2, f32 f3, f32 f4, f32 f5, f32 f6, f32 f7, f32 f8,
       f32 f9, f32 f10, f32 f11, f32 f12, f32 f13, f32 f14, f32 f15);
M4 m4_v4(V4 c0, V4 c1, V4 c2, V4 c3);
f32 m2_sum(M2 m);
f32 m3_sum(M3 m);
f32 m4_sum(M4 m);
M2 m2_add(M2 m1, M2 m2);
M3 m3_add(M3 m1, M3 m2);
M4 m4_add(M4 m1, M4 m2);
M2 m2_sub(M2 m1, M2 m2);
M3 m3_sub(M3 m1, M3 m2);
M4 m4_sub(M4 m1, M4 m2);
M2 m2_s_multi(M2 m, f32 s);
M3 m3_s_multi(M3 m, f32 s);
M4 m4_s_multi(M4 m, f32 s);
V2 m2_v2_multi(M2 m, V2 v);
V3 m3_v3_multi(M3 m, V3 v);
V3 m4_v3_multi(M4 m, V3 v);
V4 m4_v4_multi(M4 m, V4 v);
M2 m2_multi(M2 m1, M2 m2);
M3 m3_multi(M3 m1, M3 m2);
M4 m4_multi(M4 m1, M4 m2);
M4 m4_s_div(M4 m, f32 s);
b8 m2_equal(M2 m1, M2 m2);
b8 m3_equal(M3 m1, M3 m2);
b8 m4_equal(M4 m1, M4 m2);
b8 m2_less(M2 m1, M2 m2);
b8 m3_less(M3 m1, M3 m2);
b8 m4_less(M4 m1, M4 m2);
b8 m2_more(M2 m1, M2 m2);
b8 m3_more(M3 m1, M3 m2);
b8 m4_more(M4 m1, M4 m2);
b8 vertex_equal(const Vertex* f, const Vertex* s);
b8 vp_equal(const VP* f, const VP* s);
Vertex vertex_create(V3 pos, V3 normal, V2 tex_coords, V4 color, f32 tex_index);
Polygon2D poly2D(V2 pos, V2* p_arr, V2* n_arr, u32 n_sides);
f32 abs_f32(f32 in);
f32 clampf32(f32 value, f32 min, f32 max);
f32 clampf32_low(f32 value, f32 min);
V4 clamp(V4 v1, V4 min, V4 max);
f32 minf32(f32 f1, f32 f2);
f32 maxf32(f32 f1, f32 f2);
f32 v2_len(V2 v2);
f32 v2_len_squared(V2 v2);
f32 v3_len_squared(V3 v3);
f32 v3_len(V3 v3);
V3 v3_lerp(V3 v1, V3 v2, f32 t);
f32 v2_dot(V2 v1, V2 v2);
f32 v3_dot(V3 v1, V3 v2);
f32 v3_angle(V3 v1, V3 v2);
V2 v2_normalize(V2 v2);
V3 v3_normalize(V3 v3);
f32 v2_cross(V2 v1, V2 v2);
V3 v3_cross(V3 v1, V3 v2);
V3 v3_project(V3 v1, V3 v2);
V3 v3_reject(V3 v1, V3 v2);
f32 v2_distance(V2 v1, V2 v2);
f32 v3_distance_squared(V3 v1, V3 v2);
f32 v3_distance(V3 v1, V3 v2);
f32 p3_distance(P3 p1, P3 p2);
f32 p3_distance_sqrt(P3 p1, P3 p2);
P3 p3_lerp(P3 p1, P3 p2, f32 t);
P3 p3_min(P3 p1, P3 p2);
P3 p3_max(P3 p1, P3 p2);
P3 p3_abs(P3 p);
f32 radians(f32 deg);
f32 m3_determinant(M3 m3);
f32 m4_determinant(M4 m4);
M3 m3_transpose(M3 m3);
M4 m4_transpose(M4 m4);
M3 m3_rotate(M3 m3, f32 rad);
M4 rotate_x(f32 rad);
M4 rotate_y(f32 rad);
M4 rotate_z(f32 rad);
V3 v3_rotate(V3 v3, f32 rad, V3 normal);
M3 translate(V2 v);
M4 m4_translate(V3 v3);
M3 scale(V2 v);
M4 m4_scale(V3 v);
M4 m4_shear(V3 v, V2 hx, V2 hy, V2 hz);
M4 ortho(f32 left, f32 right, f32 bottom, f32 top, f32 sy_near, f32 sy_far);
M4 view(V3 eye, V3 center, V3 up);
M4 perspective(f32 fov, f32 aspect, f32 sy_near, f32 sy_far);
M4 inverse(M4 m);
b8 is_poly2d_convex(Polygon2D p);
Plane plane(P3 a, P3 b, P3 c);

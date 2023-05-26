#pragma once
#include "vectors.h"

#define M3_FMT(m)                                                                   \
    "|%f,%f,%f|\n|%f,%f,%f|\n|%f,%f,%f|\n\n", (m).data[0][0], (m).data[1][0],       \
        (m).data[2][0], (m).data[0][1], (m).data[1][1], (m).data[2][1],             \
        (m).data[0][2], (m).data[1][2], (m).data[2][2]

#define M4_FMT(m)                                                                   \
    "|%f,%f,%f,%f|\n|%f,%f,%f,%f|\n|%f,%f,%f,%f|\n|%f,%f,%f,%f|\n\n",               \
        (m).data[0][0], (m).data[1][0], (m).data[2][0], (m).data[3][0],             \
        (m).data[0][1], (m).data[1][1], (m).data[2][1], (m).data[3][1],             \
        (m).data[0][2], (m).data[1][2], (m).data[2][2], (m).data[3][2],             \
        (m).data[0][3], (m).data[1][3], (m).data[2][3], (m).data[3][3]

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

M2 m2d(void);
M2 m2i(f32 i);

M3 m3d(void);
M3 m3i(f32 i);
M3 m3f(f32 f0, f32 f1, f32 f2, f32 f3, f32 f4, f32 f5, f32 f6, f32 f7, f32 f8);
M3 m3_m4(M4 matrix);

M4 m4d(void);
M4 m4i(f32 i);
M4 m4f(f32 f0, f32 f1, f32 f2, f32 f3, f32 f4, f32 f5, f32 f6, f32 f7, f32 f8,
       f32 f9, f32 f10, f32 f11, f32 f12, f32 f13, f32 f14, f32 f15);

M4 m4_v4(V4 c0,V4 c1,V4 c2,V4 c3);

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

b8 m2_equal(M2 m1, M2 m2);
b8 m3_equal(M3 m1, M3 m2);
b8 m4_equal(M4 m1, M4 m2);

b8 m2_less(M2 m1, M2 m2);
b8 m3_less(M3 m1, M3 m2);
b8 m4_less(M4 m1, M4 m2);

b8 m2_more(M2 m1, M2 m2);
b8 m3_more(M3 m1, M3 m2);
b8 m4_more(M4 m1, M4 m2);

#if 1
M2 operator+(const M2& m1, const M2& m2);
M3 operator+(const M3& m1, const M3& m2);
M4 operator+(const M4& m1, const M4& m2);

M2 operator-(const M2& m1, const M2& m2);
M3 operator-(const M3& m1, const M3& m2);
M4 operator-(const M4& m1, const M4& m2);

M2 operator*(const M2& m, f32 s);
M3 operator*(const M3& m, f32 s);
M4 operator*(const M4& m, f32 s);

M2 operator/(const M2& m, f32 s);
M3 operator/(const M3& m, f32 s);
M4 operator/(const M4& m, f32 s);

V2 operator*(const M2& m, const V2& v);
V3 operator*(const M3& m, const V3& v);
V3 operator*(const M4& m, const V3& v);
V4 operator*(const M4& m, const V4& v);

M2 operator*(const M2& m1, const M2& m2);
M3 operator*(const M3& m1, const M3& m2);
M4 operator*(const M4& m1, const M4& m2);

b8 operator==(const M2& m1, const M2& m2);
b8 operator==(const M3& m1, const M3& m2);
b8 operator==(const M4& m1, const M4& m2);

b8 operator<(const M2& m1, const M2& m2);
b8 operator<(const M3& m1, const M3& m2);
b8 operator<(const M4& m1, const M4& m2);

b8 operator>(const M2& m1, const M2& m2);
b8 operator>(const M3& m1, const M3& m2);
b8 operator>(const M4& m1, const M4& m2);
#endif

typedef struct Quad
{
    V3 corners[4];
} Quad;

typedef struct Vertex
{
    V3 pos;
    V3 normal;
    V2 tex_coords;
    V4 color;
    f32 tex_index;
} Vertex;

Vertex vertex_create(V3 pos, V3 normal, V2 tex_coords, V4 color, f32 tex_index);

b8 vertex_equal(const Vertex* f, const Vertex* s);

typedef struct MVP
{
    M4 model;
    M4 view;
    M4 proj;
} MVP;

b8 mvp_equal(const MVP* f, const MVP* s);

typedef struct Push_Color
{
    V4 color;
} Push_Color;

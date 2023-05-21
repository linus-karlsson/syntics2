#pragma once

#include "defines.h"

#define V3_FMT(v) "(x: %f, y: %f, z: %f)\n", (v).x, (v).y, (v).z
#define V4_FMT(v) "(x: %f, y: %f, z: %f, w: %f)\n", (v).x, (v).y, (v).z, (v).w

typedef struct Vec2
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
            f32 u;
            f32 v;
        };

        struct
        {
            f32 l;
            f32 r;
        };
    };
} Vec2, V2;

typedef struct Vec3
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
    };
} Vec3, V3;

typedef struct Vec4
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

} Vec4, V4;

V2 v2d(void);
V2 v2i(f32 i);
V2 v2f(f32 x, f32 y);
V2 v2_v3(V3 v3);
V2 v2_v4(V4 v4);

V3 v3d(void);
V3 v3i(f32 i);
V3 v3f(f32 x, f32 y, f32 z);
V3 v3_v2(V2 v2);
V3 v3_v2f(V2 v2, f32 z);
V3 v3_v4(V4 v4);

V4 v4d(void);
V4 v4i(f32 i);
V4 v4f(f32 x, f32 y, f32 z, f32 w);
V4 v4_v2(V2 v2);
V4 v4_v2f(V2 v2, f32 z, f32 w);
V4 v4_v3(V3 v3);
V4 v4_v3f(V3 v3, f32 w);

f32 v2_sum(V2 v);
f32 v3_sum(V3 v);
f32 v4_sum(V4 v);

V2 v2_neg(V2 v);
V3 v3_neg(V3 v);
V4 v4_neg(V4 v);

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

#if 0
V2 operator+(const V2& v1, const V2& v2);
V3 operator+(const V3& v1, const V3& v2);
V4 operator+(const V4& v1, const V4& v2);

V2 operator-(const V2& v1, const V2& v2);
V3 operator-(const V3& v1, const V3& v2);
V4 operator-(const V4& v1, const V4& v2);

V2 operator+(const V2& v1, f32 s);
V3 operator+(const V3& v1, f32 s);
V4 operator+(const V4& v1, f32 s);

V2 operator-(const V2& v1, f32 s);
V3 operator-(const V3& v1, f32 s);
V4 operator-(const V4& v1, f32 s);

V2 operator-(const V2& v1);
V3 operator-(const V3& v1);
V4 operator-(const V4& v1);

V2 operator*(const V2& v1, const V2& v2);
V3 operator*(const V3& v1, const V3& v2);
V4 operator*(const V4& v1, const V4& v2);

V2 operator*(const V2& v1, f32 s);
V3 operator*(const V3& v1, f32 s);
V4 operator*(const V4& v1, f32 s);

V2 operator*(f32 s, const V2& v1);
V3 operator*(f32 s, const V3& v1);
V4 operator*(f32 s, const V4& v1);

V2 operator/(const V2& v1, f32 s);
V3 operator/(const V3& v1, f32 s);
V4 operator/(const V4& v1, f32 s);

V2& operator+=(V2& v1, const V2& v2);
V3& operator+=(V3& v1, const V3& v2);
V4& operator+=(V4& v1, const V4& v2);

V2& operator+=(V2& v1, f32 s);
V3& operator+=(V3& v1, f32 s);
V4& operator+=(V4& v1, f32 s);

V2& operator-=(V2& v1, const V2& v2);
V3& operator-=(V3& v1, const V3& v2);
V4& operator-=(V4& v1, const V4& v2);

V2& operator-=(V2& v1, f32 s);
V3& operator-=(V3& v1, f32 s);
V4& operator-=(V4& v1, f32 s);

V2& operator*=(V2& v1, const V2& v2);
V3& operator*=(V3& v1, const V3& v2);
V4& operator*=(V4& v1, const V4& v2);

V2& operator*=(V2& v1, f32 s);
V3& operator*=(V3& v1, f32 s);
V4& operator*=(V4& v1, f32 s);

b8 operator==(const V2& v1, const V2& v2);
b8 operator==(const V3& v1, const V3& v2);
b8 operator==(const V4& v1, const V4& v2);

b8 operator<(const V2& v1, const V2& v2);
b8 operator<(const V3& v1, const V3& v2);
b8 operator<(const V4& v1, const V4& v2);

b8 operator>(const V2& v1, const V2& v2);
b8 operator>(const V3& v1, const V3& v2);
b8 operator>(const V4& v1, const V4& v2);
#endif

typedef struct Point2
{
    f32 x;
    f32 y;
} Point2, P2;

typedef struct Point3
{
    f32 x;
    f32 y;
    f32 z;
} Point3, P3;

typedef struct Point4
{
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Point4, P4;

P2 p2d(void);
P2 p2i(f32 i);
P2 p2f(f32 x, f32 y);
P2 p2_p3(P3 p3);
P2 p2_p4(P4 p4);

P3 p3d(void);
P3 p3i(f32 i);
P3 p3f(f32 x, f32 y, f32 z);
P3 p3_p2(P2 p2);
P3 p3_p2f(P2 p2, f32 z);
P3 p3_p4(P4 p4);

P4 p4d(void);
P4 p4i(f32 i);
P4 p4f(f32 x, f32 y, f32 z, f32 w);
P4 p4_p2(P2 p2);
P4 p4_p2f(P2 p2, f32 z, f32 w);
P4 p4_p3(P3 p3);
P4 p4_p3f(P3 p3, f32 w);

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

typedef struct Polygon2D
{
    V2 pos;
    V2 vel;
    V2* points;
    V2* normals;
    u32 n_sides;
    u32 id;
} Polygon2D;

Polygon2D poly2D(V2 pos, V2* p_arr, V2* n_arr, u32 n_sides);

typedef struct Quad2D
{
    V2 pos;
    V2 vel;
    V2 points[4];
    V2 normals[4];
    u32 id;
} Quad2D;

b8 is_poly2d_convex(Polygon2D p);
b8 is_quad2d_convex(Quad2D q);

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
    V2 size;
    V4 color;
    V2 vel;
    u32 id;
} Rect3D;

typedef struct Plane
{
    V3 n;
    float d;
} Plane;

Plane plane(P3 a, P3 b, P3 c);

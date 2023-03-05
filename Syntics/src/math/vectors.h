#pragma once

#include "defines.h"

#define PRINT_VEC3(v) synt_LOG("(x: %f, y: %f, z: %f)\n", v.x, v.y, v.z)
#define PRINT_VEC4(v) synt_LOG("(x: %f, y: %f, z: %f, w: %f)\n", v.x, v.y, v.z, v.w)

typedef struct Vec2
{
    f32 x;
    f32 y;
} Vec2, V2;

typedef struct Vec3
{
    f32 x;
    f32 y;
    f32 z;
} Vec3, V3;

typedef struct Vec4
{
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vec4, V4;

V2 v2d();
V2 v2i(f32 i);
V2 v2f(f32 x, f32 y);
V2 v2_v3(V3 v3);
V2 v2_v4(V4 v4);

V3 v3d();
V3 v3i(f32 i);
V3 v3f(f32 x, f32 y, f32 z);
V3 v3_v2(V2 v2);
V3 v3_v2f(V2 v2, f32 z);
V3 v3_v4(V4 v4);

V4 v4d();
V4 v4i(f32 i);
V4 v4f(f32 x, f32 y, f32 z, f32 w);
V4 v4_v2(V2 v2);
V4 v4_v2f(V2 v2, f32 z, f32 w);
V4 v4_v3(V3 v3);
V4 v4_v3f(V3 v3, f32 w);

f32 v2_sum(V2 m);
f32 v3_sum(V3 m);
f32 v4_sum(V4 m);

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

typedef struct Point2f
{
    Point2f();
    Point2f(f32 x, f32 y);

    f32 x;
    f32 y;
} Point2f;

typedef struct Point3f
{
    Point3f();
    Point3f(f32 x, f32 y, f32 z);

    f32 x;
    f32 y;
    f32 z;
} Point3f;

typedef struct Point4f
{
    Point4f();
    Point4f(f32 x, f32 y, f32 z, f32 w);

    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Point4f;

Point3f p3f(f32 x, f32 y, f32 z);
Point3f p3i(f32 i);

f32 p2_sum(Point2f v);
f32 p3_sum(Point3f v);
f32 p4_sum(Point4f v);

Point2f operator+(const Point2f& p1, const Point2f& p2);
Point3f operator+(const Point3f& p1, const Point3f& p2);
Point4f operator+(const Point4f& p1, const Point4f& p2);

V2 operator-(const Point2f& p1, const Point2f& p2);
V3 operator-(const Point3f& p1, const Point3f& p2);
V4 operator-(const Point4f& p1, const Point4f& p2);

Point2f operator-(const Point2f& p, const V2& v);
Point3f operator-(const Point3f& p, const V3& v);
Point4f operator-(const Point4f& p, const V4& v);

Point2f operator*(const Point2f& p1, const Point2f& p2);
Point3f operator*(const Point3f& p1, const Point3f& p2);
Point4f operator*(const Point4f& p1, const Point4f& p2);

Point2f operator*(const Point2f& p1, f32 s);
Point3f operator*(const Point3f& p1, f32 s);
Point4f operator*(const Point4f& p1, f32 s);

Point2f operator*(f32 s, const Point2f& p1);
Point3f operator*(f32 s, const Point3f& p1);
Point4f operator*(f32 s, const Point4f& p1);

Point2f& operator+=(Point2f& p1, const V2& v);
Point3f& operator+=(Point3f& p1, const V3& v);
Point4f& operator+=(Point4f& p1, const V4& v);

Point2f& operator-=(Point2f& p, const V2& v);
Point3f& operator-=(Point3f& p, const V3& v);
Point4f& operator-=(Point4f& p, const V4& v);

Point2f& operator*=(Point2f& p1, const Point2f& p2);
Point3f& operator*=(Point3f& p1, const Point3f& p2);
Point4f& operator*=(Point4f& p1, const Point4f& p2);

b8 operator==(const Point2f& p1, const Point2f& p2);
b8 operator==(const Point3f& p1, const Point3f& p2);
b8 operator==(const Point4f& p1, const Point4f& p2);

b8 operator<(const Point2f& p1, const Point2f& p2);
b8 operator<(const Point3f& p1, const Point3f& p2);
b8 operator<(const Point4f& p1, const Point4f& p2);

b8 operator>(const Point2f& p1, const Point2f& p2);
b8 operator>(const Point3f& p1, const Point3f& p2);
b8 operator>(const Point4f& p1, const Point4f& p2);

#if 0
typedef struct Polygon
{
    V2 pos;
    V4 color;
    V2 vel;
    Point3f* points;
    V3* normals;
    u32n_sides;
    u32id;
} Polygon;
#endif

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

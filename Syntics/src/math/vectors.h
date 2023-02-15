#pragma once

#include "defines.h"

#define PRINT_VEC3(v) synt_LOG("(x: %f, y: %f, z: %f)\n", v.x, v.y, v.z)
#define PRINT_VEC4(v) synt_LOG("(x: %f, y: %f, z: %f, w: %f)\n", v.x, v.y, v.z, v.w)

typedef struct Vec2 V2;
typedef struct Vec3 V3;
typedef struct Vec4 V4;

struct Vec2
{
    Vec2();
    Vec2(f32 i);
    Vec2(f32 x, f32 y);

    f32 x;
    f32 y;
};

struct Vec3
{
    Vec3();
    Vec3(f32 i);
    Vec3(f32 x, f32 y, f32 z);
    Vec3(Vec4 v4);

    f32 x;
    f32 y;
    f32 z;
};

struct Vec4
{
    Vec4();
    Vec4(f32 i);
    Vec4(f32 x, f32 y, f32 z, f32 w);

    f32 x;
    f32 y;
    f32 z;
    f32 w;
};

Vec3 v3f(f32 x, f32 y, f32 z);
Vec3 v3i(f32 i);

f32 sum_v2(const Vec2& m);
f32 sum_v3(const Vec3& m);
f32 sum_v4(const Vec4& m);

Vec2 operator+(const Vec2& v1, const Vec2& v2);
Vec3 operator+(const Vec3& v1, const Vec3& v2);
Vec4 operator+(const Vec4& v1, const Vec4& v2);

Vec2 operator-(const Vec2& v1, const Vec2& v2);
Vec3 operator-(const Vec3& v1, const Vec3& v2);
Vec4 operator-(const Vec4& v1, const Vec4& v2);

Vec2 operator+(const Vec2& v1, f32 s);
Vec3 operator+(const Vec3& v1, f32 s);
Vec4 operator+(const Vec4& v1, f32 s);

Vec2 operator-(const Vec2& v1, f32 s);
Vec3 operator-(const Vec3& v1, f32 s);
Vec4 operator-(const Vec4& v1, f32 s);

Vec2 operator*(const Vec2& v1, const Vec2& v2);
Vec3 operator*(const Vec3& v1, const Vec3& v2);
Vec4 operator*(const Vec4& v1, const Vec4& v2);

Vec2 operator*(const Vec2& v1, f32 s);
Vec3 operator*(const Vec3& v1, f32 s);
Vec4 operator*(const Vec4& v1, f32 s);

Vec2 operator*(f32 s, const Vec2& v1);
Vec3 operator*(f32 s, const Vec3& v1);
Vec4 operator*(f32 s, const Vec4& v1);

Vec2 operator/(const Vec2& v1, f32 s);
Vec3 operator/(const Vec3& v1, f32 s);
Vec4 operator/(const Vec4& v1, f32 s);

Vec2& operator+=(Vec2& v1, const Vec2& v2);
Vec3& operator+=(Vec3& v1, const Vec3& v2);
Vec4& operator+=(Vec4& v1, const Vec4& v2);

Vec2& operator-=(Vec2& v1, const Vec2& v2);
Vec3& operator-=(Vec3& v1, const Vec3& v2);
Vec4& operator-=(Vec4& v1, const Vec4& v2);

Vec2& operator*=(Vec2& v1, const Vec2& v2);
Vec3& operator*=(Vec3& v1, const Vec3& v2);
Vec4& operator*=(Vec4& v1, const Vec4& v2);

Vec2& operator*=(Vec2& v1, f32 s);
Vec3& operator*=(Vec3& v1, f32 s);
Vec4& operator*=(Vec4& v1, f32 s);

b8 operator==(const Vec2& v1, const Vec2& v2);
b8 operator==(const Vec3& v1, const Vec3& v2);
b8 operator==(const Vec4& v1, const Vec4& v2);

b8 operator<(const Vec2& v1, const Vec2& v2);
b8 operator<(const Vec3& v1, const Vec3& v2);
b8 operator<(const Vec4& v1, const Vec4& v2);

b8 operator>(const Vec2& v1, const Vec2& v2);
b8 operator>(const Vec3& v1, const Vec3& v2);
b8 operator>(const Vec4& v1, const Vec4& v2);

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

f32 sum_v2(const Point2f& m);
f32 sum_v3(const Point3f& m);
f32 sum_v4(const Point4f& m);

Point2f operator+(const Point2f& p1, const Point2f& p2);
Point3f operator+(const Point3f& p1, const Point3f& p2);
Point4f operator+(const Point4f& p1, const Point4f& p2);

Vec2 operator-(const Point2f& p1, const Point2f& p2);
Vec3 operator-(const Point3f& p1, const Point3f& p2);
Vec4 operator-(const Point4f& p1, const Point4f& p2);

Point2f operator-(const Point2f& p, const Vec2& v);
Point3f operator-(const Point3f& p, const Vec3& v);
Point4f operator-(const Point4f& p, const Vec4& v);

Point2f operator*(const Point2f& p1, const Point2f& p2);
Point3f operator*(const Point3f& p1, const Point3f& p2);
Point4f operator*(const Point4f& p1, const Point4f& p2);

Point2f operator*(const Point2f& p1, f32 s);
Point3f operator*(const Point3f& p1, f32 s);
Point4f operator*(const Point4f& p1, f32 s);

Point2f operator*(f32 s, const Point2f& p1);
Point3f operator*(f32 s, const Point3f& p1);
Point4f operator*(f32 s, const Point4f& p1);

Point2f& operator+=(Point2f& p1, const Vec2& v);
Point3f& operator+=(Point3f& p1, const Vec3& v);
Point4f& operator+=(Point4f& p1, const Vec4& v);

Point2f& operator-=(Point2f& p, const Vec2& v);
Point3f& operator-=(Point3f& p, const Vec3& v);
Point4f& operator-=(Point4f& p, const Vec4& v);

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
    Vec2 pos;
    Vec4 color;
    Vec2 vel;
    Point3f* points;
    Vec3* normals;
    u32n_sides;
    u32id;
} Polygon;
#endif

typedef struct Rect2D
{
    Vec2 pos;
    Vec2 size;
    Vec4 color;
    Vec2 vel;
    u32 id;
} Rect2D;

typedef struct Rect3D
{
    Vec3 pos;
    Vec2 size;
    Vec4 color;
    Vec2 vel;
    u32 id;
} Rect3D;

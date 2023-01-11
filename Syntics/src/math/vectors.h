#pragma once

#include "defines.h"

#define PRINT_VEC3(v) synt_LOG("(x: %f, y: %f, z: %f)\n", v.x, v.y, v.z)
#define PRINT_VEC4(v) synt_LOG("(x: %f, y: %f, z: %f, w: %f)\n", v.x, v.y, v.z, v.w)

namespace synt {

typedef struct Vec2 Vec2;
typedef struct Vec3 Vec3;
typedef struct Vec4 Vec4;

struct Vec2
{
    Vec2();
    Vec2(float i);
    Vec2(float x, float y);

    float x;
    float y;
};

struct Vec3
{
    Vec3();
    Vec3(float i);
    Vec3(float x, float y, float z);
    Vec3(Vec4 v4);

    float x;
    float y;
    float z;
};

struct Vec4
{
    Vec4();
    Vec4(float i);
    Vec4(float x, float y, float z, float w);

    float x;
    float y;
    float z;
    float w;
};

Vec3 v3f(float x, float y, float z);
Vec3 v3i(float i);

float sum_v2(const Vec2& m);
float sum_v3(const Vec3& m);
float sum_v4(const Vec4& m);

Vec2 operator+(const Vec2& v1, const Vec2& v2);
Vec3 operator+(const Vec3& v1, const Vec3& v2);
Vec4 operator+(const Vec4& v1, const Vec4& v2);

Vec2 operator-(const Vec2& v1, const Vec2& v2);
Vec3 operator-(const Vec3& v1, const Vec3& v2);
Vec4 operator-(const Vec4& v1, const Vec4& v2);

Vec2 operator+(const Vec2& v1, float s);
Vec3 operator+(const Vec3& v1, float s);
Vec4 operator+(const Vec4& v1, float s);

Vec2 operator-(const Vec2& v1, float s);
Vec3 operator-(const Vec3& v1, float s);
Vec4 operator-(const Vec4& v1, float s);

Vec2 operator*(const Vec2& v1, const Vec2& v2);
Vec3 operator*(const Vec3& v1, const Vec3& v2);
Vec4 operator*(const Vec4& v1, const Vec4& v2);

Vec2 operator*(const Vec2& v1, float s);
Vec3 operator*(const Vec3& v1, float s);
Vec4 operator*(const Vec4& v1, float s);

Vec2 operator*(float s, const Vec2& v1);
Vec3 operator*(float s, const Vec3& v1);
Vec4 operator*(float s, const Vec4& v1);

Vec2 operator/(const Vec2& v1, float s);
Vec3 operator/(const Vec3& v1, float s);
Vec4 operator/(const Vec4& v1, float s);

Vec2& operator+=(Vec2& v1, const Vec2& v2);
Vec3& operator+=(Vec3& v1, const Vec3& v2);
Vec4& operator+=(Vec4& v1, const Vec4& v2);

Vec2& operator-=(Vec2& v1, const Vec2& v2);
Vec3& operator-=(Vec3& v1, const Vec3& v2);
Vec4& operator-=(Vec4& v1, const Vec4& v2);

Vec2& operator*=(Vec2& v1, const Vec2& v2);
Vec3& operator*=(Vec3& v1, const Vec3& v2);
Vec4& operator*=(Vec4& v1, const Vec4& v2);

Vec2& operator*=(Vec2& v1, float s);
Vec3& operator*=(Vec3& v1, float s);
Vec4& operator*=(Vec4& v1, float s);

bool operator==(const Vec2& v1, const Vec2& v2);
bool operator==(const Vec3& v1, const Vec3& v2);
bool operator==(const Vec4& v1, const Vec4& v2);

bool operator<(const Vec2& v1, const Vec2& v2);
bool operator<(const Vec3& v1, const Vec3& v2);
bool operator<(const Vec4& v1, const Vec4& v2);

bool operator>(const Vec2& v1, const Vec2& v2);
bool operator>(const Vec3& v1, const Vec3& v2);
bool operator>(const Vec4& v1, const Vec4& v2);

typedef struct Point2f
{
    Point2f();
    Point2f(float x, float y);

    float x;
    float y;
} Point2f;

typedef struct Point3f
{
    Point3f();
    Point3f(float x, float y, float z);

    float x;
    float y;
    float z;
} Point3f;

typedef struct Point4f
{
    Point4f();
    Point4f(float x, float y, float z, float w);

    float x;
    float y;
    float z;
    float w;
} Point4f;

Point3f p3f(float x, float y, float z);
Point3f p3i(float i);

float sum_v2(const Point2f& m);
float sum_v3(const Point3f& m);
float sum_v4(const Point4f& m);

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

Point2f operator*(const Point2f& p1, float s);
Point3f operator*(const Point3f& p1, float s);
Point4f operator*(const Point4f& p1, float s);

Point2f operator*(float s, const Point2f& p1);
Point3f operator*(float s, const Point3f& p1);
Point4f operator*(float s, const Point4f& p1);

Point2f& operator+=(Point2f& p1, const Vec2& v);
Point3f& operator+=(Point3f& p1, const Vec3& v);
Point4f& operator+=(Point4f& p1, const Vec4& v);

Point2f& operator-=(Point2f& p, const Vec2& v);
Point3f& operator-=(Point3f& p, const Vec3& v);
Point4f& operator-=(Point4f& p, const Vec4& v);

Point2f& operator*=(Point2f& p1, const Point2f& p2);
Point3f& operator*=(Point3f& p1, const Point3f& p2);
Point4f& operator*=(Point4f& p1, const Point4f& p2);

bool operator==(const Point2f& p1, const Point2f& p2);
bool operator==(const Point3f& p1, const Point3f& p2);
bool operator==(const Point4f& p1, const Point4f& p2);

bool operator<(const Point2f& p1, const Point2f& p2);
bool operator<(const Point3f& p1, const Point3f& p2);
bool operator<(const Point4f& p1, const Point4f& p2);

bool operator>(const Point2f& p1, const Point2f& p2);
bool operator>(const Point3f& p1, const Point3f& p2);
bool operator>(const Point4f& p1, const Point4f& p2);

#if 0
typedef struct Polygon
{
    Vec2 pos;
    Vec4 color;
    Vec2 vel;
    Point3f* points;
    Vec3* normals;
    uint32 n_sides;
    uint32 id;
} Polygon;
#endif

typedef struct Rect
{
    Vec2 pos;
    Vec2 size;
    Vec4 color;
    Vec2 vel;
    uint32 id;
} Rect;

} // namespace synt

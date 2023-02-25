#include "vectors.h"

Vec2::Vec2() : x(0.0f), y(0.0f)
{
}
Vec2::Vec2(f32 i) : x(i), y(i)
{
}
Vec2::Vec2(f32 x, f32 y) : x(x), y(y)
{
}

Vec3::Vec3() : x(0.0f), y(0.0f), z(0.0f)
{
}
Vec3::Vec3(f32 i) : x(i), y(i), z(i)
{
}
Vec3::Vec3(f32 x, f32 y, f32 z) : x(x), y(y), z(z)
{
}
Vec3::Vec3(Vec4 v4) : x(v4.x), y(v4.y), z(v4.z)
{
}

Vec4::Vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f)
{
}
Vec4::Vec4(f32 i) : x(i), y(i), z(i), w(i)
{
}
Vec4::Vec4(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w)
{
}

Vec3 v3f(f32 x, f32 y, f32 z)
{
    return { x, y, z };
}

Vec3 v3i(f32 i)
{
    return { i, i, i };
}

Vec2 operator+(const Vec2& v1, const Vec2& v2)
{
    return { v1.x + v2.x, v1.y + v2.y };
}

Vec3 operator+(const Vec3& v1, const Vec3& v2)
{
    return {
        v1.x + v2.x,
        v1.y + v2.y,
        v1.z + v2.z,
    };
}

Vec4 operator+(const Vec4& v1, const Vec4& v2)
{
    return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w };
}

Vec2 operator-(const Vec2& v1, const Vec2& v2)
{
    return { v1.x - v2.x, v1.y - v2.y };
}

Vec3 operator-(const Vec3& v1, const Vec3& v2)
{
    return {
        v1.x - v2.x,
        v1.y - v2.y,
        v1.z - v2.z,
    };
}

Vec4 operator-(const Vec4& v1, const Vec4& v2)
{
    return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w };
}

Vec2 operator+(const Vec2& v1, f32 s)
{
    return { v1.x + s, v1.y + s };
}

Vec3 operator+(const Vec3& v1, f32 s)
{
    return { v1.x + s, v1.y + s, v1.z + s };
}

Vec4 operator+(const Vec4& v1, f32 s)
{
    return { v1.x + s, v1.y + s, v1.z + s, v1.w + s };
}

Vec2 operator-(const Vec2& v1, f32 s)
{
    return { v1.x - s, v1.y - s };
}

Vec3 operator-(const Vec3& v1, f32 s)
{
    return { v1.x - s, v1.y - s, v1.z - s };
}

Vec4 operator-(const Vec4& v1, f32 s)
{
    return { v1.x - s, v1.y - s, v1.z - s, v1.w - s };
}

Vec2 operator*(const Vec2& v1, const Vec2& v2)
{
    return { v1.x * v2.x, v1.y * v2.y };
}

Vec3 operator*(const Vec3& v1, const Vec3& v2)
{
    return {
        v1.x * v2.x,
        v1.y * v2.y,
        v1.z * v2.z,
    };
}

Vec4 operator*(const Vec4& v1, const Vec4& v2)
{
    return { v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w };
}

Vec2& operator+=(Vec2& v1, const Vec2& v2)
{
    v1 = v1 + v2;
    return v1;
}

Vec3& operator+=(Vec3& v1, const Vec3& v2)
{
    v1 = v1 + v2;
    return v1;
}

Vec4& operator+=(Vec4& v1, const Vec4& v2)
{
    v1 = v1 + v2;
    return v1;
}

Vec2& operator-=(Vec2& v1, const Vec2& v2)
{
    v1 = v1 - v2;
    return v1;
}

Vec3& operator-=(Vec3& v1, const Vec3& v2)
{
    v1 = v1 - v2;
    return v1;
}

Vec4& operator-=(Vec4& v1, const Vec4& v2)
{
    v1 = v1 - v2;
    return v1;
}

Vec2& operator*=(Vec2& v1, const Vec2& v2)
{
    v1 = v1 * v2;
    return v1;
}
Vec3& operator*=(Vec3& v1, const Vec3& v2)
{
    v1 = v1 * v2;
    return v1;
}

Vec4& operator*=(Vec4& v1, const Vec4& v2)
{
    v1 = v1 * v2;
    return v1;
}

Vec2& operator*=(Vec2& v1, f32 s)
{
    v1 = v1 * s;
    return v1;
}

Vec3& operator*=(Vec3& v1, f32 s)
{
    v1 = v1 * s;
    return v1;
}

Vec4& operator*=(Vec4& v1, f32 s)
{
    v1 = v1 * s;
    return v1;
}

Vec2 operator*(const Vec2& v1, f32 s)
{
    return { v1.x * s, v1.y * s };
}

Vec3 operator*(const Vec3& v1, f32 s)
{
    return { v1.x * s, v1.y * s, v1.z * s };
}

Vec4 operator*(const Vec4& v1, f32 s)
{
    return { v1.x * s, v1.y * s, v1.z * s, v1.w * s };
}

Vec2 operator*(f32 s, const Vec2& v1)
{
    return { v1.x * s, v1.y * s };
}

Vec3 operator*(f32 s, const Vec3& v1)
{
    return { v1.x * s, v1.y * s, v1.z * s };
}

Vec4 operator*(f32 s, const Vec4& v1)
{
    return { v1.x * s, v1.y * s, v1.z * s, v1.w * s };
}

Vec2 operator/(const Vec2& v1, f32 s)
{
    return { v1.x / s, v1.y / s };
}

Vec3 operator/(const Vec3& v1, f32 s)
{
    return { v1.x / s, v1.y / s, v1.z / s };
}
Vec4 operator/(const Vec4& v1, f32 s)
{
    return { v1.x / s, v1.y / s, v1.z / s, v1.w / s };
}

b8 operator==(const Vec2& v1, const Vec2& v2)
{
    return v1.x == v2.x && v1.y == v2.y;
}

b8 operator==(const Vec3& v1, const Vec3& v2)
{
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

b8 operator==(const Vec4& v1, const Vec4& v2)
{
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w;
}

f32 sum_v2(const Vec2& v)
{
    return (v.x + v.y);
}

f32 sum_v3(const Vec3& v)
{
    return (v.x + v.y + v.z);
}

f32 sum_v4(const Vec4& v)
{
    return (v.x + v.y + v.z + v.w);
}

b8 operator<(const Vec2& v1, const Vec2& v2)
{
    return (sum_v2(v1) < sum_v2(v2));
}
b8 operator<(const Vec3& v1, const Vec3& v2)
{
    return (sum_v3(v1) < sum_v3(v2));
}
b8 operator<(const Vec4& v1, const Vec4& v2)
{
    return (sum_v4(v1) < sum_v4(v2));
}

b8 operator>(const Vec2& v1, const Vec2& v2)
{
    return (sum_v2(v1) > sum_v2(v2));
}
b8 operator>(const Vec3& v1, const Vec3& v2)
{
    return (sum_v3(v1) > sum_v3(v2));
}
b8 operator>(const Vec4& v1, const Vec4& v2)
{
    return (sum_v4(v1) > sum_v4(v2));
}

Point2f::Point2f() : x(0.0f), y(0.0f)
{
}
Point2f::Point2f(f32 x, f32 y) : x(x), y(y)
{
}

Point3f::Point3f() : x(0.0f), y(0.0f), z(0.0f)
{
}
Point3f::Point3f(f32 x, f32 y, f32 z) : x(x), y(y), z(z)
{
}

Point4f::Point4f() : x(0.0f), y(0.0f), z(0.0f), w(0.0f)
{
}
Point4f::Point4f(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w)
{
}

Point3f p3f(f32 x, f32 y, f32 z)
{
    return { x, y, z };
}

Point3f p3i(f32 i)
{
    return { i, i, i };
}

Point2f operator+(const Point2f& p1, const Point2f& p2)
{
    return { p1.x + p2.x, p1.y + p2.y };
}

Point3f operator+(const Point3f& p1, const Point3f& p2)
{
    return {
        p1.x + p2.x,
        p1.y + p2.y,
        p1.z + p2.z,
    };
}

Point4f operator+(const Point4f& p1, const Point4f& p2)
{
    return { p1.x + p2.x, p1.y + p2.y, p1.z + p2.z, p1.w + p2.w };
}

Vec2 operator-(const Point2f& p1, const Point2f& p2)
{
    return { p1.x - p2.x, p1.y - p2.y };
}

Vec3 operator-(const Point3f& p1, const Point3f& p2)
{
    return {
        p1.x - p2.x,
        p1.y - p2.y,
        p1.z - p2.z,
    };
}

Vec4 operator-(const Point4f& p1, const Point4f& p2)
{
    return { p1.x - p2.x, p1.y - p2.y, p1.z - p2.z, p1.w - p2.w };
}

Point2f operator-(const Point2f& p, const Vec2& v)
{
    return { p.x - v.x, p.y - v.y };
}

Point3f operator-(const Point3f& p, const Vec3& v)
{
    return { p.x - v.x, p.y - v.y, p.z - v.z };
}

Point4f operator-(const Point4f& p, const Vec4& v)
{
    return { p.x - v.x, p.y - v.y, p.z - v.z, p.w - v.w };
}

Point2f operator*(const Point2f& p1, const Point2f& p2)
{
    return { p1.x * p2.x, p1.y * p2.y };
}

Point3f operator*(const Point3f& p1, const Point3f& p2)
{
    return {
        p1.x * p2.x,
        p1.y * p2.y,
        p1.z * p2.z,
    };
}

Point4f operator*(const Point4f& p1, const Point4f& p2)
{
    return { p1.x * p2.x, p1.y * p2.y, p1.z * p2.z, p1.w * p2.w };
}

Point2f& operator+=(Point2f& p1, const Point2f& p2)
{
    p1 = p1 + p2;
    return p1;
}

Point3f& operator+=(Point3f& p1, const Point3f& p2)
{
    p1 = p1 + p2;
    return p1;
}

Point4f& operator+=(Point4f& p1, const Point4f& p2)
{
    p1 = p1 + p2;
    return p1;
}

Point2f& operator-=(Point2f& p, const Vec2& v)
{
    p = p - v;
    return p;
}

Point3f& operator-=(Point3f& p, const Vec3& v)
{
    p = p - v;
    return p;
}

Point4f& operator-=(Point4f& p, const Vec4& v)
{
    p = p - v;
    return p;
}

Point2f& operator*=(Point2f& p1, const Point2f& p2)
{
    p1 = p1 * p2;
    return p1;
}
Point3f& operator*=(Point3f& p1, const Point3f& p2)
{
    p1 = p1 * p2;
    return p1;
}

Point4f& operator*=(Point4f& p1, const Point4f& p2)
{
    p1 = p1 * p2;
    return p1;
}

Point2f operator*(const Point2f& p1, f32 s)
{
    return { p1.x * s, p1.y * s };
}

Point3f operator*(const Point3f& p1, f32 s)
{
    return { p1.x * s, p1.y * s, p1.z * s };
}

Point4f operator*(const Point4f& p1, f32 s)
{
    return { p1.x * s, p1.y * s, p1.z * s, p1.w * s };
}

Point2f operator*(f32 s, const Point2f& p1)
{
    return { p1.x * s, p1.y * s };
}

Point3f operator*(f32 s, const Point3f& p1)
{
    return { p1.x * s, p1.y * s, p1.z * s };
}

Point4f operator*(f32 s, const Point4f& p1)
{
    return { p1.x * s, p1.y * s, p1.z * s, p1.w * s };
}

b8 operator==(const Point2f& p1, const Point2f& p2)
{
    return p1.x == p2.x && p1.y == p2.y;
}

b8 operator==(const Point3f& p1, const Point3f& p2)
{
    return p1.x == p2.x && p1.y == p2.y && p1.z == p2.z;
}

b8 operator==(const Point4f& p1, const Point4f& p2)
{
    return p1.x == p2.x && p1.y == p2.y && p1.z == p2.z && p1.w == p2.w;
}

f32 sum_v2(const Point2f& v)
{
    return (v.x + v.y);
}

f32 sum_v3(const Point3f& v)
{
    return (v.x + v.y + v.z);
}

f32 sum_v4(const Point4f& v)
{
    return (v.x + v.y + v.z + v.w);
}

b8 operator<(const Point2f& p1, const Point2f& p2)
{
    return (sum_v2(p1) < sum_v2(p2));
}
b8 operator<(const Point3f& p1, const Point3f& p2)
{
    return (sum_v3(p1) < sum_v3(p2));
}
b8 operator<(const Point4f& p1, const Point4f& p2)
{
    return (sum_v4(p1) < sum_v4(p2));
}

b8 operator>(const Point2f& p1, const Point2f& p2)
{
    return (sum_v2(p1) > sum_v2(p2));
}
b8 operator>(const Point3f& p1, const Point3f& p2)
{
    return (sum_v3(p1) > sum_v3(p2));
}
b8 operator>(const Point4f& p1, const Point4f& p2)
{
    return (sum_v4(p1) > sum_v4(p2));
}


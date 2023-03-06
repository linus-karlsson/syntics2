#include "vectors.h"

V2 v2d()
{
    return (V2){ 0, 0 };
}

V2 v2i(f32 i)
{
    return (V2){ i, i };
}

V2 v2f(f32 x, f32 y)
{
    return (V2){ x, y };
}

V2 v2_v3(V3 v3)
{
    return (V2){ v3.x, v3.y };
}

V2 v2_v4(V4 v4)
{
    return (V2){ v4.x, v4.y };
}

V3 v3d()
{
    return (V3){ 0, 0, 0 };
}

V3 v3i(f32 i)
{
    return (V3){ i, i, i };
}

V3 v3f(f32 x, f32 y, f32 z)
{
    return (V3){ x, y, z };
}

V3 v3_v2(V2 v2)
{
    return (V3){ v2.x, v2.y, 0 };
}

V3 v3_v2f(V2 v2, f32 z)
{
    return (V3){ v2.x, v2.y, z };
}

V3 v3_v4(V4 v4)
{
    return (V3){ v4.x, v4.y, v4.z };
}

V4 v4d()
{
    return (V4){ 0, 0, 0, 0 };
}

V4 v4i(f32 i)
{
    return (V4){ i, i, i, i };
}

V4 v4f(f32 x, f32 y, f32 z, f32 w)
{
    return (V4){ x, y, z, w };
}

V4 v4_v2(V2 v2)
{
    return (V4){ v2.x, v2.y, 0, 0 };
}

V4 v4_v2f(V2 v2, f32 z, f32 w)
{
    return (V4){ v2.x, v2.y, z, w };
}

V4 v4_v3(V3 v3)
{
    return (V4){ v3.x, v3.y, v3.z, 0 };
}

V4 v4_v3f(V3 v3, f32 w)
{
    return (V4){ v3.x, v3.y, v3.z, w };
}

f32 v2_sum(V2 v)
{
    return (v.x + v.y);
}

f32 v3_sum(V3 v)
{
    return (v.x + v.y + v.z);
}

f32 v4_sum(V4 v)
{
    return (v.x + v.y + v.z + v.w);
}

V2 v2_neg(V2 v)
{
    return v2_s_multi(v, -1.0f);
}

V3 v3_neg(V3 v)
{
    return v3_s_multi(v, -1.0f);
}

V4 v4_neg(V4 v)
{
    return v4_s_multi(v, -1.0f);
}

V2 v2_add(V2 v1, V2 v2)
{
    return v2f(v1.x + v2.x, v1.y + v2.y);
}

V3 v3_add(V3 v1, V3 v2)
{
    return v3f(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

V4 v4_add(V4 v1, V4 v2)
{
    return v4f(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

V2 v2_sub(V2 v1, V2 v2)
{
    return v2f(v1.x - v2.x, v1.y - v2.y);
}

V3 v3_sub(V3 v1, V3 v2)
{
    return v3f(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

V4 v4_sub(V4 v1, V4 v2)
{
    return v4f(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
}

V2 v2_s_add(V2 v1, f32 s)
{
    return v2d(v1.x + s, v1.y + s);
}

V3 v3_s_add(V3 v1, f32 s)
{
    return v3f(v1.x + s, v1.y + s, v1.z + s);
}

V4 v4_s_add(V4 v1, f32 s)
{
    return v4f(v1.x + s, v1.y + s, v1.z + s, v1.w + s);
}

V2 v2_s_sub(V2 v1, f32 s)
{
    return v2d(v1.x - s, v1.y - s);
}

V3 v3_s_sub(V3 v1, f32 s)
{
    return v3f(v1.x - s, v1.y - s, v1.z - s);
}

V4 v4_s_sub(V4 v1, f32 s)
{
    return v4f(v1.x - s, v1.y - s, v1.z - s, v1.w - s);
}

V2 v2_s_multi(V2 v1, f32 s)
{
    return v2d(v1.x * s, v1.y * s);
}

V3 v3_s_multi(V3 v1, f32 s)
{
    return v3f(v1.x * s, v1.y * s, v1.z * s);
}

V4 v4_s_multi(V4 v1, f32 s)
{
    return v4f(v1.x * s, v1.y * s, v1.z * s, v1.w * s);
}

V2 v2_multi(V2 v1, V2 v2)
{
    return v2d(v1.x * v2.x, v1.y * v2.y);
}

V3 v3_multi(V3 v1, V3 v2)
{
    return v3f(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}

V4 v4_multi(V4 v1, V4 v2)
{
    return v4f(v1.x * v2.x, v1.y * v2.y, v1.x * v2.y, v1.z);
}

V2 v2_s_div(V2 v1, f32 s)
{
    return v2d(v1.x / s, v1.y / s);
}

V3 v3_s_div(V3 v1, f32 s)
{
    return v3f(v1.x / s, v1.y / s, v1.z / s);
}

V4 v4_s_div(V4 v1, f32 s)
{
    return v4f(v1.x / s, v1.y / s, v1.z / s, v1.w / s);
}

void v2_add_equal(V2* v1, V2 v2)
{
    *v1 = v2_add(*v1, v2);
}

void v3_add_equal(V3* v1, V3 v2)
{
    *v1 = v3_add(*v1, v2);
}

void v4_add_equal(V4* v1, V4 v2)
{
    *v1 = v4_add(*v1, v2);
}

void v2_sub_equal(V2* v1, V2 v2)
{
    *v1 = v2_sub(*v1, v2);
}

void v3_sub_equal(V3* v1, V3 v2)
{
    *v1 = v3_sub(*v1, v2);
}

void v4_sub_equal(V4* v1, V4 v2)
{
    *v1 = v4_sub(*v1, v2);
}

void v2_s_add_equal(V2* v1, f32 s)
{
    *v1 = v2_s_add(*v1, s);
}

void v3_s_add_equal(V3* v1, f32 s)
{
    *v1 = v3_s_add(*v1, s);
}

void v4_s_add_equal(V4* v1, f32 s)
{
    *v1 = v4_s_add(*v1, s);
}

void v2_s_sub_equal(V2* v1, f32 s)
{
    *v1 = v2_s_sub(*v1, s);
}

void v3_s_sub_equal(V3* v1, f32 s)
{
    *v1 = v3_s_sub(*v1, s);
}

void v4_s_sub_equal(V4* v1, f32 s)
{
    *v1 = v4_s_sub(*v1, s);
}

void v2_s_multi_equal(V2* v1, f32 s)
{
    *v1 = v2_s_multi(*v1, s);
}

void v3_s_multi_equal(V3* v1, f32 s)
{
    *v1 = v3_s_multi(*v1, s);
}

void v4_s_multi_equal(V4* v1, f32 s)
{
    *v1 = v4_s_multi(*v1, s);
}

void v2_s_div_equal(V2* v1, f32 s)
{
    *v1 = v2_s_div(*v1, s);
}

void v3_s_div_equal(V3* v1, f32 s)
{
    *v1 = v3_s_div(*v1, s);
}

void v4_s_div_equal(V4* v1, f32 s)
{
    *v1 = v4_s_div(*v1, s);
}

b8 v2_equal(V2 v1, V2 v2)
{
    return v1.x == v2.x && v1.y == v2.y;
}

b8 v3_equal(V3 v1, V3 v2)
{
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

b8 v4_equal(V4 v1, V4 v2)
{
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w;
}

b8 v2_less(V2 v1, V2 v2)
{
    return (v2_sum(v1) < v2_sum(v2));
}

b8 v3_less(V3 v1, V3 v2)
{
    return (v3_sum(v1) < v3_sum(v2));
}

b8 v4_less(V4 v1, V4 v2)
{
    return (v4_sum(v1) < v4_sum(v2));
}

b8 v2_more(V2 v1, V2 v2)
{
    return (v2_sum(v1) > v2_sum(v2));
}

b8 v3_more(V3 v1, V3 v2)
{
    return (v3_sum(v1) > v3_sum(v2));
}

b8 v4_more(V4 v1, V4 v2)
{
    return (v4_sum(v1) > v4_sum(v2));
}

#if 0
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

V2 operator-(const Point2f& p1, const Point2f& p2)
{
    return { p1.x - p2.x, p1.y - p2.y };
}

V3 operator-(const Point3f& p1, const Point3f& p2)
{
    return {
        p1.x - p2.x,
        p1.y - p2.y,
        p1.z - p2.z,
    };
}

V4 operator-(const Point4f& p1, const Point4f& p2)
{
    return { p1.x - p2.x, p1.y - p2.y, p1.z - p2.z, p1.w - p2.w };
}

Point2f operator-(const Point2f& p, const V2& v)
{
    return { p.x - v.x, p.y - v.y };
}

Point3f operator-(const Point3f& p, const V3& v)
{
    return { p.x - v.x, p.y - v.y, p.z - v.z };
}

Point4f operator-(const Point4f& p, const V4& v)
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

Point2f& operator-=(Point2f& p, const V2& v)
{
    p = p - v;
    return p;
}

Point3f& operator-=(Point3f& p, const V3& v)
{
    p = p - v;
    return p;
}

Point4f& operator-=(Point4f& p, const V4& v)
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

f32 p2_sum(Point2f v)
{
    return (v.x + v.y);
}

f32 p2_sum(Point3f v)
{
    return (v.x + v.y + v.z);
}

f32 p2_sum(Point4f v)
{
    return (v.x + v.y + v.z + v.w);
}

b8 operator<(const Point2f& p1, const Point2f& p2)
{
    return (p2_sum(p1) < p2_sum(p2));
}
b8 operator<(const Point3f& p1, const Point3f& p2)
{
    return (p2_sum(p1) < p2_sum(p2));
}
b8 operator<(const Point4f& p1, const Point4f& p2)
{
    return (p2_sum(p1) < p2_sum(p2));
}

b8 operator>(const Point2f& p1, const Point2f& p2)
{
    return (p2_sum(p1) > p2_sum(p2));
}
b8 operator>(const Point3f& p1, const Point3f& p2)
{
    return (p3_sum(p1) > p3_sum(p2));
}
b8 operator>(const Point4f& p1, const Point4f& p2)
{
    return (p4_sum(p1) > p4_sum(p2));
}
#endif


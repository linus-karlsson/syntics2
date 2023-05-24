#include "vectors.h"

V2 v2d()
{
    V2 res = {};
    return res;
}

V2 v2i(f32 i)
{
    V2 res;
    res.x = i;
    res.y = i;
    return res;
}

V2 v2f(f32 x, f32 y)
{
    V2 res;
    res.x = x;
    res.y = y;
    return res;
}

V2 v2_v3(V3 v3)
{
    return v2f(v3.x, v3.y);
}

V2 v2_v4(V4 v4)
{
    return v2f(v4.x, v4.y);
}

V3 v3d()
{
    V3 res = {};
    return res;
}

V3 v3i(f32 i)
{
    V3 res;
    res.x = i;
    res.y = i;
    res.z = i;
    return res;
}

V3 v3f(f32 x, f32 y, f32 z)
{
    V3 res;
    res.x = x;
    res.y = y;
    res.z = z;
    return res;
}

V3 v3_v2(V2 v2)
{
    return v3f(v2.x, v2.y, 0.0f);
}

V3 v3_v2f(V2 v2, f32 z)
{
    return v3f(v2.x, v2.y, z);
}

V3 v3_v4(V4 v4)
{
    return v3f(v4.x, v4.y, v4.z);
}

V4 v4d()
{
    V4 res = {};
    return res;
}

V4 v4i(f32 i)
{
    V4 res;
    res.x = i;
    res.y = i;
    res.z = i;
    res.w = i;
    return res;
}

V4 v4f(f32 x, f32 y, f32 z, f32 w)
{
    V4 res;
    res.x = x;
    res.y = y;
    res.z = z;
    res.w = w;
    return res;
}

V4 v4_v2(V2 v2)
{
    return v4f(v2.x, v2.y, 0.0f, 0.0f);
}

V4 v4_v2f(V2 v2, f32 z, f32 w)
{
    return v4f(v2.x, v2.y, z, w);
}

V4 v4_v3(V3 v3)
{
    return v4f(v3.x, v3.y, v3.z, 0.0f);
}

V4 v4_v3f(V3 v3, f32 w)
{
    return v4f(v3.x, v3.y, v3.z, w);
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
    return v2f(v1.x + s, v1.y + s);
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
    return v2f(v1.x - s, v1.y - s);
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
    return v2f(v1.x * s, v1.y * s);
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
    return v2f(v1.x * v2.x, v1.y * v2.y);
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
    return v2f(v1.x / s, v1.y / s);
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

V2 operator+(const V2& v1, const V2& v2)
{
    return v2f(v1.x + v2.x, v1.y + v2.y);
}

V3 operator+(const V3& v1, const V3& v2)
{
    return v3f(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z );
}

V4 operator+(const V4& v1, const V4& v2)
{
    return v4f(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

V2 operator-(const V2& v1, const V2& v2)
{
    return v2f(v1.x - v2.x, v1.y - v2.y);
}

V3 operator-(const V3& v1, const V3& v2)
{
    return v3f(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z );
}

V4 operator-(const V4& v1, const V4& v2)
{
    return v4f(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
}

V2 operator+(const V2& v1, f32 s)
{
    return v2f(v1.x + s, v1.y + s);
}

V3 operator+(const V3& v1, f32 s)
{
    return v3f(v1.x + s, v1.y + s, v1.z + s);
}

V4 operator+(const V4& v1, f32 s)
{
    return v4f(v1.x + s, v1.y + s, v1.z + s, v1.w + s);
}

V2 operator-(const V2& v1, f32 s)
{
    return v2f(v1.x - s, v1.y - s);
}

V3 operator-(const V3& v1, f32 s)
{
    return v3f(v1.x - s, v1.y - s, v1.z - s);
}

V4 operator-(const V4& v1, f32 s)
{
    return v4f(v1.x - s, v1.y - s, v1.z - s, v1.w - s);
}

V2 operator-(const V2& v1)
{
    return v2f(-v1.x, -v1.y);
}

V3 operator-(const V3& v1)
{
    return v3f(-v1.x, -v1.y, -v1.z);
}

V4 operator-(const V4& v1)
{
    return v4f(-v1.x, -v1.y, -v1.z, -v1.w);
}

V2 operator*(const V2& v1, const V2& v2)
{
    return v2f(v1.x * v2.x, v1.y * v2.y);
}

V3 operator*(const V3& v1, const V3& v2)
{
    return v3f(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z );
}

V4 operator*(const V4& v1, const V4& v2)
{
    return v4f(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w);
}

V2& operator+=(V2& v1, const V2& v2)
{
    v1 = v1 + v2;
    return v1;
}

V3& operator+=(V3& v1, const V3& v2)
{
    v1 = v1 + v2;
    return v1;
}

V4& operator+=(V4& v1, const V4& v2)
{
    v1 = v1 + v2;
    return v1;
}

V2& operator+=(V2& v1, f32 s)
{
    v1 = v1 + s;
    return v1;
}

V3& operator+=(V3& v1, f32 s)
{
    v1 = v1 + s;
    return v1;
}

V4& operator+=(V4& v1, f32 s)
{
    v1 = v1 + s;
    return v1;
}

V2& operator-=(V2& v1, const V2& v2)
{
    v1 = v1 - v2;
    return v1;
}

V3& operator-=(V3& v1, const V3& v2)
{
    v1 = v1 - v2;
    return v1;
}

V4& operator-=(V4& v1, const V4& v2)
{
    v1 = v1 - v2;
    return v1;
}

V2& operator-=(V2& v1, f32 s)
{
    v1 = v1 - s;
    return v1;
}

V3& operator-=(V3& v1, f32 s)
{
    v1 = v1 - s;
    return v1;
}

V4& operator-=(V4& v1, f32 s)
{
    v1 = v1 - s;
    return v1;
}

V2& operator*=(V2& v1, const V2& v2)
{
    v1 = v1 * v2;
    return v1;
}
V3& operator*=(V3& v1, const V3& v2)
{
    v1 = v1 * v2;
    return v1;
}

V4& operator*=(V4& v1, const V4& v2)
{
    v1 = v1 * v2;
    return v1;
}

V2& operator*=(V2& v1, f32 s)
{
    v1 = v1 * s;
    return v1;
}

V3& operator*=(V3& v1, f32 s)
{
    v1 = v1 * s;
    return v1;
}

V4& operator*=(V4& v1, f32 s)
{
    v1 = v1 * s;
    return v1;
}

V2 operator*(const V2& v1, f32 s)
{
    return { v1.x * s, v1.y * s };
}

V3 operator*(const V3& v1, f32 s)
{
    return { v1.x * s, v1.y * s, v1.z * s };
}

V4 operator*(const V4& v1, f32 s)
{
    return { v1.x * s, v1.y * s, v1.z * s, v1.w * s };
}

V2 operator*(f32 s, const V2& v1)
{
    return { v1.x * s, v1.y * s };
}

V3 operator*(f32 s, const V3& v1)
{
    return { v1.x * s, v1.y * s, v1.z * s };
}

V4 operator*(f32 s, const V4& v1)
{
    return { v1.x * s, v1.y * s, v1.z * s, v1.w * s };
}

V2 operator/(const V2& v1, f32 s)
{
    return { v1.x / s, v1.y / s };
}

V3 operator/(const V3& v1, f32 s)
{
    return { v1.x / s, v1.y / s, v1.z / s };
}
V4 operator/(const V4& v1, f32 s)
{
    return { v1.x / s, v1.y / s, v1.z / s, v1.w / s };
}

b8 operator==(const V2& v1, const V2& v2)
{
    return v1.x == v2.x && v1.y == v2.y;
}

b8 operator==(const V3& v1, const V3& v2)
{
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

b8 operator==(const V4& v1, const V4& v2)
{
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w;
}

b8 operator<(const V2& v1, const V2& v2)
{
    return (v2_sum(v1) < v2_sum(v2));
}

b8 operator<(const V3& v1, const V3& v2)
{
    return (v3_sum(v1) < v3_sum(v2));
}

b8 operator<(const V4& v1, const V4& v2)
{
    return (v4_sum(v1) < v4_sum(v2));
}

b8 operator>(const V2& v1, const V2& v2)
{
    return (v2_sum(v1) > v2_sum(v2));
}

b8 operator>(const V3& v1, const V3& v2)
{
    return (v3_sum(v1) > v3_sum(v2));
}

b8 operator>(const V4& v1, const V4& v2)
{
    return (v4_sum(v1) > v4_sum(v2));
}

P2 p2d()
{
    P2 res = { 0 };
    return res;
}

P2 p2i(f32 i)
{
    P2 res = { i, i };
    return res;
}

P2 p2f(f32 x, f32 y)
{
    P2 res = { x, y };
    return res;
}

P2 p2_p3(P3 p3)
{
    return p2f(p3.x, p3.y);
}

P2 p2_p4(P4 p4)
{
    return p2f(p4.x, p4.y);
}

P3 p3d()
{
    P3 res = { 0 };
    return res;
}

P3 p3i(f32 i)
{
    P3 res = { i, i, i };
    return res;
}

P3 p3f(f32 x, f32 y, f32 z)
{
    P3 res = { x, y, z };
    return res;
}

P3 p3_p2(P2 p2)
{
    return p3f(p2.x, p2.y, 0.0f);
}

P3 v3_p2f(P2 p2, f32 z)
{
    return p3f(p2.x, p2.y, z);
}

P3 p3_p4(P4 p4)
{
    return p3f(p4.x, p4.y, p4.z);
}

P4 p4d()
{
    P4 res = { 0 };
    return res;
}

P4 p4i(f32 i)
{
    P4 res = { i, i, i, i };
    return res;
}

P4 p4f(f32 x, f32 y, f32 z, f32 w)
{
    P4 res = { x, y, z, w };
    return res;
}

P4 p4_p2(P2 p2)
{
    return p4f(p2.x, p2.y, 0.0f, 0.0f);
}

P4 p4_p2f(P2 p2, f32 z, f32 w)
{
    return p4f(p2.x, p2.y, z, w);
}

P4 p4_p3(P3 p3)
{
    return p4f(p3.x, p3.y, p3.z, 0.0f);
}

f32 p2_sum(P2 p)

{
    return (p.x + p.y);
}

f32 p3_sum(P3 p)
{
    return (p.x + p.y + p.z);
}

f32 p4_sum(P4 p)
{
    return (p.x + p.y + p.z + p.w);
}

P2 p2_add(P2 p1, P2 p2)
{
    return p2f(p1.x + p2.x, p1.y + p2.y);
}

P3 p3_add(P3 p1, P3 p2)
{
    return p3f(p1.x + p2.x, p1.y + p2.y, p1.z + p2.z);
}

P4 p4_add(P4 p1, P4 p2)
{
    return p4f(p1.x + p2.x, p1.y + p2.y, p1.z + p2.z, p1.w + p2.w);
}

V2 p2_sub(P2 p1, P2 p2)
{
    return v2f(p1.x - p2.x, p1.y - p2.y);
}

V3 p3_sub(P3 p1, P3 p2)
{
    return v3f(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z);
}

V4 p4_sub(P4 p1, P4 p2)
{
    return v4f(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z, p1.w - p2.w);
}

P2 p2_s_multi(P2 p1, f32 s)
{
    return p2f(p1.x * s, p1.y * s);
}

P3 p3_s_multi(P3 p1, f32 s)
{
    return p3f(p1.x * s, p1.y * s, p1.z * s);
}

P4 p4_s_multi(P4 p1, f32 s)
{
    return p4f(p1.x * s, p1.y * s, p1.z * s, p1.w * s);
}

Polygon2D poly2D(V2 pos, V2* p_arr, V2* n_arr, u32 n_sides)
{
    Polygon2D res = { 0 };
    res.pos = pos;
    res.points = p_arr;
    res.normals = n_arr;
    res.n_sides = n_sides;
    return res;
}

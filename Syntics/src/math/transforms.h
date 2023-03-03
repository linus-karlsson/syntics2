#pragma once
#include "matrix.h"
#include "vectors.h"

typedef enum Axis
{
    X,
    Y,
    Z,
    XY,
    XZ,
    YZ,
    XYZ,
} Axis;

f32 clampf32(f32 value, f32 min, f32 max);
f32 abs_f32(f32 value);
f32 clampf32_low(f32 value, f32 min);
V4 clamp(const V4& v1, const V4& min, const V4& max);

f32 len_v2(const V2& v2);
f32 len_v3(const V3& v3);

V3 lerp_v3(const V3& v1, const V3& v2, f32 t);

f32 dot(const V2& v2_1, const V2& v2_2);
f32 dot(const V3& v3_1, const V3& v3_2);
f32 angle(const V3& v3_1, const V3& v3_2);

V2 normalize(const V2& v2);
V3 normalize(const V3& v3);
V3 normalize(const V3& v3, f32 len);

V3 cross(const V3& v3_1, const V3& v3_2);

f32 distance(const V3& v1, const V3& v2);
f32 distance(const V2& v1, const V2& v2);

f32 minf32(f32 f1, f32 f2);
f32 maxf32(f32 f1, f32 f2);

f32 distance(const Point3f& p1, const Point3f& p2);
f32 distance_sqrt(const Point3f& p1, const Point3f& p2);
Point3f lerp(f32 s, const Point3f& p1, const Point3f& p2);
Point3f min_pf(const Point3f& p1, const Point3f& p2);
Point3f max_pf(const Point3f& p1, const Point3f& p2);
Point3f floor_pf(const Point3f& p);
Point3f ceil_pf(const Point3f& p);
Point3f abs_pf(const Point3f& p);

f32 radians(f32 deg);

f32 determinant(M3 m3);
f32 determinant(M4 m4);

M4 inverse(const M4& m);

M3 transpose(M3 m3);
M4 transpose(M4 m4);

M3 rotate(M3 m3, f64 rad);
M4 rotate(M4 m4, f64 rad, Axis axis);
V3 rotate(V3 v3, f64 rad, V3 normal);

M3 translate(M3 m3, Vec2 v2);
M4 translate(M4 m4, V3 v3);

M3 scale(M3 m3, Vec2 v2);
M4 scale(M4 m4, V3 v3);

M4 view(V3 eye, V3 center, V3 up);
M4 ortho(f32 left, f32 floor, f32 right, f32 ceiling, f32 near, f32 far);

M4 perspective(f32 fov, f32 aspect, f32 near, f32 far);


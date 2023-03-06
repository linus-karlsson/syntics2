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
V4 clamp(V4 v1, V4 min, V4 max);

f32 v2_len(V2 v2);
f32 v3_len(V3 v3);

V3 v3_lerp(V3 v1, V3 v2, f32 t);

f32 v2_dot(V2 v2_1, V2 v2_2);
f32 v3_dot(V3 v3_1, V3 v3_2);
f32 v3_angle(V3 v3_1, V3 v3_2);

V2 v2_normalize(V2 v2);
V3 v3_normalize(V3 v3);
V3 v3_normalize_len(V3 v3, f32 len);

V3 cross(V3 v3_1, V3 v3_2);

f32 v2_distance(V2 v1, V2 v2);
f32 v3_distance(V3 v1, V3 v2);

f32 minf32(f32 f1, f32 f2);
f32 maxf32(f32 f1, f32 f2);

f32 distance(Point3f p1, Point3f p2);
f32 distance_sqrt(Point3f p1, Point3f p2);
Point3f lerp(f32 s, Point3f p1, Point3f p2);
Point3f min_pf(Point3f p1, Point3f p2);
Point3f max_pf(Point3f p1, Point3f p2);
Point3f floor_pf(Point3f p);
Point3f ceil_pf(Point3f p);
Point3f abs_pf(Point3f p);

f32 radians(f32 deg);

f32 m3_determinant(M3 m3);
f32 m4_determinant(M4 m4);

M4 inverse(M4 m);

M3 m3_transpose(M3 m3);
M4 m4_transpose(M4 m4);

M3 m3_rotate(M3 m3, f64 rad);
M4 m4_rotate(M4 m4, f64 rad, Axis axis);
V3 v3_rotate(V3 v3, f64 rad, V3 normal);

M3 m3_translate(M3 m3, Vec2 v2);
M4 m4_translate(M4 m4, V3 v3);

M3 m3_scale(M3 m3, Vec2 v2);
M4 m4_scale(M4 m4, V3 v3);

M4 view(V3 eye, V3 center, V3 up);
M4 ortho(f32 left, f32 floor, f32 right, f32 ceiling, f32 near, f32 far);

M4 perspective(f32 fov, f32 aspect, f32 near, f32 far);


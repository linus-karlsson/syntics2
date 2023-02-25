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
Vec4 clamp(const Vec4& v1, const Vec4& min, const Vec4& max);
f32 vec3Len(const Vec3& v3);
f32 dot(const V2& v2_1, const V2& v2_2);
f32 dot(const V3& v3_1, const V3& v3_2);
f32 angle(const V3& v3_1, const V3& v3_2);
Vec3 normalize(const Vec3& v3);
Vec3 cross(const V3& v3_1, const V3& v3_2);

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

f32 determinant(Mat3f m3);
f32 determinant(Mat4f m4);

Mat4f inverse(const Mat4f& m);

Mat3f transpose(Mat3f m3);
Mat4f transpose(Mat4f m4);

Mat3f rotate(Mat3f m3, f64 rad);
Mat4f rotate(Mat4f m4, f64 rad, Axis axis);
Vec3 rotate(Vec3 v3, f64 rad, Vec3 normal);

Mat3f translate(Mat3f m3, Vec2 v2);
Mat4f translate(Mat4f m4, Vec3 v3);

Mat3f scale(Mat3f m3, Vec2 v2);
Mat4f scale(Mat4f m4, Vec3 v3);

Mat4f view(Vec3 eye, Vec3 center, Vec3 up);
Mat4f ortho(f32 left, f32 floor, f32 right, f32 ceiling, f32 near, f32 far);

Mat4f perspective(f32 fov, f32 aspect, f32 near, f32 far);


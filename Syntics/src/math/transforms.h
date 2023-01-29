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

float clampf32(float value, float min, float max);
float clampf32_low(float value, float min);
Vec4 clamp(const Vec4& v1, const Vec4& min, const Vec4& max);
float vec3Len(const Vec3& v3);
float dot(const Vec3& v3One, const Vec3& v3Two);
float angle(const Vec3& v3One, const Vec3& v3Two);
Vec3 normalize(const Vec3& v3);
Vec3 cross(const Vec3& v3One, const Vec3& v3Two);

float minf32(float f1, float f2);
float maxf32(float f1, float f2);

float distance(const Point3f& p1, const Point3f& p2);
float distance_sqrt(const Point3f& p1, const Point3f& p2);
Point3f lerp(float s, const Point3f& p1, const Point3f& p2);
Point3f min_pf(const Point3f& p1, const Point3f& p2);
Point3f max_pf(const Point3f& p1, const Point3f& p2);
Point3f floor_pf(const Point3f& p);
Point3f ceil_pf(const Point3f& p);
Point3f abs_pf(const Point3f& p);

float radians(float deg);

float determinant(Mat3f m3);
float determinant(Mat4f m4);

Mat4f inverse(const Mat4f& m);

Mat3f transpose(Mat3f m3);
Mat4f transpose(Mat4f m4);

Mat3f rotate(Mat3f m3, double rad);
Mat4f rotate(Mat4f m4, double rad, Axis axis);
Vec3 rotate(Vec3 v3, double rad, Vec3 normal);

Mat3f translate(Mat3f m3, Vec2 v2);
Mat4f translate(Mat4f m4, Vec3 v3);

Mat3f scale(Mat3f m3, Vec2 v2);
Mat4f scale(Mat4f m4, Vec3 v3);

Mat4f view(Vec3 eye, Vec3 center, Vec3 up);
Mat4f ortho(float left, float floor, float right, float ceiling, float near,
            float far);

Mat4f perspective(float fov, float aspect, float near, float far);


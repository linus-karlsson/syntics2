#pragma once
#include "matrix.h"
#include "vectors.h"

namespace synt {

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
    float vec3Len(const Vec3& v3);
    float dot(const Vec3& v3One, const Vec3& v3Two);
    float angle(const Vec3& v3One, const Vec3& v3Two);
    Vec3 normalize(const Vec3& v3);
    Vec3 cross(const Vec3& v3One, const Vec3& v3Two);

    float radians(float deg);

    float determinant(Mat3f m3);
    float determinant(Mat4f m4);

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

    Mat4f perspective(float fovy, float aspect, float near, float far);

} // namespace synt

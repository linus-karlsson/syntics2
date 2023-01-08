#include "transforms.h"
#include <math.h>
#include <stdio.h>

namespace synt {

static float abs_f32(float in)
{
    return in < 0.0f ? in * -1.0f : 1.0f;
}

float clampf32(float value, float min, float max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

Vec4 clamp(const Vec4& v1, const Vec4& min, const Vec4& max)
{
    return (Vec4){ clampf32(v1.x, min.x, max.x), clampf32(v1.y, min.y, max.y),
                   clampf32(v1.z, min.z, max.z), clampf32(v1.w, min.w, max.w) };
}

float minf32(float f1, float f2)
{
    return (f1 < f2) ? f1 : f2;
}

float maxf32(float f1, float f2)
{
    return (f1 > f2) ? f1 : f2;
}

float vec3Len(const Vec3& v3)
{
    return sqrtf((v3.x * v3.x) + (v3.y * v3.y) + (v3.z * v3.z));
}

float dot(const Vec3& v3_1, const Vec3& v3_2)
{
    return ((v3_1.x * v3_2.x) + (v3_1.y * v3_2.y) + (v3_1.z * v3_2.z));
}

float angle(const Vec3& v3_1, const Vec3& v3_2)
{
    return acosf(clampf32(dot(v3_1, v3_2), -1.0f, 1.0f));
}

Vec3 normalize(const Vec3& v3)
{
    Vec3 out;
    float length = vec3Len(v3);
    if (length > 0)
    {
        float inverse = 1 / length;
        out = (Vec3){ (v3.x * inverse), (v3.y * inverse), (v3.z * inverse) };
    }
    return out;
}

Vec3 cross(const Vec3& v3_1, const Vec3& v3_2)
{
    Vec3 out;

    out.x = ((v3_1.y * v3_2.z) - (v3_1.z * v3_2.y));
    out.y = -((v3_1.x * v3_2.z) - (v3_1.z * v3_2.x));
    out.z = ((v3_1.x * v3_2.y) - (v3_1.y * v3_2.x));

    return out;
}

float distance(const Point3f& p1, const Point3f& p2)
{
    return vec3Len(p1 - p2);
}

float distance_sqrt(const Point3f& p1, const Point3f& p2)
{
    return sqrt(vec3Len(p1 - p2));
}

Point3f lerp(float s, const Point3f& p1, const Point3f& p2)
{
    return ((1 - s) * p1) + (s * p2);
}

Point3f min_pf(const Point3f& p1, const Point3f& p2)
{
    return (Point3f){ minf32(p1.x, p2.x), minf32(p1.y, p2.y), minf32(p1.z, p2.z) };
}
Point3f max_pf(const Point3f& p1, const Point3f& p2)
{
    return (Point3f){ maxf32(p1.x, p2.x), maxf32(p1.x, p2.x), maxf32(p1.x, p2.x) };
}
Point3f floor_pf(const Point3f& p)
{
    return (Point3f){ floorf(p.x), floorf(p.y), floorf(p.z) };
}
Point3f ceil_pf(const Point3f& p)
{
    return (Point3f){ ceilf(p.x), ceilf(p.y), ceilf(p.z) };
}
Point3f abs_pf(const Point3f& p)
{
    return (Point3f){ abs_f32(p.x), abs_f32(p.y), abs_f32(p.z) };
}

float radians(float deg)
{
    const float PI = 3.1415936;
    return (deg * PI) / 180.0;
}

float determinant(Mat3f m3)
{
    float out;

    out = (m3.data[0][0] * m3.data[1][1] * m3.data[2][2]) +
          (m3.data[0][1] * m3.data[1][2] * m3.data[0][2]) +
          (m3.data[0][2] * m3.data[1][0] * m3.data[2][1]) -
          (m3.data[0][2] * m3.data[1][1] * m3.data[0][2]) -
          (m3.data[0][1] * m3.data[1][0] * m3.data[2][2]) -
          (m3.data[0][0] * m3.data[1][2] * m3.data[2][1]);

    return out;
}

float determinant(Mat4f m4)
{
    float out = 0;

    return out;
}

Mat4f inverse(const Mat4f& m)
{

    float coef00 = m.data[2][2] * m.data[3][3] - m.data[3][2] * m.data[2][3];
    float coef02 = m.data[1][2] * m.data[3][3] - m.data[3][2] * m.data[1][3];
    float coef03 = m.data[1][2] * m.data[2][3] - m.data[2][2] * m.data[1][3];

    float coef04 = m.data[2][1] * m.data[3][3] - m.data[3][1] * m.data[2][3];
    float coef06 = m.data[1][1] * m.data[3][3] - m.data[3][1] * m.data[1][3];
    float coef07 = m.data[1][1] * m.data[2][3] - m.data[2][1] * m.data[1][3];

    float coef08 = m.data[2][1] * m.data[3][2] - m.data[3][1] * m.data[2][2];
    float coef10 = m.data[1][1] * m.data[3][2] - m.data[3][1] * m.data[1][2];
    float coef11 = m.data[1][1] * m.data[2][2] - m.data[2][1] * m.data[1][2];

    float coef12 = m.data[2][0] * m.data[3][3] - m.data[3][0] * m.data[2][3];
    float coef14 = m.data[1][0] * m.data[3][3] - m.data[3][0] * m.data[1][3];
    float coef15 = m.data[1][0] * m.data[2][3] - m.data[2][0] * m.data[1][3];

    float coef16 = m.data[2][0] * m.data[3][2] - m.data[3][0] * m.data[2][2];
    float coef18 = m.data[1][0] * m.data[3][2] - m.data[3][0] * m.data[1][2];
    float coef19 = m.data[1][0] * m.data[2][2] - m.data[2][0] * m.data[1][2];

    float coef20 = m.data[2][0] * m.data[3][1] - m.data[3][0] * m.data[2][1];
    float coef22 = m.data[1][0] * m.data[3][1] - m.data[3][0] * m.data[1][1];
    float coef23 = m.data[1][0] * m.data[2][1] - m.data[2][0] * m.data[1][1];

    Vec4 fac0(coef00, coef00, coef02, coef03);
    Vec4 fac1(coef04, coef04, coef06, coef07);
    Vec4 fac2(coef08, coef08, coef10, coef11);
    Vec4 fac3(coef12, coef12, coef14, coef15);
    Vec4 fac4(coef16, coef16, coef18, coef19);
    Vec4 fac5(coef20, coef20, coef22, coef23);

    Vec4 vec0(m.data[1][0], m.data[0][0], m.data[0][0], m.data[0][0]);
    Vec4 vec1(m.data[1][1], m.data[0][1], m.data[0][1], m.data[0][1]);
    Vec4 vec2(m.data[1][2], m.data[0][2], m.data[0][2], m.data[0][2]);
    Vec4 vec3(m.data[1][3], m.data[0][3], m.data[0][3], m.data[0][3]);

    Vec4 inv0(vec1 * fac0 - vec2 * fac1 + vec3 * fac2);
    Vec4 inv1(vec0 * fac0 - vec2 * fac3 + vec3 * fac4);
    Vec4 inv2(vec0 * fac1 - vec1 * fac3 + vec3 * fac5);
    Vec4 inv3(vec0 * fac2 - vec1 * fac4 + vec2 * fac5);

    Vec4 signA(+1, -1, +1, -1);
    Vec4 signB(-1, +1, -1, +1);

    Vec4 in1 = inv0 * signA;
    Vec4 in2 = inv1 * signB;
    Vec4 in3 = inv2 * signA;
    Vec4 in4 = inv3 * signB;

    Mat4f inverse;

    inverse.data[0][0] = in1.x;
    inverse.data[1][0] = in1.y;
    inverse.data[2][0] = in1.z;
    inverse.data[3][0] = in1.w;

    inverse.data[0][1] = in2.x;
    inverse.data[1][1] = in2.y;
    inverse.data[2][1] = in2.z;
    inverse.data[3][1] = in2.w;

    inverse.data[0][2] = in3.x;
    inverse.data[1][2] = in3.y;
    inverse.data[2][2] = in3.z;
    inverse.data[3][2] = in3.w;

    inverse.data[0][3] = in4.x;
    inverse.data[1][3] = in4.y;
    inverse.data[2][3] = in4.z;
    inverse.data[3][3] = in4.w;

    Vec4 Row0(inverse.data[0][0], inverse.data[1][0], inverse.data[2][0],
              inverse.data[3][0]);

    Vec4 to(m.data[0][0], m.data[0][1], m.data[0][2], m.data[0][3]);
    Vec4 Dot0(to * Row0);
    float Dot1 = (Dot0.x + Dot0.y) + (Dot0.z + Dot0.w);

    float OneOverDeterminant = 1.0f / Dot1;

    return inverse * OneOverDeterminant;
}

Mat3f transpose(Mat3f m3)
{
    Mat3f out;

    out.data[0][0] = m3.data[0][0];
    out.data[0][1] = m3.data[1][0];
    out.data[0][2] = m3.data[2][0];

    out.data[1][0] = m3.data[0][1];
    out.data[1][1] = m3.data[1][1];
    out.data[1][2] = m3.data[2][1];

    out.data[2][0] = m3.data[0][2];
    out.data[2][1] = m3.data[1][2];
    out.data[2][2] = m3.data[2][2];

    return out;
}

Mat4f transpose(Mat4f m4)
{
    Mat4f out;

    out.data[0][0] = m4.data[0][0];
    out.data[0][1] = m4.data[1][0];
    out.data[0][2] = m4.data[2][0];
    out.data[0][3] = m4.data[3][0];

    out.data[1][0] = m4.data[0][1];
    out.data[1][1] = m4.data[1][1];
    out.data[1][2] = m4.data[2][1];
    out.data[1][3] = m4.data[3][1];

    out.data[2][0] = m4.data[0][2];
    out.data[2][1] = m4.data[1][2];
    out.data[2][2] = m4.data[2][2];
    out.data[2][3] = m4.data[3][2];

    out.data[3][0] = m4.data[0][3];
    out.data[3][1] = m4.data[1][3];
    out.data[3][2] = m4.data[2][3];
    out.data[3][3] = m4.data[3][3];

    return out;
}

Mat3f rotate(Mat3f m3, double rad)
{
    return (Mat3f){ { { (float)cos(rad), (float)sin(rad), m3.data[2][0] },
                      { (float)-sin(rad), (float)cos(rad), m3.data[2][1] },
                      { 0.0f, 0.0f, m3.data[2][2] } } };
}

static inline Mat4f RotateX(const Mat4f* m4, double rad)
{
    return (Mat4f){
        { { m4->data[0][0], m4->data[1][0], m4->data[2][0], m4->data[3][0] },
          { m4->data[0][1], (float)cos(rad), (float)sin(rad), m4->data[3][1] },
          { m4->data[0][2], (float)-sin(rad), (float)cos(rad), m4->data[3][2] },
          { m4->data[0][3], m4->data[1][3], m4->data[2][3], m4->data[3][3] } }
    };
}

static inline Mat4f RotateY(const Mat4f* m4, double rad)
{
    return (Mat4f){
        { { (float)cos(rad), m4->data[1][0], (float)-sin(rad), m4->data[3][0] },
          { m4->data[0][1], m4->data[1][1], m4->data[2][1], m4->data[3][1] },
          { (float)sin(rad), m4->data[1][2], (float)cos(rad), m4->data[3][2] },
          { m4->data[0][3], m4->data[1][3], m4->data[2][3], m4->data[3][3] } }
    };
}

static inline Mat4f RotateZ(const Mat4f* m4, double rad)
{
    return (Mat4f){
        { { (float)cos(rad), (float)sin(rad), m4->data[2][0], m4->data[3][0] },
          { (float)-sin(rad), (float)cos(rad), m4->data[2][1], m4->data[3][1] },
          { m4->data[0][2], m4->data[1][2], m4->data[2][2], m4->data[3][2] },
          { m4->data[0][3], m4->data[1][3], m4->data[2][3], m4->data[3][3] } }
    };
}

Mat4f rotate(Mat4f m4, double rad, Axis axis)
{

    switch (axis)
    {
        case X:
        {
            return RotateX(&m4, rad);
        }
        case Y:
        {
            return RotateY(&m4, rad);
        }
        case Z:
        {
            return RotateZ(&m4, rad);
        }
        case XY:
        {
            Mat4f tempX = RotateX(&m4, rad);
            Mat4f tempY = RotateY(&m4, rad);
            return tempY * tempX;
        }
        case XZ:
        {
            Mat4f tempX = RotateX(&m4, rad);
            return RotateZ(&tempX, rad);
        }
        case YZ:
        {
            Mat4f tempY = RotateY(&m4, rad);
            return RotateZ(&tempY, rad);
        }
        case XYZ:
        {
            Mat4f tempX = RotateX(&m4, rad);
            Mat4f tempY = RotateY(&tempX, rad);
            return RotateZ(&tempY, rad);
        }
        default: return m4;
    }
}

Vec3 rotate(Vec3 v3, double rad, Vec3 normal)
{
    float cos = (float)cosf(radians(rad));
    float sin = (float)sinf(radians(rad));

    return (v3 * cos + ((v3 * normal) * (1.0f - cos)) * normal +
            cross(v3, normal) * sin);
}

Mat3f translate(Mat3f m3, Vec2 v2)
{
    Mat3f out;

    out.data[0][0] = m3.data[0][0];
    out.data[0][1] = m3.data[0][1];
    out.data[0][2] = m3.data[0][2];

    out.data[1][0] = m3.data[1][0];
    out.data[1][1] = m3.data[1][1];
    out.data[1][2] = m3.data[1][2];

    const Vec3 temp1 = {
        m3.data[0][0] * v2.x,
        m3.data[0][1] * v2.x,
        m3.data[0][2] * v2.x,
    };
    const Vec3 temp2 = {
        m3.data[1][0] * v2.y,
        m3.data[1][1] * v2.y,
        m3.data[1][2] * v2.y,
    };
    const Vec3 temp3 = {
        m3.data[2][0],
        m3.data[2][1],
        m3.data[2][2],
    };
    const Vec3 res = ((temp1 + temp2) + temp3);

    out.data[2][0] = res.x;
    out.data[2][1] = res.y;
    out.data[2][2] = res.z;

    return out;
}

Mat4f translate(Mat4f m4, Vec3 v3)
{
    Mat4f out;

    out.data[0][0] = m4.data[0][0];
    out.data[0][1] = m4.data[0][1];
    out.data[0][2] = m4.data[0][2];
    out.data[0][3] = m4.data[0][3];

    out.data[1][0] = m4.data[1][0];
    out.data[1][1] = m4.data[1][1];
    out.data[1][2] = m4.data[1][2];
    out.data[1][3] = m4.data[1][3];

    out.data[2][0] = m4.data[2][0];
    out.data[2][1] = m4.data[2][1];
    out.data[2][2] = m4.data[2][2];
    out.data[2][3] = m4.data[2][3];

    const Vec4 temp1 = {
        m4.data[0][0] * v3.x,
        m4.data[0][1] * v3.x,
        m4.data[0][2] * v3.x,
        m4.data[0][3] * v3.x,
    };
    const Vec4 temp2 = {
        m4.data[1][0] * v3.y,
        m4.data[1][1] * v3.y,
        m4.data[1][2] * v3.y,
        m4.data[1][3] * v3.y,
    };
    const Vec4 temp3 = {
        m4.data[2][0] * v3.z,
        m4.data[2][1] * v3.z,
        m4.data[2][2] * v3.z,
        m4.data[2][3] * v3.z,
    };
    const Vec4 temp4 = {
        m4.data[3][0],
        m4.data[3][1],
        m4.data[3][2],
        m4.data[3][3],
    };
    const Vec4 res = ((temp1 + temp2) + (temp3 + temp4));

    out.data[3][0] = res.x;
    out.data[3][1] = res.y;
    out.data[3][2] = res.z;
    out.data[3][3] = res.w;

    return out;
}

Mat3f scale(Mat3f m3, Vec2 v2)
{
    Mat3f out;

    out.data[0][0] = m3.data[0][0] * v2.x;
    out.data[0][1] = m3.data[0][1] * v2.x;
    out.data[0][2] = m3.data[0][2] * v2.x;

    out.data[1][0] = m3.data[1][0] * v2.y;
    out.data[1][1] = m3.data[1][1] * v2.y;
    out.data[1][2] = m3.data[1][2] * v2.y;

    out.data[2][0] = m3.data[2][0];
    out.data[2][1] = m3.data[2][1];
    out.data[2][2] = m3.data[2][2];

    return out;
}

Mat4f scale(Mat4f m4, Vec3 v3)
{
    Mat4f out;

    out.data[0][0] = m4.data[0][0] * v3.x;
    out.data[0][1] = m4.data[0][1] * v3.x;
    out.data[0][2] = m4.data[0][2] * v3.x;
    out.data[0][3] = m4.data[0][3] * v3.x;

    out.data[1][0] = m4.data[1][0] * v3.y;
    out.data[1][1] = m4.data[1][1] * v3.y;
    out.data[1][2] = m4.data[1][2] * v3.y;
    out.data[1][3] = m4.data[1][3] * v3.y;

    out.data[2][0] = m4.data[2][0] * v3.z;
    out.data[2][1] = m4.data[2][1] * v3.z;
    out.data[2][2] = m4.data[2][2] * v3.z;
    out.data[2][3] = m4.data[2][3] * v3.z;

    out.data[3][0] = m4.data[3][0];
    out.data[3][1] = m4.data[3][1];
    out.data[3][2] = m4.data[3][2];
    out.data[3][3] = m4.data[3][3];

    return out;
}

Mat4f view(Vec3 eye, Vec3 center, Vec3 up)
{
    Mat4f out = mat4i(1.0f);

    const Vec3 temp1 = normalize(center - eye);
    const Vec3 temp2 = normalize(cross(temp1, up));
    const Vec3 temp3 = cross(temp2, temp1);

    out.data[0][0] = temp2.x;
    out.data[0][1] = temp3.x;
    out.data[0][2] = -temp1.x;

    out.data[1][0] = temp2.y;
    out.data[1][1] = temp3.y;
    out.data[1][2] = -temp1.y;

    out.data[2][1] = temp3.z;
    out.data[2][0] = temp2.z;
    out.data[2][2] = -temp1.z;

    out.data[3][0] = -dot(temp2, eye);
    out.data[3][1] = -dot(temp3, eye);
    out.data[3][2] = dot(temp1, eye);

    return out;
}

Mat4f ortho(float left, float floor, float right, float ceiling, float near,
            float far)
{
    Mat4f out = mat4i(1.0f);

    out.data[0][0] = 2.0f / (right - left);
    out.data[1][1] = 2.0f / (ceiling - floor);
    out.data[2][2] = -1.0f / (far - near);
    out.data[3][0] = -(right + left) / (right - left);
    out.data[3][1] = -(ceiling + floor) / (ceiling - floor);
    out.data[3][2] = -(far + near) / (far - near);

    return out;
}

Mat4f perspective(float fov, float aspect, float near, float far)
{
    Mat4f out = mat4i(0.0f);

    const float fov_temp = (float)tan(fov / 2.0f);

    out.data[0][0] = (1.0f / (aspect * fov_temp));
    out.data[1][1] = -(1.0f / (fov_temp));
    out.data[2][2] = (far / (near - far));
    out.data[2][3] = -1.0f;
    out.data[3][2] = (-(far * near) / (far - near));

    return out;
}

} // namespace synt

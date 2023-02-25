#include "transforms.h"
#include <math.h>
#include <stdio.h>

f32 abs_f32(f32 in)
{
    return in < 0.0f ? in * -1.0f : in;
}

f32 clampf32(f32 value, f32 min, f32 max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

f32 clampf32_low(f32 value, f32 min)
{
    if (value < min) return min;
    return value;
}

V4 clamp(const V4& v1, const V4& min, const V4& max)
{
    return { clampf32(v1.x, min.x, max.x), clampf32(v1.y, min.y, max.y),
             clampf32(v1.z, min.z, max.z), clampf32(v1.w, min.w, max.w) };
}

f32 minf32(f32 f1, f32 f2)
{
    return (f1 < f2) ? f1 : f2;
}

f32 maxf32(f32 f1, f32 f2)
{
    return (f1 > f2) ? f1 : f2;
}

f32 vec3Len(const V3& v3)
{
    return sqrtf((v3.x * v3.x) + (v3.y * v3.y) + (v3.z * v3.z));
}

f32 dot(const V2& v2_1, const V2& v2_2)
{
    return (v2_1.x * v2_2.x) + (v2_1.y * v2_2.y);
}

f32 dot(const V3& v3_1, const V3& v3_2)
{
    return ((v3_1.x * v3_2.x) + (v3_1.y * v3_2.y) + (v3_1.z * v3_2.z));
}

f32 angle(const V3& v3_1, const V3& v3_2)
{
    return acosf(clampf32(dot(v3_1, v3_2), -1.0f, 1.0f));
}

V3 normalize(const V3& v3)
{
    V3 out;
    f32 length = vec3Len(v3);
    if (length > 0)
    {
        f32 inverse = 1 / length;
        out = { (v3.x * inverse), (v3.y * inverse), (v3.z * inverse) };
    }
    return out;
}

V3 cross(const V3& v3_1, const V3& v3_2)
{
    V3 out;

    out.x = ((v3_1.y * v3_2.z) - (v3_1.z * v3_2.y));
    out.y = -((v3_1.x * v3_2.z) - (v3_1.z * v3_2.x));
    out.z = ((v3_1.x * v3_2.y) - (v3_1.y * v3_2.x));

    return out;
}

f32 distance(const Point3f& p1, const Point3f& p2)
{
    return vec3Len(p1 - p2);
}

f32 distance_sqrt(const Point3f& p1, const Point3f& p2)
{
    return sqrt(vec3Len(p1 - p2));
}

Point3f lerp(f32 s, const Point3f& p1, const Point3f& p2)
{
    return ((1 - s) * p1) + (s * p2);
}

Point3f min_pf(const Point3f& p1, const Point3f& p2)
{
    return { minf32(p1.x, p2.x), minf32(p1.y, p2.y), minf32(p1.z, p2.z) };
}
Point3f max_pf(const Point3f& p1, const Point3f& p2)
{
    return { maxf32(p1.x, p2.x), maxf32(p1.x, p2.x), maxf32(p1.x, p2.x) };
}
Point3f floor_pf(const Point3f& p)
{
    return { floorf(p.x), floorf(p.y), floorf(p.z) };
}
Point3f ceil_pf(const Point3f& p)
{
    return { ceilf(p.x), ceilf(p.y), ceilf(p.z) };
}
Point3f abs_pf(const Point3f& p)
{
    return { abs_f32(p.x), abs_f32(p.y), abs_f32(p.z) };
}

f32 radians(f32 deg)
{
    const f32 PI = 3.1415936;
    return (deg * PI) / 180.0;
}

f32 determinant(M3 m3)
{
    f32 out;

    out = (m3.data[0][0] * m3.data[1][1] * m3.data[2][2]) +
          (m3.data[0][1] * m3.data[1][2] * m3.data[0][2]) +
          (m3.data[0][2] * m3.data[1][0] * m3.data[2][1]) -
          (m3.data[0][2] * m3.data[1][1] * m3.data[0][2]) -
          (m3.data[0][1] * m3.data[1][0] * m3.data[2][2]) -
          (m3.data[0][0] * m3.data[1][2] * m3.data[2][1]);

    return out;
}

f32 determinant(M4 m4)
{
    f32 out = 0;

    return out;
}

M4 inverse(const M4& m)
{

    f32 coef00 = m.data[2][2] * m.data[3][3] - m.data[3][2] * m.data[2][3];
    f32 coef02 = m.data[1][2] * m.data[3][3] - m.data[3][2] * m.data[1][3];
    f32 coef03 = m.data[1][2] * m.data[2][3] - m.data[2][2] * m.data[1][3];

    f32 coef04 = m.data[2][1] * m.data[3][3] - m.data[3][1] * m.data[2][3];
    f32 coef06 = m.data[1][1] * m.data[3][3] - m.data[3][1] * m.data[1][3];
    f32 coef07 = m.data[1][1] * m.data[2][3] - m.data[2][1] * m.data[1][3];

    f32 coef08 = m.data[2][1] * m.data[3][2] - m.data[3][1] * m.data[2][2];
    f32 coef10 = m.data[1][1] * m.data[3][2] - m.data[3][1] * m.data[1][2];
    f32 coef11 = m.data[1][1] * m.data[2][2] - m.data[2][1] * m.data[1][2];

    f32 coef12 = m.data[2][0] * m.data[3][3] - m.data[3][0] * m.data[2][3];
    f32 coef14 = m.data[1][0] * m.data[3][3] - m.data[3][0] * m.data[1][3];
    f32 coef15 = m.data[1][0] * m.data[2][3] - m.data[2][0] * m.data[1][3];

    f32 coef16 = m.data[2][0] * m.data[3][2] - m.data[3][0] * m.data[2][2];
    f32 coef18 = m.data[1][0] * m.data[3][2] - m.data[3][0] * m.data[1][2];
    f32 coef19 = m.data[1][0] * m.data[2][2] - m.data[2][0] * m.data[1][2];

    f32 coef20 = m.data[2][0] * m.data[3][1] - m.data[3][0] * m.data[2][1];
    f32 coef22 = m.data[1][0] * m.data[3][1] - m.data[3][0] * m.data[1][1];
    f32 coef23 = m.data[1][0] * m.data[2][1] - m.data[2][0] * m.data[1][1];

    V4 fac0(coef00, coef00, coef02, coef03);
    V4 fac1(coef04, coef04, coef06, coef07);
    V4 fac2(coef08, coef08, coef10, coef11);
    V4 fac3(coef12, coef12, coef14, coef15);
    V4 fac4(coef16, coef16, coef18, coef19);
    V4 fac5(coef20, coef20, coef22, coef23);

    V4 vec0(m.data[1][0], m.data[0][0], m.data[0][0], m.data[0][0]);
    V4 vec1(m.data[1][1], m.data[0][1], m.data[0][1], m.data[0][1]);
    V4 vec2(m.data[1][2], m.data[0][2], m.data[0][2], m.data[0][2]);
    V4 vec3(m.data[1][3], m.data[0][3], m.data[0][3], m.data[0][3]);

    V4 inv0(vec1 * fac0 - vec2 * fac1 + vec3 * fac2);
    V4 inv1(vec0 * fac0 - vec2 * fac3 + vec3 * fac4);
    V4 inv2(vec0 * fac1 - vec1 * fac3 + vec3 * fac5);
    V4 inv3(vec0 * fac2 - vec1 * fac4 + vec2 * fac5);

    V4 signA(+1, -1, +1, -1);
    V4 signB(-1, +1, -1, +1);

    V4 in1 = inv0 * signA;
    V4 in2 = inv1 * signB;
    V4 in3 = inv2 * signA;
    V4 in4 = inv3 * signB;

    M4 inverse;

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

    V4 Row0(inverse.data[0][0], inverse.data[1][0], inverse.data[2][0],
            inverse.data[3][0]);

    V4 to(m.data[0][0], m.data[0][1], m.data[0][2], m.data[0][3]);
    V4 Dot0(to * Row0);
    f32 Dot1 = (Dot0.x + Dot0.y) + (Dot0.z + Dot0.w);

    f32 OneOverDeterminant = 1.0f / Dot1;

    return inverse * OneOverDeterminant;
}

M3 transpose(M3 m3)
{
    M3 out;

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

M4 transpose(M4 m4)
{
    M4 out;

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

M3 rotate(M3 m3, f64 rad)
{
    return { { { (f32)cos(rad), (f32)sin(rad), m3.data[2][0] },
               { (f32)-sin(rad), (f32)cos(rad), m3.data[2][1] },
               { 0.0f, 0.0f, m3.data[2][2] } } };
}

static inline M4 RotateX(const M4* m4, f64 rad)
{
    return { { { m4->data[0][0], m4->data[1][0], m4->data[2][0], m4->data[3][0] },
               { m4->data[0][1], (f32)cos(rad), (f32)sin(rad), m4->data[3][1] },
               { m4->data[0][2], (f32)-sin(rad), (f32)cos(rad), m4->data[3][2] },
               { m4->data[0][3], m4->data[1][3], m4->data[2][3],
                 m4->data[3][3] } } };
}

static inline M4 RotateY(const M4* m4, f64 rad)
{
    return { { { (f32)cos(rad), m4->data[1][0], (f32)-sin(rad), m4->data[3][0] },
               { m4->data[0][1], m4->data[1][1], m4->data[2][1], m4->data[3][1] },
               { (f32)sin(rad), m4->data[1][2], (f32)cos(rad), m4->data[3][2] },
               { m4->data[0][3], m4->data[1][3], m4->data[2][3],
                 m4->data[3][3] } } };
}

static inline M4 RotateZ(const M4* m4, f64 rad)
{
    return { { { (f32)cos(rad), (f32)sin(rad), m4->data[2][0], m4->data[3][0] },
               { (f32)-sin(rad), (f32)cos(rad), m4->data[2][1], m4->data[3][1] },
               { m4->data[0][2], m4->data[1][2], m4->data[2][2], m4->data[3][2] },
               { m4->data[0][3], m4->data[1][3], m4->data[2][3],
                 m4->data[3][3] } } };
}

M4 rotate(M4 m4, f64 rad, Axis axis)
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
            M4 tempX = RotateX(&m4, rad);
            M4 tempY = RotateY(&m4, rad);
            return tempY * tempX;
        }
        case XZ:
        {
            M4 tempX = RotateX(&m4, rad);
            return RotateZ(&tempX, rad);
        }
        case YZ:
        {
            M4 tempY = RotateY(&m4, rad);
            return RotateZ(&tempY, rad);
        }
        case XYZ:
        {
            M4 tempX = RotateX(&m4, rad);
            M4 tempY = RotateY(&tempX, rad);
            return RotateZ(&tempY, rad);
        }
        default: return m4;
    }
}

V3 rotate(V3 v3, f64 rad, V3 normal)
{
    f32 cos = (f32)cosf(radians(rad));
    f32 sin = (f32)sinf(radians(rad));

    return (v3 * cos + ((v3 * normal) * (1.0f - cos)) * normal +
            cross(v3, normal) * sin);
}

M3 translate(M3 m3, Vec2 v2)
{
    M3 out;

    out.data[0][0] = m3.data[0][0];
    out.data[0][1] = m3.data[0][1];
    out.data[0][2] = m3.data[0][2];

    out.data[1][0] = m3.data[1][0];
    out.data[1][1] = m3.data[1][1];
    out.data[1][2] = m3.data[1][2];

    const V3 temp1 = {
        m3.data[0][0] * v2.x,
        m3.data[0][1] * v2.x,
        m3.data[0][2] * v2.x,
    };
    const V3 temp2 = {
        m3.data[1][0] * v2.y,
        m3.data[1][1] * v2.y,
        m3.data[1][2] * v2.y,
    };
    const V3 temp3 = {
        m3.data[2][0],
        m3.data[2][1],
        m3.data[2][2],
    };
    const V3 res = ((temp1 + temp2) + temp3);

    out.data[2][0] = res.x;
    out.data[2][1] = res.y;
    out.data[2][2] = res.z;

    return out;
}

M4 translate(M4 m4, V3 v3)
{
    M4 out;

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

    const V4 temp1 = {
        m4.data[0][0] * v3.x,
        m4.data[0][1] * v3.x,
        m4.data[0][2] * v3.x,
        m4.data[0][3] * v3.x,
    };
    const V4 temp2 = {
        m4.data[1][0] * v3.y,
        m4.data[1][1] * v3.y,
        m4.data[1][2] * v3.y,
        m4.data[1][3] * v3.y,
    };
    const V4 temp3 = {
        m4.data[2][0] * v3.z,
        m4.data[2][1] * v3.z,
        m4.data[2][2] * v3.z,
        m4.data[2][3] * v3.z,
    };
    const V4 temp4 = {
        m4.data[3][0],
        m4.data[3][1],
        m4.data[3][2],
        m4.data[3][3],
    };
    const V4 res = ((temp1 + temp2) + (temp3 + temp4));

    out.data[3][0] = res.x;
    out.data[3][1] = res.y;
    out.data[3][2] = res.z;
    out.data[3][3] = res.w;

    return out;
}

M3 scale(M3 m3, Vec2 v2)
{
    M3 out;

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

M4 scale(M4 m4, V3 v3)
{
    M4 out;

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

M4 view(V3 eye, V3 center, V3 up)
{
    M4 out = mat4i(1.0f);

    const V3 temp1 = normalize(center - eye);
    const V3 temp2 = normalize(cross(temp1, up));
    const V3 temp3 = cross(temp2, temp1);

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

M4 ortho(f32 left, f32 floor, f32 right, f32 ceiling, f32 near, f32 far)
{
    M4 out = mat4i(1.0f);

    out.data[0][0] = 2.0f / (right - left);
    out.data[1][1] = 2.0f / (ceiling - floor);
    out.data[2][2] = -1.0f / (far - near);
    out.data[3][0] = -(right + left) / (right - left);
    out.data[3][1] = -(ceiling + floor) / (ceiling - floor);
    out.data[3][2] = -(far + near) / (far - near);

    return out;
}

M4 perspective(f32 fov, f32 aspect, f32 near, f32 far)
{
    M4 out = mat4i(0.0f);

    const f32 fov_temp = (f32)tan(fov / 2.0f);

    out.data[0][0] = (1.0f / (aspect * fov_temp));
    out.data[1][1] = -(1.0f / (fov_temp));
    out.data[2][2] = (far / (near - far));
    out.data[2][3] = -1.0f;
    out.data[3][2] = (-(far * near) / (far - near));

    return out;
}


#include "transforms.h"
#include "noise.h"
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

V4 clamp(V4 v1, V4 min, V4 max)
{
    return v4f(clampf32(v1.x, min.x, max.x), clampf32(v1.y, min.y, max.y),
               clampf32(v1.z, min.z, max.z), clampf32(v1.w, min.w, max.w));
}

f32 minf32(f32 f1, f32 f2)
{
    return (f1 < f2) ? f1 : f2;
}

f32 maxf32(f32 f1, f32 f2)
{
    return (f1 > f2) ? f1 : f2;
}

f32 v2_len(V2 v2)
{
    return sqrtf((v2.x * v2.x) + (v2.y * v2.y));
}

f32 v3_len(V3 v3)
{
    return sqrtf((v3.x * v3.x) + (v3.y * v3.y) + (v3.z * v3.z));
}

V3 lerp_v3(V3 v1, V3 v2, f32 t)
{
    return v3_add(v1, v3_s_multi(v3_sub(v2, v1), t));
}

f32 v2_dot(V2 v2_1, V2 v2_2)
{
    return (v2_1.x * v2_2.x) + (v2_1.y * v2_2.y);
}

f32 v3_dot(V3 v3_1, V3 v3_2)
{
    return ((v3_1.x * v3_2.x) + (v3_1.y * v3_2.y) + (v3_1.z * v3_2.z));
}

f32 v3_angle(V3 v3_1, V3 v3_2)
{
    return acosf(clampf32(v3_dot(v3_1, v3_2), -1.0f, 1.0f));
}

V2 v2_normalize(V2 v2)
{
    V2 out = v2d();
    f32 len = v2_len(v2);
    if (len > 0)
    {
        f32 inverse = 1 / len;
        out = v2f((v2.x * inverse), (v2.y * inverse));
    }
    return out;
}

V3 v3_normalize(V3 v3)
{
    V3 out = v3d();
    f32 length = v3_len(v3);
    if (length > 0)
    {
        f32 inverse = 1 / length;
        out = v3f((v3.x * inverse), (v3.y * inverse), (v3.z * inverse));
    }
    return out;
}

V3 v3_normalize_len(V3 v3, f32 len)
{
    V3 out = v3d();
    if (len > 0)
    {
        f32 inverse = 1 / len;
        out = v3f((v3.x * inverse), (v3.y * inverse), (v3.z * inverse));
    }
    return out;
}

V3 cross(V3 v3_1, V3 v3_2)
{
    V3 out;

    out.x = ((v3_1.y * v3_2.z) - (v3_1.z * v3_2.y));
    out.y = -((v3_1.x * v3_2.z) - (v3_1.z * v3_2.x));
    out.z = ((v3_1.x * v3_2.y) - (v3_1.y * v3_2.x));

    return out;
}

f32 v2_distance(V2 v1, V2 v2)
{
    return v2_len(v2_sub(v1, v2));
}

f32 v3_distance(V3 v1, V3 v2)
{
    return v3_len(v3_sub(v1, v2));
}

#if 0
f32 distance(Point3f p1, Point3f p2)
{
    return v3_len(p1 - p2);
}

f32 distance_sqrt(Point3f p1, Point3f p2)
{
    return sqrt(len_v3(p1 - p2));
}

Point3f lerp(f32 s, Point3f p1, Point3f p2)
{
    return ((1 - s) * p1) + (s * p2);
}

Point3f min_pf(Point3f p1, Point3f p2)
{
    return { minf32(p1.x, p2.x), minf32(p1.y, p2.y), minf32(p1.z, p2.z) };
}
Point3f max_pf(Point3f p1, Point3f p2)
{
    return { maxf32(p1.x, p2.x), maxf32(p1.x, p2.x), maxf32(p1.x, p2.x) };
}
Point3f floor_pf(Point3f p)
{
    return { floorf(p.x), floorf(p.y), floorf(p.z) };
}
Point3f ceil_pf(Point3f p)
{
    return { ceilf(p.x), ceilf(p.y), ceilf(p.z) };
}
Point3f abs_pf(Point3f p)
{
    return { abs_f32(p.x), abs_f32(p.y), abs_f32(p.z) };
}
#endif

f32 radians(f32 deg)
{
    const f32 PI = 3.1415936f;
    return (f32)((deg * PI) / 180.0);
}

f32 m3_determinant(M3 m3)
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

f32 m4_determinant(M4 m4)
{
    f32 out = 0;

    return out;
}

M4 inverse(M4 m)
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

    V4 fac0 = v4f(coef00, coef00, coef02, coef03);
    V4 fac1 = v4f(coef04, coef04, coef06, coef07);
    V4 fac2 = v4f(coef08, coef08, coef10, coef11);
    V4 fac3 = v4f(coef12, coef12, coef14, coef15);
    V4 fac4 = v4f(coef16, coef16, coef18, coef19);
    V4 fac5 = v4f(coef20, coef20, coef22, coef23);

    V4 vec0 = v4f(m.data[1][0], m.data[0][0], m.data[0][0], m.data[0][0]);
    V4 vec1 = v4f(m.data[1][1], m.data[0][1], m.data[0][1], m.data[0][1]);
    V4 vec2 = v4f(m.data[1][2], m.data[0][2], m.data[0][2], m.data[0][2]);
    V4 vec3 = v4f(m.data[1][3], m.data[0][3], m.data[0][3], m.data[0][3]);

    V4 inv0 = v4_add(v4_sub(v4_multi(vec1, fac0), v4_multi(vec2, fac1)),
                     v4_multi(vec3, fac2));
    V4 inv1 = v4_add(v4_sub(v4_multi(vec0, fac0), v4_multi(vec2, fac3)),
                     v4_multi(vec3, fac4));
    V4 inv2 = v4_add(v4_sub(v4_multi(vec0, fac1), v4_multi(vec1, fac4)),
                     v4_multi(vec3, fac5));
    V4 inv3 = v4_add(v4_sub(v4_multi(vec0, fac2), v4_multi(vec1, fac4)),
                     v4_multi(vec2, fac5));

    V4 signA = v4f(+1, -1, +1, -1);
    V4 signB = v4f(-1, +1, -1, +1);

    V4 in1 = v4_multi(inv0, signA);
    V4 in2 = v4_multi(inv1, signB);
    V4 in3 = v4_multi(inv2, signA);
    V4 in4 = v4_multi(inv3, signB);

    M4 inverse = m4d();

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

    V4 Row0 = v4f(inverse.data[0][0], inverse.data[1][0], inverse.data[2][0],
                  inverse.data[3][0]);

    V4 to = v4f(m.data[0][0], m.data[0][1], m.data[0][2], m.data[0][3]);
    V4 dot0 = v4_multi(to, Row0);
    f32 dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);

    if (!dot1)
    {
        return m;
    }

    f32 OneOverDeterminant = 1.0f / dot1;

    return m4_s_multi(inverse, OneOverDeterminant);
}

M3 m3_transpose(M3 m3)
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

M4 m4_transpose(M4 m4)
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

M3 m3_rotate(M3 m3, f64 rad)
{
    M3 res;
    res.data[0][0] = cosf((f32)rad);
    res.data[1][0] = -sinf((f32)rad);
    res.data[2][0] = m3.data[2][0];
    res.data[0][1] = sinf((f32)rad);
    res.data[1][1] = cosf((f32)rad);
    res.data[2][1] = m3.data[2][1];
    res.data[0][2] = m3.data[0][2];
    res.data[1][2] = m3.data[1][2];
    res.data[2][2] = m3.data[2][2];
    return res;
}

static inline M4 rotate_x(const M4* m4, f64 rad)
{
    M4 res;
    res.data[0][0] = m4->data[0][0];
    res.data[1][0] = m4->data[1][0];
    res.data[2][0] = m4->data[2][0];
    res.data[3][0] = m4->data[3][0];
    res.data[0][1] = m4->data[0][1];
    res.data[1][1] = cosf((f32)rad);
    res.data[2][1] = -sinf((f32)rad);
    res.data[3][1] = m4->data[3][1];
    res.data[0][2] = m4->data[0][2];
    res.data[1][2] = sinf((f32)rad);
    res.data[2][2] = cosf((f32)rad);
    res.data[3][2] = m4->data[3][2];
    res.data[0][3] = m4->data[0][3];
    res.data[1][3] = m4->data[1][3];
    res.data[2][3] = m4->data[2][3];
    res.data[3][3] = m4->data[3][3];
    return res;
}

static inline M4 rotate_y(const M4* m4, f64 rad)
{
    M4 res;
    res.data[0][0] = cosf((f32)rad);
    res.data[1][0] = m4->data[1][0];
    res.data[2][0] = sinf((f32)rad);
    res.data[3][0] = m4->data[3][0];
    res.data[0][1] = m4->data[0][1];
    res.data[1][1] = m4->data[1][1];
    res.data[2][1] = m4->data[2][1];
    res.data[3][1] = m4->data[3][1];
    res.data[0][2] = -sinf((f32)rad);
    res.data[1][2] = m4->data[1][2];
    res.data[2][2] = cosf((f32)rad);
    res.data[3][2] = m4->data[3][2];
    res.data[0][3] = m4->data[0][3];
    res.data[1][3] = m4->data[1][3];
    res.data[2][3] = m4->data[2][3];
    res.data[3][3] = m4->data[3][3];
    return res;
}

static inline M4 rotate_z(const M4* m4, f64 rad)
{
    M4 res;
    res.data[0][0] = cosf((f32)rad);
    res.data[1][0] = -sinf((f32)rad);
    res.data[2][0] = m4->data[2][0];
    res.data[3][0] = m4->data[3][0];
    res.data[0][1] = sinf((f32)rad);
    res.data[1][1] = cosf((f32)rad);
    res.data[2][1] = m4->data[2][1];
    res.data[3][1] = m4->data[3][1];
    res.data[0][2] = m4->data[0][2];
    res.data[1][2] = m4->data[1][2];
    res.data[2][2] = m4->data[2][2];
    res.data[3][2] = m4->data[3][2];
    res.data[0][3] = m4->data[0][3];
    res.data[1][3] = m4->data[1][3];
    res.data[2][3] = m4->data[2][3];
    res.data[3][3] = m4->data[3][3];
    return res;
}

M4 m4_rotate(M4 m4, f64 rad, Axis axis)
{

    switch (axis)
    {
        case X:
        {
            return rotate_x(&m4, rad);
        }
        case Y:
        {
            return rotate_y(&m4, rad);
        }
        case Z:
        {
            return rotate_z(&m4, rad);
        }
        default: return m4;
    }
}

V3 v3_rotate(V3 v3, f64 rad, V3 normal)
{
    f32 cos = cosf(radians((f32)rad));
    f32 sin = sinf(radians((f32)rad));

    return v3_add(
        v3_add(v3_s_multi(v3, cos),
               v3_multi(v3_s_multi(v3_multi(v3, normal), (1.0f - cos)), normal)),
        v3_s_multi(cross(v3, normal), sin));
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
    const V3 res = v3_add(v3_add(temp1, temp2), temp3);

    out.data[2][0] = res.x;
    out.data[2][1] = res.y;
    out.data[2][2] = res.z;

    return out;
}

M4 m4_translate(M4 m4, V3 v3)
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
    const V4 res = v4_add(v4_add(temp1, temp2), v4_add(temp3, temp4));

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

M4 m4_scale(M4 m4, V3 v3)
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
    M4 out = m4i(1.0f);

    const V3 temp1 = v3_normalize(v3_sub(center, eye));
    const V3 temp2 = v3_normalize(cross(temp1, up));
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

    out.data[3][0] = -v3_dot(temp2, eye);
    out.data[3][1] = -v3_dot(temp3, eye);
    out.data[3][2] = v3_dot(temp1, eye);

    return out;
}

M4 ortho(f32 left, f32 floor, f32 right, f32 ceiling, f32 near, f32 far)
{
    M4 out = m4i(1.0f);

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
    // TODO: perspective bug
    M4 out = m4i(1.0f);

    const f32 fov_temp = (f32)tan(fov / 2.0f);

    out.data[0][0] = (1.0f / (aspect * fov_temp));
    out.data[1][1] = -(1.0f / (fov_temp));
    out.data[2][2] = (far / (near - far));
    out.data[2][3] = -1.0f;
    out.data[3][2] = (-(far * near) / (far - near));

    return out;
}


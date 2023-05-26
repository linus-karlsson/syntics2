#include "transforms.h"
#include "defines.h"
#include "noise.h"
#include "logging.h"
#include <math.h>

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

V3 v3_lerp(V3 v1, V3 v2, f32 t)
{
    return v3_add(v1, v3_s_multi(v3_sub(v2, v1), t));
}

f32 v2_dot(V2 v1, V2 v2)
{
    return (v1.x * v2.x) + (v1.y * v2.y);
}

f32 v3_dot(V3 v1, V3 v2)
{
    return ((v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z));
}

f32 v3_angle(V3 v1, V3 v2)
{
    f32 len_v1 = v3_len(v1);
    f32 len_v2 = v3_len(v2);

    if (len_v1 > EPSILON && len_v2 > EPSILON)
    {
        return acosf(v3_dot(v1, v2) / (len_v1 * len_v2));
    }
    return 0.0f;
}

V2 v2_normalize(V2 v2)
{
    V2 out = v2d();
    f32 len = v2_len(v2);
    if (len > EPSILON)
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
    if (length > EPSILON)
    {
        f32 inverse = 1.0f / length;
        out = v3f((v3.x * inverse), (v3.y * inverse), (v3.z * inverse));
    }
    return out;
}

V3 v3_normalize_len(V3 v3, f32 len)
{
    V3 out = v3d();
    if (len > EPSILON)
    {
        f32 inverse = 1 / len;
        out = v3f((v3.x * inverse), (v3.y * inverse), (v3.z * inverse));
    }
    return out;
}

f32 v2_cross(V2 v1, V2 v2)
{
    f32 out = (v1.x * v2.y) - (v1.y * v2.x);
    return out;
}

V3 v3_cross(V3 v1, V3 v2)
{
    V3 out;

    out.x = (v1.y * v2.z) - (v1.z * v2.y);
    out.y = (v1.z * v2.x) - (v1.x * v2.z);
    out.z = (v1.x * v2.y) - (v1.y * v2.x);

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

f32 p3_distance(P3 p1, P3 p2)
{
    return v3_len(p3_sub(p1, p2));
}

f32 p3_distance_sqrt(P3 p1, P3 p2)
{
    return sqrtf(v3_len(p3_sub(p1, p2)));
}

P3 p3_lerp(P3 p1, P3 p2, f32 t)
{
    // TODO: should implement these functions
    V3 v3 = v3_s_multi(p3_sub(p2, p1), t);
    return p3_add(p1, *(P3*)&v3);
}

P3 p3_min(P3 p1, P3 p2)
{
    return p3f(minf32(p1.x, p2.x), minf32(p1.y, p2.y), minf32(p1.z, p2.z));
}
P3 p3_max(P3 p1, P3 p2)
{
    return p3f(maxf32(p1.x, p2.x), maxf32(p1.y, p2.y), maxf32(p1.z, p2.z));
}
P3 p3_floor(P3 p)
{
    return p3f(floorf(p.x), floorf(p.y), floorf(p.z));
}
P3 p3_ceil(P3 p)
{
    return p3f(ceilf(p.x), ceilf(p.y), ceilf(p.z));
}
P3 p3_abs(P3 p)
{
    return p3f(abs_f32(p.x), abs_f32(p.y), abs_f32(p.z));
}

f32 radians(f32 deg)
{
    return (f32)((deg * PI) / 180.0f);
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

M3 m3_transpose(M3 m3)
{
    M3 out;

    out.data[0][1] = m3.data[1][0];
    out.data[0][2] = m3.data[2][0];

    out.data[1][0] = m3.data[0][1];
    out.data[1][2] = m3.data[2][1];

    out.data[2][0] = m3.data[0][2];
    out.data[2][1] = m3.data[1][2];

    return out;
}

M4 m4_transpose(M4 m4)
{
    M4 out;

    out.data[0][1] = m4.data[1][0];
    out.data[0][2] = m4.data[2][0];
    out.data[0][3] = m4.data[3][0];

    out.data[1][0] = m4.data[0][1];
    out.data[1][2] = m4.data[2][1];
    out.data[1][3] = m4.data[3][1];

    out.data[2][0] = m4.data[0][2];
    out.data[2][1] = m4.data[1][2];
    out.data[2][3] = m4.data[3][2];

    out.data[3][0] = m4.data[0][3];
    out.data[3][1] = m4.data[1][3];
    out.data[3][2] = m4.data[2][3];

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

#if 0
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
#endif
static inline M4 rotate_x(f64 rad)
{
    M4 res = m4i(1.0f);
    res.data[1][1] = cosf((f32)rad);
    res.data[2][1] = -sinf((f32)rad);
    res.data[1][2] = sinf((f32)rad);
    res.data[2][2] = cosf((f32)rad);
    return res;
}

static inline M4 rotate_y(f64 rad)
{
    M4 res = m4i(1.0f);
    res.data[0][0] = cosf((f32)rad);
    res.data[2][0] = sinf((f32)rad);
    res.data[0][2] = -sinf((f32)rad);
    res.data[2][2] = cosf((f32)rad);
    return res;
}

static inline M4 rotate_z(f64 rad)
{
    M4 res = m4i(1.0f);
    res.data[0][0] = cosf((f32)rad);
    res.data[1][0] = -sinf((f32)rad);
    res.data[0][1] = sinf((f32)rad);
    res.data[1][1] = cosf((f32)rad);
    return res;
}

M4 m4_rotate(f64 rad, Axis axis)
{

    switch (axis)
    {
        case X:
        {
            return rotate_x(rad);
        }
        case Y:
        {
            return rotate_y(rad);
        }
        case Z:
        {
            return rotate_z(rad);
        }
        default: return m4i(1.0f);
    }
}

V3 v3_rotate(V3 v3, f64 rad, V3 normal)
{
    f32 cos = cosf(radians((f32)rad));
    f32 sin = sinf(radians((f32)rad));

    return v3_add(
        v3_add(v3_s_multi(v3, cos),
               v3_multi(v3_s_multi(v3_multi(v3, normal), (1.0f - cos)), normal)),
        v3_s_multi(v3_cross(v3, normal), sin));
}

M3 translate(V2 v)
{
    M3 out = m3i(1.0f);
    out.data[2][0] = v.x;
    out.data[2][1] = v.y;
    return out;
}

M4 m4_translate(V3 v3)
{
    M4 out = m4i(1.0f);
    out.data[3][0] = v3.x;
    out.data[3][1] = v3.y;
    out.data[3][2] = v3.z;
    return out;
}

M3 scale(V2 v)
{
    M3 out = m3i(1.0f);

    out.data[0][0] *= v.x;
    out.data[1][1] *= v.y;

    return out;
}

M4 m4_scale(V3 v)
{
    M4 out = m4i(1.0f);

    out.data[0][0] *= v.x;
    out.data[1][1] *= v.y;
    out.data[2][2] *= v.z;

    return out;
}

M4 ortho(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far)
{
    M4 out = m4i(1.0f);

    out.data[0][0] = 2.0f / (right - left);
    out.data[1][1] = 2.0f / (top - bottom);
    out.data[2][2] = 2.0f / (near - far);

    out.data[3][0] = (left + right) / (left - right);
    out.data[3][1] = (bottom + top) / (bottom - top);
    out.data[3][2] = (near + far) / (near - far);

    return out;
}

M4 view(V3 eye, V3 center, V3 up)
{
    M4 out = m4i(1.0f);

    const V3 temp1 = v3_normalize(v3_sub(center, eye));
    const V3 temp2 = v3_normalize(v3_cross(temp1, up));
    const V3 temp3 = v3_cross(temp2, temp1);

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

M4 perspective(f32 fov, f32 aspect, f32 near, f32 far)
{
    const f32 f = 1.0f / tanf(fov * 0.5f);
    const f32 X = f / aspect;
    const f32 Y = -f;
    const f32 Z1 = (far + near) / (near - far);
    const f32 Z2 = (2.0f * far * near) / (near - far);

    M4 out = m4f(X, 0, 0, 0, 0, Y, 0, 0, 0, 0, Z1, Z2, 0, 0, -1.0f, 0.0f);

    return out;
}

M4 inverse(M4 m)
{
    f32 sf00 = m.data[2][2] * m.data[3][3] - m.data[3][2] * m.data[2][3];
    f32 sf01 = m.data[2][1] * m.data[3][3] - m.data[3][1] * m.data[2][3];
    f32 sf02 = m.data[2][1] * m.data[3][2] - m.data[3][1] * m.data[2][2];
    f32 sf03 = m.data[2][0] * m.data[3][3] - m.data[3][0] * m.data[2][3];
    f32 sf04 = m.data[2][0] * m.data[3][2] - m.data[3][0] * m.data[2][2];
    f32 sf05 = m.data[2][0] * m.data[3][1] - m.data[3][0] * m.data[2][1];
    f32 sf06 = m.data[1][2] * m.data[3][3] - m.data[3][2] * m.data[1][3];
    f32 sf07 = m.data[1][1] * m.data[3][3] - m.data[3][1] * m.data[1][3];
    f32 sf08 = m.data[1][1] * m.data[3][2] - m.data[3][1] * m.data[1][2];
    f32 sf09 = m.data[1][0] * m.data[3][3] - m.data[3][0] * m.data[1][3];
    f32 sf10 = m.data[1][0] * m.data[3][2] - m.data[3][0] * m.data[1][2];
    f32 sf11 = m.data[1][0] * m.data[3][1] - m.data[3][0] * m.data[1][1];
    f32 sf12 = m.data[1][2] * m.data[2][3] - m.data[2][2] * m.data[1][3];
    f32 sf13 = m.data[1][1] * m.data[2][3] - m.data[2][1] * m.data[1][3];
    f32 sf14 = m.data[1][1] * m.data[2][2] - m.data[2][1] * m.data[1][2];
    f32 sf15 = m.data[1][0] * m.data[2][3] - m.data[2][0] * m.data[1][3];
    f32 sf16 = m.data[1][0] * m.data[2][2] - m.data[2][0] * m.data[1][2];
    f32 sf17 = m.data[1][0] * m.data[2][1] - m.data[2][0] * m.data[1][1];

    M4 res = {};
    res.data[0][0] =
        +(m.data[1][1] * sf00 - m.data[1][2] * sf01 + m.data[1][3] * sf02);
    res.data[0][1] =
        -(m.data[1][0] * sf00 - m.data[1][2] * sf03 + m.data[1][3] * sf04);
    res.data[0][2] =
        +(m.data[1][0] * sf01 - m.data[1][1] * sf03 + m.data[1][3] * sf05);
    res.data[0][3] =
        -(m.data[1][0] * sf02 - m.data[1][1] * sf04 + m.data[1][2] * sf05);

    res.data[1][0] =
        -(m.data[0][1] * sf00 - m.data[0][2] * sf01 + m.data[0][3] * sf02);
    res.data[1][1] =
        +(m.data[0][0] * sf00 - m.data[0][2] * sf03 + m.data[0][3] * sf04);
    res.data[1][2] =
        -(m.data[0][0] * sf01 - m.data[0][1] * sf03 + m.data[0][3] * sf05);
    res.data[1][3] =
        +(m.data[0][0] * sf02 - m.data[0][1] * sf04 + m.data[0][2] * sf05);

    res.data[2][0] =
        +(m.data[0][1] * sf06 - m.data[0][2] * sf07 + m.data[0][3] * sf08);
    res.data[2][1] =
        -(m.data[0][0] * sf06 - m.data[0][2] * sf09 + m.data[0][3] * sf10);
    res.data[2][2] =
        +(m.data[0][0] * sf07 - m.data[0][1] * sf09 + m.data[0][3] * sf11);
    res.data[2][3] =
        -(m.data[0][0] * sf08 - m.data[0][1] * sf10 + m.data[0][2] * sf11);

    res.data[3][0] =
        -(m.data[0][1] * sf12 - m.data[0][2] * sf13 + m.data[0][3] * sf14);
    res.data[3][1] =
        +(m.data[0][0] * sf12 - m.data[0][2] * sf15 + m.data[0][3] * sf16);
    res.data[3][2] =
        -(m.data[0][0] * sf13 - m.data[0][1] * sf15 + m.data[0][3] * sf17);
    res.data[3][3] =
        +(m.data[0][0] * sf14 - m.data[0][1] * sf16 + m.data[0][2] * sf17);

    f32 d = +m.data[0][0] * res.data[0][0] + m.data[0][1] * res.data[0][1] +
            m.data[0][2] * res.data[0][2] + m.data[0][3] * res.data[0][3];

    res = res / d;

    // Column major
    return m4_transpose(res);
}

#if 0
b8 is_quad2d_convex(Quad2D q)
{
    P2* A = &q.points[0];
    P2* B = &q.points[1];
    P2* C = &q.points[2];
    P2* D = &q.points[3];

    // Source Christer Ericson Real Time Collision Detection
    V3 v0 = v3_cross(v3_v2(p2_sub(*D, *B)), v3_v2(p2_sub(*A, *B)));
    V3 v1 = v3_cross(v3_v2(p2_sub(*D, *B)), v3_v2(p2_sub(*C, *B)));

    if (v3_dot(v0, v1) >= 0.0f) return false;

    V3 v2 = v3_cross(v3_v2(p2_sub(*C, *A)), v3_v2(p2_sub(*D, *A)));
    V3 v3 = v3_cross(v3_v2(p2_sub(*C, *A)), v3_v2(p2_sub(*B, *A)));

    b8 res = v3_dot(v2, v3) < 0.0f;

    return res;
#if 0
    float cp0 = v2_cross(p2_sub(*B, *A), p2_sub(*C, *B));
    float cp1 = v2_cross(p2_sub(*C, *B), p2_sub(*D, *C));
    float cp2 = v2_cross(p2_sub(*D, *C), p2_sub(*A, *D));
    float cp3 = v2_cross(p2_sub(*A, *D), p2_sub(*B, *A));

    b8 res = cp0 * cp1 > 0 && cp1 * cp2 > 0 && cp2 * cp3 > 0;

    return res;
#endif
}
#endif

b8 is_poly2d_convex(Polygon2D p)
{
    if (p.n_sides < 3) return false;

    b32 positive_or_neg = true;
    for_range(i, p.n_sides)
    {
        u32 j = (i + 1) % p.n_sides;
        u32 k = (j + 1) % p.n_sides;
        V2* f0 = &p.points[i];
        V2* f1 = &p.points[j];
        V2* f2 = &p.points[k];
        float cp0 = v2_cross(v2_sub(*f1, *f0), v2_sub(*f2, *f1));
        if (i == 0 && cp0 < 0)
        {
            positive_or_neg = false;
        }
        else if ((cp0 < 0 && positive_or_neg) || (cp0 >= 0 && !positive_or_neg))
        {
            return false;
        }
    }
    return true;
}

Plane plane(P3 a, P3 b, P3 c)
{
    Plane res = { 0 };
    res.n = v3_normalize(v3_cross(p3_sub(b, a), p3_sub(c, a)));
    res.d = v3_dot(res.n, v3f(a.x, a.y, a.z));
    return res;
}

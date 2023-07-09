#include "matrix.h"
#include "defines.h"

M2 m2d()
{
    return m2i(1.0f);
}

M2 m2i(f32 i)
{
    M2 res = { 0 };
    res.data[0][0] = i;
    res.data[1][1] = i;
    return res;
}

M3 m3d()
{
    return m3i(1.0f);
}

M3 m3i(f32 i)
{
    M3 res = { 0 };
    res.data[0][0] = i;
    res.data[1][1] = i;
    res.data[2][2] = i;
    return res;
}

M3 m3f(f32 f0, f32 f1, f32 f2, f32 f3, f32 f4, f32 f5, f32 f6, f32 f7, f32 f8)
{
    M3 res;

    res.data[0][0] = f0;
    res.data[1][0] = f1;
    res.data[2][0] = f2;

    res.data[0][1] = f3;
    res.data[1][1] = f4;
    res.data[2][1] = f5;

    res.data[0][2] = f6;
    res.data[1][2] = f7;
    res.data[2][2] = f8;

    return res;
}

M3 m3_m4(M4 matrix)
{
    M3 res = {};
    res.data[0][0] = matrix.data[0][0];
    res.data[0][1] = matrix.data[0][1];
    res.data[0][2] = matrix.data[0][2];

    res.data[1][0] = matrix.data[1][0];
    res.data[1][1] = matrix.data[1][1];
    res.data[1][2] = matrix.data[1][2];

    res.data[2][0] = matrix.data[2][0];
    res.data[2][1] = matrix.data[2][1];
    res.data[2][2] = matrix.data[2][2];

    return res;
}

M4 m4d()
{
    return m4i(1.0f);
}

M4 m4i(f32 i)
{
    M4 res = { 0 };
    res.data[0][0] = i;
    res.data[1][1] = i;
    res.data[2][2] = i;
    res.data[3][3] = i;
    return res;
}

M4 m4f(f32 f0, f32 f1, f32 f2, f32 f3, f32 f4, f32 f5, f32 f6, f32 f7, f32 f8,
       f32 f9, f32 f10, f32 f11, f32 f12, f32 f13, f32 f14, f32 f15)
{

    M4 res;
    res.data[0][0] = f0;
    res.data[0][1] = f4;
    res.data[0][2] = f8;
    res.data[0][3] = f12;

    res.data[1][0] = f1;
    res.data[1][1] = f5;
    res.data[1][2] = f9;
    res.data[1][3] = f13;

    res.data[2][0] = f2;
    res.data[2][1] = f6;
    res.data[2][2] = f10;
    res.data[2][3] = f14;

    res.data[3][0] = f3;
    res.data[3][1] = f7;
    res.data[3][2] = f11;
    res.data[3][3] = f15;
    return res;
}

M4 m4_v4(V4 c0, V4 c1, V4 c2, V4 c3)
{
    M4 res;
    res.data[0][0] = c0.x;
    res.data[0][1] = c0.y;
    res.data[0][2] = c0.z;
    res.data[0][3] = c0.w;

    res.data[1][0] = c1.x;
    res.data[1][1] = c1.y;
    res.data[1][2] = c1.z;
    res.data[1][3] = c1.w;

    res.data[2][0] = c2.x;
    res.data[2][1] = c2.y;
    res.data[2][2] = c2.z;
    res.data[2][3] = c2.w;

    res.data[3][0] = c3.x;
    res.data[3][1] = c3.y;
    res.data[3][2] = c3.z;
    res.data[3][3] = c3.w;
    return res;
}

f32 m2_sum(M2 m)
{
    f32 sum = 0.0f;

    for (i32 c = 0; c < 2; c++)
        for (i32 r = 0; r < 2; r++)
            sum += m.data[c][r];

    return sum;
}

f32 m3_sum(M3 m)
{
    f32 sum = 0.0f;

    for (i32 c = 0; c < 3; c++)
        for (i32 r = 0; r < 3; r++)
            sum += m.data[c][r];

    return sum;
}

f32 m4_sum(M4 m)
{
    f32 sum = 0.0f;

    for (i32 c = 0; c < 4; c++)
        for (i32 r = 0; r < 4; r++)
            sum += m.data[c][r];

    return sum;
}

M2 m2_add(M2 m1, M2 m2)
{
    m1.data[0][0] += m2.data[0][0];
    m1.data[0][1] += m2.data[0][1];

    m1.data[1][0] += m2.data[1][0];
    m1.data[1][1] += m2.data[1][1];

    return m1;
}

M3 m3_add(M3 m1, M3 m2)
{
    m1.data[0][0] += m2.data[0][0];
    m1.data[0][1] += m2.data[0][1];
    m1.data[0][2] += m2.data[0][2];

    m1.data[1][0] += m2.data[1][0];
    m1.data[1][1] += m2.data[1][1];
    m1.data[1][2] += m2.data[1][2];

    m1.data[2][0] += m2.data[2][0];
    m1.data[2][1] += m2.data[2][1];
    m1.data[2][2] += m2.data[2][2];

    return m1;
}

M4 m4_add(M4 m1, M4 m2)
{
    m1.data[0][0] += m2.data[0][0];
    m1.data[0][1] += m2.data[0][1];
    m1.data[0][2] += m2.data[0][2];
    m1.data[0][3] += m2.data[0][3];

    m1.data[1][0] += m2.data[1][0];
    m1.data[1][1] += m2.data[1][1];
    m1.data[1][2] += m2.data[1][2];
    m1.data[1][3] += m2.data[1][3];

    m1.data[2][0] += m2.data[2][0];
    m1.data[2][1] += m2.data[2][1];
    m1.data[2][2] += m2.data[2][2];
    m1.data[2][3] += m2.data[2][3];

    m1.data[3][0] += m2.data[3][0];
    m1.data[3][1] += m2.data[3][1];
    m1.data[3][2] += m2.data[3][2];
    m1.data[3][3] += m2.data[3][3];

    return m1;
}

M2 m2_sub(M2 m1, M2 m2)
{
    m1.data[0][0] -= m2.data[0][0];
    m1.data[0][1] -= m2.data[0][1];

    m1.data[1][0] -= m2.data[1][0];
    m1.data[1][1] -= m2.data[1][1];

    return m1;
}

M3 m3_sub(M3 m1, M3 m2)
{
    m1.data[0][0] -= m2.data[0][0];
    m1.data[0][1] -= m2.data[0][1];
    m1.data[0][2] -= m2.data[0][2];

    m1.data[1][0] -= m2.data[1][0];
    m1.data[1][1] -= m2.data[1][1];
    m1.data[1][2] -= m2.data[1][2];

    m1.data[2][0] -= m2.data[2][0];
    m1.data[2][1] -= m2.data[2][1];
    m1.data[2][2] -= m2.data[2][2];

    return m1;
}

M4 m4_sub(M4 m1, M4 m2)
{
    m1.data[0][0] -= m2.data[0][0];
    m1.data[0][1] -= m2.data[0][1];
    m1.data[0][2] -= m2.data[0][2];
    m1.data[0][3] -= m2.data[0][3];

    m1.data[1][0] -= m2.data[1][0];
    m1.data[1][1] -= m2.data[1][1];
    m1.data[1][2] -= m2.data[1][2];
    m1.data[1][3] -= m2.data[1][3];

    m1.data[2][0] -= m2.data[2][0];
    m1.data[2][1] -= m2.data[2][1];
    m1.data[2][2] -= m2.data[2][2];
    m1.data[2][3] -= m2.data[2][3];

    m1.data[3][0] -= m2.data[3][0];
    m1.data[3][1] -= m2.data[3][1];
    m1.data[3][2] -= m2.data[3][2];
    m1.data[3][3] -= m2.data[3][3];

    return m1;
}

M2 m2_s_multi(M2 m, f32 s)
{
    m.data[0][0] *= s;
    m.data[0][1] *= s;

    m.data[1][0] *= s;
    m.data[1][1] *= s;

    return m;
}

M3 m3_s_multi(M3 m, f32 s)
{
    m.data[0][0] *= s;
    m.data[0][1] *= s;
    m.data[0][2] *= s;

    m.data[1][0] *= s;
    m.data[1][1] *= s;
    m.data[1][2] *= s;

    m.data[2][0] *= s;
    m.data[2][1] *= s;
    m.data[2][2] *= s;

    return m;
}

M4 m4_s_multi(M4 m, f32 s)
{
    m.data[0][0] *= s;
    m.data[0][1] *= s;
    m.data[0][2] *= s;
    m.data[0][3] *= s;

    m.data[1][0] *= s;
    m.data[1][1] *= s;
    m.data[1][2] *= s;
    m.data[1][3] *= s;

    m.data[2][0] *= s;
    m.data[2][1] *= s;
    m.data[2][2] *= s;
    m.data[2][3] *= s;

    m.data[3][0] *= s;
    m.data[3][1] *= s;
    m.data[3][2] *= s;
    m.data[3][3] *= s;

    return m;
}

V2 m2_v2_multi(M2 m, V2 v)
{
    V2 out;
    out.x = (m.data[0][0] * v.x) + (m.data[1][0] * v.y);
    out.y = (m.data[0][1] * v.x) + (m.data[1][1] * v.y);
    return out;
}

V3 m3_v3_multi(M3 m, V3 v)
{
    V3 out;
    out.x = (m.data[0][0] * v.x) + (m.data[1][0] * v.y) + (m.data[2][0] * v.z);
    out.y = (m.data[0][1] * v.x) + (m.data[1][1] * v.y) + (m.data[2][1] * v.z);
    out.z = (m.data[0][2] * v.x) + (m.data[1][2] * v.y) + (m.data[2][2] * v.z);
    return out;
}

V3 m4_v3_multi(M4 m, V3 v)
{
    V3 out;
    out.x = (m.data[0][0] * v.x) + (m.data[1][0] * v.y) + (m.data[2][0] * v.z) +
            (m.data[3][0] * 1.0f);

    out.y = (m.data[0][1] * v.x) + (m.data[1][1] * v.y) + (m.data[2][1] * v.z) +
            (m.data[3][1] * 1.0f);

    out.z = (m.data[0][2] * v.x) + (m.data[1][2] * v.y) + (m.data[2][2] * v.z) +
            (m.data[3][2] * 1.0f);

    return out;
}

V4 m4_v4_multi(M4 m, V4 v)
{
    V4 out;
    out.x = (m.data[0][0] * v.x) + (m.data[1][0] * v.y) + (m.data[2][0] * v.z) +
            (m.data[3][0] * v.w);

    out.y = (m.data[0][1] * v.x) + (m.data[1][1] * v.y) + (m.data[2][1] * v.z) +
            (m.data[3][1] * v.w);

    out.z = (m.data[0][2] * v.x) + (m.data[1][2] * v.y) + (m.data[2][2] * v.z) +
            (m.data[3][2] * v.w);

    out.w = (m.data[0][3] * v.x) + (m.data[1][3] * v.y) + (m.data[2][3] * v.z) +
            (m.data[3][3] * v.w);
    return out;
}

M2 m2_multi(M2 m1, M2 m2)
{
    M2 out = {};

    for (u32 col = 0; col < 2; col++)
        for (u32 row = 0; row < 2; row++)
            for (u32 i = 0; i < 2; i++)
                out.data[col][row] += m1.data[i][row] * m2.data[col][i];

    return out;
}

M3 m3_multi(M3 m1, M3 m2)
{
    M3 out = m3f(m1.data[0][0] * m2.data[0][0] + m1.data[1][0] * m2.data[0][1] +
                     m1.data[2][0] * m2.data[0][2],
                 m1.data[0][0] * m2.data[1][0] + m1.data[1][0] * m2.data[1][1] +
                     m1.data[2][0] * m2.data[1][2],
                 m1.data[0][0] * m2.data[2][0] + m1.data[1][0] * m2.data[2][1] +
                     m1.data[2][0] * m2.data[2][2],

                 m1.data[0][1] * m2.data[0][0] + m1.data[1][1] * m2.data[0][1] +
                     m1.data[2][1] * m2.data[0][2],
                 m1.data[0][1] * m2.data[1][0] + m1.data[1][1] * m2.data[1][1] +
                     m1.data[2][1] * m2.data[1][2],
                 m1.data[0][1] * m2.data[2][0] + m1.data[1][1] * m2.data[2][1] +
                     m1.data[2][1] * m2.data[2][2],

                 m1.data[0][2] * m2.data[0][0] + m1.data[1][2] * m2.data[0][1] +
                     m1.data[2][2] * m2.data[0][2],
                 m1.data[0][2] * m2.data[1][0] + m1.data[1][2] * m2.data[1][1] +
                     m1.data[2][2] * m2.data[1][2],
                 m1.data[0][2] * m2.data[2][0] + m1.data[1][2] * m2.data[2][1] +
                     m1.data[2][2] * m2.data[2][2]);

    return out;
}

M4 m4_multi(M4 m1, M4 m2)
{
    M4 out = m4f(m1.data[0][0] * m2.data[0][0] + m1.data[1][0] * m2.data[0][1] +
                     m1.data[2][0] * m2.data[0][2] + m1.data[3][0] * m2.data[0][3],
                 m1.data[0][0] * m2.data[1][0] + m1.data[1][0] * m2.data[1][1] +
                     m1.data[2][0] * m2.data[1][2] + m1.data[3][0] * m2.data[1][3],
                 m1.data[0][0] * m2.data[2][0] + m1.data[1][0] * m2.data[2][1] +
                     m1.data[2][0] * m2.data[2][2] + m1.data[3][0] * m2.data[2][3],
                 m1.data[0][0] * m2.data[3][0] + m1.data[1][0] * m2.data[3][1] +
                     m1.data[2][0] * m2.data[3][2] + m1.data[3][0] * m2.data[3][3],

                 m1.data[0][1] * m2.data[0][0] + m1.data[1][1] * m2.data[0][1] +
                     m1.data[2][1] * m2.data[0][2] + m1.data[3][1] * m2.data[0][3],
                 m1.data[0][1] * m2.data[1][0] + m1.data[1][1] * m2.data[1][1] +
                     m1.data[2][1] * m2.data[1][2] + m1.data[3][1] * m2.data[1][3],
                 m1.data[0][1] * m2.data[2][0] + m1.data[1][1] * m2.data[2][1] +
                     m1.data[2][1] * m2.data[2][2] + m1.data[3][1] * m2.data[2][3],
                 m1.data[0][1] * m2.data[3][0] + m1.data[1][1] * m2.data[3][1] +
                     m1.data[2][1] * m2.data[3][2] + m1.data[3][1] * m2.data[3][3],

                 m1.data[0][2] * m2.data[0][0] + m1.data[1][2] * m2.data[0][1] +
                     m1.data[2][2] * m2.data[0][2] + m1.data[3][2] * m2.data[0][3],
                 m1.data[0][2] * m2.data[1][0] + m1.data[1][2] * m2.data[1][1] +
                     m1.data[2][2] * m2.data[1][2] + m1.data[3][2] * m2.data[1][3],
                 m1.data[0][2] * m2.data[2][0] + m1.data[1][2] * m2.data[2][1] +
                     m1.data[2][2] * m2.data[2][2] + m1.data[3][2] * m2.data[2][3],
                 m1.data[0][2] * m2.data[3][0] + m1.data[1][2] * m2.data[3][1] +
                     m1.data[2][2] * m2.data[3][2] + m1.data[3][2] * m2.data[3][3],

                 m1.data[0][3] * m2.data[0][0] + m1.data[1][3] * m2.data[0][1] +
                     m1.data[2][3] * m2.data[0][2] + m1.data[3][3] * m2.data[0][3],
                 m1.data[0][3] * m2.data[1][0] + m1.data[1][3] * m2.data[1][1] +
                     m1.data[2][3] * m2.data[1][2] + m1.data[3][3] * m2.data[1][3],
                 m1.data[0][3] * m2.data[2][0] + m1.data[1][3] * m2.data[2][1] +
                     m1.data[2][3] * m2.data[2][2] + m1.data[3][3] * m2.data[2][3],
                 m1.data[0][3] * m2.data[3][0] + m1.data[1][3] * m2.data[3][1] +
                     m1.data[2][3] * m2.data[3][2] + m1.data[3][3] * m2.data[3][3]);

    return out;
}
b8 m2_equal(M2 m1, M2 m2)
{
    for (i32 c = 0; c < 2; c++)
        for (i32 r = 0; r < 2; r++)
            if (m1.data[c][r] != m2.data[c][r]) return false;

    return true;
}

b8 m3_equal(M3 m1, M3 m2)
{
    for (i32 c = 0; c < 3; c++)
        for (i32 r = 0; r < 3; r++)
            if (m1.data[c][r] != m2.data[c][r]) return false;

    return true;
}

b8 m4_equal(M4 m1, M4 m2)
{
    for (i32 c = 0; c < 4; c++)
        for (i32 r = 0; r < 4; r++)
            if (m1.data[c][r] != m2.data[c][r]) return false;

    return true;
}

b8 m2_less(M2 m1, M2 m2)
{
    return (m2_sum(m1) < m2_sum(m2));
}

b8 m3_less(M3 m1, M3 m2)
{
    return (m3_sum(m1) < m3_sum(m2));
}

b8 m4_less(M4 m1, M4 m2)
{
    return (m4_sum(m1) < m4_sum(m2));
}

b8 m2_more(M2 m1, M2 m2)
{
    return (m2_sum(m1) > m2_sum(m2));
}

b8 m3_more(M3 m1, M3 m2)
{
    return (m3_sum(m1) > m3_sum(m2));
}

b8 m4_more(M4 m1, M4 m2)
{
    return (m4_sum(m1) > m4_sum(m2));
}

b8 vertex_equal(const Vertex* f, const Vertex* s)
{
    return v3_equal(f->pos, s->pos) && v4_equal(f->color, s->color) &&
           v2_equal(f->tex_coords, s->tex_coords) && f->tex_index == f->tex_index;
}

b8 vp_equal(const VP* f, const VP* s)
{
    return m4_equal(f->view, s->view) && m4_equal(f->proj, s->proj);
}

Vertex vertex_create(V3 pos, V3 normal, V2 tex_coords, V4 color, f32 tex_index)
{
    Vertex result;
    result.pos = pos;
    result.normal = normal;
    result.tex_coords = tex_coords;
    result.color = color;
    result.tex_index = tex_index;
    return result;
}

#if 1
M2 operator+(const M2& m1, const M2& m2)
{
    M2 out = m2_add(m1, m2);
    return out;
}

M3 operator+(const M3& m1, const M3& m2)
{
    M3 out = m3_add(m1, m2);
    return out;
}

M4 operator+(const M4& m1, const M4& m2)
{
    M4 out = m4_add(m1, m2);
    return out;
}

M2 operator-(const M2& m1, const M2& m2)
{
    M2 out = m2_sub(m1, m2);
    return out;
}

M3 operator-(const M3& m1, const M3& m2)
{
    M3 out = m3_sub(m1, m2);
    return out;
}

M4 operator-(const M4& m1, const M4& m2)
{
    M4 out = m4_sub(m1, m2);
    return out;
}

V2 operator*(const M2& m, const V2& v)
{
    V2 out;
    out.x = (m.data[0][0] * v.x) + (m.data[1][0] * v.y);
    out.y = (m.data[0][1] * v.x) + (m.data[1][1] * v.y);
    return out;
}

V3 operator*(const M3& m, const V3& v)
{
    V3 out;
    out.x = (m.data[0][0] * v.x) + (m.data[1][0] * v.y) + (m.data[2][0] * v.z);
    out.y = (m.data[0][1] * v.x) + (m.data[1][1] * v.y) + (m.data[2][1] * v.z);
    out.z = (m.data[0][2] * v.x) + (m.data[1][2] * v.y) + (m.data[2][2] * v.z);
    return out;
}

V3 operator*(const M4& m, const V3& v)
{
    V3 out;
    out.x = (m.data[0][0] * v.x) + (m.data[1][0] * v.y) + (m.data[2][0] * v.z) +
            (m.data[3][0] * 1.0f);

    out.y = (m.data[0][1] * v.x) + (m.data[1][1] * v.y) + (m.data[2][1] * v.z) +
            (m.data[3][1] * 1.0f);

    out.z = (m.data[0][2] * v.x) + (m.data[1][2] * v.y) + (m.data[2][2] * v.z) +
            (m.data[3][2] * 1.0f);

    return out;
}

V4 operator*(const M4& m, const V4& v)
{
    V4 out;
    out.x = (m.data[0][0] * v.x) + (m.data[1][0] * v.y) + (m.data[2][0] * v.z) +
            (m.data[3][0] * v.w);

    out.y = (m.data[0][1] * v.x) + (m.data[1][1] * v.y) + (m.data[2][1] * v.z) +
            (m.data[3][1] * v.w);

    out.z = (m.data[0][2] * v.x) + (m.data[1][2] * v.y) + (m.data[2][2] * v.z) +
            (m.data[3][2] * v.w);

    out.w = (m.data[0][3] * v.x) + (m.data[1][3] * v.y) + (m.data[2][3] * v.z) +
            (m.data[3][3] * v.w);
    return out;
}

M2 operator*(const M2& m, f32 s)
{
    M2 out = m2_s_multi(m, s);
    return out;
}
M3 operator*(const M3& m, f32 s)
{
    M3 out = m3_s_multi(m, s);
    return out;
}
M4 operator*(const M4& m, f32 s)
{
    M4 out = m4_s_multi(m, s);
    return out;
}

M4 operator/(const M4& m, f32 s)
{
    M4 out = m;
    out.data[0][0] /= s;
    out.data[0][1] /= s;
    out.data[0][2] /= s;
    out.data[0][3] /= s;

    out.data[1][0] /= s;
    out.data[1][1] /= s;
    out.data[1][2] /= s;
    out.data[1][3] /= s;

    out.data[2][0] /= s;
    out.data[2][1] /= s;
    out.data[2][2] /= s;
    out.data[2][3] /= s;

    out.data[3][0] /= s;
    out.data[3][1] /= s;
    out.data[3][2] /= s;
    out.data[3][3] /= s;
    return out;
}

M2 operator*(const M2& m1, const M2& m2)
{
    M2 out = m2_multi(m1, m2);
    return out;
}

M3 operator*(const M3& m1, const M3& m2)
{
    M3 out = m3_multi(m1, m2);
    return out;
}

M4 operator*(const M4& m1, const M4& m2)
{
    M4 out = m4_multi(m1, m2);
    return out;
}

b8 operator==(const M2& m1, const M2& m2)
{
    for (i32 c = 0; c < 2; c++)
        for (i32 r = 0; r < 2; r++)
            if (m1.data[c][r] != m2.data[c][r]) return 0;

    return 1;
}

b8 operator==(const M3& m1, const M3& m2)
{
    for (i32 c = 0; c < 3; c++)
        for (i32 r = 0; r < 3; r++)
            if (m1.data[c][r] != m2.data[c][r]) return 0;

    return 1;
}

b8 operator==(const M4& m1, const M4& m2)
{
    for (i32 c = 0; c < 4; c++)
        for (i32 r = 0; r < 4; r++)
            if (m1.data[c][r] != m2.data[c][r]) return 0;

    return 1;
}

b8 operator<(const M2& m1, const M2& m2)
{
    return (m2_sum(m1) < m2_sum(m2));
}

b8 operator<(const M3& m1, const M3& m2)
{
    return (m3_sum(m1) < m3_sum(m2));
};

b8 operator<(const M4& m1, const M4& m2)
{
    return (m4_sum(m1) < m4_sum(m2));
};

b8 operator>(const M2& m1, const M2& m2)
{
    return (m2_sum(m1) > m2_sum(m2));
}

b8 operator>(const M3& m1, const M3& m2)
{
    return (m3_sum(m1) > m3_sum(m2));
}

b8 operator>(const M4& m1, const M4& m2)
{
    return (m4_sum(m1) > m4_sum(m2));
}
#endif

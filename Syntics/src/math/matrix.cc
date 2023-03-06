#include "matrix.h"
#include "defines.h"

M2 m2d()
{
    return m2i(1.0f);
}

M2 m2i(f32 i)
{
    M2 res;
    res.data[0][0] = i;
    res.data[1][0] = 0.0f;
    res.data[0][1] = 0.0f;
    res.data[1][1] = i;
    return res;
}

M3 m3d()
{
    return m3i(1.0f);
}

M3 m3i(f32 i)
{
    M3 res;
    res.data[0][0] = i;
    res.data[1][0] = 0.0f;
    res.data[2][0] = 0.0f;
    res.data[0][1] = 0.0f;
    res.data[1][1] = i;
    res.data[2][1] = 0.0f;
    res.data[0][2] = 0.0f;
    res.data[1][2] = 0.0f;
    res.data[2][2] = i;
    return res;
}

M4 m4d()
{
    return m4i(1.0f);
}

M4 m4i(f32 i)
{
    M4 res;
    res.data[0][0] = i;
    res.data[1][0] = 0.0f;
    res.data[2][0] = 0.0f;
    res.data[3][0] = 0.0f;
    res.data[0][1] = 0.0f;
    res.data[1][1] = i;
    res.data[2][1] = 0.0f;
    res.data[3][1] = 0.0f;
    res.data[0][2] = 0.0f;
    res.data[1][2] = 0.0f;
    res.data[2][2] = i;
    res.data[3][2] = 0.0f;
    res.data[0][3] = 0.0f;
    res.data[1][3] = 0.0f;
    res.data[2][3] = 0.0f;
    res.data[3][3] = i;
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
    M2 out;

    for (i32 c = 0; c < 2; c++)
        for (i32 r = 0; r < 2; r++)
            out.data[c][r] = m1.data[c][r] + m2.data[c][r];

    return out;
}

M3 m3_add(M3 m1, M3 m2)
{
    M3 out;

    for (i32 c = 0; c < 3; c++)
        for (i32 r = 0; r < 3; r++)
            out.data[c][r] = m1.data[c][r] + m2.data[c][r];

    return out;
}

M4 m4_add(M4 m1, M4 m2)
{
    M4 out;

    for (i32 c = 0; c < 4; c++)
        for (i32 r = 0; r < 4; r++)
            out.data[c][r] = m1.data[c][r] + m2.data[c][r];

    return out;
}

M2 m2_sub(M2 m1, M2 m2)
{
    M2 out;

    for (i32 c = 0; c < 2; c++)
        for (i32 r = 0; r < 2; r++)
            out.data[c][r] = m1.data[c][r] - m2.data[c][r];

    return out;
}

M3 m3_sub(M3 m1, M3 m2)
{
    M3 out;

    for (i32 c = 0; c < 3; c++)
        for (i32 r = 0; r < 3; r++)
            out.data[c][r] = m1.data[c][r] - m2.data[c][r];

    return out;
}

M4 m4_sub(M4 m1, M4 m2)
{
    M4 out;

    for (i32 c = 0; c < 4; c++)
        for (i32 r = 0; r < 4; r++)
            out.data[c][r] = m1.data[c][r] - m2.data[c][r];

    return out;
}

M2 m2_s_multi(M2 m, f32 s)
{
    M2 out;
    for (i32 c = 0; c < 2; c++)
        for (i32 r = 0; r < 2; r++)
            out.data[c][r] = m.data[c][r] * s;
    return out;
}

M3 m3_s_multi(M3 m, f32 s)
{
    Mat3f out;
    for (i32 c = 0; c < 3; c++)
        for (i32 r = 0; r < 3; r++)
            out.data[c][r] = m.data[c][r] * s;
    return out;
}

M4 m4_s_multi(M4 m, f32 s)
{
    M4 out;
    for (i32 c = 0; c < 4; c++)
        for (i32 r = 0; r < 4; r++)
            out.data[c][r] = m.data[c][r] * s;
    return out;
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

// TODO: if math bug!! i think this is correct
M2 m2_multi(M2 m1, M2 m2)
{
    M2 out = m2i(0.0f);

    for (i32 ro = 0; ro < 2; ro++)
        for (i32 ri = 0; ri < 2; ri++)
            for (i32 c = 0; c < 2; c++)
                out.data[ri][ro] += m1.data[c][ro] * m2.data[ri][c];

    return out;
}

M3 m3_multi(M3 m1, M3 m2)
{
    M3 out = m3i(0.0f);

    for (i32 ro = 0; ro < 3; ro++)
        for (i32 ri = 0; ri < 3; ri++)
            for (i32 c = 0; c < 3; c++)
                out.data[ri][ro] += m1.data[c][ro] * m2.data[ri][c];

    return out;
}

M4 m4_multi(M4 m1, M4 m2)
{
    M4 out;

    for (i32 ro = 0; ro < 4; ro++)
        for (i32 ri = 0; ri < 4; ri++)
            for (i32 c = 0; c < 4; c++)
                out.data[ri][ro] += m1.data[c][ro] * m2.data[ri][c];

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

b8 mvp_equal(const MVP* f, const MVP* s)
{
    return m4_equal(f->model, s->model) && m4_equal(f->view, s->view) &&
           m4_equal(f->proj, s->proj);
}

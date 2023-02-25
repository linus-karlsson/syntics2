#include "matrix.h"
#include "defines.h"

Mat2f mat2i(f32 i)
{
    return { { { i, 0.0f }, { 0.0f, i } } };
}

Mat3f mat3i(f32 i)
{
    return { { { i, 0.0f, 0.0f }, { 0.0f, i, 0.0f }, { 0.0f, 0.0f, i } } };
}

Mat4f mat4i(f32 i)
{
    return { { { i, 0.0f, 0.0f, 0.0f },
               { 0.0f, i, 0.0f, 0.0f },
               { 0.0f, 0.0f, i, 0.0f },
               { 0.0f, 0.0f, 0.0f, i } } };
}

Mat2f operator+(const Mat2f& m1, const Mat2f& m2)
{
    Mat2f out;

    for (i32 c = 0; c < 2; c++)
        for (i32 r = 0; r < 2; r++)
            out.data[c][r] = m1.data[c][r] + m2.data[c][r];

    return out;
}

Mat3f operator+(const Mat3f& m1, const Mat3f& m2)
{
    Mat3f out;

    for (i32 c = 0; c < 3; c++)
        for (i32 r = 0; r < 3; r++)
            out.data[c][r] = m1.data[c][r] + m2.data[c][r];

    return out;
}

Mat4f operator+(const Mat4f& m1, const Mat4f& m2)
{
    Mat4f out;

    for (i32 c = 0; c < 4; c++)
        for (i32 r = 0; r < 4; r++)
            out.data[c][r] = m1.data[c][r] + m2.data[c][r];

    return out;
}

Mat2f operator-(const Mat2f& m1, const Mat2f& m2)
{
    Mat2f out;

    for (i32 c = 0; c < 2; c++)
        for (i32 r = 0; r < 2; r++)
            out.data[c][r] = m1.data[c][r] - m2.data[c][r];

    return out;
}

Mat3f operator-(const Mat3f& m1, const Mat3f& m2)
{
    Mat3f out;

    for (i32 c = 0; c < 3; c++)
        for (i32 r = 0; r < 3; r++)
            out.data[c][r] = m1.data[c][r] - m2.data[c][r];

    return out;
}

Mat4f operator-(const Mat4f& m1, const Mat4f& m2)
{
    Mat4f out;

    for (i32 c = 0; c < 4; c++)
        for (i32 r = 0; r < 4; r++)
            out.data[c][r] = m1.data[c][r] - m2.data[c][r];

    return out;
}

Mat2f operator*(const Mat2f& m, f32 s)
{
    Mat2f out;
    for (i32 ro = 0; ro < 2; ro++)
        for (i32 c = 0; c < 2; c++)
            out.data[ro][c] = m.data[ro][c] * s;
    return out;
}
Mat3f operator*(const Mat3f& m, f32 s)
{
    Mat3f out;
    for (i32 ro = 0; ro < 3; ro++)
        for (i32 c = 0; c < 3; c++)
            out.data[ro][c] = m.data[ro][c] * s;
    return out;
}
Mat4f operator*(const Mat4f& m, f32 s)
{
    Mat4f out;
    for (i32 ro = 0; ro < 4; ro++)
        for (i32 c = 0; c < 4; c++)
            out.data[c][ro] = m.data[c][ro] * s;
    return out;
}

Vec2 operator*(const Mat2f& m, const Vec2& v)
{
    Vec2 out;
    out.x = (m.data[0][0] * v.x) + (m.data[0][1] * v.y);
    out.y = (m.data[1][0] * v.x) + (m.data[1][1] * v.y);
    return out;
}
Vec3 operator*(const Mat3f& m, const Vec3& v)
{
    Vec3 out;
    out.x = (m.data[0][0] * v.x) + (m.data[0][1] * v.y) + (m.data[0][2] * v.z);
    out.y = (m.data[1][0] * v.x) + (m.data[1][1] * v.y) + (m.data[1][2] * v.z);
    out.z = (m.data[2][0] * v.x) + (m.data[2][1] * v.y) + (m.data[2][2] * v.z);
    return out;
}
// Vec3 out;
// out.x = (m.data[0][0] * v.x) + (m.data[1][0] * v.y) + (m.data[2][0] * v.z);
// out.y = (m.data[0][1] * v.x) + (m.data[1][1] * v.y) + (m.data[2][1] * v.z);
// out.z = (m.data[0][2] * v.x) + (m.data[1][2] * v.y) + (m.data[2][2] * v.z);
// return out;

Vec3 operator*(const Mat4f& m, const Vec3& v)
{
    Vec3 out;
    out.x = (m.data[0][0] * v.x) + (m.data[1][0] * v.y) + (m.data[2][0] * v.z) +
            (m.data[3][0] * 1.0f);

    out.y = (m.data[0][1] * v.x) + (m.data[1][1] * v.y) + (m.data[2][1] * v.z) +
            (m.data[3][1] * 1.0f);

    out.z = (m.data[0][2] * v.x) + (m.data[1][2] * v.y) + (m.data[2][2] * v.z) +
            (m.data[3][2] * 1.0f);

    return out;
}

Vec4 operator*(const Mat4f& m, const Vec4& v)
{
    Vec4 out;
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

Mat2f operator*(const Mat2f& m1, const Mat2f& m2)
{
    Mat2f out;

    for (i32 ro = 0; ro < 2; ro++)
        for (i32 ri = 0; ri < 2; ri++)
            for (i32 c = 0; c < 2; c++)
                out.data[ri][ro] += m1.data[c][ro] * m2.data[ri][c];

    return out;
}

Mat3f operator*(const Mat3f& m1, const Mat3f& m2)
{
    Mat3f out;

    for (i32 ro = 0; ro < 3; ro++)
        for (i32 ri = 0; ri < 3; ri++)
            for (i32 c = 0; c < 3; c++)
                out.data[ri][ro] += m1.data[c][ro] * m2.data[ri][c];

    return out;
}

Mat4f operator*(const Mat4f& m1, const Mat4f& m2)
{
    Mat4f out;

    for (i32 ro = 0; ro < 4; ro++)
        for (i32 ri = 0; ri < 4; ri++)
            for (i32 c = 0; c < 4; c++)
                out.data[ri][ro] += m1.data[c][ro] * m2.data[ri][c];

    return out;
}

b8 operator==(const Mat2f& m1, const Mat2f& m2)
{
    for (i32 c = 0; c < 2; c++)
        for (i32 r = 0; r < 2; r++)
            if (m1.data[c][r] != m2.data[c][r]) return 0;

    return 1;
}

b8 operator==(const Mat3f& m1, const Mat3f& m2)
{
    for (i32 c = 0; c < 3; c++)
        for (i32 r = 0; r < 3; r++)
            if (m1.data[c][r] != m2.data[c][r]) return 0;

    return 1;
}

b8 operator==(const Mat4f& m1, const Mat4f& m2)
{
    for (i32 c = 0; c < 4; c++)
        for (i32 r = 0; r < 4; r++)
            if (m1.data[c][r] != m2.data[c][r]) return 0;

    return 1;
}

f32 sum_m2(const Mat2f& m)
{
    f32 sum = 0.0f;

    for (i32 c = 0; c < 2; c++)
        for (i32 r = 0; r < 2; r++)
            sum += m.data[c][r];

    return sum;
}

f32 sum_m3(const Mat3f& m)
{
    f32 sum = 0.0f;

    for (i32 c = 0; c < 3; c++)
        for (i32 r = 0; r < 3; r++)
            sum += m.data[c][r];

    return sum;
}

f32 sum_m4(const Mat4f& m)
{
    f32 sum = 0.0f;

    for (i32 c = 0; c < 4; c++)
        for (i32 r = 0; r < 4; r++)
            sum += m.data[c][r];

    return sum;
}

b8 operator<(const Mat2f& m1, const Mat2f& m2)
{
    return (sum_m2(m1) < sum_m2(m2));
}

b8 operator<(const Mat3f& m1, const Mat3f& m2)
{
    return (sum_m3(m1) < sum_m3(m2));
};

b8 operator<(const Mat4f& m1, const Mat4f& m2)
{
    return (sum_m4(m1) < sum_m4(m2));
};

b8 operator>(const Mat2f& m1, const Mat2f& m2)
{
    return (sum_m2(m1) > sum_m2(m2));
}

b8 operator>(const Mat3f& m1, const Mat3f& m2)
{
    return (sum_m3(m1) > sum_m3(m2));
}

b8 operator>(const Mat4f& m1, const Mat4f& m2)
{
    return (sum_m4(m1) > sum_m4(m2));
}

b8 operator==(const Vertex& f, const Vertex& s)
{
    return f.pos == s.pos && f.color == s.color && f.tex_coords == s.tex_coords &&
           f.tex_index == f.tex_coords;
}

b8 operator==(const MVP& f, const MVP& s)
{
    return f.model == s.model && f.view == s.view && f.proj == s.proj;
}

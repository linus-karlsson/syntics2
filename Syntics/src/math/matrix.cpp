#include "matrix.h"
#include "defines.h"

namespace synt {

Mat2f mat2i(float i) { return (Mat2f){ { { i, 0.0f }, { 0.0f, i } } }; }

Mat3f mat3i(float i)
{
    return (Mat3f){ { { i, 0.0f, 0.0f }, { 0.0f, i, 0.0f }, { 0.0f, 0.0f, i } } };
}

Mat4f mat4i(float i)
{
    return (Mat4f){ { { i, 0.0f, 0.0f, 0.0f },
                      { 0.0f, i, 0.0f, 0.0f },
                      { 0.0f, 0.0f, i, 0.0f },
                      { 0.0f, 0.0f, 0.0f, i } } };
}

Mat2f operator+(const Mat2f& m1, const Mat2f& m2)
{
    Mat2f out;

    for (int c = 0; c < 2; c++)
        for (int r = 0; r < 2; r++)
            out.data[c][r] = m1.data[c][r] + m2.data[c][r];

    return out;
}

Mat3f operator+(const Mat3f& m1, const Mat3f& m2)
{
    Mat3f out;

    for (int c = 0; c < 3; c++)
        for (int r = 0; r < 3; r++)
            out.data[c][r] = m1.data[c][r] + m2.data[c][r];

    return out;
}

Mat4f operator+(const Mat4f& m1, const Mat4f& m2)
{
    Mat4f out;

    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            out.data[c][r] = m1.data[c][r] + m2.data[c][r];

    return out;
}

Mat2f operator-(const Mat2f& m1, const Mat2f& m2)
{
    Mat2f out;

    for (int c = 0; c < 2; c++)
        for (int r = 0; r < 2; r++)
            out.data[c][r] = m1.data[c][r] - m2.data[c][r];

    return out;
}

Mat3f operator-(const Mat3f& m1, const Mat3f& m2)
{
    Mat3f out;

    for (int c = 0; c < 3; c++)
        for (int r = 0; r < 3; r++)
            out.data[c][r] = m1.data[c][r] - m2.data[c][r];

    return out;
}

Mat4f operator-(const Mat4f& m1, const Mat4f& m2)
{
    Mat4f out;

    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            out.data[c][r] = m1.data[c][r] - m2.data[c][r];

    return out;
}

Mat2f operator*(const Mat2f& m, float s)
{
    Mat2f out;
    for (int ro = 0; ro < 2; ro++)
        for (int c = 0; c < 2; c++)
            out.data[ro][c] = m.data[ro][c] * s;
    return out;
}
Mat3f operator*(const Mat3f& m, float s)
{
    Mat3f out;
    for (int ro = 0; ro < 3; ro++)
        for (int c = 0; c < 3; c++)
            out.data[ro][c] = m.data[ro][c] * s;
    return out;
}
Mat4f operator*(const Mat4f& m, float s)
{
    Mat4f out;
    for (int ro = 0; ro < 4; ro++)
        for (int c = 0; c < 4; c++)
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

    for (int ro = 0; ro < 2; ro++)
        for (int ri = 0; ri < 2; ri++)
            for (int c = 0; c < 2; c++)
                out.data[ri][ro] += m1.data[c][ro] * m2.data[ri][c];

    return out;
}

Mat3f operator*(const Mat3f& m1, const Mat3f& m2)
{
    Mat3f out;

    for (int ro = 0; ro < 3; ro++)
        for (int ri = 0; ri < 3; ri++)
            for (int c = 0; c < 3; c++)
                out.data[ri][ro] += m1.data[c][ro] * m2.data[ri][c];

    return out;
}

Mat4f operator*(const Mat4f& m1, const Mat4f& m2)
{
    Mat4f out;

    for (int ro = 0; ro < 4; ro++)
        for (int ri = 0; ri < 4; ri++)
            for (int c = 0; c < 4; c++)
                out.data[ri][ro] += m1.data[c][ro] * m2.data[ri][c];

    return out;
}

bool operator==(const Mat2f& m1, const Mat2f& m2)
{
    for (int c = 0; c < 2; c++)
        for (int r = 0; r < 2; r++)
            if (m1.data[c][r] != m2.data[c][r]) return 0;

    return 1;
}

bool operator==(const Mat3f& m1, const Mat3f& m2)
{
    for (int c = 0; c < 3; c++)
        for (int r = 0; r < 3; r++)
            if (m1.data[c][r] != m2.data[c][r]) return 0;

    return 1;
}

bool operator==(const Mat4f& m1, const Mat4f& m2)
{
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            if (m1.data[c][r] != m2.data[c][r]) return 0;

    return 1;
}

float sum_m2(const Mat2f& m)
{
    float sum = 0.0f;

    for (int c = 0; c < 2; c++)
        for (int r = 0; r < 2; r++)
            sum += m.data[c][r];

    return sum;
}

float sum_m3(const Mat3f& m)
{
    float sum = 0.0f;

    for (int c = 0; c < 3; c++)
        for (int r = 0; r < 3; r++)
            sum += m.data[c][r];

    return sum;
}

float sum_m4(const Mat4f& m)
{
    float sum = 0.0f;

    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            sum += m.data[c][r];

    return sum;
}

bool operator<(const Mat2f& m1, const Mat2f& m2) { return (sum_m2(m1) < sum_m2(m2)); }

bool operator<(const Mat3f& m1, const Mat3f& m2) { return (sum_m3(m1) < sum_m3(m2)); };

bool operator<(const Mat4f& m1, const Mat4f& m2) { return (sum_m4(m1) < sum_m4(m2)); };

bool operator>(const Mat2f& m1, const Mat2f& m2) { return (sum_m2(m1) > sum_m2(m2)); }

bool operator>(const Mat3f& m1, const Mat3f& m2) { return (sum_m3(m1) > sum_m3(m2)); }

bool operator>(const Mat4f& m1, const Mat4f& m2) { return (sum_m4(m1) > sum_m4(m2)); }

} // namespace synt


#include "matrix.h"

namespace synt {

    Mat2f mat2i(float i) { return (Mat2f){ { { i, 0.0f }, { 0.0f, i } } }; }

    Mat3f mat3i(float i)
    {
        return (
            Mat3f){ { { i, 0.0f, 0.0f }, { 0.0f, i, 0.0f }, { 0.0f, 0.0f, i } } };
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

    bool operator<(const Mat2f& m1, const Mat2f& m2)
    {
        return (sum_m2(m1) < sum_m2(m2));
    }

    bool operator<(const Mat3f& m1, const Mat3f& m2)
    {
        return (sum_m3(m1) < sum_m3(m2));
    };

    bool operator<(const Mat4f& m1, const Mat4f& m2)
    {
        return (sum_m4(m1) < sum_m4(m2));
    };

    bool operator>(const Mat2f& m1, const Mat2f& m2)
    {
        return (sum_m2(m1) > sum_m2(m2));
    }

    bool operator>(const Mat3f& m1, const Mat3f& m2)
    {
        return (sum_m3(m1) > sum_m3(m2));
    }

    bool operator>(const Mat4f& m1, const Mat4f& m2)
    {
        return (sum_m4(m1) > sum_m4(m2));
    }

} // namespace synt


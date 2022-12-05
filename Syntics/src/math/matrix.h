#pragma once
#include "vectors.h"

namespace synt {

typedef struct Mat2f
{
    float data[2][2] = { { 1.0f, 0.0f }, { 0.0f, 1.0f } };
} Mat2f;

typedef struct Mat3f
{
    float data[3][3] = { { 1.0f, 0.0f, 0.0 },
                         { 0.0f, 1.0f, 0.0f },
                         { 0.0f, 0.0f, 1.0f } };
} Mat3f;

typedef struct Mat4f
{
    float data[4][4] = { { 1.0f, 0.0f, 0.0f, 0.0f },
                         { 0.0f, 1.0f, 0.0f, 0.0f },
                         { 0.0f, 0.0f, 1.0f, 0.0f },
                         { 0.0f, 0.0f, 0.0f, 1.0f } };
} Mat4f;

Mat2f mat2i(float i);
Mat3f mat3i(float i);
Mat4f mat4i(float i);

float sum_m2(const Mat2f& m);
float sum_m3(const Mat3f& m);
float sum_m4(const Mat4f& m);

Mat2f operator+(const Mat2f& m1, const Mat2f& m2);
Mat3f operator+(const Mat3f& m1, const Mat3f& m2);
Mat4f operator+(const Mat4f& m1, const Mat4f& m2);

Mat2f operator-(const Mat2f& m1, const Mat2f& m2);
Mat3f operator-(const Mat3f& m1, const Mat3f& m2);
Mat4f operator-(const Mat4f& m1, const Mat4f& m2);

Mat2f operator*(const Mat2f& m, float s);
Mat3f operator*(const Mat3f& m, float s);
Mat4f operator*(const Mat4f& m, float s);

Vec2 operator*(const Mat2f& m, const Vec2& v);
Vec3 operator*(const Mat3f& m, const Vec3& v);
Vec3 operator*(const Mat4f& m, const Vec3& v);
Vec4 operator*(const Mat4f& m, const Vec4& v);

Mat2f operator*(const Mat2f& m1, const Mat2f& m2);
Mat3f operator*(const Mat3f& m1, const Mat3f& m2);
Mat4f operator*(const Mat4f& m1, const Mat4f& m2);

bool operator==(const Mat2f& m1, const Mat2f& m2);
bool operator==(const Mat3f& m1, const Mat3f& m2);
bool operator==(const Mat4f& m1, const Mat4f& m2);

bool operator<(const Mat2f& m1, const Mat2f& m2);
bool operator<(const Mat3f& m1, const Mat3f& m2);
bool operator<(const Mat4f& m1, const Mat4f& m2);

bool operator>(const Mat2f& m1, const Mat2f& m2);
bool operator>(const Mat3f& m1, const Mat3f& m2);
bool operator>(const Mat4f& m1, const Mat4f& m2);

} // namespace synt

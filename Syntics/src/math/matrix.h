#pragma once
#include "vectors.h"

typedef struct Mat2f M2;
typedef struct Mat3f M3;
typedef struct Mat4f M4;

struct Mat2f
{
    f32 data[2][2] = { { 1.0f, 0.0f }, { 0.0f, 1.0f } };
};

struct Mat3f
{
    f32 data[3][3] = { { 1.0f, 0.0f, 0.0 },
                       { 0.0f, 1.0f, 0.0f },
                       { 0.0f, 0.0f, 1.0f } };
};

struct Mat4f
{
    f32 data[4][4] = { { 1.0f, 0.0f, 0.0f, 0.0f },
                       { 0.0f, 1.0f, 0.0f, 0.0f },
                       { 0.0f, 0.0f, 1.0f, 0.0f },
                       { 0.0f, 0.0f, 0.0f, 1.0f } };
};

Mat2f mat2i(f32 i);
Mat3f mat3i(f32 i);
Mat4f mat4i(f32 i);

f32 sum_m2(const Mat2f& m);
f32 sum_m3(const Mat3f& m);
f32 sum_m4(const Mat4f& m);

Mat2f operator+(const Mat2f& m1, const Mat2f& m2);
Mat3f operator+(const Mat3f& m1, const Mat3f& m2);
Mat4f operator+(const Mat4f& m1, const Mat4f& m2);

Mat2f operator-(const Mat2f& m1, const Mat2f& m2);
Mat3f operator-(const Mat3f& m1, const Mat3f& m2);
Mat4f operator-(const Mat4f& m1, const Mat4f& m2);

Mat2f operator*(const Mat2f& m, f32 s);
Mat3f operator*(const Mat3f& m, f32 s);
Mat4f operator*(const Mat4f& m, f32 s);

Vec2 operator*(const Mat2f& m, const Vec2& v);
Vec3 operator*(const Mat3f& m, const Vec3& v);
Vec3 operator*(const Mat4f& m, const Vec3& v);
Vec4 operator*(const Mat4f& m, const Vec4& v);

Mat2f operator*(const Mat2f& m1, const Mat2f& m2);
Mat3f operator*(const Mat3f& m1, const Mat3f& m2);
Mat4f operator*(const Mat4f& m1, const Mat4f& m2);

b8 operator==(const Mat2f& m1, const Mat2f& m2);
b8 operator==(const Mat3f& m1, const Mat3f& m2);
b8 operator==(const Mat4f& m1, const Mat4f& m2);

b8 operator<(const Mat2f& m1, const Mat2f& m2);
b8 operator<(const Mat3f& m1, const Mat3f& m2);
b8 operator<(const Mat4f& m1, const Mat4f& m2);

b8 operator>(const Mat2f& m1, const Mat2f& m2);
b8 operator>(const Mat3f& m1, const Mat3f& m2);
b8 operator>(const Mat4f& m1, const Mat4f& m2);

typedef struct Quad
{
    V3 corners[4];
} Quad;

typedef struct Vertex
{
    V4 pos;
    V4 color;
    V2 tex_coords;
    f32 tex_index;
} Vertex;

b8 operator==(const Vertex& f, const Vertex& s);

typedef struct MVP
{
    M4 model;
    M4 view;
    M4 proj;
    V3 light_pos;
} MVP;

b8 operator==(const MVP& f, const MVP& s);

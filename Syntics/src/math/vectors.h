#pragma once

namespace synt {

    typedef struct Vec2
    {
        Vec2();
        Vec2(float x, float y);
        float x;
        float y;
    } Vec2;

    typedef struct Vec3
    {
        Vec3();
        Vec3(float x, float y, float z);
        float x;
        float y;
        float z;
    } Vec3;

    typedef struct Vec4
    {
        Vec4();
        Vec4(float x, float y, float z, float w);
        float x;
        float y;
        float z;
        float w;
    } Vec4;

    Vec3 v3f(float x, float y, float z);
    Vec3 v3i(float i);

    float sum_v2(const Vec2& m);
    float sum_v3(const Vec3& m);
    float sum_v4(const Vec4& m);

    Vec2 operator+(const Vec2& v1, const Vec2& v2);
    Vec3 operator+(const Vec3& v1, const Vec3& v2);
    Vec4 operator+(const Vec4& v1, const Vec4& v2);

    Vec2 operator-(const Vec2& v1, const Vec2& v2);
    Vec3 operator-(const Vec3& v1, const Vec3& v2);
    Vec4 operator-(const Vec4& v1, const Vec4& v2);

    Vec2 operator*(const Vec2& v1, const Vec2& v2);
    Vec3 operator*(const Vec3& v1, const Vec3& v2);
    Vec4 operator*(const Vec4& v1, const Vec4& v2);

    Vec2 operator*(const Vec2& v1, float f);
    Vec3 operator*(const Vec3& v1, float f);
    Vec4 operator*(const Vec4& v1, float f);

    Vec2 operator*(float f, const Vec2& v1);
    Vec3 operator*(float f, const Vec3& v1);
    Vec4 operator*(float f, const Vec4& v1);

    Vec2& operator+=(Vec2& v1, const Vec2& v2);
    Vec3& operator+=(Vec3& v1, const Vec3& v2);
    Vec4& operator+=(Vec4& v1, const Vec4& v2);

    Vec2& operator-=(Vec2& v1, const Vec2& v2);
    Vec3& operator-=(Vec3& v1, const Vec3& v2);
    Vec4& operator-=(Vec4& v1, const Vec4& v2);

    Vec2& operator*=(Vec2& v1, const Vec2& v2);
    Vec3& operator*=(Vec3& v1, const Vec3& v2);
    Vec4& operator*=(Vec4& v1, const Vec4& v2);

    bool operator==(const Vec2& v1, const Vec2& v2);
    bool operator==(const Vec3& v1, const Vec3& v2);
    bool operator==(const Vec4& v1, const Vec4& v2);

    bool operator<(const Vec2& v1, const Vec2& v2);
    bool operator<(const Vec3& v1, const Vec3& v2);
    bool operator<(const Vec4& v1, const Vec4& v2);

    bool operator>(const Vec2& v1, const Vec2& v2);
    bool operator>(const Vec3& v1, const Vec3& v2);
    bool operator>(const Vec4& v1, const Vec4& v2);

} // namespace synt

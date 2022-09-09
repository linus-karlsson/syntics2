#include "vectors.h"

namespace synt {

    Vec2::Vec2() : x(0.0f), y(0.0f) {}
    Vec2::Vec2(float x, float y) : x(x), y(y) {}

    Vec3::Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3::Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec4::Vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    Vec4::Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(0.0f) {}

    Vec3 v3f(float x, float y, float z) { return (Vec3){ x, y, z }; }

    Vec3 v3i(float i) { return (Vec3){ i, i, i }; }

    Vec2 operator+(const Vec2& v1, const Vec2& v2)
    {
        return (Vec2){ v1.x + v2.x, v1.y + v2.y };
    }

    Vec3 operator+(const Vec3& v1, const Vec3& v2)
    {
        return (Vec3){
            v1.x + v2.x,
            v1.y + v2.y,
            v1.z + v2.z,
        };
    }

    Vec4 operator+(const Vec4& v1, const Vec4& v2)
    {
        return (Vec4){ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w };
    }

    Vec2 operator-(const Vec2& v1, const Vec2& v2)
    {
        return (Vec2){ v1.x - v2.x, v1.y - v2.y };
    }

    Vec3 operator-(const Vec3& v1, const Vec3& v2)
    {
        return (Vec3){
            v1.x - v2.x,
            v1.y - v2.y,
            v1.z - v2.z,
        };
    }

    Vec4 operator-(const Vec4& v1, const Vec4& v2)
    {
        return (Vec4){ v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w };
    }

    Vec2 operator*(const Vec2& v1, const Vec2& v2)
    {
        return (Vec2){ v1.x * v2.x, v1.y * v2.y };
    }

    Vec3 operator*(const Vec3& v1, const Vec3& v2)
    {
        return (Vec3){
            v1.x * v2.x,
            v1.y * v2.y,
            v1.z * v2.z,
        };
    }

    Vec4 operator*(const Vec4& v1, const Vec4& v2)
    {
        return (Vec4){ v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w };
    }

    Vec2& operator+=(Vec2& v1, const Vec2& v2)
    {
        v1 = v1 + v2;
        return v1;
    }

    Vec3& operator+=(Vec3& v1, const Vec3& v2)
    {
        v1 = v1 + v2;
        return v1;
    }

    Vec4& operator+=(Vec4& v1, const Vec4& v2)
    {
        v1 = v1 + v2;
        return v1;
    }

    Vec2& operator-=(Vec2& v1, const Vec2& v2)
    {
        v1 = v1 - v2;
        return v1;
    }

    Vec3& operator-=(Vec3& v1, const Vec3& v2)
    {
        v1 = v1 - v2;
        return v1;
    }

    Vec4& operator-=(Vec4& v1, const Vec4& v2)
    {
        v1 = v1 - v2;
        return v1;
    }

    Vec2& operator*=(Vec2& v1, const Vec2& v2)
    {
        v1 = v1 * v2;
        return v1;
    }
    Vec3& operator*=(Vec3& v1, const Vec3& v2)
    {
        v1 = v1 * v2;
        return v1;
    }

    Vec4& operator*=(Vec4& v1, const Vec4& v2)
    {
        v1 = v1 * v2;
        return v1;
    }

    Vec2 operator*(const Vec2& v1, float f) { return (Vec2){ v1.x * f, v1.y * f }; }

    Vec3 operator*(const Vec3& v1, float f)
    {
        return (Vec3){ v1.x * f, v1.y * f, v1.z * f };
    }

    Vec4 operator*(const Vec4& v1, float f)
    {
        return (Vec4){ v1.x * f, v1.y * f, v1.z * f, v1.w * f };
    }

    Vec2 operator*(float f, const Vec2& v1) { return (Vec2){ v1.x * f, v1.y * f }; }

    Vec3 operator*(float f, const Vec3& v1)
    {
        return (Vec3){ v1.x * f, v1.y * f, v1.z * f };
    }

    Vec4 operator*(float f, const Vec4& v1)
    {
        return (Vec4){ v1.x * f, v1.y * f, v1.z * f, v1.w * f };
    }

    bool operator==(const Vec2& v1, const Vec2& v2)
    {
        return v1.x == v2.x && v1.y == v2.y;
    }

    bool operator==(const Vec3& v1, const Vec3& v2)
    {
        return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
    }

    bool operator==(const Vec4& v1, const Vec4& v2)
    {
        return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w;
    }

    float sum_v2(const Vec2& v) { return (v.x + v.y); }

    float sum_v3(const Vec3& v) { return (v.x + v.y + v.z); }

    float sum_v4(const Vec4& v) { return (v.x + v.y + v.z + v.w); }

    bool operator<(const Vec2& v1, const Vec2& v2)
    {
        return (sum_v2(v1) < sum_v2(v2));
    }
    bool operator<(const Vec3& v1, const Vec3& v2)
    {
        return (sum_v3(v1) < sum_v3(v2));
    }
    bool operator<(const Vec4& v1, const Vec4& v2)
    {
        return (sum_v4(v1) < sum_v4(v2));
    }

    bool operator>(const Vec2& v1, const Vec2& v2)
    {
        return (sum_v2(v1) > sum_v2(v2));
    }
    bool operator>(const Vec3& v1, const Vec3& v2)
    {
        return (sum_v3(v1) > sum_v3(v2));
    }
    bool operator>(const Vec4& v1, const Vec4& v2)
    {
        return (sum_v4(v1) > sum_v4(v2));
    }

} // namespace synt

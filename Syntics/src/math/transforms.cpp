#include "transforms.h"
#include <math.h>

namespace synt {

    float clampf32(float value, float min, float max)
    {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }

    float vec3Len(const Vec3& v3)
    {
        return (float)sqrt((v3.x * v3.x) + (v3.y * v3.y) + (v3.z * v3.z));
    }

    float dot(const Vec3& v3One, const Vec3& v3Two)
    {
        return ((v3One.x * v3Two.x) + (v3One.y * v3Two.y) + (v3One.z * v3Two.z));
    }

    float angle(const Vec3& v3One, const Vec3& v3Two)
    {
        return acos(clampf32(dot(v3One, v3Two), -1.0f, 1.0f));
    }

    Vec3 normalize(const Vec3& v3)
    {
        float length = vec3Len(v3);
        return (Vec3){ (v3.x / length), (v3.y / length), (v3.z / length) };
    }

    Vec3 cross(const Vec3& v3One, const Vec3& v3Two)
    {
        Vec3 out;

        out.x = ((v3One.y * v3Two.z) - (v3One.z * v3Two.y));
        out.y = -((v3One.x * v3Two.z) - (v3One.z * v3Two.x));
        out.z = ((v3One.x * v3Two.y) - (v3One.y * v3Two.x));

        return out;
    }

    float radians(float deg)
    {
        const float PI = 3.1415936;
        return (deg * PI) / 180.0;
    }

    float determinant(Mat3f m3)
    {
        float out;

        out = (m3.data[0][0] * m3.data[1][1] * m3.data[2][2]) +
              (m3.data[0][1] * m3.data[1][2] * m3.data[0][2]) +
              (m3.data[0][2] * m3.data[1][0] * m3.data[2][1]) -
              (m3.data[0][2] * m3.data[1][1] * m3.data[0][2]) -
              (m3.data[0][1] * m3.data[1][0] * m3.data[2][2]) -
              (m3.data[0][0] * m3.data[1][2] * m3.data[2][1]);

        return out;
    }

    float determinant(Mat4f m4)
    {
        float out;

        return out;
    }

    Mat3f transpose(Mat3f m3)
    {
        Mat3f out;

        out.data[0][0] = m3.data[0][0];
        out.data[0][1] = m3.data[1][0];
        out.data[0][2] = m3.data[2][0];

        out.data[1][0] = m3.data[0][1];
        out.data[1][1] = m3.data[1][1];
        out.data[1][2] = m3.data[2][1];

        out.data[2][0] = m3.data[0][2];
        out.data[2][1] = m3.data[1][2];
        out.data[2][2] = m3.data[2][2];

        return out;
    }

    Mat4f transpose(Mat4f m4)
    {
        Mat4f out;

        out.data[0][0] = m4.data[0][0];
        out.data[0][1] = m4.data[1][0];
        out.data[0][2] = m4.data[2][0];
        out.data[0][3] = m4.data[3][0];

        out.data[1][0] = m4.data[0][1];
        out.data[1][1] = m4.data[1][1];
        out.data[1][2] = m4.data[2][1];
        out.data[1][3] = m4.data[3][1];

        out.data[2][0] = m4.data[0][2];
        out.data[2][1] = m4.data[1][2];
        out.data[2][2] = m4.data[2][2];
        out.data[2][3] = m4.data[3][2];

        out.data[3][0] = m4.data[0][3];
        out.data[3][1] = m4.data[1][3];
        out.data[3][2] = m4.data[2][3];
        out.data[3][3] = m4.data[3][3];

        return out;
    }

    Mat3f rotate(Mat3f m3, double rad)
    {
        return (Mat3f){ { { (float)cos(rad), (float)sin(rad), m3.data[2][0] },
                          { (float)-sin(rad), (float)cos(rad), m3.data[2][1] },
                          { 0.0f, 0.0f, m3.data[2][2] } } };
    }

    static inline Mat4f RotateX(const Mat4f* m4, double rad)
    {
        return (Mat4f){
            { { m4->data[0][0], m4->data[1][0], m4->data[2][0], m4->data[3][0] },
              { m4->data[0][1], (float)cos(rad), (float)sin(rad), m4->data[3][1] },
              { m4->data[0][2], (float)-sin(rad), (float)cos(rad), m4->data[3][2] },
              { m4->data[0][3], m4->data[1][3], m4->data[2][3], m4->data[3][3] } }
        };
    }

    static inline Mat4f RotateY(const Mat4f* m4, double rad)
    {
        return (Mat4f){
            { { (float)cos(rad), m4->data[1][0], (float)-sin(rad), m4->data[3][0] },
              { m4->data[0][1], m4->data[1][1], m4->data[2][1], m4->data[3][1] },
              { (float)sin(rad), m4->data[1][2], (float)cos(rad), m4->data[3][2] },
              { m4->data[0][3], m4->data[1][3], m4->data[2][3], m4->data[3][3] } }
        };
    }

    static inline Mat4f RotateZ(const Mat4f* m4, double rad)
    {
        return (Mat4f){
            { { (float)cos(rad), (float)sin(rad), m4->data[2][0], m4->data[3][0] },
              { (float)-sin(rad), (float)cos(rad), m4->data[2][1], m4->data[3][1] },
              { m4->data[0][2], m4->data[1][2], m4->data[2][2], m4->data[3][2] },
              { m4->data[0][3], m4->data[1][3], m4->data[2][3], m4->data[3][3] } }
        };
    }

    Mat4f rotate(Mat4f m4, double rad, Axis axis)
    {

        switch (axis)
        {
            case X:
            {
                return RotateX(&m4, rad);
            }
            case Y:
            {
                return RotateY(&m4, rad);
            }
            case Z:
            {
                return RotateZ(&m4, rad);
            }
            case XY:
            {
                Mat4f tempX = RotateX(&m4, rad);
                Mat4f tempY = RotateY(&m4, rad);
                return tempY * tempX;
            }
            case XZ:
            {
                Mat4f tempX = RotateX(&m4, rad);
                return RotateZ(&tempX, rad);
            }
            case YZ:
            {
                Mat4f tempY = RotateY(&m4, rad);
                return RotateZ(&tempY, rad);
            }
            case XYZ:
            {
                Mat4f tempX = RotateX(&m4, rad);
                Mat4f tempY = RotateY(&tempX, rad);
                return RotateZ(&tempY, rad);
            }
            default: return m4;
        }
    }

    Vec3 rotate(Vec3 v3, double rad, Vec3 normal)
    {
        float cos = (float)std::cos(radians(rad));
        float sin = (float)std::sin(radians(rad));

        return (v3 * cos + ((v3 * normal) * (1.0f - cos)) * normal +
                cross(v3, normal) * sin);
    }

    Mat3f translate(Mat3f m3, Vec2 v2)
    {
        Mat3f out;

        out.data[0][0] = m3.data[0][0];
        out.data[0][1] = m3.data[0][1];
        out.data[0][2] = m3.data[0][2];

        out.data[1][0] = m3.data[1][0];
        out.data[1][1] = m3.data[1][1];
        out.data[1][2] = m3.data[1][2];

        const Vec3 temp1 = {
            m3.data[0][0] * v2.x,
            m3.data[0][1] * v2.x,
            m3.data[0][2] * v2.x,
        };
        const Vec3 temp2 = {
            m3.data[1][0] * v2.y,
            m3.data[1][1] * v2.y,
            m3.data[1][2] * v2.y,
        };
        const Vec3 temp3 = {
            m3.data[2][0],
            m3.data[2][1],
            m3.data[2][2],
        };
        const Vec3 res = ((temp1 + temp2) + temp3);

        out.data[2][0] = res.x;
        out.data[2][1] = res.y;
        out.data[2][2] = res.z;

        return out;
    }

    Mat4f translate(Mat4f m4, Vec3 v3)
    {
        Mat4f out;

        out.data[0][0] = m4.data[0][0];
        out.data[0][1] = m4.data[0][1];
        out.data[0][2] = m4.data[0][2];
        out.data[0][3] = m4.data[0][3];

        out.data[1][0] = m4.data[1][0];
        out.data[1][1] = m4.data[1][1];
        out.data[1][2] = m4.data[1][2];
        out.data[1][3] = m4.data[1][3];

        out.data[2][0] = m4.data[2][0];
        out.data[2][1] = m4.data[2][1];
        out.data[2][2] = m4.data[2][2];
        out.data[2][3] = m4.data[2][3];

        const Vec4 temp1 = {
            m4.data[0][0] * v3.x,
            m4.data[0][1] * v3.x,
            m4.data[0][2] * v3.x,
            m4.data[0][3] * v3.x,
        };
        const Vec4 temp2 = {
            m4.data[1][0] * v3.y,
            m4.data[1][1] * v3.y,
            m4.data[1][2] * v3.y,
            m4.data[1][3] * v3.y,
        };
        const Vec4 temp3 = {
            m4.data[2][0] * v3.z,
            m4.data[2][1] * v3.z,
            m4.data[2][2] * v3.z,
            m4.data[2][3] * v3.z,
        };
        const Vec4 temp4 = {
            m4.data[3][0],
            m4.data[3][1],
            m4.data[3][2],
            m4.data[3][3],
        };
        const Vec4 res = ((temp1 + temp2) + (temp3 + temp4));

        out.data[3][0] = res.x;
        out.data[3][1] = res.y;
        out.data[3][2] = res.z;
        out.data[3][3] = res.w;

        return out;
    }

    Mat3f scale(Mat3f m3, Vec2 v2)
    {
        Mat3f out;

        out.data[0][0] = m3.data[0][0] * v2.x;
        out.data[0][1] = m3.data[0][1] * v2.x;
        out.data[0][2] = m3.data[0][2] * v2.x;

        out.data[1][0] = m3.data[1][0] * v2.y;
        out.data[1][1] = m3.data[1][1] * v2.y;
        out.data[1][2] = m3.data[1][2] * v2.y;

        out.data[2][0] = m3.data[2][0];
        out.data[2][1] = m3.data[2][1];
        out.data[2][2] = m3.data[2][2];

        return out;
    }

    Mat4f scale(Mat4f m4, Vec3 v3)
    {
        Mat4f out;

        out.data[0][0] = m4.data[0][0] * v3.x;
        out.data[0][1] = m4.data[0][1] * v3.x;
        out.data[0][2] = m4.data[0][2] * v3.x;
        out.data[0][3] = m4.data[0][3] * v3.x;

        out.data[1][0] = m4.data[1][0] * v3.y;
        out.data[1][1] = m4.data[1][1] * v3.y;
        out.data[1][2] = m4.data[1][2] * v3.y;
        out.data[1][3] = m4.data[1][3] * v3.y;

        out.data[2][0] = m4.data[2][0] * v3.z;
        out.data[2][1] = m4.data[2][1] * v3.z;
        out.data[2][2] = m4.data[2][2] * v3.z;
        out.data[2][3] = m4.data[2][3] * v3.z;

        out.data[3][0] = m4.data[3][0];
        out.data[3][1] = m4.data[3][1];
        out.data[3][2] = m4.data[3][2];
        out.data[3][3] = m4.data[3][3];

        return out;
    }

    Mat4f view(Vec3 eye, Vec3 center, Vec3 up)
    {
        Mat4f out = mat4i(1.0f);

        const Vec3 temp1 = normalize(center - eye);
        const Vec3 temp2 = normalize(cross(temp1, up));
        const Vec3 temp3 = cross(temp2, temp1);

        out.data[0][0] = temp2.x;
        out.data[0][1] = temp3.x;
        out.data[0][2] = -temp1.x;

        out.data[1][0] = temp2.y;
        out.data[1][1] = temp3.y;
        out.data[1][2] = -temp1.y;

        out.data[2][1] = temp3.z;
        out.data[2][0] = temp2.z;
        out.data[2][2] = -temp1.z;

        out.data[3][0] = -dot(temp2, eye);
        out.data[3][1] = -dot(temp3, eye);
        out.data[3][2] = dot(temp1, eye);

        return out;
    }

    Mat4f perspective(float fovy, float aspect, float near, float far)
    {
        Mat4f out = mat4i(0.0f);

        const float fovyTemp = (float)tan(fovy / 2.0f);

        out.data[0][0] = (1.0f / (aspect * fovyTemp));
        out.data[1][1] = -(1.0f / (fovyTemp));
        out.data[2][2] = (far / (near - far));
        out.data[2][3] = -1.0f;
        out.data[3][2] = (-(far * near) / (far - near));

        return out;
    }

} // namespace synt

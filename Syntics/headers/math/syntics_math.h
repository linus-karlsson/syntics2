#pragma once

#define V3_FMT(v) "(x: %f, y: %f, z: %f)\n", (v).x, (v).y, (v).z
#define V4_FMT(v) "(x: %f, y: %f, z: %f, w: %f)\n", (v).x, (v).y, (v).z, (v).w

#define M3_FMT(m)                                                                   \
    "|%f,%f,%f|\n|%f,%f,%f|\n|%f,%f,%f|\n\n", (m).data[0][0], (m).data[1][0],       \
        (m).data[2][0], (m).data[0][1], (m).data[1][1], (m).data[2][1],             \
        (m).data[0][2], (m).data[1][2], (m).data[2][2]

#define M4_FMT(m)                                                                   \
    "|%f,%f,%f,%f|\n|%f,%f,%f,%f|\n|%f,%f,%f,%f|\n|%f,%f,%f,%f|\n\n",               \
        (m).data[0][0], (m).data[1][0], (m).data[2][0], (m).data[3][0],             \
        (m).data[0][1], (m).data[1][1], (m).data[2][1], (m).data[3][1],             \
        (m).data[0][2], (m).data[1][2], (m).data[2][2], (m).data[3][2],             \
        (m).data[0][3], (m).data[1][3], (m).data[2][3], (m).data[3][3]

typedef struct V2 V2;
struct V2
{
    union
    {
        struct
        {
            f32 x;
            f32 y;
        };

        struct
        {
            f32 width;
            f32 height;
        };

        struct
        {
            f32 min;
            f32 max;
        };

        struct
        {
            f32 u;
            f32 v;
        };

        struct
        {
            f32 l;
            f32 r;
        };
    };
};

typedef struct V3 V3;
struct V3
{
    union
    {
        struct
        {
            f32 x;
            f32 y;
            f32 z;
        };
        struct
        {
            f32 r;
            f32 g;
            f32 b;
        };
    };
};

typedef struct V4 V4;
struct V4
{
    union
    {
        struct
        {
            f32 x;
            f32 y;
            f32 z;
            f32 w;
        };

        struct
        {
            f32 r;
            f32 g;
            f32 b;
            f32 a;
        };
    };
};

typedef struct P2
{
    f32 x;
    f32 y;
} P2;

typedef struct P3
{
    f32 x;
    f32 y;
    f32 z;
} P3;

typedef struct P4
{
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} P4;

typedef struct Mat2f
{
    f32 data[2][2];
} Mat2f, M2;

typedef struct Mat3f
{
    f32 data[3][3];
} Mat3f, M3;

typedef struct Mat4f
{
    f32 data[4][4];
} Mat4f, M4;

typedef struct Vertex
{
    V3 pos;
    V3 normal;
    V2 tex_coords;
    V4 color;
    f32 tex_index;
} Vertex;

typedef struct VP
{
    M4 view;
    M4 proj;
} VP;

typedef struct Push_Color
{
    V4 color;
} Push_Color;

typedef struct Polygon2D
{
    V2 pos;
    V2 vel;
    V2* points;
    V2* normals;
    u32 n_sides;
    u32 id;
} Polygon2D;

typedef struct Quad2D
{
    V2 pos;
    V2 vel;
    V2 points[4];
    V2 normals[4];
    u32 id;
} Quad2D;

typedef struct Quad3D
{
    V2 pos;
    V2 vel;
    P2 points[4];
    V2 normals[8];
    u32 id;
} Quad3D;

typedef struct Rect2D
{
    V2 pos;
    V2 size;
    V4 color;
    V2 vel;
    u32 id;
} Rect2D;

typedef struct Rect3D
{
    V3 pos;
    V3 size;
    u32 id;
    f32 misc; // Filling out padding
} Rect3D;

typedef struct Plane
{
    V3 n;
    float d;
} Plane;

typedef enum Axis
{
    X,
    Y,
    Z,
} Axis;

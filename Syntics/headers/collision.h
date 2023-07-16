#pragma once

typedef struct AABB_3D
{
    V3 min;
    V3 size;
    u32 id;
} AABB_3D;

typedef struct AABB_2D
{
    V2 min;
    V2 size;
    u32 id;
} AABB_2D;

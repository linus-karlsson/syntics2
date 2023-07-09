#pragma once

typedef struct Camera_3D
{
    VP vp;
    V3 acc;
    V3 vel;
    V3 pos;
    V3 ori;
    V3 up;
    f32 speed;
    f32 sens;

} Camera_3D;


typedef struct Camera_2D
{
    VP vp;
    V2 acc;
    V2 vel;
    V2 pos;
    V3 ori;
    V3 up;
    f32 speed;
    f32 sens;
    f32 z;

} Camera_2D;

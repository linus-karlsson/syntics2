#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#include "math/syntics_math.h"
#endif

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

Camera_2D camera_2dd(void);
Camera_2D camera_2di(f32 speed, f32 sensitivity);
Camera_3D camera_3dd(void);
Camera_3D camera_3di(f32 speed, f32 sensitivity);

b8 camera_update(Camera_3D* camera, const Platform* platform, f32 delta_time, b8 off_the_ground, b8 edit_mode);
void camera_print(const Camera_3D* camera);

V2 mouse_get_rotation(const Platform* platform, f32 sens, b8* first_clicked, i16* last_x, i16* last_y, f32 delta_time);


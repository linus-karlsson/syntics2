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

/*
Camera_3D cam_3dd(void);
Camera_3D cam_3di(f32 speed, f32 sensitivity);

Camera_2D cam_2dd(void);
Camera_2D cam_2di(f32 speed, f32 sensitivity);

b8 update_camera(Camera_3D* camera, const Events* mouse_evt, f32 delta_time, b8 off_the_ground, b8 edit_mode);

void print_camera(const Camera_3D* camera);
*/

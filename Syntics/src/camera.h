#pragma once

#include "math/transforms.h"

typedef struct Events Events;

typedef struct Camera
{
    Camera();
    Camera(f32 speed, f32 sensitivity);

    MVP mvp;
    V3 acc;
    V3 vel;
    V3 pos;
    V3 ori;
    V3 up;
    f32 speed;
    f32 sens;

} Camera;

void update_camera(Camera* camera, const Events* mouse_evt, f32 delta_time);

void print_camera(const Camera& camera);


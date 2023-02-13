#pragma once

#include "vulkan_types.h"

typedef struct Events Events;

typedef struct Camera
{
    Camera();
    Camera(f32 speed, f32 sensitivity);

    MVP mvp;
    Vec3 velocity;
    Vec3 position;
    Vec3 orientation;
    Vec3 up;
    f32 speed;
    f32 sensitivity;

} Camera;

void update_camera(Camera* camera, const Events* mouse_evt, f32 delta_time);

void print_camera(const Camera& camera);


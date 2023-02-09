#pragma once

#include "vulkan_types.h"

typedef struct Events Events;

typedef struct Camera
{
    Camera();
    Camera(float speed, float sensitivity);

    MVP mvp;
    Vec3 velocity;
    Vec3 position;
    Vec3 orientation;
    Vec3 up;
    float speed;
    float sensitivity;

} Camera;

void update_camera(Camera* camera, const Events* mouse_evt, float delta_time);

void print_camera(const Camera& camera);


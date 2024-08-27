#ifndef SY_UNIT_BUILD
#include "camera.h"
#include "math/syntics_math.h"
#include "syntics_platform.h"
#include "event_system.h"
#endif

Camera_2D camera_2dd(void)
{
    Camera_2D res;
    res.position = v2d();
    res.up = v3f(0.0f, 1.0f, 0.0f);
    res.speed = 1.5f;
    res.sensitivity = 5.0f;
    return res;
}

Camera_2D camera_2di(f32 speed, f32 sensitivity)
{
    Camera_2D res;
    res.position = v2d();
    res.up = v3f(0.0f, 1.0f, 0.0f);
    res.speed = speed;
    res.sensitivity = sensitivity;
    return res;
}

Camera_3D camera_3dd(void)
{
    Camera_3D res;
    res.position = v3f(0.0f, 0.0f, 1.0f);
    res.orientation = v3f(0.0f, 0.0f, -1.0f);
    res.up = v3f(0.0f, 1.0f, 0.0f);
    res.velocity = v3d();
    res.vp.view = view(res.position, v3_add(res.position, res.orientation), res.up);
    res.speed = 1.5f;
    res.sensitivity = 5.0f;
    return res;
}

Camera_3D camera_3di(f32 speed, f32 sensitivity)
{
    Camera_3D res;
    res.position = v3f(0.0f, 0.0f, 0.0f);
    res.orientation = v3f(0.0f, 0.0f, -1.0f);
    res.up = v3f(0.0f, 1.0f, 0.0f);
    res.velocity = v3d();
    res.vp.view = view(res.position, v3_add(res.position, res.orientation), res.up);
    res.speed = speed;
    res.sensitivity = sensitivity;
    return res;
}

V2 mouse_get_rotation(const Platform* platform, f32 sensitivity, b8* first_clicked, i16* last_x,
                      i16* last_y, f32 delta_time)
{
    u16 width, height;
    platform_window_get_size(platform, &width, &height);

    const u16 half_width = width / 2;
    const u16 half_height = height / 2;

    i16 mouse_x, mouse_y;
    platform_cursor_get_pos(&mouse_x, &mouse_y);

    if (mouse_x >= width - 300 || mouse_x <= 300)
    {
        platform_cursor_set_pos(platform, half_width, mouse_y);
        mouse_x = half_width;
        *last_x = mouse_x;
    }
    if (mouse_y >= height - 200 || mouse_y <= 200)
    {
        platform_cursor_set_pos(platform, mouse_x, half_height);
        mouse_y = half_height;
        *last_y = mouse_y;
    }

    V2 rotation = { 0 };

    if (!*first_clicked)
    {
        rotation.x = sensitivity * (f32)((mouse_x - *last_x)) * delta_time;
        rotation.y = sensitivity * (f32)((mouse_y - *last_y)) * delta_time;
    }
    else
        *first_clicked = false;

    *last_x = mouse_x;
    *last_y = mouse_y;
    return rotation;
}

b8 camera_update(Camera_3D* camera, const Platform* platform, f32 delta_time, b8 off_the_ground,
                 b8 edit_mode)
{

    b8 moved = false;
    if (edit_mode)
    {
        if (event_is_key_pressed(SYNT_KEY_W))
        {
            v3_add_equal(&camera->position,
                         v3_s_multi(camera->orientation, (camera->speed * delta_time)));
            moved = true;
        }
        if (event_is_key_pressed(SYNT_KEY_S))
        {
            v3_add_equal(&camera->position, v3_s_multi(v3_s_multi(camera->orientation, -1.0f),
                                                       (camera->speed * delta_time)));
            moved = true;
        }
        if (event_is_key_pressed(SYNT_KEY_A))
        {
            v3_add_equal(
                &camera->position,
                v3_s_multi(
                    v3_s_multi(v3_normalize(v3_cross(camera->orientation, camera->up)), -1.0f),
                    (camera->speed * delta_time)));
            moved = true;
        }
        if (event_is_key_pressed(SYNT_KEY_D))
        {
            v3_add_equal(&camera->position,
                         v3_s_multi(v3_normalize(v3_cross(camera->orientation, camera->up)),
                                    (camera->speed * delta_time)));
            moved = true;
        }
        if (event_is_key_pressed(SYNT_KEY_SPACE))
        {
            v3_add_equal(&camera->position, v3_s_multi(camera->up, (camera->speed * delta_time)));
            moved = true;
        }
        if (event_is_key_pressed(SYNT_KEY_CTRL))
        {
            v3_add_equal(&camera->position,
                         v3_s_multi(v3_s_multi(camera->up, -1.0f), (camera->speed * delta_time)));
            moved = true;
        }

        presist f32 old_speed = 0;
        presist b8 first = true;
        if (first)
        {
            old_speed = camera->speed;
            first = false;
        }
        if (event_is_key_pressed(SYNT_KEY_SHIFT))
        {
            camera->speed = old_speed * 4.0f;
        }
        else
        {
            camera->speed = old_speed;
        }

        const Mouse_Button_Event* mouse_evt = event_get_mouse_button_event();
        if (mouse_evt->activated)
        {
            presist b8 first_clicked = false;
            if (mouse_evt->action == SYNT_PRESS && mouse_evt->button == SYNT_RIGHT_BUTTON)
            {
                platform_cursor_hide(platform);

                static i16 last_x = 0;
                static i16 last_y = 0;
                V2 rotation = mouse_get_rotation(platform, camera->sensitivity, &first_clicked, &last_x,
                                                 &last_y, delta_time);

                V3 temp_orientation =
                    v3_rotate(camera->orientation, radians(rotation.y),
                              v3_normalize(v3_cross(camera->orientation, camera->up)));

                if (abs_f32(v3_angle(temp_orientation, camera->up) - radians(90.0f)) <=
                    radians(85.0f))
                {
                    camera->orientation = temp_orientation;
                }

                camera->orientation =
                    v3_rotate(camera->orientation, radians(rotation.x), camera->up);
            }
            else if (mouse_evt->action == SYNT_RELEASE && !first_clicked)
            {
                platform_cursor_show_last_pos(platform);
                first_clicked = true;
            }
        }
    }

    return moved;
}

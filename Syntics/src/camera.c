#include "camera.h"
#include "event_system.h"
#include "logging.h"

Camera_3D cam_3dd(void)
{
    Camera_3D res;
    res.pos = v3f(0.0f, 0.0f, 1.0f);
    res.ori = v3f(0.0f, 0.0f, -1.0f);
    res.up = v3f(0.0f, 1.0f, 0.0f);
    res.mvp.model = m4_scale(m4i(1.0f), v3f(1.0f, 1.0f, 1.0f));
    res.mvp.view = view(res.pos, v3_add(res.pos, res.ori), res.up);
    res.speed = 1.5f;
    res.sens = 5.0f;
    return res;
}

Camera_3D cam_3di(f32 speed, f32 sensitivity)
{
    Camera_3D res;
    res.pos = v3f(0.0f, 0.0f, 1.0f);
    res.ori = v3f(0.0f, 0.0f, -1.0f);
    res.up = v3f(0.0f, 1.0f, 0.0f);
    res.mvp.model = m4_scale(m4i(1.0f), v3f(1.0f, 1.0f, 1.0f));
    res.mvp.view = view(res.pos, v3_add(res.pos, res.ori), res.up);
    res.speed = speed;
    res.sens = sensitivity;
    return res;
}

Camera_2D cam_2dd(void)
{
    Camera_2D res;
    res.pos = v2d();
    res.up = v3f(0.0f, 1.0f, 0.0f);
    res.speed = 1.5f;
    res.sens = 5.0f;
    return res;
}

Camera_2D cam_2di(f32 speed, f32 sensitivity)
{
    Camera_2D res;
    res.pos = v2d();
    res.up = v3f(0.0f, 1.0f, 0.0f);
    res.speed = speed;
    res.sens = sensitivity;
    return res;
}

void update_camera(Camera_3D* camera, const Events* mouse_evt, f32 delta_time)
{
    if (is_key_pressed(SYNT_KEY_W))
    {
        v3_add_equal(&camera->pos,
                     v3_s_multi(camera->ori, (camera->speed * delta_time)));
    }
    if (is_key_pressed(SYNT_KEY_A))
    {
        v3_add_equal(
            &camera->pos,
            v3_s_multi(
                v3_s_multi(v3_normalize(v3_cross(camera->ori, camera->up)), -1.0f),
                (camera->speed * delta_time)));
    }
    if (is_key_pressed(SYNT_KEY_S))
    {
        v3_add_equal(&camera->pos, v3_s_multi(v3_s_multi(camera->ori, -1.0f),
                                              (camera->speed * delta_time)));
    }
    if (is_key_pressed(SYNT_KEY_D))
    {
        v3_add_equal(&camera->pos,
                     v3_s_multi(v3_normalize(v3_cross(camera->ori, camera->up)),
                                (camera->speed * delta_time)));
    }
    if (is_key_pressed(SYNT_KEY_SPACE))
    {
        v3_add_equal(&camera->pos,
                     v3_s_multi(camera->up, (camera->speed * delta_time)));
    }
    if (is_key_pressed(SYNT_KEY_CTRL))
    {
        v3_add_equal(&camera->pos, v3_s_multi(v3_s_multi(camera->up, -1.0f),
                                              (camera->speed * delta_time)));
    }

    static f32 old_speed = 0;
    static b8 first = true;
    if (first)
    {
        old_speed = camera->speed;
        first = false;
    }
    if (is_key_pressed(SYNT_KEY_SHIFT))
    {
        camera->speed = old_speed * 2.5f;
    }
    else if (!is_key_pressed(SYNT_KEY_SHIFT))
    {
        camera->speed = old_speed;
    }

    if (mouse_evt->activated)
    {
        static b8 first_clicked = false;
        if (mouse_evt->mouse_evt.button_evt.action == SYNT_BUTTON_PRESS)
        {
            hide_cursor();

            u16 width, height;
            get_window_size(&width, &height);

            const u16 half_width = width / 2;
            const u16 half_height = height / 2;

            i16 mouse_x = mouse_evt->mouse_evt.move_evt.pos_x;
            i16 mouse_y = mouse_evt->mouse_evt.move_evt.pos_y;

            static i16 last_x = 0;
            static i16 last_y = 0;

            if (mouse_x >= width - 300 || mouse_x <= 300)
            {
                set_mouse_pos(half_width, mouse_y);
                mouse_x = half_width;
                last_x = mouse_x;
            }
            if (mouse_y >= height - 200 || mouse_y <= 200)
            {
                set_mouse_pos(mouse_x, half_height);
                mouse_y = half_height;
                last_y = mouse_y;
            }

            f32 rotation_x = 0.0f;
            f32 rotation_y = 0.0f;

            if (!first_clicked)
            {
                rotation_x = camera->sens * (f32)((mouse_y - last_y));
                rotation_y = camera->sens * (f32)((mouse_x - last_x));
            }
            else
                first_clicked = false;

            last_x = mouse_x;
            last_y = mouse_y;

            V3 temp_orientation =
                v3_rotate(camera->ori, radians(rotation_x),
                          v3_normalize(v3_cross(camera->ori, camera->up)));

            if (abs_f32(v3_angle(temp_orientation, camera->up) - radians(90.0f)) <=
                radians(85.0f))
            {
                camera->ori = temp_orientation;
            }

            camera->ori = v3_rotate(camera->ori, radians(rotation_y), camera->up);
        }
        else if (mouse_evt->mouse_evt.button_evt.action == SYNT_BUTTON_RELEASE &&
                 !first_clicked)
        {
            show_cursor_last_pos();
            first_clicked = true;
        }
    }
}

void print_camera(const Camera_3D* camera)
{
    synt_LOG_Term("Pos: (x: %f, y: %f, z: %f)\n", camera->pos.x, camera->pos.y,
                  camera->pos.z);

    synt_LOG_Term("Ori: (x: %f, y: %f, z: %f)\n", camera->ori.x, camera->ori.y,
                  camera->ori.z);
}


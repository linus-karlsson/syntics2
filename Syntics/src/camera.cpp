#include "camera.h"
#include "event_system.h"
#include "logging.h"

Camera::Camera()
    : position(v3f(0.0f, 0.0f, -1.0f)), up(v3f(0.0f, 1.0f, 0.0f)), speed(1.5f),
      sensitivity(5.0f)
{
}

Camera::Camera(f32 speed, f32 sensitivity)
    : position(v3f(0.0f, 0.0f, -1.0f)), up(v3f(0.0f, 1.0f, 0.0f)), speed(speed),
      sensitivity(sensitivity)
{
}

void update_camera(Camera* camera, const Events* mouse_evt, f32 delta_time)
{
    if (is_key_pressed(SYNT_W_PRESSED))
    {
        camera->position += ((camera->speed * delta_time) * camera->orientation);
    }
    if (is_key_pressed(SYNT_A_PRESSED))
    {
        camera->position +=
            ((camera->speed * delta_time) *
             (-1.0f * normalize(cross(camera->orientation, camera->up))));
    }
    if (is_key_pressed(SYNT_S_PRESSED))
    {
        camera->position +=
            ((camera->speed * delta_time) * (camera->orientation * -1.0f));
    }
    if (is_key_pressed(SYNT_D_PRESSED))
    {
        camera->position += ((camera->speed * delta_time) *
                             normalize(cross(camera->orientation, camera->up)));
    }
    if (is_key_pressed(SYNT_SPACE_PRESSED))
    {
        camera->position += ((camera->speed * delta_time) * camera->up);
    }
    if (is_key_pressed(SYNT_CTRL_PRESSED))
    {
        camera->position += ((camera->speed * delta_time) * (-1.0f * camera->up));
    }

    static f32 old_speed = camera->speed;
    if (is_key_pressed(SYNT_SHIFT_PRESSED))
    {
        camera->speed = old_speed * 2.5f;
    }
    else if (!is_key_pressed(SYNT_SHIFT_PRESSED))
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

            static i16 last_x = mouse_x;
            static i16 last_y = mouse_y;

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
                rotation_x = camera->sensitivity * (f32)((mouse_y - last_y));
                rotation_y = camera->sensitivity * (f32)((mouse_x - last_x));
            }
            else
                first_clicked = false;

            last_x = mouse_x;
            last_y = mouse_y;

            Vec3 temp_orientation =
                rotate(camera->orientation, radians(rotation_x),
                       normalize(cross(camera->orientation, camera->up)));

            if (abs_f32(angle(temp_orientation, camera->up) - radians(90.0f)) <=
                radians(85.0f))
            {
                camera->orientation = temp_orientation;
            }

            camera->orientation =
                rotate(camera->orientation, radians(rotation_y), camera->up);
        }
        else if (mouse_evt->mouse_evt.button_evt.action == SYNT_BUTTON_RELEASE &&
                 !first_clicked)
        {
            show_cursor_last_pos();
            first_clicked = true;
        }
    }
}

void print_camera(const Camera& camera)
{
    synt_LOG_Term("Pos: (x: %f, y: %f, z: %f)\n", camera.position.x,
                  camera.position.y, camera.position.z);

    synt_LOG_Term("Orientation: (x: %f, y: %f, z: %f)\n", camera.orientation.x,
                  camera.orientation.y, camera.orientation.z);
}


#include "camera.h"
#include "event_system.h"
#include "logging.h"
#include <math.h>
#include <stdio.h>

namespace synt {

Camera::Camera()
    : position(v3f(0.0f, 0.0f, -1.0f)), up(v3f(0.0f, 1.0f, 0.0f)), speed(1.5f),
      sensitivity(5.0f)
{
}

Camera::Camera(float speed, float sensitivity)
    : position(v3f(0.0f, 0.0f, -1.0f)), up(v3f(0.0f, 1.0f, 0.0f)), speed(speed),
      sensitivity(sensitivity)
{
}

void update_camera(Camera* camera, const Events* mouse_evt, float delta_time)
{
    if (is_key_pressed(SYNT_W_PRESSED))
    {
        camera->position += ((camera->speed * delta_time) * camera->orientation);
    }
    if (is_key_pressed(SYNT_A_PRESSED))
    {
        camera->position +=
            ((camera->speed * delta_time) *
             (-1.0f * synt::normalize(synt::cross(camera->orientation, camera->up))));
    }
    if (is_key_pressed(SYNT_S_PRESSED))
    {
        camera->position +=
            ((camera->speed * delta_time) * (camera->orientation * -1.0f));
    }
    if (is_key_pressed(SYNT_D_PRESSED))
    {
        camera->position +=
            ((camera->speed * delta_time) *
             synt::normalize(synt::cross(camera->orientation, camera->up)));
    }
    if (is_key_pressed(SYNT_SPACE_PRESSED))
    {
        camera->position += ((camera->speed * delta_time) * camera->up);
    }
    if (is_key_pressed(SYNT_CTRL_PRESSED))
    {
        camera->position += ((camera->speed * delta_time) * (-1.0f * camera->up));
    }

    static float old_speed = camera->speed;
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
        static bool first_clicked = true;
        if (mouse_evt->mouse_evt.button_evt.action == SYNT_BUTTON_PRESS)
        {
            hide_cursor();

            uint16 width, height;
            get_window_size(&width, &height);

            float mouse_x = (float)mouse_evt->mouse_evt.move_evt.pos_x;
            float mouse_y = (float)mouse_evt->mouse_evt.move_evt.pos_y;

            static float last_x = mouse_x;
            static float last_y = mouse_y;

            if (mouse_x >= (float)width - 30.0f)
            {
                set_mouse_pos(width / 2, mouse_y);
                mouse_x = width / 2.0f;
                last_x  = mouse_x;
            }
            else if (mouse_x <= 30.0f)
            {
                set_mouse_pos(width / 2, mouse_y);
                mouse_x = width / 2.0f;
                last_x  = mouse_x;
            }

            if (mouse_y >= (float)height - 30.0f)
            {
                set_mouse_pos(mouse_x, height / 2);
                mouse_y = height / 2.0f;
                last_y  = mouse_y;
            }
            else if (mouse_y <= 30.0f)
            {
                set_mouse_pos(mouse_x, height / 2);
                mouse_y = height / 2.0f;
                last_y  = mouse_y;
            }

            float rotation_x = 0.0f;
            float rotation_y = 0.0f;

            if (!first_clicked)
            {
                rotation_x = camera->sensitivity * (float)((mouse_y - last_y));
                rotation_y = camera->sensitivity * (float)((mouse_x - last_x));
            }
            else
                first_clicked = false;

            last_x = mouse_x;
            last_y = mouse_y;

            synt::Vec3 temp_orientation = synt::rotate(
                camera->orientation, synt::radians(rotation_x),
                synt::normalize(synt::cross(camera->orientation, camera->up)));

            if (abs(synt::angle(temp_orientation, camera->up) - synt::radians(90.0f)) <=
                synt::radians(85.0f))
            {
                camera->orientation = temp_orientation;
            }

            camera->orientation =
                synt::rotate(camera->orientation, synt::radians(rotation_y), camera->up);
        }
        else if (mouse_evt->mouse_evt.button_evt.action == SYNT_BUTTON_RELEASE)
        {
            show_cursor_last_pos();
            first_clicked = true;
        }
    }

    camera->mvp.view =
        synt::view(camera->position, camera->position + camera->orientation, camera->up);
}

void print_camera(const Camera& camera)
{
    synt_LOG("Pos: (x: %f, y: %f, z: %f)\n", camera.position.x, camera.position.y,
             camera.position.z);

    synt_LOG("Orientation: (x: %f, y: %f, z: %f)\n", camera.orientation.x,
             camera.orientation.y, camera.orientation.z);
}

} // namespace synt

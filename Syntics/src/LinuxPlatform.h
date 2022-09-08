#pragma once

#include <stdbool.h>
#include <xcb/xcb.h>

namespace synt {

    typedef struct Linux_Platform
    {
        int screen_number;
        xcb_connection_t* connection;
        xcb_screen_t* screen;
        xcb_window_t window;
        uint16_t width, height;

    } Linux_Platform;

    void init_platform(Linux_Platform* xcb, uint16_t width, uint16_t height);

    void set_event_callbacks(void (*on_key_pressed)(uint8_t key),
                             void (*on_key_released)(uint8_t key),
                             void (*on_button_pressed)(uint8_t key),
                             void (*on_button_released)(uint8_t key),
                             void (*on_mouse_move)(uint16_t pos_x, uint16_t pos_y),
                             void (*set_window_focused)(bool focused));

    void event_fire();

    void shut_down_platform();

} // namespace synt


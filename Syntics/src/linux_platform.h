#pragma once

#include "defines.h"
#include <xcb/xcb.h>

namespace synt {

typedef struct Linux_Platform
{
    int screen_number;
    xcb_connection_t* connection;
    xcb_screen_t* screen;
    xcb_window_t window;
    uint16 width, height;

} Linux_Platform;

const Linux_Platform& get_platform_state();

void init_platform(const char* title, uint16 width, uint16 height);

void set_event_callbacks(void (*on_key_pressed)(uint8 key),
                         void (*on_key_released)(uint8 key),
                         void (*on_button_pressed)(uint8 key),
                         void (*on_button_released)(uint8 key),
                         void (*on_mouse_move)(uint16 pos_x, uint16 pos_y),
                         void (*set_window_focused)(bool focused));

void change_title(const char* title, uint32 len);

void get_window_size(uint16* width, uint16* height);

void event_fire();

void shut_down_platform();

} // namespace synt


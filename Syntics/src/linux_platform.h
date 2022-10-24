#pragma once

#include "defines.h"
#include <xcb/xcb.h>
#include <X11/Xlib.h>

namespace synt {

typedef struct Linux_Platform
{
    Display* display;
    xcb_connection_t* connection;
    xcb_screen_t* screen;
    xcb_window_t window;
    uint16 width, height;

} Linux_Platform;

const Linux_Platform& get_platform_state();

void init_platform(const char* title, uint16 width, uint16 height);

void set_event_callbacks(void (*on_key_pressed)(uint8 key, uint16 op),
                         void (*on_key_released)(uint8 key, uint16 op),
                         void (*on_button_pressed)(uint8 key, uint16 op),
                         void (*on_button_released)(uint8 key, uint16 op),
                         void (*on_mouse_move)(uint16 pos_x, uint16 pos_y,
                                               uint16 op),
                         void (*on_window_focused)(bool focused, uint16 op),
                         void (*on_enter_leave)(bool e_l, uint16 op));

void change_title(const char* title, uint32 len);

void get_window_size(uint16* width, uint16* height);

void event_fire();

double get_time();

void linux_sleep(uint64 milli);

void shut_down_platform();

} // namespace synt


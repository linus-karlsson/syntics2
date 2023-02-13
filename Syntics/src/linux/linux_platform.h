#pragma once

#include "defines.h"
#include <xcb/xcb.h>

#define TOTAL_CURSORS 4

typedef struct Linux_Platform
{
    xcb_connection_t* connection;
    xcb_screen_t* screen;
    xcb_window_t window;
    u16 width, height;
    xcb_cursor_t cursors[TOTAL_CURSORS];

} Linux_Platform;

const Linux_Platform& get_platform_state();

void init_platform(const char* title, u16 width, u16 height);

void set_event_callbacks(void (*on_key_pressed)(u16 key, u16 op),
                         void (*on_key_released)(u16 key, u16 op),
                         void (*on_button_pressed)(u8 key, u16 op),
                         void (*on_button_released)(u8 key, u16 op),
                         void (*on_mouse_move)(i16 pos_x, i16 pos_y, u16 op),
                         void (*on_window_focused)(b8 focused, u16 op),
                         void (*on_enter_leave)(b8 e_l, u16 op));

void change_title(const char* title, u32 len);

xcb_window_t child_window(const char* title, u16 width, u16 height);

void get_window_size(u16& width, u16& height);

void event_fire();

double get_time();

void platform_sleep(u32milli);

void shut_down_platform();


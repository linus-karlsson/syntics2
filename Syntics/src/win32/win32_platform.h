#pragma once
#include "defines.h"

// TODO: Bug
typedef struct HWND__* HWND;

void init_platform(const char* title, b8 fullscreen, u16 width, u16 height);

void set_event_callbacks(void (*on_key_pressed)(u16 key, u16 op),
                         void (*on_key_released)(u16 key, u16 op),
                         void (*on_button_pressed)(u8 key, u16 op),
                         void (*on_button_released)(u8 key, u16 op),
                         void (*on_mouse_move)(i16 pos_x, i16 pos_y, u16 op),
                         void (*on_mouse_wheel)(i16 z_delta),
                         void (*on_window_focused)(b8 focused, u16 op),
                         void (*on_enter_leave)(b8 e_l, u16 op),
                         void (*on_window_resize)(u16 width, u16 height));

HWND get_win();

void change_title(const char* title, u32 len);

void get_window_size(u16* width, u16* height);

void event_fire();

double get_time();

void platform_sleep(u64 milli);

void shut_down_platform();


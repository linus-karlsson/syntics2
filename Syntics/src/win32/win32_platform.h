#pragma once
#include "defines.h"
#include <Windows.h>

void init_platform(const char* title, uint16 width, uint16 height);

void set_event_callbacks(void (*on_key_pressed)(uint16 key, uint16 op),
                         void (*on_key_released)(uint16 key, uint16 op),
                         void (*on_button_pressed)(uint8 key, uint16 op),
                         void (*on_button_released)(uint8 key, uint16 op),
                         void (*on_mouse_move)(int16 pos_x, int16 pos_y, uint16 op),
                         void (*on_mouse_wheel)(int16 z_delta),
                         void (*on_window_focused)(bool focused, uint16 op),
                         void (*on_enter_leave)(bool e_l, uint16 op));

HWND get_win();

void change_title(const char* title, uint32 len);

void get_window_size(uint16& width, uint16& height);

void event_fire();

double get_time();

void platform_sleep(uint64 milli);

void shut_down_platform();


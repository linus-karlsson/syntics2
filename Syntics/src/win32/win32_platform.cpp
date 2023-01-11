#include "win32_platform.h"
#include <Windows.h>

namespace synt {

typedef struct Callbacks
{
    void (*on_key_pressed)(uint16 key, uint16 op);
    void (*on_key_released)(uint16 key, uint16 op);
    void (*on_button_pressed)(uint8 key, uint16 op);
    void (*on_button_released)(uint8 key, uint16 op);
    void (*on_mouse_move)(int16 pos_x, int16 pos_y, uint16 op);
    void (*on_window_focused)(bool focused, uint16 op);
    void (*on_enter_leave)(bool e_l, uint16 op);
} Callbacks;

static Callbacks callback_handler;

void init_platform(const char* title, uint16 width, uint16 height)
{
}

void set_event_callbacks(void (*on_key_pressed)(uint16 key, uint16 op),
                         void (*on_key_released)(uint16 key, uint16 op),
                         void (*on_button_pressed)(uint8 key, uint16 op),
                         void (*on_button_released)(uint8 key, uint16 op),
                         void (*on_mouse_move)(int16 pos_x, int16 pos_y, uint16 op),
                         void (*on_window_focused)(bool focused, uint16 op),
                         void (*on_enter_leave)(bool e_l, uint16 op))
{
    callback_handler.on_key_pressed = on_key_pressed;
    callback_handler.on_key_released = on_key_released;
    callback_handler.on_button_pressed = on_button_pressed;
    callback_handler.on_button_released = on_button_released;
    callback_handler.on_mouse_move = on_mouse_move;
    callback_handler.on_window_focused = on_window_focused;
    callback_handler.on_enter_leave = on_enter_leave;
}

void change_title(const char* title, uint32 len)
{
}

void get_window_size(uint16* width, uint16* height)
{
}

void event_fire()
{
}

void hide_cursor()
{
}

void show_cursor()
{
}

void show_cursor_centered()
{
}

void show_cursor_last_pos()
{
}

void change_cursor(uint32 cursor_id)
{
}

void set_mouse_pos(int16 pos_x, int16 pos_y)
{
}

void set_mouse_last_pos()
{
}

void get_pos(int16& pos_x, int16& pos_y)
{
}

double get_time()
{
    return 0.0;
}

void platform_sleep(uint64 milli)
{
}

void shut_down_platform()
{
}

} // namespace synt

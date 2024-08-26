#ifndef SY_UNIT_BUILD
#include "event_system.h"
#include "logging.h"
#include "region_alloc.h"
#include "ansi_keycodes.h"
#include "syntics_platform.h"
#endif

#define HIGHEST_KEY_VALUE 191
#define HIGHEST_BOTTON_VALUE 3
#define QUEUE_SIZE 10

#define DOUBLE_CLICK_THRESHOLD 0.3

typedef struct EventContext
{
    Key_Buffer key_buffer;

    Key_Event key_event;
    Mouse_Move_Event mouse_move_event;
    Mouse_Button_Event mouse_button_event;
    Mouse_Wheel_Event mouse_wheel_event;
    Window_Resize_Event window_resize_event;

    V2 position;
    b8* running_ptr;

    double last_click_time;
    int last_button;
    V2 last_position;

    b8 key_pressed[HIGHEST_KEY_VALUE + 1];
    b8 window_focused;
    b8 enter_leave;
} EventContext;

global EventContext event_context = { .last_button = -1 };

void event_quit_event(void)
{
    assert(event_context.running_ptr);
    *event_context.running_ptr = false;
}

internal void on_key_event(u16 key, u32 mod, u8 action)
{
    event_context.key_event.key = key;
    event_context.key_event.ctrl_pressed = mod & SY_MOD_CONTROL;
    event_context.key_event.alt_pressed = mod & SY_MOD_ALT;
    event_context.key_event.shift_pressed = mod & SY_MOD_SHIFT;
    event_context.key_event.action = action;
    event_context.key_event.activated = true;

    if (action == 0)
    {
        event_context.key_pressed[key] = false;
    }
    else
    {
        event_context.key_pressed[key] = true;
    }
}

internal void on_key_pressed(u16 key, u32 mod)
{
    on_key_event(key, mod, 1);
}

internal void on_key_released(u16 key)
{
    on_key_event(key, 0, 0);
}

internal void on_mouse_button_event(u8 button, u8 action, u32 mod)
{
    const f64 time = platform_get_time();
    const f64 time_since_last = time - event_context.last_click_time;
    const b8 double_clicked = (action == 0) && (button == event_context.last_button) &&
                              (time_since_last <= DOUBLE_CLICK_THRESHOLD) &&
                              v2_equal(event_context.last_position, event_context.position);

    event_context.mouse_button_event.button = button;
    event_context.mouse_button_event.action = action;
    event_context.mouse_button_event.double_clicked = double_clicked;
    event_context.mouse_button_event.activated = true;

    if (action == 0)
    {
        event_context.last_click_time = time;
        event_context.last_button = button;
        event_context.last_position = event_context.position;
    }
}

internal void on_button_pressed(u8 button)
{
    if (button == 5)
    {
        button = 1;
    }
    else if (button == 6)
    {
        button = 2;
    }
    on_mouse_button_event(button, 1, 0);
}

internal void on_button_released(u8 button)
{
    on_mouse_button_event(button, 0, 0);
}

internal void on_mouse_move_event(i16 x_pos, i16 y_pos)
{
    event_context.mouse_move_event.position_x = (f32)x_pos;
    event_context.mouse_move_event.position_y = (f32)y_pos;
    event_context.mouse_move_event.activated = true;
}

internal void on_mouse_wheel_event(i16 x_offset, i16 y_offset)
{
    event_context.mouse_wheel_event.x_offset = (f32)x_offset;
    event_context.mouse_wheel_event.y_offset = (f32)y_offset;
    event_context.mouse_wheel_event.activated = true;
}

internal void on_window_focused(b8 focused)
{
    event_context.window_focused = focused;
}

internal void on_enter_leave(b8 e_l)
{
    event_context.enter_leave = e_l;
}

internal void on_resize(u16 width, u16 height)
{
    event_context.window_resize_event.width = width;
    event_context.window_resize_event.height = height;
    event_context.window_resize_event.activated = true;
}

internal void on_key_stroke(char key)
{
    Key_Buffer* buffer = &event_context.key_buffer;
    if (buffer->size < KEY_BUFFER_CAPACITY)
    {
        buffer->buffer[buffer->size++] = key;
        buffer->buffer[buffer->size] = '\0';
    }
}

void event_init(Region_Alloc* region, Platform* platform, u32 size, b8* running_ptr)
{
    platform_event_set_on_key_pressed(platform, on_key_pressed);
    platform_event_set_on_key_released(platform, on_key_released);
    platform_event_set_on_button_pressed(platform, on_button_pressed);
    platform_event_set_on_button_released(platform, on_button_released);
    platform_event_set_on_mouse_move(platform, on_mouse_move_event);
    platform_event_set_on_mouse_wheel(platform, on_mouse_wheel_event);
    platform_event_set_on_window_focused(platform, on_window_focused);
    platform_event_set_on_window_enter_leave(platform, on_enter_leave);
    platform_event_set_on_window_resize(platform, on_resize);
    platform_event_set_on_key_stroke(platform, on_key_stroke);
    event_context.running_ptr = running_ptr;
}

void event_poll(Platform* platform, V2 mouse_position)
{
    event_context.key_event.activated = false;
    event_context.mouse_move_event.activated = false;
    event_context.mouse_button_event.activated = false;
    event_context.mouse_wheel_event.activated = false;
    event_context.window_resize_event.activated = false;

    event_context.mouse_button_event.double_clicked = false;

    event_context.position = mouse_position;
    platform_event_fire(platform);
}

void event_update_position(V2 mouse_position)
{
    event_context.position = mouse_position;
}

const Key_Event* event_get_key_event(void)
{
    return &event_context.key_event;
}

const Mouse_Move_Event* event_get_mouse_move_event(void)
{
    return &event_context.mouse_move_event;
}

const Mouse_Button_Event* event_get_mouse_button_event(void)
{
    return &event_context.mouse_button_event;
}

const Mouse_Wheel_Event* event_get_mouse_wheel_event(void)
{
    return &event_context.mouse_wheel_event;
}

const Window_Resize_Event* event_get_window_resize_event(void)
{
    return &event_context.window_resize_event;
}

V2 event_get_mouse_position(void)
{
    return event_context.position;
}

b8 event_is_ctrl_and_key_pressed(u32 key)
{
    const Key_Event* event = event_get_key_event();
    return event->activated && event->action == 1 && event->ctrl_pressed && event->key == key;
}

b8 event_is_ctrl_and_key_range_pressed(u32 key_low, u32 key_high)
{
    const Key_Event* event = event_get_key_event();
    return event->activated && event->action == 1 && event->ctrl_pressed &&
           (closed_interval(key_low, event->key, key_high));
}

b8 event_is_key_clicked(u32 key)
{
    const Key_Event* event = event_get_key_event();
    return event->activated && event->action == 0 && event->key == key;
}

b8 event_is_key_pressed(u32 key)
{
    const Key_Event* event = event_get_key_event();
    return event->action == 1 && event->key == key;
}

b8 event_is_key_pressed_once(u32 key)
{
    const Key_Event* event = event_get_key_event();
    return event->activated && event->action == 1 && event->key == key;
}

b8 event_is_mouse_button_clicked(u8 button)
{
    const Mouse_Button_Event* event = event_get_mouse_button_event();
    return event->activated && event->action == 0 && event->button == button;
}

b8 event_is_mouse_button_pressed_once(u8 button)
{
    const Mouse_Button_Event* event = event_get_mouse_button_event();
    return event->activated && event->action == 1 && event->button == button;
}

b8 event_is_mouse_button_pressed(u8 button)
{
    const Mouse_Button_Event* event = event_get_mouse_button_event();
    return event->action == 1 && event->button == button;
}

b8 event_is_window_focused(void)
{
    return event_context.window_focused;
}

u16 event_code_to_ascii(u16 key)
{
    switch (key)
    {
        case SYNT_KEY_Q:
        {
            return SYNT_ASCII_KEY_Q;
        }
        case SYNT_KEY_W:
        {
            return SYNT_ASCII_KEY_W;
        }
        case SYNT_KEY_E:
        {
            return SYNT_ASCII_KEY_E;
        }
        case SYNT_KEY_R:
        {
            return SYNT_ASCII_KEY_R;
        }
        case SYNT_KEY_T:
        {
            return SYNT_ASCII_KEY_T;
        }
        case SYNT_KEY_Y:
        {
            return SYNT_ASCII_KEY_Y;
        }
        case SYNT_KEY_U:
        {
            return SYNT_ASCII_KEY_U;
        }
        case SYNT_KEY_I:
        {
            return SYNT_ASCII_KEY_I;
        }
        case SYNT_KEY_O:
        {
            return SYNT_ASCII_KEY_O;
        }
        case SYNT_KEY_P:
        {
            return SYNT_ASCII_KEY_P;
        }
        case SYNT_KEY_A:
        {
            return SYNT_ASCII_KEY_A;
        }
        case SYNT_KEY_S:
        {
            return SYNT_ASCII_KEY_S;
        }
        case SYNT_KEY_D:
        {
            return SYNT_ASCII_KEY_D;
        }
        case SYNT_KEY_F:
        {
            return SYNT_ASCII_KEY_F;
        }
        case SYNT_KEY_G:
        {
            return SYNT_ASCII_KEY_G;
        }
        case SYNT_KEY_H:
        {
            return SYNT_ASCII_KEY_H;
        }
        case SYNT_KEY_J:
        {
            return SYNT_ASCII_KEY_J;
        }
        case SYNT_KEY_K:
        {
            return SYNT_ASCII_KEY_K;
        }
        case SYNT_KEY_L:
        {
            return SYNT_ASCII_KEY_L;
        }
        case SYNT_KEY_Z:
        {
            return SYNT_ASCII_KEY_Z;
        }
        case SYNT_KEY_X:
        {
            return SYNT_ASCII_KEY_X;
        }
        case SYNT_KEY_C:
        {
            return SYNT_ASCII_KEY_C;
        }
        case SYNT_KEY_V:
        {
            return SYNT_ASCII_KEY_V;
        }
        case SYNT_KEY_B:
        {
            return SYNT_ASCII_KEY_B;
        }
        case SYNT_KEY_N:
        {
            return SYNT_ASCII_KEY_N;
        }
        case SYNT_KEY_M:
        {
            return SYNT_ASCII_KEY_M;
        }
        case SYNT_KEY_PERIOD:
        {
            return SYNT_ASCII_KEY_PERIOD;
        }
        case SYNT_KEY_0:
        {
            return SYNT_ASCII_KEY_0;
        }
        case SYNT_KEY_1:
        {
            return SYNT_ASCII_KEY_1;
        }
        case SYNT_KEY_2:
        {
            return SYNT_ASCII_KEY_2;
        }
        case SYNT_KEY_3:
        {
            return SYNT_ASCII_KEY_3;
        }
        case SYNT_KEY_4:
        {
            return SYNT_ASCII_KEY_4;
        }
        case SYNT_KEY_5:
        {
            return SYNT_ASCII_KEY_5;
        }
        case SYNT_KEY_6:
        {
            return SYNT_ASCII_KEY_6;
        }
        case SYNT_KEY_7:
        {
            return SYNT_ASCII_KEY_7;
        }
        case SYNT_KEY_8:
        {
            return SYNT_ASCII_KEY_8;
        }
        case SYNT_KEY_9:
        {
            return SYNT_ASCII_KEY_9;
        }
        case SYNT_KEY_ENTER:
        {
            return SYNT_ASCII_KEY_ENTER;
        }
        case SYNT_KEY_SPACE:
        {
            return SYNT_ASCII_KEY_SPACE;
        }
        case SYNT_KEY_CTRL:
        {
            return SYNT_ASCII_KEY_LEFT_CTRL;
        }
        case SYNT_KEY_SHIFT:
        {
            return SYNT_ASCII_KEY_LEFT_SHIFT;
        }
        case SYNT_KEY_MINUS:
        {
            return SYNT_ASCII_KEY_MINUS;
        }
        case SYNT_KEY_APOSTROPHE:
        {
            return SYNT_ASCII_KEY_APOSTROPHE;
        }
        default:
        {
            return 0;
        }
    }
}

Key_Buffer event_get_key_buffer(void)
{
    return event_context.key_buffer;
}

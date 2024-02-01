#ifndef SY_UNIT_BUILD
#include "event_system.h"
#include "logging.h"
#include "region_alloc.h"
#include "ansi_keycodes.h"
#include "platform.h"
#endif
// TODO: Have different arrays for all different events; To save itarations
// if it gets to much but right now it's like 7 total so latch

#define HIGHEST_KEY_VALUE 191
#define HIGHEST_BOTTON_VALUE 3
#define QUEUE_SIZE 10

typedef struct Evt_Node
{
    Events evt;
    Events** back_ptr;
} Evt_Node;

typedef struct Event_Context
{
    Evt_Node* evt_linked;
    Events* events;
    u32* free_idxs;

    // Used for getting keystrokes
    Key_Buffer key_buffer;
    u8 key_pressed[HIGHEST_KEY_VALUE + 1];
    u8 key_released[HIGHEST_KEY_VALUE + 1];
    u8 button_pressed[HIGHEST_BOTTON_VALUE + 1];
    u8 button_released[HIGHEST_BOTTON_VALUE + 1];
    Event_State key_state;
    Event_State button_state;
    u32 event_count;

    b8 initialized : 1;
    b8 window_focused : 1;
    b8 enter_leave : 1;
    b8 any_key_pressed : 1;
    b8 new_key_is_released : 1;
    b8 any_button_pressed : 1;
    b8 new_button_is_released : 1;
    b8* running_ptr;
} Event_Context;

global Event_Context EVENT_CTX = { 0 };

void quit_event(void)
{
    assert(EVENT_CTX.running_ptr);
    *EVENT_CTX.running_ptr = false;
}

internal void on_key_pressed(u16 key)
{
    EVENT_CTX.any_key_pressed = 1;
    for (u32 i = 0; i < EVENT_CTX.event_count; i++)
    {
        if (EVENT_CTX.evt_linked[i].evt.evt_type == EVT_KEY)
        {
            EVENT_CTX.evt_linked[i].evt.key_evt.key = key;
            EVENT_CTX.evt_linked[i].evt.key_evt.action = 1;
            EVENT_CTX.evt_linked[i].evt.activated = 1;
        }
    }
    if (key <= HIGHEST_KEY_VALUE)
    {
        EVENT_CTX.key_pressed[key] = 1;
        EVENT_CTX.key_state = DOWN;
    }
}

internal void on_key_released(u16 key)
{
    EVENT_CTX.any_key_pressed = 0;
    for (u32 i = 0; i < EVENT_CTX.event_count; i++)
    {
        if (EVENT_CTX.evt_linked[i].evt.evt_type == EVT_KEY)
        {
            EVENT_CTX.evt_linked[i].evt.key_evt.key = key;
            EVENT_CTX.evt_linked[i].evt.key_evt.action = 0;
            EVENT_CTX.evt_linked[i].evt.activated = 1;
        }
    }
    if (key <= HIGHEST_KEY_VALUE)
    {
        EVENT_CTX.key_pressed[key] = 0;
        EVENT_CTX.key_released[key] = 1;
        EVENT_CTX.new_key_is_released = 1;
        EVENT_CTX.key_state = UP;
    }
}

// TODO: temp, if you release button outside window a realse event does not
// occur
void button_unpressed_set(void)
{
    EVENT_CTX.any_button_pressed = 0;
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
    EVENT_CTX.any_button_pressed = 1;
    EVENT_CTX.button_state = DOWN;
    for (u32 i = 0; i < EVENT_CTX.event_count; i++)
    {
        if (EVENT_CTX.evt_linked[i].evt.evt_type == EVT_MOUSE)
        {
            EVENT_CTX.evt_linked[i].evt.mouse_evt.button_evt.button = button;
            EVENT_CTX.evt_linked[i].evt.mouse_evt.button_evt.action = 1;
            EVENT_CTX.evt_linked[i].evt.activated = 1;
        }
    }
    EVENT_CTX.button_pressed[button] = 1;
}

internal void on_button_released(u8 button)
{
    EVENT_CTX.any_button_pressed = 0;
    EVENT_CTX.button_state = UP;
    for (u32 i = 0; i < EVENT_CTX.event_count; i++)
    {
        if (EVENT_CTX.evt_linked[i].evt.evt_type == EVT_MOUSE)
        {
            EVENT_CTX.evt_linked[i].evt.mouse_evt.button_evt.button = button;
            EVENT_CTX.evt_linked[i].evt.mouse_evt.button_evt.action = 0;
            EVENT_CTX.evt_linked[i].evt.activated = 1;
        }
    }
    EVENT_CTX.button_pressed[button] = 0;
    EVENT_CTX.button_released[button] = 1;
    EVENT_CTX.new_button_is_released = 1;
}

internal void on_mouse_move(i16 pos_x, i16 pos_y)
{
    for (u32 i = 0; i < EVENT_CTX.event_count; i++)
    {
        if (EVENT_CTX.evt_linked[i].evt.evt_type == EVT_MOUSE)
        {
            EVENT_CTX.evt_linked[i].evt.mouse_evt.move_evt.pos_x = pos_x;
            EVENT_CTX.evt_linked[i].evt.mouse_evt.move_evt.pos_y = pos_y;
            EVENT_CTX.evt_linked[i].evt.activated = 1;
        }
    }
}

internal void on_mouse_wheel(i16 z_delta)
{
    for (u32 i = 0; i < EVENT_CTX.event_count; i++)
    {
        if (EVENT_CTX.evt_linked[i].evt.evt_type == EVT_WHEEL)
        {
            EVENT_CTX.evt_linked[i].evt.wheel_evt.z_delta = z_delta;
            EVENT_CTX.evt_linked[i].evt.activated = 1;
        }
    }
}

internal void on_window_focused(b8 focused)
{
    EVENT_CTX.window_focused = focused;
}

internal void on_enter_leave(b8 e_l)
{
    EVENT_CTX.enter_leave = e_l;
}

internal void on_window_resize(u16 width, u16 height)
{
    for (u32 i = 0; i < EVENT_CTX.event_count; i++)
    {
        if (EVENT_CTX.evt_linked[i].evt.evt_type == EVT_RESIZE)
        {
            EVENT_CTX.evt_linked[i].evt.resize_evt.width = width;
            EVENT_CTX.evt_linked[i].evt.resize_evt.height = height;
            EVENT_CTX.evt_linked[i].evt.resize_evt.is_resized = true;
        }
    }
}

internal void on_key_stroke(char key)
{
    Key_Buffer* buffer = &EVENT_CTX.key_buffer;
    if (buffer->size < KEY_BUFFER_CAPACITY)
    {
        buffer->buffer[buffer->size++] = key;
        buffer->buffer[buffer->size] = '\0';
    }
}

void event_init(Region_Alloc* region, Platform* platform, u32 size,
                b8* running_ptr)
{
    assert(!EVENT_CTX.initialized);

    EVENT_CTX.evt_linked = region_array(region, size, Evt_Node);
    EVENT_CTX.events = region_array(region, size, Events);
    EVENT_CTX.free_idxs = region_array(region, size, u32);
    EVENT_CTX.initialized = 1;
    syntics_platform_event_set_on_key_pressed(platform, on_key_pressed);
    syntics_platform_event_set_on_key_released(platform, on_key_released);
    syntics_platform_event_set_on_button_pressed(platform, on_button_pressed);
    syntics_platform_event_set_on_button_released(platform, on_button_released);
    syntics_platform_event_set_on_mouse_move(platform, on_mouse_move);
    syntics_platform_event_set_on_mouse_wheel(platform, on_mouse_wheel);
    syntics_platform_event_set_on_window_focused(platform, on_window_focused);
    syntics_platform_event_set_on_window_resize(platform, on_window_resize);
    syntics_platform_event_set_on_window_enter_leave(platform, on_enter_leave);
    syntics_platform_event_set_on_key_stroke(platform, on_key_stroke);
    EVENT_CTX.running_ptr = running_ptr;
}

void event_subscribe(Events** evt, Event_Type evt_type)
{
    ASSERT(evt, "");
    ASSERT(EVENT_CTX.initialized, "");

    Evt_Node evt_node = { 0 };
    Events evt_out = { 0 };
    u32 size = array_size(EVENT_CTX.evt_linked);
    evt_out.initialize = 1;
    evt_out.evt_type = evt_type;
    evt_out.index = size;
    evt_node.evt = evt_out;
    evt_node.back_ptr = evt;
    region_array_push(EVENT_CTX.evt_linked, evt_node);
    *evt = &EVENT_CTX.evt_linked[evt_out.index].evt;
    EVENT_CTX.event_count++;
}

void event_unsubscribe(Events** evt)
{
    ASSERT(evt != NULL || *evt != NULL, "");

    if ((*evt)->initialize)
    {
        u32 index = (*evt)->index;

        u32* size_ptr = &region_array_head(EVENT_CTX.evt_linked)->size;
        u32 size = *size_ptr;
        if (index > size - 1)
        {
            sy_print("Wrong index on evt\n");
            return;
        }
        else if (index == size - 1)
        {
            EVENT_CTX.evt_linked[index].evt.initialize = 0;
        }
        else
        {
            *EVENT_CTX.evt_linked[size - 1].back_ptr =
                &EVENT_CTX.evt_linked[index].evt;
            EVENT_CTX.evt_linked[index] = EVENT_CTX.evt_linked[size - 1];
            EVENT_CTX.evt_linked[index].evt.index = index;
            EVENT_CTX.evt_linked[size - 1].evt.initialize = 0;
        }
        (*size_ptr)--;
        *evt = NULL;
        EVENT_CTX.event_count--;
    }
}

void event_poll(Platform* platform)
{
    for (u32 i = 0; i < EVENT_CTX.event_count; i++)
    {
        EVENT_CTX.evt_linked[i].evt.activated = 0;
    }
    EVENT_CTX.key_state = NONE;
    EVENT_CTX.button_state = NONE;
    EVENT_CTX.key_buffer.size = 0;
    EVENT_CTX.key_buffer.buffer[0] = '\0';
    if (EVENT_CTX.new_key_is_released)
    {
        memset(EVENT_CTX.key_released, 0, sy_SIZE(EVENT_CTX.key_released));
    }
    if (EVENT_CTX.new_button_is_released)
    {
        memset(EVENT_CTX.button_released, 0,
               sy_SIZE(EVENT_CTX.button_released));
    }
    EVENT_CTX.new_button_is_released = 0;
    EVENT_CTX.new_key_is_released = 0;
    syntics_platform_event_fire(platform);
}

b8 is_key_pressed(u32 key_pressed)
{
    if (key_pressed <= HIGHEST_KEY_VALUE)
        return EVENT_CTX.key_pressed[key_pressed];
    return 0;
}

b8 is_any_key_pressed(void)
{
    return EVENT_CTX.any_key_pressed;
}

b8 is_key_clicked(u32 key_pressed)
{
    assert(key_pressed < HIGHEST_KEY_VALUE);
    return EVENT_CTX.key_pressed[key_pressed] && EVENT_CTX.key_state == DOWN;
}

b8 is_key_released(u32 key_pressed)
{
    assert(key_pressed < HIGHEST_KEY_VALUE);
    return EVENT_CTX.key_released[key_pressed];
}

b8 is_any_key_clicked(void)
{
    return EVENT_CTX.key_state == DOWN;
}

b8 is_any_button_pressed(void)
{
    return EVENT_CTX.any_button_pressed;
}

b8 is_any_button_clicked(void)
{
    return EVENT_CTX.button_state == DOWN;
}

b8 is_button_clicked(u32 button)
{
    assert(button < HIGHEST_BOTTON_VALUE);
    return EVENT_CTX.button_pressed[button] && EVENT_CTX.button_state == DOWN;
}

b8 is_button_pressed(u32 button)
{
    assert(button < HIGHEST_BOTTON_VALUE);
    return EVENT_CTX.button_pressed[button];
}

b8 is_button_released(u32 button)
{
    assert(button < HIGHEST_BOTTON_VALUE);
    return EVENT_CTX.button_released[button];
}

b8 is_window_focused(void)
{
    return EVENT_CTX.window_focused;
}

u16 code_to_ascii(u16 key)
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

Key_Buffer get_key_buffer(void)
{
    return EVENT_CTX.key_buffer;
}

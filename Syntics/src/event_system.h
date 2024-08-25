#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#endif

#ifdef LINUX
#define SYNT_KEY_PRESS 1
#define SYNT_KEY_RELEASE 0
#define SYNT_KEY_Q 24
#define SYNT_KEY_W 25
#define SYNT_KEY_E 26
#define SYNT_KEY_R 27
#define SYNT_KEY_T 28
#define SYNT_KEY_Y 29
#define SYNT_KEY_U 30
#define SYNT_KEY_I 31
#define SYNT_KEY_O 32
#define SYNT_KEY_P 33
#define SYNT_KEY_A 38
#define SYNT_KEY_S 39
#define SYNT_KEY_D 40
#define SYNT_KEY_F 41
#define SYNT_KEY_G 42
#define SYNT_KEY_H 43
#define SYNT_KEY_J 44
#define SYNT_KEY_K 45
#define SYNT_KEY_L 46
#define SYNT_KEY_Z 52
#define SYNT_KEY_X 53
#define SYNT_KEY_C 54
#define SYNT_KEY_V 55
#define SYNT_KEY_B 56
#define SYNT_KEY_N 57
#define SYNT_KEY_M 58
#define SYNT_KEY_ENTER 36
#define SYNT_KEY_SPACE 65
#define SYNT_KEY_CTRL 37
#define SYNT_KEY_SHIFT 50
#define SYNT_KEY_PERIOD 60
#define SYNT_KEY_BACKSPACE 22
#define SYNT_KEY_MINUS 61
#define SYNT_KEY_0 19
#define SYNT_KEY_9 18
#define SYNT_KEY_8 17
#define SYNT_KEY_7 16
#define SYNT_KEY_6 15
#define SYNT_KEY_5 14
#define SYNT_KEY_4 13
#define SYNT_KEY_3 12
#define SYNT_KEY_2 11
#define SYNT_KEY_1 10

#define SYNT_KEY_LEFT 113
#define SYNT_KEY_UP 111
#define SYNT_KEY_RIGHT 114
#define SYNT_KEY_DOWN 116

#define SYNT_LEFT_BUTTON 1
#define SYNT_MIDDLE_BUTTON 2
#define SYNT_RIGHT_BUTTON 3

// TODO: find these
#define SYNT_KEY_APOSTROPHE 191
#define SYNT_KEY_CAPS 20
#define SYNT_KEY_TAB 9

#else
#define SYNT_KEY_SPACE 32
#define SYNT_KEY_COMMA 44
#define SYNT_KEY_MINUS 189
#define SYNT_KEY_PERIOD 190
#define SYNT_KEY_SHIFT 16
#define SYNT_KEY_CTRL 17
#define SYNT_KEY_BACKSPACE 8
#define SYNT_KEY_TAB 9
#define SYNT_KEY_ENTER 13
#define SYNT_KEY_CAPS 20
#define SYNT_KEY_SPACE 32
#define SYNT_KEY_APOSTROPHE 191

#define SYNT_KEY_SLASH 47
#define SYNT_KEY_0 48
#define SYNT_KEY_1 49
#define SYNT_KEY_2 50
#define SYNT_KEY_3 51
#define SYNT_KEY_4 52
#define SYNT_KEY_5 53
#define SYNT_KEY_6 54
#define SYNT_KEY_7 55
#define SYNT_KEY_8 56
#define SYNT_KEY_9 57
#define SYNT_KEY_SEMICOLON 59
#define SYNT_KEY_EQUAL 61
#define SYNT_KEY_A 65
#define SYNT_KEY_B 66
#define SYNT_KEY_C 67
#define SYNT_KEY_D 68
#define SYNT_KEY_E 69
#define SYNT_KEY_F 70
#define SYNT_KEY_G 71
#define SYNT_KEY_H 72
#define SYNT_KEY_I 73
#define SYNT_KEY_J 74
#define SYNT_KEY_K 75
#define SYNT_KEY_L 76
#define SYNT_KEY_M 77
#define SYNT_KEY_N 78
#define SYNT_KEY_O 79
#define SYNT_KEY_P 80
#define SYNT_KEY_Q 81
#define SYNT_KEY_R 82
#define SYNT_KEY_S 83
#define SYNT_KEY_T 84
#define SYNT_KEY_U 85
#define SYNT_KEY_V 86
#define SYNT_KEY_W 87
#define SYNT_KEY_X 88
#define SYNT_KEY_Y 89
#define SYNT_KEY_Z 90

#define SYNT_KEY_LEFT 37
#define SYNT_KEY_UP 38
#define SYNT_KEY_RIGHT 39
#define SYNT_KEY_DOWN 40

#define SYNT_LEFT_BUTTON 1
#define SYNT_MIDDLE_BUTTON 3
#define SYNT_RIGHT_BUTTON 2

#endif

#define SYNT_PRESS 1
#define SYNT_RELEASE 0

#define SYNT_NORMAL_CURSOR 0
#define SYNT_HAND_CURSOR 1
#define SYNT_RESIZE_H_CURSOR 2
#define SYNT_RESIZE_V_CURSOR 3
#define SYNT_RESIZE_NW_CURSOR 4
#define SYNT_MOVE_CURSOR 5
#define SYNT_HIDDEN_CURSOR 6

#define KEY_BUFFER_CAPACITY 50

typedef enum Event_State
{
    NONE,
    DOWN,
    UP
} Event_State;

typedef enum Event_Type
{
    EVT_KEY,
    EVT_MOUSE,
    EVT_WHEEL,
    EVT_CLOSE,
    EVT_RESIZE
} Event_Type;

typedef struct KeyEvent
{
    u16 key;
    u16 action;
    b8 ctrl_pressed;
    b8 alt_pressed;
    b8 shift_pressed;
    b8 activated;
} KeyEvent;

typedef struct MouseMoveEvent
{
    f32 position_x;
    f32 position_y;
    b8 activated;
} MouseMoveEvent;

typedef struct MouseButtonEvent
{
    u8 button;
    u8 action;
    b8 double_clicked;
    b8 activated;
} MouseButtonEvent;

typedef struct MouseWheelEvent
{
    f32 x_offset;
    f32 y_offset;
    b8 activated;
} MouseWheelEvent;

typedef struct Key_Buffer
{
    u32 size;
    char buffer[KEY_BUFFER_CAPACITY];
} Key_Buffer;

void event_quit_event(void);
void event_button_unpressed_set(void);

void event_initialize(Region_Alloc* region, Platform* platform, u32 size, b8* running_ptr);
void event_poll(Platform* platform, V2 mouse_position);

void event_update_position(V2 mouse_position);
const KeyEvent* event_get_key_event();
const MouseMoveEvent* event_get_mouse_move_event();
const MouseButtonEvent* event_get_mouse_button_event();
const MouseWheelEvent* event_get_mouse_wheel_event();
V2 event_get_mouse_position();

b8 event_is_ctrl_and_key_pressed(u32 key);
b8 event_is_ctrl_and_key_range_pressed(u32 key_low, u32 key_high);
b8 event_is_key_clicked(u32 key);
b8 event_is_key_pressed(u32 key);
b8 event_is_key_pressed_once(u32 key);

b8 event_is_mouse_button_clicked(u8 button);
b8 event_is_mouse_button_pressed_once(u8 button);
b8 event_is_mouse_button_pressed(u8 button);

b8 event_is_window_focused(void);
u16 event_code_to_ascii(u16 key);
Key_Buffer event_get_key_buffer(void);

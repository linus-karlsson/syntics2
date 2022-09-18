#pragma once

#include "defines.h"

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
#define SYNT_KEY_SPACE 65
#define SYNT_KEY_CTRL 37
#define SYNT_KEY_SHIFT 50

#define SYNT_BUTTON_PRESS 1
#define SYNT_BUTTON_RELEASE 0
#define SYNT_LEFT_BUTTON 1
#define SYNT_MIDDLE_BUTTON 2
#define SYNT_RIGHT_BUTTON 3

#define SYNT_Q_PRESSED 0
#define SYNT_W_PRESSED 1
#define SYNT_E_PRESSED 2
#define SYNT_A_PRESSED 3
#define SYNT_S_PRESSED 4
#define SYNT_D_PRESSED 5
#define SYNT_R_PRESSED 6
#define SYNT_SPACE_PRESSED 7
#define SYNT_CTRL_PRESSED 8
#define SYNT_SHIFT_PRESSED 9
#define TOTAL_NUM_KEYS 10

namespace synt {

typedef enum Event_Type
{
    EVT_KEY,
    EVT_MOUSE,
    EVT_CLOSE,
} Event_Type;

typedef struct Key_Event
{
    Key_Event();

    uint8 key;
    uint8 action;
} Key_Event;

typedef struct Button_Event
{
    Button_Event();

    uint8 action;
    uint8 button;
} Button_Event;

typedef struct Mouse_Move_Event
{
    Mouse_Move_Event();

    uint8 action;
    uint16 pos_x;
    uint16 pos_y;
} Mouse_Move_Event;

typedef struct Mouse_Event
{
    Button_Event button_evt;
    Mouse_Move_Event move_evt;
} Mouse_Event;

typedef struct Events
{
    Events();

    Event_Type evt_type;
    uint32 index;
    bool initialize;
    bool activated;
    union
    {
        Key_Event key_evt;
        Mouse_Event mouse_evt;
        uint8 close_evt;
    };
} Events;

typedef struct Region_Alloc Region_Alloc;

void init_events(Region_Alloc* region, uint32 size);
void subscribe(Events** evt, Event_Type evt_type);
void unsubscribe(Events** evt);

void get_window_size(uint16* width, uint16* height);
void hide_cursor();
void show_cursor();
void show_cursor_last_pos();
void set_mouse_pos(uint16 pos_x, uint16 pos_y);
void set_mouse_last_pos();
void get_pos(float& pos_x, float& pos_y);

void poll_events();

bool is_key_pressed(uint32 key_pressed_flag);
bool is_window_focused();

} // namespace synt

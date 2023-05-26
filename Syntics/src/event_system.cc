#include "event_system.h"
#include "logging.h"
#include "region_alloc.h"
#include "ansi_keycodes.h"
#include "win32/win32_platform.h"

// TODO: Have different arrays for all different events; To save itarations
// if it gets to much but right now it's like 7 total so latch

typedef struct Evt_Node
{
    Events evt;
    Events** back_ptr;
} Evt_Node;

typedef struct Event_Storage
{
    Evt_Node* evt_linked;
    Events* events;
    u32* free_idxs;
} Event_Storage;

static Event_Storage STORAGE;
static b8 WINDOW_FOCUSED = 0;
static b8 ENTER_LEAVE = 0;
static b8 INITIALIZED = 0;
static b8 ANY_KEY_PRESSED = 0;
static b8 ANY_BUTTON_PRESSED = 0;

static u32 NUM_EVENTS = 0;

#define HIGHEST_KEY_VALUE 191
static u8 KEY_PRESSED[HIGHEST_KEY_VALUE + 1] = { 0 };

static u16 _CAPS_ON = 0;

global u16* key_buffer = 0;
global u16* op_buffer = 0;
global b32 store_or_not = false;
global b32 dd = false;

static void on_key_pressed(u16 key, u16 op)
{
    if (store_or_not)
    {
        synt_push(key_buffer, key);
        synt_push(op_buffer, op);
        return;
    }
    _CAPS_ON = op;
    ANY_KEY_PRESSED = 1;
    for (u32 i = 0; i < NUM_EVENTS; i++)
    {
        if (STORAGE.evt_linked[i].evt.evt_type == EVT_KEY)
        {
            STORAGE.evt_linked[i].evt.key_evt.key = key;
            STORAGE.evt_linked[i].evt.key_evt.action = 1;
            STORAGE.evt_linked[i].evt.activated = 1;
        }
    }
    if (key <= HIGHEST_KEY_VALUE)
    {
        KEY_PRESSED[key] = 1;
    }
    store_or_not = true;
}

static void on_key_released(u16 key)
{
    ANY_KEY_PRESSED = 0;
    for (u32 i = 0; i < NUM_EVENTS; i++)
    {
        if (STORAGE.evt_linked[i].evt.evt_type == EVT_KEY)
        {
            STORAGE.evt_linked[i].evt.key_evt.key = key;
            STORAGE.evt_linked[i].evt.key_evt.action = 0;
            STORAGE.evt_linked[i].evt.activated = 1;
        }
    }
    if (key <= HIGHEST_KEY_VALUE)
    {
        KEY_PRESSED[key] = 0;
    }
}

// TODO: temp, if you release button outside window a realse event does not occur
void set_button_unpressed(void)
{
    ANY_BUTTON_PRESSED = 0;
}

static void on_button_pressed(u8 button)
{
    ANY_BUTTON_PRESSED = 1;
    for (u32 i = 0; i < NUM_EVENTS; i++)
    {
        if (STORAGE.evt_linked[i].evt.evt_type == EVT_MOUSE)
        {
            STORAGE.evt_linked[i].evt.mouse_evt.button_evt.button = button;
            STORAGE.evt_linked[i].evt.mouse_evt.button_evt.action = 1;
            STORAGE.evt_linked[i].evt.activated = 1;
        }
    }
}

static void on_button_released(u8 button)
{
    ANY_BUTTON_PRESSED = 0;
    for (u32 i = 0; i < NUM_EVENTS; i++)
    {
        if (STORAGE.evt_linked[i].evt.evt_type == EVT_MOUSE)
        {
            STORAGE.evt_linked[i].evt.mouse_evt.button_evt.button = button;
            STORAGE.evt_linked[i].evt.mouse_evt.button_evt.action = 0;
            STORAGE.evt_linked[i].evt.activated = 1;
        }
    }
}

static void on_mouse_move(i16 pos_x, i16 pos_y)
{
    for (u32 i = 0; i < NUM_EVENTS; i++)
    {
        if (STORAGE.evt_linked[i].evt.evt_type == EVT_MOUSE)
        {
            STORAGE.evt_linked[i].evt.mouse_evt.move_evt.pos_x = pos_x;
            STORAGE.evt_linked[i].evt.mouse_evt.move_evt.pos_y = pos_y;
            STORAGE.evt_linked[i].evt.activated = 1;
        }
    }
}

static void on_mouse_wheel(i16 z_delta)
{
    for (u32 i = 0; i < NUM_EVENTS; i++)
    {
        if (STORAGE.evt_linked[i].evt.evt_type == EVT_WHEEL)
        {
            STORAGE.evt_linked[i].evt.wheel_evt.z_delta = z_delta;
            STORAGE.evt_linked[i].evt.activated = 1;
        }
    }
}

static void on_window_focused(b8 focused)
{
    WINDOW_FOCUSED = focused;
}

static void on_enter_leave(b8 e_l)
{
    ENTER_LEAVE = e_l;
}

static void on_window_resize(u16 width, u16 height)
{
    for (u32 i = 0; i < NUM_EVENTS; i++)
    {
        if (STORAGE.evt_linked[i].evt.evt_type == EVT_RESIZE)
        {
            STORAGE.evt_linked[i].evt.resize_evt.width = width;
            STORAGE.evt_linked[i].evt.resize_evt.height = height;
            STORAGE.evt_linked[i].evt.resize_evt.is_resized = true;
        }
    }
}

void init_events(Region_Alloc* region, u32 size)
{
    if (!INITIALIZED)
    {
        key_buffer = dyn_arrayP(region, 10, u16);
        op_buffer = dyn_arrayP(region, 10, u16);
        STORAGE.evt_linked = dyn_arrayP(region, size, Evt_Node);
        STORAGE.events = dyn_arrayP(region, size, Events);
        STORAGE.free_idxs = dyn_arrayP(region, size, u32);
        INITIALIZED = 1;
        set_event_callbacks(on_key_pressed, on_key_released, on_button_pressed,
                            on_button_released, on_mouse_move, on_mouse_wheel,
                            on_window_focused, on_enter_leave, on_window_resize);
        NUM_EVENTS = 0;
    }
}

void subscribe(Events** evt, Event_Type evt_type)
{
    ASSERT(evt, "");
    ASSERT(INITIALIZED, "");

    Evt_Node evt_node = { };
    Events evt_out = {};
    u32 size = size_arr(STORAGE.evt_linked);
    evt_out.initialize = 1;
    evt_out.evt_type = evt_type;
    evt_out.index = size;
    evt_node.evt = evt_out;
    evt_node.back_ptr = evt;
    synt_push(STORAGE.evt_linked, evt_node);
    *evt = &STORAGE.evt_linked[evt_out.index].evt;
    NUM_EVENTS++;
}

void unsubscribe(Events** evt)
{
    ASSERT(evt != NULL || *evt != NULL, "");

    if ((*evt)->initialize)
    {
        u32 index = (*evt)->index;

        u32* size_ptr = &get_head(STORAGE.evt_linked)->size;
        u32 size = *size_ptr;
        if (index > size - 1)
        {
            print("Wrong index on evt\n");
            return;
        }
        else if (index == size - 1)
        {
            STORAGE.evt_linked[index].evt.initialize = 0;
        }
        else
        {
            *STORAGE.evt_linked[size - 1].back_ptr = &STORAGE.evt_linked[index].evt;
            STORAGE.evt_linked[index] = STORAGE.evt_linked[size - 1];
            STORAGE.evt_linked[index].evt.index = index;
            STORAGE.evt_linked[size - 1].evt.initialize = 0;
        }
        (*size_ptr)--;
        *evt = NULL;
        NUM_EVENTS--;
    }
}

void poll_events(void)
{
    store_or_not = false;
    for (u32 i = 0; i < NUM_EVENTS; i++)
    {
        STORAGE.evt_linked[i].evt.activated = 0;
    }
    if (size_arr(key_buffer))
    {
        on_key_pressed(synt_pop(key_buffer), synt_pop(op_buffer));
        dd = true;
        return;
    }
    event_fire();
}

b8 is_key_pressed(u32 key_pressed)
{
    if (key_pressed <= HIGHEST_KEY_VALUE) return KEY_PRESSED[key_pressed];
    return 0;
}

b8 is_any_key_pressed(void)
{
    return ANY_KEY_PRESSED;
}

static b8 check_clicked(b8 pressed, b8* first_clicked)
{
    if (pressed)
    {
        if (*first_clicked)
        {
            *first_clicked = false;
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        *first_clicked = true;
    }
    return false;
}

b8 is_key_clicked(b8* first_clicked, u32 key_pressed)
{
    b8 correct_key = key_pressed <= HIGHEST_KEY_VALUE;
    ASSERT(correct_key, "is_key_clicked");
    if (correct_key) return check_clicked(KEY_PRESSED[key_pressed], first_clicked);
    return 0;
}

b8 is_any_key_clicked(b8* first_clicked)
{
    return check_clicked(ANY_KEY_PRESSED, first_clicked);
}

b8 is_any_button_pressed(void)
{
    return ANY_BUTTON_PRESSED;
}

b8 is_any_button_clicked(b8* first_clicked)
{
    return check_clicked(ANY_BUTTON_PRESSED, first_clicked);
}

b8 is_window_focused(void)
{
    return WINDOW_FOCUSED;
}

b8 is_caps_on(void)
{
    return _CAPS_ON != 0;
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


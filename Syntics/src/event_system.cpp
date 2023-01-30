#include "event_system.h"
#include "region_alloc.h"
#include "ansi_keycodes.h"
#include <stdlib.h>

void event_fire();
void set_event_callbacks(void (*on_key_pressed)(uint16 key, uint16 op),
                         void (*on_key_released)(uint16 key, uint16 op),
                         void (*on_button_pressed)(uint8 key, uint16 op),
                         void (*on_button_released)(uint8 key, uint16 op),
                         void (*on_mouse_move)(int16 pos_x, int16 pos_y, uint16 op),
                         void (*on_mouse_wheel)(int16 z_delta),
                         void (*on_window_focused)(bool focused, uint16 op),
                         void (*on_enter_leave)(bool e_l, uint16 op),
                         void (*on_window_resize)(uint16 width, uint16 height));

void get_window_size(uint16* width, uint16* height);

typedef struct Event_Storage
{
    Events* events;
    uint32* free_idxs;
} Event_Storage;

static Event_Storage STORAGE;
static uint32 NUM_EVENTS = 0;
static bool WINDOW_FOCUSED = 0;
static bool ENTER_LEAVE = 0;
static bool INITIALIZED = 0;
static bool ANY_KEY_PRESSED = 0;
static bool ANY_BUTTON_PRESSED = 0;

static uint8 KEY_PRESSED[TOTAL_NUM_KEYS] = { 0 };

static uint16 _CAPS_ON = 0;

static void on_key_pressed(uint16 key, uint16 op)
{
    _CAPS_ON = op;
    ANY_KEY_PRESSED = 1;
    for (uint32 i = 0; i < NUM_EVENTS; i++)
    {
        if (STORAGE.events[i].evt_type == EVT_KEY &&
            STORAGE.events[i].initialize == 1)
        {
            STORAGE.events[i].key_evt.key = key;
            STORAGE.events[i].key_evt.action = 1;
            STORAGE.events[i].activated = 1;
        }
    }
#ifdef LINUX
    switch (key)
    {
        case SYNT_KEY_Q:
        {
            KEY_PRESSED[SYNT_Q_PRESSED] = 1;
            return;
        }
        case SYNT_KEY_W:
        {
            KEY_PRESSED[SYNT_W_PRESSED] = 1;
            return;
        }
        case SYNT_KEY_S:
        {
            KEY_PRESSED[SYNT_S_PRESSED] = 1;
            return;
        }
        case SYNT_KEY_A:
        {
            KEY_PRESSED[SYNT_A_PRESSED] = 1;
            return;
        }
        case SYNT_KEY_D:
        {
            KEY_PRESSED[SYNT_D_PRESSED] = 1;
            return;
        }
        case SYNT_KEY_E:
        {
            KEY_PRESSED[SYNT_E_PRESSED] = 1;
            return;
        }
        case SYNT_KEY_R:
        {
            KEY_PRESSED[SYNT_R_PRESSED] = 1;
            return;
        }
        case SYNT_KEY_F:
        {
            KEY_PRESSED[SYNT_F_PRESSED] = 1;
            return;
        }
        case SYNT_KEY_H:
        {
            KEY_PRESSED[SYNT_H_PRESSED] = 1;
            return;
        }
        case SYNT_KEY_SPACE:
        {
            KEY_PRESSED[SYNT_SPACE_PRESSED] = 1;
            return;
        }
        case SYNT_KEY_CTRL:
        {
            KEY_PRESSED[SYNT_CTRL_PRESSED] = 1;
            return;
        }
        case SYNT_KEY_SHIFT:
        {
            KEY_PRESSED[SYNT_SHIFT_PRESSED] = 1;
            return;
        }
        case SYNT_KEY_LEFT:
        {
            KEY_PRESSED[SYNT_LEFT_PRESSED] = 1;
            return;
        }
        case SYNT_KEY_UP:
        {
            KEY_PRESSED[SYNT_UP_PRESSED] = 1;
            return;
        }
        case SYNT_KEY_RIGHT:
        {
            KEY_PRESSED[SYNT_RIGHT_PRESSED] = 1;
            return;
        }
        case SYNT_KEY_DOWN:
        {
            KEY_PRESSED[SYNT_DOWN_PRESSED] = 1;
            return;
        }
        default:
        {
            synt_LOG("%d\n", key);
            return;
        }
    }
#else
    switch (key)
    {
        case SYNT_ASCII_KEY_Q:
        {
            KEY_PRESSED[SYNT_Q_PRESSED] = 1;
            return;
        }
        case SYNT_ASCII_KEY_W:
        {
            KEY_PRESSED[SYNT_W_PRESSED] = 1;
            return;
        }
        case SYNT_ASCII_KEY_S:
        {
            KEY_PRESSED[SYNT_S_PRESSED] = 1;
            return;
        }
        case SYNT_ASCII_KEY_A:
        {
            KEY_PRESSED[SYNT_A_PRESSED] = 1;
            return;
        }
        case SYNT_ASCII_KEY_D:
        {
            KEY_PRESSED[SYNT_D_PRESSED] = 1;
            return;
        }
        case SYNT_ASCII_KEY_E:
        {
            KEY_PRESSED[SYNT_E_PRESSED] = 1;
            return;
        }
        case SYNT_ASCII_KEY_R:
        {
            KEY_PRESSED[SYNT_R_PRESSED] = 1;
            return;
        }
        case SYNT_ASCII_KEY_F:
        {
            KEY_PRESSED[SYNT_F_PRESSED] = 1;
            return;
        }
        case SYNT_ASCII_KEY_H:
        {
            KEY_PRESSED[SYNT_H_PRESSED] = 1;
            return;
        }
        case SYNT_ASCII_KEY_SPACE:
        {
            KEY_PRESSED[SYNT_SPACE_PRESSED] = 1;
            return;
        }
        case SYNT_ASCII_KEY_LEFT_CTRL:
        {
            KEY_PRESSED[SYNT_CTRL_PRESSED] = 1;
            return;
        }
        case SYNT_ASCII_KEY_LEFT_SHIFT:
        {
            KEY_PRESSED[SYNT_SHIFT_PRESSED] = 1;
            return;
        }
        case SYNT_ASCII_KEY_LEFT:
        {
            KEY_PRESSED[SYNT_LEFT_PRESSED] = 1;
            return;
        }
        case SYNT_ASCII_KEY_UP:
        {
            KEY_PRESSED[SYNT_UP_PRESSED] = 1;
            return;
        }
        case SYNT_ASCII_KEY_RIGHT:
        {
            KEY_PRESSED[SYNT_RIGHT_PRESSED] = 1;
            return;
        }
        case SYNT_ASCII_KEY_DOWN:
        {
            KEY_PRESSED[SYNT_DOWN_PRESSED] = 1;
            return;
        }
        default:
        {
            synt_LOG("%d\n", key);
            return;
        }
    }
#endif
}

static void on_key_released(uint16 key, uint16 op)
{
    ANY_KEY_PRESSED = 0;
    for (uint32 i = 0; i < NUM_EVENTS; i++)
    {
        if (STORAGE.events[i].evt_type == EVT_KEY &&
            STORAGE.events[i].initialize == 1)
        {
            STORAGE.events[i].key_evt.key = key;
            STORAGE.events[i].key_evt.action = 0;
            STORAGE.events[i].activated = 1;
        }
    }

#ifdef LINUX
    switch (key)
    {
        case SYNT_KEY_Q:
        {
            KEY_PRESSED[SYNT_Q_PRESSED] = 0;
            return;
        }
        case SYNT_KEY_W:
        {
            KEY_PRESSED[SYNT_W_PRESSED] = 0;
            return;
        }
        case SYNT_KEY_S:
        {
            KEY_PRESSED[SYNT_S_PRESSED] = 0;
            return;
        }
        case SYNT_KEY_A:
        {
            KEY_PRESSED[SYNT_A_PRESSED] = 0;
            return;
        }
        case SYNT_KEY_D:
        {
            KEY_PRESSED[SYNT_D_PRESSED] = 0;
            return;
        }
        case SYNT_KEY_E:
        {
            KEY_PRESSED[SYNT_E_PRESSED] = 0;
            return;
        }
        case SYNT_KEY_R:
        {
            KEY_PRESSED[SYNT_R_PRESSED] = 0;
            return;
        }
        case SYNT_KEY_F:
        {
            KEY_PRESSED[SYNT_F_PRESSED] = 0;
            return;
        }
        case SYNT_KEY_H:
        {
            KEY_PRESSED[SYNT_H_PRESSED] = 0;
            return;
        }
        case SYNT_KEY_SPACE:
        {
            KEY_PRESSED[SYNT_SPACE_PRESSED] = 0;
            return;
        }
        case SYNT_KEY_CTRL:
        {
            KEY_PRESSED[SYNT_CTRL_PRESSED] = 0;
            return;
        }
        case SYNT_KEY_SHIFT:
        {
            KEY_PRESSED[SYNT_SHIFT_PRESSED] = 0;
            return;
        }
        case SYNT_KEY_LEFT:
        {
            KEY_PRESSED[SYNT_LEFT_PRESSED] = 0;
            return;
        }
        case SYNT_KEY_UP:
        {
            KEY_PRESSED[SYNT_UP_PRESSED] = 0;
            return;
        }
        case SYNT_KEY_RIGHT:
        {
            KEY_PRESSED[SYNT_RIGHT_PRESSED] = 0;
            return;
        }
        case SYNT_KEY_DOWN:
        {
            KEY_PRESSED[SYNT_DOWN_PRESSED] = 0;
            return;
        }
        default:
        {
            return;
        }
    }

#else
    switch (key)
    {
        case SYNT_ASCII_KEY_Q:
        {
            KEY_PRESSED[SYNT_Q_PRESSED] = 0;
            return;
        }
        case SYNT_ASCII_KEY_W:
        {
            KEY_PRESSED[SYNT_W_PRESSED] = 0;
            return;
        }
        case SYNT_ASCII_KEY_S:
        {
            KEY_PRESSED[SYNT_S_PRESSED] = 0;
            return;
        }
        case SYNT_ASCII_KEY_A:
        {
            KEY_PRESSED[SYNT_A_PRESSED] = 0;
            return;
        }
        case SYNT_ASCII_KEY_D:
        {
            KEY_PRESSED[SYNT_D_PRESSED] = 0;
            return;
        }
        case SYNT_ASCII_KEY_E:
        {
            KEY_PRESSED[SYNT_E_PRESSED] = 0;
            return;
        }
        case SYNT_ASCII_KEY_R:
        {
            KEY_PRESSED[SYNT_R_PRESSED] = 0;
            return;
        }
        case SYNT_ASCII_KEY_F:
        {
            KEY_PRESSED[SYNT_F_PRESSED] = 0;
            return;
        }
        case SYNT_ASCII_KEY_H:
        {
            KEY_PRESSED[SYNT_H_PRESSED] = 0;
            return;
        }
        case SYNT_ASCII_KEY_SPACE:
        {
            KEY_PRESSED[SYNT_SPACE_PRESSED] = 0;
            return;
        }
        case SYNT_ASCII_KEY_LEFT_CTRL:
        {
            KEY_PRESSED[SYNT_CTRL_PRESSED] = 0;
            return;
        }
        case SYNT_ASCII_KEY_LEFT_SHIFT:
        {
            KEY_PRESSED[SYNT_SHIFT_PRESSED] = 0;
            return;
        }
        case SYNT_ASCII_KEY_LEFT:
        {
            KEY_PRESSED[SYNT_LEFT_PRESSED] = 0;
            return;
        }
        case SYNT_ASCII_KEY_UP:
        {
            KEY_PRESSED[SYNT_UP_PRESSED] = 0;
            return;
        }
        case SYNT_ASCII_KEY_RIGHT:
        {
            KEY_PRESSED[SYNT_RIGHT_PRESSED] = 0;
            return;
        }
        case SYNT_ASCII_KEY_DOWN:
        {
            KEY_PRESSED[SYNT_DOWN_PRESSED] = 0;
            return;
        }
        default:
        {
            synt_LOG("%d\n", key);
            return;
        }
    }
#endif
}

static void on_button_pressed(uint8 button, uint16 op)
{
    ANY_BUTTON_PRESSED = 1;
    for (uint32 i = 0; i < NUM_EVENTS; i++)
    {
        if (STORAGE.events[i].evt_type == EVT_MOUSE &&
            STORAGE.events[i].initialize == 1)
        {
            STORAGE.events[i].mouse_evt.button_evt.button = button;
            STORAGE.events[i].mouse_evt.button_evt.action = 1;
            STORAGE.events[i].activated = 1;
        }
    }
}

static void on_button_released(uint8 button, uint16 op)
{
    ANY_BUTTON_PRESSED = 0;
    for (uint32 i = 0; i < NUM_EVENTS; i++)
    {
        if (STORAGE.events[i].evt_type == EVT_MOUSE &&
            STORAGE.events[i].initialize == 1)
        {
            STORAGE.events[i].mouse_evt.button_evt.button = button;
            STORAGE.events[i].mouse_evt.button_evt.action = 0;
            STORAGE.events[i].activated = 1;
        }
    }
}

static void on_mouse_move(int16 pos_x, int16 pos_y, uint16 op)
{
    for (uint32 i = 0; i < NUM_EVENTS; i++)
    {
        if (STORAGE.events[i].evt_type == EVT_MOUSE &&
            STORAGE.events[i].initialize == 1)
        {
            STORAGE.events[i].mouse_evt.move_evt.pos_x = pos_x;
            STORAGE.events[i].mouse_evt.move_evt.pos_y = pos_y;
            STORAGE.events[i].activated = 1;
        }
    }
}

static void on_mouse_wheel(int16 z_delta)
{
    for (uint32 i = 0; i < NUM_EVENTS; i++)
    {
        if (STORAGE.events[i].evt_type == EVT_WHEEL &&
            STORAGE.events[i].initialize == 1)
        {
            STORAGE.events[i].wheel_evt.z_delta = z_delta;
            STORAGE.events[i].activated = 1;
        }
    }
}

static void on_window_focused(bool focused, uint16 op)
{
    WINDOW_FOCUSED = focused;
}

static void on_enter_leave(bool e_l, uint16 op)
{
    ENTER_LEAVE = e_l;
}

static void on_window_resize(uint16 width, uint16 height)
{
    for (uint32 i = 0; i < NUM_EVENTS; i++)
    {
        if (STORAGE.events[i].evt_type == EVT_RESIZE &&
            STORAGE.events[i].initialize == 1)
        {
            STORAGE.events[i].resize_evt.width = width;
            STORAGE.events[i].resize_evt.height = height;
            STORAGE.events[i].resize_evt.is_resized = true;
        }
    }
}

void init_events(Region_Alloc* region, uint32 size)
{
    if (!INITIALIZED)
    {
        STORAGE.events = dyn_array(region, size, Events, PERM_ARRAY);
        STORAGE.free_idxs = dyn_array(region, size, uint32, PERM_ARRAY);
        INITIALIZED = 1;
        set_event_callbacks(on_key_pressed, on_key_released, on_button_pressed,
                            on_button_released, on_mouse_move, on_mouse_wheel,
                            on_window_focused, on_enter_leave, on_window_resize);
    }
}

void subscribe(Events** evt, Event_Type evt_type)
{
    assert(evt);
    assert(INITIALIZED);

    assert(NUM_EVENTS <= capacity_arr(STORAGE.events));

    INIT_0(Events, evt_out);
    uint32 size = size_arr(STORAGE.events);
    evt_out.initialize = 1;
    evt_out.evt_type = evt_type;

    if (size_arr(STORAGE.free_idxs) > 0)
    {
        uint32 idx = STORAGE.free_idxs[get_head(STORAGE.free_idxs)->size--];
        evt_out.index = idx;
        STORAGE.events[idx] = evt_out;
        *evt = STORAGE.events + idx;
    }
    else
    {
        evt_out.index = size;
        synt_push(STORAGE.events, evt_out);
        *evt = STORAGE.events + evt_out.index;
    }

    NUM_EVENTS++;
}

void unsubscribe(Events** evt)
{
    assert(evt != NULL || *evt != NULL);

    uint32 index = (*evt)->index;

    STORAGE.events[index].initialize = 0;
    STORAGE.events[index].activated = 0;

    synt_push(STORAGE.free_idxs, index);

    *evt = NULL;
}

void poll_events()
{
    for (uint32 i = 0; i < NUM_EVENTS; i++)
    {
        if (STORAGE.events[i].initialize) STORAGE.events[i].activated = 0;
    }
    event_fire();

    // TODO: dont have enter leave event windows
#if 0
    if (!ENTER_LEAVE)
    {
        for (uint32 i = 0; i < TOTAL_NUM_KEYS; i++)
        {
            KEY_PRESSED[i] = 0;
        }
    }
#endif
}

bool is_key_pressed(uint32 key_pressed_flag)
{
    if (key_pressed_flag < TOTAL_NUM_KEYS) return KEY_PRESSED[key_pressed_flag];
    return 0;
}
bool is_any_key_pressed()
{
    return ANY_KEY_PRESSED;
}

static bool check_clicked(bool pressed, bool& first_clicked)
{
    if (pressed)
    {
        if (first_clicked)
        {
            first_clicked = false;
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        first_clicked = true;
    }
    return false;
}

bool is_any_key_clicked(bool& first_clicked)
{
    return check_clicked(ANY_KEY_PRESSED, first_clicked);
}

bool is_any_button_pressed()
{
    return ANY_BUTTON_PRESSED;
}

bool is_any_button_clicked(bool& first_clicked)
{
    return check_clicked(ANY_BUTTON_PRESSED, first_clicked);
}

bool is_window_focused()
{
    return WINDOW_FOCUSED;
}

bool is_caps_on()
{
    return _CAPS_ON != 0;
}

uint16 code_to_ascii(uint16 key)
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
        default:
        {
            return 0;
        }
    }
}


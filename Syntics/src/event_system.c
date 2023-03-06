#include "event_system.h"
#include "logging.h"
#include "region_alloc.h"
#include "ansi_keycodes.h"

void event_fire();
void set_event_callbacks(void (*on_key_pressed)(u16 key, u16 op),
                         void (*on_key_released)(u16 key, u16 op),
                         void (*on_button_pressed)(u8 key, u16 op),
                         void (*on_button_released)(u8 key, u16 op),
                         void (*on_mouse_move)(i16 pos_x, i16 pos_y, u16 op),
                         void (*on_mouse_wheel)(i16 z_delta),
                         void (*on_window_focused)(b8 focused, u16 op),
                         void (*on_enter_leave)(b8 e_l, u16 op),
                         void (*on_window_resize)(u16 width, u16 height));

void get_window_size(u16* width, u16* height);

// TODO: Have different arrays for all different events; To save itarations
// if it gets to much but right now it's like 7 total so latch
struct Evt_Node
{
    Events evt;
    Events** back_ptr;
};
struct Event_Storage
{
    Evt_Node* evt_linked;
    Events* events;
    u32* free_idxs;
};

static Event_Storage STORAGE;
static b8 WINDOW_FOCUSED = 0;
static b8 ENTER_LEAVE = 0;
static b8 INITIALIZED = 0;
static b8 ANY_KEY_PRESSED = 0;
static b8 ANY_BUTTON_PRESSED = 0;

static u32 NUM_EVENTS = 0;

static u8 KEY_PRESSED[TOTAL_NUM_KEYS] = { 0 };

static u16 _CAPS_ON = 0;

static void on_key_pressed(u16 key, u16 op)
{
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
        case SYNT_ASCII_KEY_L:
        {
            KEY_PRESSED[SYNT_L_PRESSED] = 1;
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
            return;
        }
    }
#endif
}

static void on_key_released(u16 key, u16 op)
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
        case SYNT_ASCII_KEY_L:
        {
            KEY_PRESSED[SYNT_L_PRESSED] = 0;
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
#endif
}

// TODO: temp, if you release button outside window a realse event does not occur
void set_button_unpressed()
{
    ANY_BUTTON_PRESSED = 0;
}

static void on_button_pressed(u8 button, u16 op)
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

static void on_button_released(u8 button, u16 op)
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

static void on_mouse_move(i16 pos_x, i16 pos_y, u16 op)
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

static void on_window_focused(b8 focused, u16 op)
{
    WINDOW_FOCUSED = focused;
}

static void on_enter_leave(b8 e_l, u16 op)
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
        STORAGE.evt_linked = dyn_array(region, size, Evt_Node, PERM_ARRAY);
        STORAGE.events = dyn_array(region, size, Events, PERM_ARRAY);
        STORAGE.free_idxs = dyn_array(region, size, u32, PERM_ARRAY);
        INITIALIZED = 1;
        set_event_callbacks(on_key_pressed, on_key_released, on_button_pressed,
                            on_button_released, on_mouse_move, on_mouse_wheel,
                            on_window_focused, on_enter_leave, on_window_resize);
        NUM_EVENTS = 0;
    }
}

void subscribe(Events** evt, Event_Type evt_type)
{
    assert(evt);
    assert(INITIALIZED);

    INIT_0(Evt_Node, evt_node);
    INIT_0(Events, evt_out);
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
    assert(evt != NULL || *evt != NULL);

    if ((*evt)->initialize)
    {
        u32 index = (*evt)->index;

        u32* size_ptr = &get_head(STORAGE.evt_linked)->size;
        u32 size = *size_ptr;
        if (index > size - 1)
        {
            synt_LOG_Term("Wrong index on evt\n");
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

void poll_events()
{
    for (u32 i = 0; i < NUM_EVENTS; i++)
    {
        STORAGE.evt_linked[i].evt.activated = 0;
    }
    event_fire();

    // TODO: dont have enter leave event windows
#if 0
    if (!ENTER_LEAVE)
    {
        for (u32 i = 0; i < TOTAL_NUM_KEYS; i++)
        {
            KEY_PRESSED[i] = 0;
        }
    }
#endif
}

b8 is_key_pressed(u32 key_pressed_flag)
{
    if (key_pressed_flag < TOTAL_NUM_KEYS) return KEY_PRESSED[key_pressed_flag];
    return 0;
}

b8 is_any_key_pressed()
{
    return ANY_KEY_PRESSED;
}

static b8 check_clicked(b8 pressed, b8& first_clicked)
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

b8 is_key_clicked(b8* first_clicked, u32 key_pressed_flag)
{
    assert(key_pressed_flag < TOTAL_NUM_KEYS);
    return check_clicked(KEY_PRESSED[key_pressed_flag], *first_clicked);
}

b8 is_any_key_clicked(b8* first_clicked)
{
    return check_clicked(ANY_KEY_PRESSED, *first_clicked);
}

b8 is_any_button_pressed()
{
    return ANY_BUTTON_PRESSED;
}

b8 is_any_button_clicked(b8* first_clicked)
{
    return check_clicked(ANY_BUTTON_PRESSED, *first_clicked);
}

b8 is_window_focused()
{
    return WINDOW_FOCUSED;
}

b8 is_caps_on()
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


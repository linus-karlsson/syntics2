#include "EventSystem.h"
#include "RegionAlloc.h"
#include <stdlib.h>
#include <stdio.h>

namespace synt {

    void event_fire();
    void set_event_callbacks(void (*on_key_pressed)(uint8_t key),
                             void (*on_key_released)(uint8_t key),
                             void (*on_button_pressed)(uint8_t key),
                             void (*on_button_released)(uint8_t key),
                             void (*on_mouse_move)(uint16_t pos_x, uint16_t pos_y),
                             void (*set_window_focused)(bool focused));

    typedef struct Event_Storage
    {
        Events* events;
        uint32_t* free_idxs;
    } Event_Storage;

    static Event_Storage STORAGE;
    static uint32_t NUM_EVENTS = 0;
    static bool WINDOW_FOCUSED = false;
    static bool INITIALIZED    = false;

    static uint8_t KEY_PRESSED[TOTAL_NUM_KEYS] = { 0 };

    Events::Events() : initialize(false), activated(false) {}

    Key_Event::Key_Event() : key(0), action(0) {}

    Mouse_Event::Mouse_Event() : action(0), button(0), pos_x(0), pos_y(0) {}

    static void on_key_pressed(uint8_t key)
    {
        for (uint32_t i = 0; i < NUM_EVENTS; i++)
        {
            if (STORAGE.events[i].evt_type == EVT_KEY &&
                STORAGE.events[i].initialize == true)
            {
                STORAGE.events[i].key_evt.key    = key;
                STORAGE.events[i].key_evt.action = 1;
                STORAGE.events[i].activated      = true;
            }
        }
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
            default:
            {
                return;
            }
        }
    }

    static void on_key_released(uint8_t key)
    {
        for (uint32_t i = 0; i < NUM_EVENTS; i++)
        {
            if (STORAGE.events[i].evt_type == EVT_KEY &&
                STORAGE.events[i].initialize == true)
            {
                STORAGE.events[i].key_evt.key    = key;
                STORAGE.events[i].key_evt.action = 0;
                STORAGE.events[i].activated      = true;
            }
        }

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
            default:
            {
                return;
            }
        }
    }

    static void on_button_pressed(uint8_t button)
    {
        for (uint32_t i = 0; i < NUM_EVENTS; i++)
        {
            if (STORAGE.events[i].evt_type == EVT_MOUSE &&
                STORAGE.events[i].initialize == true)
            {
                STORAGE.events[i].mouse_evt.button = button;
                STORAGE.events[i].mouse_evt.action = 1;
                STORAGE.events[i].activated        = true;
            }
        }
    }
    static void on_button_released(uint8_t button)
    {
        for (uint32_t i = 0; i < NUM_EVENTS; i++)
        {
            if (STORAGE.events[i].evt_type == EVT_MOUSE &&
                STORAGE.events[i].initialize == true)
            {
                STORAGE.events[i].mouse_evt.button = button;
                STORAGE.events[i].mouse_evt.action = 0;
                STORAGE.events[i].activated        = true;
            }
        }
    }

    static void on_mouse_move(uint16_t pos_x, uint16_t pos_y)
    {
        for (uint32_t i = 0; i < NUM_EVENTS; i++)
        {
            if (STORAGE.events[i].evt_type == EVT_MOUSE &&
                STORAGE.events[i].initialize == true)
            {
                STORAGE.events[i].mouse_evt.pos_x = pos_x;
                STORAGE.events[i].mouse_evt.pos_y = pos_y;
                STORAGE.events[i].activated       = true;
            }
        }
    }

    static void set_window_focused(bool focused) { WINDOW_FOCUSED = focused; }

    void init_events(Region_Alloc* region, uint32_t size)
    {
        if (!INITIALIZED)
        {
            STORAGE.events    = dyn_array(region, size, Events, PERM_ARRAY);
            STORAGE.free_idxs = dyn_array(region, size, uint32_t, PERM_ARRAY);
            INITIALIZED       = true;
            set_event_callbacks(on_key_pressed, on_key_released, on_button_pressed,
                                on_button_released, on_mouse_move,
                                set_window_focused);
        }
    }

    void subscribe(Events** evt, Event_Type evt_type)
    {
        assert(evt);
        assert(INITIALIZED);

        Events evt_out;
        uint32_t size      = size_arr(STORAGE.events);
        evt_out.initialize = true;
        evt_out.evt_type   = evt_type;
        if (size_arr(STORAGE.free_idxs) > 0)
        {
            uint32_t idx        = synt_pop(STORAGE.free_idxs);
            evt_out.index       = idx;
            STORAGE.events[idx] = evt_out;
            *evt                = STORAGE.events + idx;
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

        uint32_t index = (*evt)->index;

        STORAGE.events[index].initialize = false;
        STORAGE.events[index].activated  = false;

        synt_push(STORAGE.free_idxs, index);

        *evt = NULL;
    }

    void poll_events()
    {
        for (uint32_t i = 0; i < NUM_EVENTS; i++)
        {
            if (STORAGE.events[i].initialize) STORAGE.events[i].activated = false;
        }
        event_fire();
    }

    bool is_key_pressed(uint32_t key_pressed_flag)
    {
        if (key_pressed_flag < TOTAL_NUM_KEYS) return KEY_PRESSED[key_pressed_flag];
        return false;
    }
    bool is_window_focused() { return WINDOW_FOCUSED; }

} // namespace synt

#include "linux_platform.h"
#include "event_system.h"
#include <xcb/xfixes.h>
#include "logging.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

namespace synt {

typedef struct Callbacks
{
    void (*on_key_pressed)(uint8 key, uint16 op);
    void (*on_key_released)(uint8 key, uint16 op);
    void (*on_button_pressed)(uint8 key, uint16 op);
    void (*on_button_released)(uint8 key, uint16 op);
    void (*on_mouse_move)(uint16 pos_x, uint16 pos_y, uint16 op);
    void (*on_window_focused)(bool focused, uint16 op);
    void (*on_enter_leave)(bool e_l, uint16 op);
} Callbacks;

static Linux_Platform xcb_internal_state;
static Callbacks callback_handler;
static bool INITIALIZED = 0;

static uint16 POS_X = 0;
static uint16 POS_Y = 0;

static uint16 SAVED_X = 0;
static uint16 SAVED_Y = 0;

typedef struct Gui_Window_Internal Gui_Window_Internal;
struct Gui_Window_Internal
{
    Linux_Platform platform;
    bool active = false;
};

static Gui_Window_Internal gui_internal[NUM_GUI_WINDOWS];
static uint32 gui_size = 0;

static uint32 gui_free_idx[NUM_GUI_WINDOWS];
static uint32 gui_idx_size = 0;

const Linux_Platform& get_platform_state() { return xcb_internal_state; }

void init_platform(const char* title, uint16 width, uint16 height)
{
    assert(INITIALIZED == 0);

    INITIALIZED = 1;

    xcb_internal_state.screen_number = 0;
    xcb_internal_state.connection =
        xcb_connect(NULL, &xcb_internal_state.screen_number);
    xcb_internal_state.screen =
        xcb_setup_roots_iterator(xcb_get_setup(xcb_internal_state.connection))
            .data;
    xcb_internal_state.window = xcb_generate_id(xcb_internal_state.connection);

    uint32 mask     = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32 values[] = {
        xcb_internal_state.screen->black_pixel,

        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
            XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_ENTER_WINDOW |
            XCB_EVENT_MASK_LEAVE_WINDOW | XCB_EVENT_MASK_KEY_PRESS |
            XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_FOCUS_CHANGE,
    };

    xcb_create_window(xcb_internal_state.connection, XCB_COPY_FROM_PARENT,
                      xcb_internal_state.window,
                      xcb_internal_state.screen->root, 0, 0, width, height, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      xcb_internal_state.screen->root_visual, mask, values);

    xcb_map_window(xcb_internal_state.connection, xcb_internal_state.window);

    xcb_flush(xcb_internal_state.connection);

    change_title(title, strlen(title));

    xcb_internal_state.width  = width;
    xcb_internal_state.height = height;
}

void set_event_callbacks(void (*on_key_pressed)(uint8 key, uint16 op),
                         void (*on_key_released)(uint8 key, uint16 op),
                         void (*on_button_pressed)(uint8 key, uint16 op),
                         void (*on_button_released)(uint8 key, uint16 op),
                         void (*on_mouse_move)(uint16 pos_x, uint16 pos_y,
                                               uint16 op),
                         void (*on_window_focused)(bool focused, uint16 op),
                         void (*on_enter_leave)(bool e_l, uint16 op))
{
    callback_handler.on_key_pressed     = on_key_pressed;
    callback_handler.on_key_released    = on_key_released;
    callback_handler.on_button_pressed  = on_button_pressed;
    callback_handler.on_button_released = on_button_released;
    callback_handler.on_mouse_move      = on_mouse_move;
    callback_handler.on_window_focused  = on_window_focused;
    callback_handler.on_enter_leave     = on_enter_leave;
}

void change_title(const char* title, uint32 len)
{
    xcb_change_property(xcb_internal_state.connection, XCB_PROP_MODE_REPLACE,
                        xcb_internal_state.window, XCB_ATOM_WM_NAME,
                        XCB_ATOM_STRING, 8, len, title);
    xcb_flush(xcb_internal_state.connection);
}

void event_fire()
{
    uint8 key = 0;
    xcb_generic_event_t* event;

    while ((event = xcb_poll_for_event(xcb_internal_state.connection)))
    {
        switch (event->response_type & ~0x80)
        {
            case XCB_KEY_PRESS:
            {
                xcb_key_press_event_t* pressEvt = (xcb_key_press_event_t*)event;

                key = pressEvt->detail;
                callback_handler.on_key_pressed(key, SYNT_OP_MAINWINDOW);

                break;
            }
            case XCB_KEY_RELEASE:
            {
                xcb_key_release_event_t* releaseEvt =
                    (xcb_key_release_event_t*)event;

                key = releaseEvt->detail;
                callback_handler.on_key_released(key, SYNT_OP_MAINWINDOW);

                break;
            }
            case XCB_BUTTON_PRESS:
            {
                xcb_button_press_event_t* button_pressed =
                    (xcb_button_press_event_t*)event;

                uint8 button = button_pressed->detail;

                callback_handler.on_button_pressed(button, SYNT_OP_MAINWINDOW);

                break;
            }
            case XCB_BUTTON_RELEASE:
            {
                xcb_button_release_event_t* button_pressed =
                    (xcb_button_release_event_t*)event;

                uint8 button = button_pressed->detail;

                callback_handler.on_button_released(button, SYNT_OP_MAINWINDOW);

                break;
            }
            case XCB_MOTION_NOTIFY:
            {
                xcb_motion_notify_event_t* mouse_moved =
                    (xcb_motion_notify_event_t*)event;

                POS_X = mouse_moved->event_x;
                POS_Y = mouse_moved->event_y;

                callback_handler.on_mouse_move(POS_X, POS_Y,
                                               SYNT_OP_MAINWINDOW);

                break;
            }
            case XCB_FOCUS_IN:
            {
                callback_handler.on_window_focused(1, SYNT_OP_MAINWINDOW);
                break;
            }
            case XCB_FOCUS_OUT:
            {
                callback_handler.on_window_focused(0, SYNT_OP_MAINWINDOW);
                break;
            }
            case XCB_ENTER_NOTIFY:
            {
                callback_handler.on_enter_leave(1, SYNT_OP_MAINWINDOW);
                break;
            }
            case XCB_LEAVE_NOTIFY:
            {
                callback_handler.on_enter_leave(0, SYNT_OP_MAINWINDOW);
                break;
            }
            default:
            {
                break;
            }
        }
        free(event);
    }
    for (uint32 i = 0; i < gui_size; i++)
    {
        if (gui_internal[i].active)
        {
            while ((event = xcb_poll_for_event(
                        gui_internal[i].platform.connection)))
            {
                switch (event->response_type & ~0x80)
                {
                    case XCB_KEY_PRESS:
                    {
                        xcb_key_press_event_t* pressEvt =
                            (xcb_key_press_event_t*)event;

                        key = pressEvt->detail;
                        callback_handler.on_key_pressed(key, i);

                        break;
                    }
                    case XCB_KEY_RELEASE:
                    {
                        xcb_key_release_event_t* releaseEvt =
                            (xcb_key_release_event_t*)event;

                        key = releaseEvt->detail;
                        callback_handler.on_key_released(key, i);

                        break;
                    }
                    case XCB_BUTTON_PRESS:
                    {
                        xcb_button_press_event_t* button_pressed =
                            (xcb_button_press_event_t*)event;

                        uint8 button = button_pressed->detail;

                        callback_handler.on_button_pressed(button, i);

                        break;
                    }
                    case XCB_BUTTON_RELEASE:
                    {
                        xcb_button_release_event_t* button_pressed =
                            (xcb_button_release_event_t*)event;

                        uint8 button = button_pressed->detail;

                        callback_handler.on_button_released(button, i);

                        break;
                    }
                    case XCB_MOTION_NOTIFY:
                    {
                        xcb_motion_notify_event_t* mouse_moved =
                            (xcb_motion_notify_event_t*)event;

                        POS_X = mouse_moved->event_x;
                        POS_Y = mouse_moved->event_y;

                        callback_handler.on_mouse_move(POS_X, POS_Y, i);

                        break;
                    }
                    case XCB_ENTER_NOTIFY:
                    {
                        callback_handler.on_window_focused(1, i);
                    }
                    case XCB_LEAVE_NOTIFY:
                    {
                        callback_handler.on_window_focused(0, i);
                    }
                    default:
                    {
                        break;
                    }
                }
                free(event);
            }
        }
    }
}

void get_window_size(uint16* width, uint16* height)
{
    xcb_get_geometry_reply_t* reply;
    xcb_get_geometry_cookie_t cookie = xcb_get_geometry(
        xcb_internal_state.connection, xcb_internal_state.window);

    if ((reply = xcb_get_geometry_reply(xcb_internal_state.connection, cookie,
                                        NULL)))
    {
        xcb_internal_state.width  = reply->width;
        xcb_internal_state.height = reply->height;
    }
    free(reply);

    if (width) *width = xcb_internal_state.width;
    if (height) *height = xcb_internal_state.height;

    xcb_flush(xcb_internal_state.connection);
}

static bool MOUSE_HIDDEN = false;

void hide_cursor()
{
    if (!MOUSE_HIDDEN)
    {
        SAVED_X = POS_X;
        SAVED_Y = POS_Y;

        xcb_xfixes_query_version(xcb_internal_state.connection, 4, 0);
        xcb_xfixes_hide_cursor(xcb_internal_state.connection,
                               xcb_internal_state.screen->root);
        xcb_flush(xcb_internal_state.connection);
    }
    MOUSE_HIDDEN = true;
}

void show_cursor()
{
    if (MOUSE_HIDDEN)
    {
        xcb_xfixes_query_version(xcb_internal_state.connection, 4, 0);
        xcb_xfixes_show_cursor(xcb_internal_state.connection,
                               xcb_internal_state.screen->root);
        xcb_flush(xcb_internal_state.connection);
    }
    MOUSE_HIDDEN = false;
}

void show_cursor_centered()
{
    if (MOUSE_HIDDEN)
    {
        set_mouse_pos(xcb_internal_state.width / 2,
                      xcb_internal_state.height / 2);
    }
    show_cursor();
    MOUSE_HIDDEN = false;
}

void show_cursor_last_pos()
{
    if (MOUSE_HIDDEN)
    {
        set_mouse_last_pos();
    }
    show_cursor();
    MOUSE_HIDDEN = false;
}

void set_mouse_pos(uint16 pos_x, uint16 pos_y)
{
    xcb_warp_pointer(xcb_internal_state.connection, xcb_internal_state.window,
                     xcb_internal_state.window, 0, 0, xcb_internal_state.width,
                     xcb_internal_state.height, pos_x, pos_y);
    xcb_flush(xcb_internal_state.connection);
    POS_X = pos_x;
    POS_Y = pos_y;
}

void set_mouse_last_pos()
{
    xcb_warp_pointer(xcb_internal_state.connection, xcb_internal_state.window,
                     xcb_internal_state.window, 0, 0, xcb_internal_state.width,
                     xcb_internal_state.height, SAVED_X, SAVED_Y);
    xcb_flush(xcb_internal_state.connection);
    POS_X = SAVED_X;
    POS_Y = SAVED_Y;
}

void get_pos(uint16& pos_x, uint16& pos_y)
{
    pos_x = POS_X;
    pos_y = POS_Y;
}

void shut_down_platform() { xcb_disconnect(xcb_internal_state.connection); }

Gui_Window::Gui_Window() : m_id(GUI_DEACTIVATED) {}
Gui_Window::Gui_Window(const char* title, uint32 width, uint32 height,
                       uint32 pos_x, uint32 pos_y)
    : m_id(GUI_DEACTIVATED)
{
    create_window(title, width, height, pos_x, pos_y);
}
Gui_Window::~Gui_Window() { destroy_window(); }

void Gui_Window::create_window(const char* title, uint32 width, uint32 height,
                               uint32 pos_x, uint32 pos_y)
{
    if (m_id == GUI_DEACTIVATED)
    {
        Gui_Window_Internal* gui_ptr = NULL;
        if (gui_idx_size > 0)
        {
            m_id    = gui_idx_size;
            gui_ptr = &gui_internal[gui_free_idx[gui_idx_size--]];
        }
        else
        {
            m_id    = gui_size;
            gui_ptr = &gui_internal[gui_size++];
        }

        gui_ptr->platform.screen_number = 0;
        gui_ptr->platform.connection =
            xcb_connect(NULL, &gui_ptr->platform.screen_number);
        gui_ptr->platform.screen =
            xcb_setup_roots_iterator(
                xcb_get_setup(gui_ptr->platform.connection))
                .data;
        gui_ptr->platform.window =
            xcb_generate_id(gui_ptr->platform.connection);

        uint32 mask     = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
        uint32 values[] = {
            gui_ptr->platform.screen->black_pixel,

            XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
                XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_ENTER_WINDOW |
                XCB_EVENT_MASK_LEAVE_WINDOW | XCB_EVENT_MASK_KEY_PRESS |
                XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_FOCUS_CHANGE,
        };

        xcb_create_window(gui_ptr->platform.connection, XCB_COPY_FROM_PARENT,
                          gui_ptr->platform.window, xcb_internal_state.window,
                          pos_x, pos_y, width, height, 0,
                          XCB_WINDOW_CLASS_INPUT_OUTPUT,
                          gui_ptr->platform.screen->root_visual, mask, values);

        xcb_map_window(gui_ptr->platform.connection, gui_ptr->platform.window);

        xcb_flush(gui_ptr->platform.connection);

        if (title != NULL) change_title(title, strlen(title));

        gui_ptr->active = true;
    }
    else
    {
        ERROR("Window already activated");
    }
}

void Gui_Window::destroy_window()
{
    if (m_id != GUI_DEACTIVATED)
    {
        xcb_disconnect(gui_internal[m_id].platform.connection);
        gui_internal[m_id].active    = false;
        gui_free_idx[gui_idx_size++] = m_id;

        m_id = GUI_DEACTIVATED;
    }
}

void move_window(uint32 window_id, uint16 pos_x, uint16 pos_y)
{
    xcb_query_pointer_reply_t* reply;
    xcb_query_pointer_cookie_t cookie =
        xcb_query_pointer(gui_internal[window_id].platform.connection,
                          gui_internal[window_id].platform.window);

    if ((reply = xcb_query_pointer_reply(
             gui_internal[window_id].platform.connection, cookie, NULL)))
    {
        int16 values[] = { reply->root_x, reply->root_y };

        xcb_configure_window(gui_internal[window_id].platform.connection,
                             gui_internal[window_id].platform.window,
                             XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);

        printf("x: %d, y: %d\n", values[0], values[1]);
    }
    free(reply);

    xcb_flush(gui_internal[window_id].platform.connection);
}

} // namespace synt

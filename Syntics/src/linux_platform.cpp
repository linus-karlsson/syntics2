#include "linux_platform.h"
#include "event_system.h"
#include "logging.h"
#include <xcb/xfixes.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

static Linux_Platform xcb_internal_contex;
static Callbacks callback_handler;
static bool INITIALIZED = 0;

static int16 POS_X = 0;
static int16 POS_Y = 0;

static int16 SAVED_X = 0;
static int16 SAVED_Y = 0;

const Linux_Platform& get_platform_state() { return xcb_internal_contex; }

void init_platform(const char* title, uint16 width, uint16 height)
{
    assert(INITIALIZED == 0);

    INITIALIZED = 1;

    xcb_internal_contex.connection = xcb_connect(NULL, NULL);

    xcb_internal_contex.screen =
        xcb_setup_roots_iterator(xcb_get_setup(xcb_internal_contex.connection)).data;

    xcb_internal_contex.window = xcb_generate_id(xcb_internal_contex.connection);

    uint32 mask     = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32 values[] = {
        xcb_internal_contex.screen->black_pixel,

        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
            XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_ENTER_WINDOW |
            XCB_EVENT_MASK_LEAVE_WINDOW | XCB_EVENT_MASK_KEY_PRESS |
            XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_FOCUS_CHANGE,
    };

    xcb_create_window(xcb_internal_contex.connection, XCB_COPY_FROM_PARENT,
                      xcb_internal_contex.window, xcb_internal_contex.screen->root,
                      0, 0, width, height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      xcb_internal_contex.screen->root_visual, mask, values);

    xcb_map_window(xcb_internal_contex.connection, xcb_internal_contex.window);

    xcb_flush(xcb_internal_contex.connection);

    change_title(title, strlen(title));

    xcb_internal_contex.width  = width;
    xcb_internal_contex.height = height;
}

void set_event_callbacks(void (*on_key_pressed)(uint16 key, uint16 op),
                         void (*on_key_released)(uint16 key, uint16 op),
                         void (*on_button_pressed)(uint8 key, uint16 op),
                         void (*on_button_released)(uint8 key, uint16 op),
                         void (*on_mouse_move)(int16 pos_x, int16 pos_y, uint16 op),
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
    xcb_change_property(xcb_internal_contex.connection, XCB_PROP_MODE_REPLACE,
                        xcb_internal_contex.window, XCB_ATOM_WM_NAME,
                        XCB_ATOM_STRING, 8, len, title);
    xcb_flush(xcb_internal_contex.connection);
}

void event_fire()
{
    uint8 key = 0;
    xcb_generic_event_t* event;

    while ((event = xcb_poll_for_event(xcb_internal_contex.connection)))
    {
        switch (event->response_type & ~0x80)
        {
            case XCB_KEY_PRESS:
            {
                xcb_key_press_event_t* pressEvt = (xcb_key_press_event_t*)event;

                key = pressEvt->detail;
                callback_handler.on_key_pressed(key, 0);

                break;
            }
            case XCB_KEY_RELEASE:
            {
                xcb_key_release_event_t* releaseEvt =
                    (xcb_key_release_event_t*)event;

                key = releaseEvt->detail;
                callback_handler.on_key_released(key, 0);

                break;
            }
            case XCB_BUTTON_PRESS:
            {

                xcb_button_press_event_t* button_pressed =
                    (xcb_button_press_event_t*)event;

                uint8 button = button_pressed->detail;

                callback_handler.on_button_pressed(button, 0);

                break;
            }
            case XCB_BUTTON_RELEASE:
            {
                xcb_button_release_event_t* button_pressed =
                    (xcb_button_release_event_t*)event;

                uint8 button = button_pressed->detail;

                callback_handler.on_button_released(button, 0);

                break;
            }
            case XCB_MOTION_NOTIFY:
            {
                xcb_motion_notify_event_t* mouse_moved =
                    (xcb_motion_notify_event_t*)event;

                POS_X = mouse_moved->event_x;
                POS_Y = mouse_moved->event_y;

                callback_handler.on_mouse_move(POS_X, POS_Y, 0);

                break;
            }
            case XCB_FOCUS_IN:
            {
                callback_handler.on_window_focused(1, 0);
                break;
            }
            case XCB_FOCUS_OUT:
            {
                callback_handler.on_window_focused(0, 0);
                break;
            }
            case XCB_ENTER_NOTIFY:
            {
                callback_handler.on_enter_leave(1, 0);
                break;
            }
            case XCB_LEAVE_NOTIFY:
            {
                callback_handler.on_enter_leave(0, 0);
                break;
            }
            default:
            {
                break;
            }
        }
        free(event);
    }
}

void move_main_window()
{

    xcb_query_pointer_reply_t* reply;
    xcb_query_pointer_cookie_t cookie = xcb_query_pointer(
        xcb_internal_contex.connection, xcb_internal_contex.window);

    if ((reply =
             xcb_query_pointer_reply(xcb_internal_contex.connection, cookie, NULL)))
    {
        int16 values[] = { reply->win_x, reply->win_y };

        xcb_configure_window(xcb_internal_contex.connection,
                             xcb_internal_contex.window,
                             XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);

        printf("x: %d, y: %d\n", values[0], values[1]);
    }
    free(reply);
}

void get_window_size(uint16* width, uint16* height)
{
    xcb_get_geometry_reply_t* reply;
    xcb_get_geometry_cookie_t cookie =
        xcb_get_geometry(xcb_internal_contex.connection, xcb_internal_contex.window);

    if ((reply =
             xcb_get_geometry_reply(xcb_internal_contex.connection, cookie, NULL)))
    {
        xcb_internal_contex.width  = reply->width;
        xcb_internal_contex.height = reply->height;
    }
    free(reply);

    if (width) *width = xcb_internal_contex.width;
    if (height) *height = xcb_internal_contex.height;

    xcb_flush(xcb_internal_contex.connection);
}

static bool MOUSE_HIDDEN = false;

void hide_cursor()
{
    if (!MOUSE_HIDDEN)
    {
        SAVED_X = POS_X;
        SAVED_Y = POS_Y;

        xcb_xfixes_query_version(xcb_internal_contex.connection, 4, 0);
        xcb_xfixes_hide_cursor(xcb_internal_contex.connection,
                               xcb_internal_contex.screen->root);
        xcb_flush(xcb_internal_contex.connection);
    }
    MOUSE_HIDDEN = true;
}

void show_cursor()
{
    if (MOUSE_HIDDEN)
    {
        xcb_xfixes_query_version(xcb_internal_contex.connection, 4, 0);
        xcb_xfixes_show_cursor(xcb_internal_contex.connection,
                               xcb_internal_contex.screen->root);
        xcb_flush(xcb_internal_contex.connection);
    }
    MOUSE_HIDDEN = false;
}

void show_cursor_centered()
{
    if (MOUSE_HIDDEN)
    {
        set_mouse_pos(xcb_internal_contex.width / 2, xcb_internal_contex.height / 2);
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

void set_mouse_pos(int16 pos_x, int16 pos_y)
{
    xcb_warp_pointer(xcb_internal_contex.connection, xcb_internal_contex.window,
                     xcb_internal_contex.window, 0, 0, xcb_internal_contex.width,
                     xcb_internal_contex.height, pos_x, pos_y);
    xcb_flush(xcb_internal_contex.connection);
    POS_X = pos_x;
    POS_Y = pos_y;
}

void set_mouse_last_pos()
{
    xcb_warp_pointer(xcb_internal_contex.connection, xcb_internal_contex.window,
                     xcb_internal_contex.window, 0, 0, xcb_internal_contex.width,
                     xcb_internal_contex.height, SAVED_X, SAVED_Y);
    xcb_flush(xcb_internal_contex.connection);
    POS_X = SAVED_X;
    POS_Y = SAVED_Y;
}

void get_pos(int16& pos_x, int16& pos_y)
{
    pos_x = POS_X;
    pos_y = POS_Y;
}

double get_time()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec + (now.tv_nsec * 0.000000001);
}

void linux_sleep(uint64 milli)
{
#if _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec  = milli / 1000;
    ts.tv_nsec = (milli % 1000) * 1000 * 1000;
    nanosleep(&ts, 0);
#else
    if (milli >= 1000)
    {
        sleep(milli / 1000);
    }
    usleep((milli % 1000) * 1000);
#endif
}

void shut_down_platform() { xcb_disconnect(xcb_internal_contex.connection); }

} // namespace synt

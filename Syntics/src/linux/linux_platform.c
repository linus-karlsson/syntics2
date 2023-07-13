#include "linux_platform.h"
#include "event_system.h"
#include "logging.h"
#include <xcb/xfixes.h>
#include <xcb/xcb_cursor.h>
#include <stdlib.h>
#include <string.h>

static u32 synt_current_cursor = SYNT_NORMAL_CURSOR;

typedef struct Callbacks
{
    void (*on_key_pressed)(u16 key, u16 op);
    void (*on_key_released)(u16 key, u16 op);
    void (*on_button_pressed)(u8 key, u16 op);
    void (*on_button_released)(u8 key, u16 op);
    void (*on_mouse_move)(i16 pos_x, i16 pos_y, u16 op);
    void (*on_window_focused)(b8 focused, u16 op);
    void (*on_enter_leave)(b8 e_l, u16 op);
} Callbacks;

static Linux_Platform xcb_internal_contex;
static Callbacks callback_handler;
static b8 INITIALIZED = 0;

static i16 POS_X = 0;
static i16 POS_Y = 0;

static i16 SAVED_X = 0;
static i16 SAVED_Y = 0;

const Linux_Platform& get_platform_state()
{
    return xcb_internal_contex;
}

void init_platform(const char* title, u16 width, u16 height)
{
    assert(INITIALIZED == 0);

    INITIALIZED = 1;

    xcb_internal_contex.connection = xcb_connect(NULL, NULL);

    xcb_internal_contex.screen =
        xcb_setup_roots_iterator(xcb_get_setup(xcb_internal_contex.connection)).data;

    xcb_internal_contex.window = xcb_generate_id(xcb_internal_contex.connection);

    xcb_cursor_context_t* ctx;
    xcb_cursor_context_new(xcb_internal_contex.connection,
                           xcb_internal_contex.screen, &ctx);
    xcb_internal_contex.cursors[0] = xcb_cursor_load_cursor(ctx, "default");
    xcb_internal_contex.cursors[1] = xcb_cursor_load_cursor(ctx, "pointing_hand");
    xcb_internal_contex.cursors[2] = xcb_cursor_load_cursor(ctx, "col-resize");
    xcb_internal_contex.cursors[3] = xcb_cursor_load_cursor(ctx, "move");
    xcb_cursor_context_free(ctx);

    u32 mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    u32 values[] = {
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

    xcb_internal_contex.width = width;
    xcb_internal_contex.height = height;
}

xcb_window_t child_window(const char* title, u16 width, u16 height)
{
    u32 mask = XCB_CW_OVERRIDE_REDIRECT;
    u32 values[] = { 1 };

    xcb_window_t child;
    child = xcb_generate_id(xcb_internal_contex.connection);
    xcb_create_window(xcb_internal_contex.connection, XCB_COPY_FROM_PARENT, child,
                      xcb_internal_contex.window, 100, 100, 200, 200, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      xcb_internal_contex.screen->root_visual, mask, values);

    xcb_map_window(xcb_internal_contex.connection, child);

    xcb_flush(xcb_internal_contex.connection);

    return child;
}

void set_event_callbacks(void (*on_key_pressed)(u16 key, u16 op),
                         void (*on_key_released)(u16 key, u16 op),
                         void (*on_button_pressed)(u8 key, u16 op),
                         void (*on_button_released)(u8 key, u16 op),
                         void (*on_mouse_move)(i16 pos_x, i16 pos_y, u16 op),
                         void (*on_window_focused)(b8 focused, u16 op),
                         void (*on_enter_leave)(b8 e_l, u16 op))
{
    callback_handler.on_key_pressed = on_key_pressed;
    callback_handler.on_key_released = on_key_released;
    callback_handler.on_button_pressed = on_button_pressed;
    callback_handler.on_button_released = on_button_released;
    callback_handler.on_mouse_move = on_mouse_move;
    callback_handler.on_window_focused = on_window_focused;
    callback_handler.on_enter_leave = on_enter_leave;
}

void change_title(const char* title, u32 len)
{
    xcb_change_property(xcb_internal_contex.connection, XCB_PROP_MODE_REPLACE,
                        xcb_internal_contex.window, XCB_ATOM_WM_NAME,
                        XCB_ATOM_STRING, 8, len, title);
    xcb_flush(xcb_internal_contex.connection);
}

void event_fire()
{
    u8 key = 0;
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

                u8 button = button_pressed->detail;

                callback_handler.on_button_pressed(button, 0);

                break;
            }
            case XCB_BUTTON_RELEASE:
            {
                xcb_button_release_event_t* button_pressed =
                    (xcb_button_release_event_t*)event;

                u8 button = button_pressed->detail;

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
            // TODO: size event
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
        i16 values[] = { reply->win_x, reply->win_y };

        xcb_configure_window(xcb_internal_contex.connection,
                             xcb_internal_contex.window,
                             XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);

        printf("x: %d, y: %d\n", values[0], values[1]);
    }
    free(reply);
}

// TODO: change to get event and only use that;
void get_window_size(u16& width, u16& height)
{
    width = xcb_internal_contex.width;
    height = xcb_internal_contex.height;
}

static b8 MOUSE_HIDDEN = false;

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

void change_cursor(u32 cursor_id)
{
    if (synt_current_cursor != cursor_id)
    {
        if (cursor_id < TOTAL_CURSORS)
        {
            synt_current_cursor = cursor_id;
        }
        else
        {
            synt_LOG("WARNING: trying to change to a cursor that doesn't exist.");
        }
    }
}

void set_mouse_pos(i16 pos_x, i16 pos_y)
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

void get_pos(i16& pos_x, i16& pos_y)
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

void platform_sleep(u32milli)
{
#if _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = milli / 1000;
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

void shut_down_platform()
{
    xcb_disconnect(xcb_internal_contex.connection);
}

void read_file(File_Attrib* file_attrib, Region_Alloc* region, const char* file_path,
               const char* operation)
{
    FILE* file = fopen(file_path, operation);

    if (file == NULL) SY_ERROR(file_path);

    fseek(file, 0, SEEK_END);
    file_attrib.size = (uint32)ftell(file);
    rewind(file);

    if (region)
    {
        file_attrib->buffer = region_mallocT(region, file_attrib->size, u8);
    }
    else
    {
        file_attrib->buffer = (u8*)malloc(file_attrib->size);
    }

    if (fread(file_attrib.buffer, 1, file_attrib.size, file) != file_attrib.size)
    {
        OutputDebugString("Read file error");
        SY_ERROR(file_path);
    }
    fclose(file);
}

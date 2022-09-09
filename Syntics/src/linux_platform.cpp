#include "linux_platform.h"
#include "event_system.h"
#include <assert.h>
#include <stdlib.h>

namespace synt {

    typedef struct Callback_Handler
    {
        void (*on_key_pressed)(uint8 key);
        void (*on_key_released)(uint8 key);
        void (*on_button_pressed)(uint8 key);
        void (*on_button_released)(uint8 key);
        void (*on_mouse_move)(uint16 pos_x, uint16 pos_y);
        void (*set_window_focused)(bool focused);
    } Callback_Handler;

    static Linux_Platform* xcb_internal_handle;
    static Callback_Handler callback_handler;
    static bool INITIALIZED = 0;

    void init_platform(Linux_Platform* xcb, uint16 width, uint16 height)
    {
        assert(xcb);

        assert(INITIALIZED == 0);

        INITIALIZED = 1;

        xcb->screen_number = 0;
        xcb->connection    = xcb_connect(NULL, &xcb->screen_number);
        xcb->screen = xcb_setup_roots_iterator(xcb_get_setup(xcb->connection)).data;
        xcb->window = xcb_generate_id(xcb->connection);
        xcb_void_cookie_t cookies[3];

        uint32 mask     = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
        uint32 values[] = {
            xcb->screen->black_pixel,

            XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
                XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_ENTER_WINDOW |
                XCB_EVENT_MASK_LEAVE_WINDOW | XCB_EVENT_MASK_KEY_PRESS |
                XCB_EVENT_MASK_KEY_RELEASE,
        };

        cookies[0] = xcb_create_window(xcb->connection, XCB_COPY_FROM_PARENT,
                                       xcb->window, xcb->screen->root, 0, 0, width,
                                       height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                                       xcb->screen->root_visual, mask, values);

        cookies[1] = xcb_map_window(xcb->connection, xcb->window);

        xcb_flush(xcb->connection);

        xcb->width          = width;
        xcb->height         = height;
        xcb_internal_handle = xcb;
    }

    void set_event_callbacks(void (*on_key_pressed)(uint8 key),
                             void (*on_key_released)(uint8 key),
                             void (*on_button_pressed)(uint8 key),
                             void (*on_button_released)(uint8 key),
                             void (*on_mouse_move)(uint16 pos_x, uint16 pos_y),
                             void (*set_window_focused)(bool focused))
    {
        callback_handler.on_key_pressed     = on_key_pressed;
        callback_handler.on_key_released    = on_key_released;
        callback_handler.on_button_pressed  = on_button_pressed;
        callback_handler.on_button_released = on_button_released;
        callback_handler.on_mouse_move      = on_mouse_move;
        callback_handler.set_window_focused = set_window_focused;
    }

    void event_fire()
    {
        uint8 key = 0;
        xcb_generic_event_t* event;

        while ((event = xcb_poll_for_event(xcb_internal_handle->connection)))
        {
            switch (event->response_type & ~0x80)
            {
                case XCB_KEY_PRESS:
                {
                    xcb_key_press_event_t* pressEvt = (xcb_key_press_event_t*)event;

                    key = pressEvt->detail;
                    callback_handler.on_key_pressed(key);

                    break;
                }
                case XCB_KEY_RELEASE:
                {
                    xcb_key_release_event_t* releaseEvt =
                        (xcb_key_release_event_t*)event;

                    key = releaseEvt->detail;
                    callback_handler.on_key_released(key);

                    break;
                }
                case XCB_BUTTON_PRESS:
                {
                    xcb_button_press_event_t* button_pressed =
                        (xcb_button_press_event_t*)event;

                    uint8 button = button_pressed->detail;

                    callback_handler.on_button_pressed(button);

                    break;
                }
                case XCB_BUTTON_RELEASE:
                {
                    xcb_button_release_event_t* button_pressed =
                        (xcb_button_release_event_t*)event;

                    uint8 button = button_pressed->detail;

                    callback_handler.on_button_released(button);

                    break;
                }
                case XCB_MOTION_NOTIFY:
                {
                    xcb_motion_notify_event_t* mouse_moved =
                        (xcb_motion_notify_event_t*)event;

                    uint16 pos_x = mouse_moved->event_x;
                    uint16 pos_y = mouse_moved->event_y;

                    callback_handler.on_mouse_move(pos_x, pos_y);

                    break;
                }
                case XCB_ENTER_NOTIFY:
                {
                    callback_handler.set_window_focused(1);
                }
                case XCB_LEAVE_NOTIFY:
                {
                    callback_handler.set_window_focused(0);
                }
                default:
                {
                    break;
                }
            }
            free(event);
        }
    }

    void shut_down_platform() { xcb_disconnect(xcb_internal_handle->connection); }

} // namespace synt

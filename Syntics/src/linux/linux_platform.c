
typedef struct Callbacks
{
    void (*on_key_pressed)(u16 key, u16 op);
    void (*on_key_released)(u16 key);
    void (*on_button_pressed)(u8 key);
    void (*on_button_released)(u8 key);
    void (*on_mouse_move)(i16 pos_x, i16 pos_y);
    void (*on_mouse_wheel)(i16 z_delta);
    void (*on_window_focused)(b8 focused);
    void (*on_enter_leave)(b8 e_l);
    void (*on_window_resize)(u16 width, u16 height);
} Callbacks;

#define SYNT_NORMAL_CURSOR 0
#define SYNT_HAND_CURSOR 1
#define SYNT_RESIZE_H_CURSOR 2
#define SYNT_RESIZE_V_CURSOR 3
#define SYNT_RESIZE_NW_CURSOR 4
#define SYNT_MOVE_CURSOR 5
#define SYNT_HIDDEN_CURSOR 6

#define TOTAL_CURSORS 7

typedef struct Linux_Platform_Internal
{
    xcb_connection_t* connection;
    xcb_screen_t* screen;
    xcb_window_t window;

    Callbacks callback_handler;

    u16 current_cursor;
    u16 width;
    u16 height;
    u16 caps_on;
    u16 shift_down;

    xcb_cursor_t cursors[TOTAL_CURSORS];

    b8 mouse_hidden;

} Linux_Platform_Internal;

global i16 POS_X_LINUXPLATFORM = 0;
global i16 POS_Y_LINUXPLATFORM = 0;
global i16 SAVED_X_LINUXPLATFORM = 0;
global i16 SAVED_Y_LINUXPLATFORM = 0;

Mutex mutex_create()
{
    Mutex mutex;
    pthread_mutex_init(&mutex, NULL);
    return mutex;
}

void mutex_lock(Mutex* mutex)
{
    pthread_mutex_lock(mutex);
}

void mutex_unlock(Mutex* mutex)
{
    pthread_mutex_unlock(mutex);
}

void mutex_destroy(Mutex* mutex)
{
    pthread_mutex_destroy(mutex);
}

Semaphore semaphore_create(i32 initial_count, i32 max_count)
{
    Semaphore sem;
    sem_init(&sem, 0, initial_count);
    return sem;
}

void semaphore_wait(Semaphore* sem)
{
    sem_wait(sem);
}

void semaphore_release(Semaphore* sem)
{
    sem_post(sem);
}

void semaphore_destroy(Semaphore* sem)
{
    sem_destroy(sem);
}

Thread_Handle thread_create(void* data,
                            thread_return_value (*thread_function)(void* data),
                            unsigned long creation_flag, unsigned long* thread_id)
{
    Thread_Handle thread;
    pthread_create(&thread, NULL, thread_function, data);
    return thread;
}

void thread_join(Thread_Handle handle)
{
    pthread_join(handle, NULL);
}

void thread_destroy(Thread_Handle handle)
{
    pthread_cancel(handle);
}

void platform_title_change(Platform* platform, const char* title, u32 len)
{
    Linux_Platform_Internal* platform_internal = (Linux_Platform_Internal*)platform;
    xcb_change_property(platform_internal->connection, XCB_PROP_MODE_REPLACE,
                        platform_internal->window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING,
                        8, len, title);
    xcb_flush(platform_internal->connection);
}

xcb_connection_t* platform_connection_get(Platform* platform)
{
    Linux_Platform_Internal* platform_internal = (Linux_Platform_Internal*)platform;
    return platform_internal->connection;
}

xcb_window_t platform_window_get(Platform* platform)
{
    Linux_Platform_Internal* platform_internal = (Linux_Platform_Internal*)platform;
    return platform_internal->window;
}

void platform_init(Region_Alloc* region, const char* title, u16* width, u16* height,
                   b32 full_screen, Platform** platform)
{
    Linux_Platform_Internal* platform_internal =
        region_calloc(region, 1, Linux_Platform_Internal);

    platform_internal->connection = xcb_connect(NULL, NULL);

    platform_internal->screen =
        xcb_setup_roots_iterator(xcb_get_setup(platform_internal->connection)).data;

    platform_internal->window = xcb_generate_id(platform_internal->connection);

    xcb_cursor_context_t* ctx;
    xcb_cursor_context_new(platform_internal->connection, platform_internal->screen,
                           &ctx);

    platform_internal->cursors[SYNT_NORMAL_CURSOR] =
        xcb_cursor_load_cursor(ctx, "default");
    platform_internal->cursors[SYNT_HAND_CURSOR] =
        xcb_cursor_load_cursor(ctx, "pointing_hand");
    platform_internal->cursors[SYNT_RESIZE_H_CURSOR] =
        xcb_cursor_load_cursor(ctx, "col-resize");
    platform_internal->cursors[SYNT_MOVE_CURSOR] =
        xcb_cursor_load_cursor(ctx, "move");
    xcb_cursor_context_free(ctx);

    u32 mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    u32 values[] = {
        platform_internal->screen->black_pixel,

        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
            XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_ENTER_WINDOW |
            XCB_EVENT_MASK_LEAVE_WINDOW | XCB_EVENT_MASK_KEY_PRESS |
            XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_FOCUS_CHANGE,
    };

    xcb_create_window(platform_internal->connection, XCB_COPY_FROM_PARENT,
                      platform_internal->window, platform_internal->screen->root, 0,
                      0, *width, *height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      platform_internal->screen->root_visual, mask, values);

    xcb_map_window(platform_internal->connection, platform_internal->window);

    xcb_flush(platform_internal->connection);

    platform_title_change(platform_internal, title, strlen(title));

    platform_internal->width = *width;
    platform_internal->height = *height;
    *platform = (Platform*)platform_internal;
}

void platform_event_set_callbacks(
    Platform* platform, void (*on_key_pressed)(u16 key, u16 op),
    void (*on_key_released)(u16 key), void (*on_button_pressed)(u8 key),
    void (*on_button_released)(u8 key), void (*on_mouse_move)(i16 pos_x, i16 pos_y),
    void (*on_mouse_wheel)(i16 z_delta), void (*on_window_focused)(b8 focused),
    void (*on_enter_leave)(b8 e_l), void (*on_window_resize)(u16 width, u16 height))
{
    Linux_Platform_Internal* platform_internal = (Linux_Platform_Internal*)platform;
    platform_internal->callback_handler.on_key_pressed = on_key_pressed;
    platform_internal->callback_handler.on_key_released = on_key_released;
    platform_internal->callback_handler.on_button_pressed = on_button_pressed;
    platform_internal->callback_handler.on_button_released = on_button_released;
    platform_internal->callback_handler.on_mouse_move = on_mouse_move;
    platform_internal->callback_handler.on_mouse_wheel = on_mouse_wheel;
    platform_internal->callback_handler.on_window_focused = on_window_focused;
    platform_internal->callback_handler.on_enter_leave = on_enter_leave;
    platform_internal->callback_handler.on_window_resize = on_window_resize;
}

void event_fire(Platform* platform)
{
    Linux_Platform_Internal* platform_internal = (Linux_Platform_Internal*)platform;

    u8 key = 0;
    xcb_generic_event_t* event;
    while ((event = xcb_poll_for_event(platform_internal->connection)))
    {
        switch (event->response_type & ~0x80)
        {
            case XCB_KEY_PRESS:
            {
                xcb_key_press_event_t* pressEvt = (xcb_key_press_event_t*)event;

                // TODO: how to find if caps lock is on
                platform_internal->caps_on = 0;
                key = pressEvt->detail;
                if (key == SYNT_KEY_SHIFT)
                {
                    platform_internal->shift_down = 1;
                }
                if (platform_internal->shift_down)
                {
                    platform_internal->caps_on =
                        platform_internal->caps_on >= 1 ? 0 : 1;
                }
                platform_internal->callback_handler.on_key_pressed(
                    key, platform_internal->caps_on);

                break;
            }
            case XCB_KEY_RELEASE:
            {
                xcb_key_release_event_t* releaseEvt =
                    (xcb_key_release_event_t*)event;

                if (key == SYNT_KEY_SHIFT)
                {
                    platform_internal->shift_down = 0;
                }
                if (!platform_internal->shift_down)
                {
                    platform_internal->caps_on =
                        platform_internal->caps_on >= 1 ? 0 : 1;
                }
                key = releaseEvt->detail;
                platform_internal->callback_handler.on_key_released(key);

                break;
            }
            case XCB_BUTTON_PRESS:
            {
                xcb_button_press_event_t* button_pressed =
                    (xcb_button_press_event_t*)event;

                u8 button = button_pressed->detail;

                platform_internal->callback_handler.on_button_pressed(button);

                break;
            }
            case XCB_BUTTON_RELEASE:
            {
                xcb_button_release_event_t* button_pressed =
                    (xcb_button_release_event_t*)event;

                u8 button = button_pressed->detail;

                platform_internal->callback_handler.on_button_released(button);

                break;
            }
            case XCB_MOTION_NOTIFY:
            {
                xcb_motion_notify_event_t* mouse_moved =
                    (xcb_motion_notify_event_t*)event;

                POS_X_LINUXPLATFORM = mouse_moved->event_x;
                POS_Y_LINUXPLATFORM = mouse_moved->event_y;

                platform_internal->callback_handler.on_mouse_move(
                    POS_X_LINUXPLATFORM, POS_Y_LINUXPLATFORM);

                break;
            }
            case XCB_FOCUS_IN:
            {
                platform_internal->callback_handler.on_window_focused(1);
                break;
            }
            case XCB_FOCUS_OUT:
            {
                platform_internal->callback_handler.on_window_focused(0);
                break;
            }
            case XCB_ENTER_NOTIFY:
            {
                platform_internal->callback_handler.on_enter_leave(1);
                break;
            }
            case XCB_LEAVE_NOTIFY:
            {
                platform_internal->callback_handler.on_enter_leave(0);
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

void move_main_window(Platform* platform)
{
    Linux_Platform_Internal* platform_internal = (Linux_Platform_Internal*)platform;

    xcb_query_pointer_reply_t* reply;
    xcb_query_pointer_cookie_t cookie =
        xcb_query_pointer(platform_internal->connection, platform_internal->window);

    if ((reply =
             xcb_query_pointer_reply(platform_internal->connection, cookie, NULL)))
    {
        i16 values[] = { reply->win_x, reply->win_y };

        xcb_configure_window(platform_internal->connection,
                             platform_internal->window,
                             XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);

        printf("x: %d, y: %d\n", values[0], values[1]);
    }
    free(reply);
}

void platform_window_get_size(const Platform* platform, u16* width, u16* height)
{
    Linux_Platform_Internal* platform_internal = (Linux_Platform_Internal*)platform;
    *width = platform_internal->width;
    *height = platform_internal->height;
}

void platform_cursor_hide(const Platform* platform)
{
    Linux_Platform_Internal* platform_internal = (Linux_Platform_Internal*)platform;
    if (!platform_internal->mouse_hidden)
    {
        SAVED_X_LINUXPLATFORM = POS_X_LINUXPLATFORM;
        SAVED_Y_LINUXPLATFORM = POS_Y_LINUXPLATFORM;

        xcb_xfixes_query_version(platform_internal->connection, 4, 0);
        xcb_xfixes_hide_cursor(platform_internal->connection,
                               platform_internal->screen->root);
        xcb_flush(platform_internal->connection);
    }
    platform_internal->mouse_hidden = true;
}

void platform_cursor_show(const Platform* platform)
{
    Linux_Platform_Internal* platform_internal = (Linux_Platform_Internal*)platform;
    if (platform_internal->mouse_hidden)
    {
        xcb_xfixes_query_version(platform_internal->connection, 4, 0);
        xcb_xfixes_show_cursor(platform_internal->connection,
                               platform_internal->screen->root);
        xcb_flush(platform_internal->connection);
    }
    platform_internal->mouse_hidden = false;
}

void platform_mouse_set_pos(const Platform* platform, i16 pos_x, i16 pos_y)
{
    Linux_Platform_Internal* platform_internal = (Linux_Platform_Internal*)platform;
    xcb_warp_pointer(platform_internal->connection, platform_internal->window,
                     platform_internal->window, 0, 0, platform_internal->width,
                     platform_internal->height, pos_x, pos_y);
    xcb_flush(platform_internal->connection);
    POS_X_LINUXPLATFORM = pos_x;
    POS_Y_LINUXPLATFORM = pos_y;
}

void platform_cursor_show_centered(const Platform* platform)
{
    Linux_Platform_Internal* platform_internal = (Linux_Platform_Internal*)platform;
    if (platform_internal->mouse_hidden)
    {
        platform_mouse_set_pos(platform, platform_internal->width / 2,
                               platform_internal->height / 2);
    }
    platform_cursor_show(platform);
    platform_internal->mouse_hidden = false;
}

void platform_mouse_set_last_pos(const Platform* platform)
{
    Linux_Platform_Internal* platform_internal = (Linux_Platform_Internal*)platform;
    xcb_warp_pointer(platform_internal->connection, platform_internal->window,
                     platform_internal->window, 0, 0, platform_internal->width,
                     platform_internal->height, SAVED_X_LINUXPLATFORM,
                     SAVED_Y_LINUXPLATFORM);
    xcb_flush(platform_internal->connection);
    POS_X_LINUXPLATFORM = SAVED_X_LINUXPLATFORM;
    POS_Y_LINUXPLATFORM = SAVED_Y_LINUXPLATFORM;
}

void platform_cursor_show_last_pos(const Platform* platform)
{
    Linux_Platform_Internal* platform_internal = (Linux_Platform_Internal*)platform;
    if (platform_internal->mouse_hidden)
    {
        platform_mouse_set_last_pos(platform);
    }
    platform_cursor_show(platform);
    platform_internal->mouse_hidden = false;
}

void platform_cursor_change(const Platform* platform, u32 cursor_id)
{
    Linux_Platform_Internal* platform_internal = (Linux_Platform_Internal*)platform;
    if (platform_internal->current_cursor != cursor_id)
    {
        if (cursor_id < TOTAL_CURSORS)
        {
            platform_internal->current_cursor = cursor_id;
        }
        else
        {
            sy_print("WARNING: trying to change to a cursor that doesn't exist.\n");
        }
    }
}

void platform_mouse_get_pos(i16* pos_x, i16* pos_y)
{
    *pos_x = POS_X_LINUXPLATFORM;
    *pos_y = POS_Y_LINUXPLATFORM;
}

double platform_get_time()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec + (now.tv_nsec * 0.000000001);
}

void platform_sleep(u64 milli)
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

void platform_shut_down(Platform* platform)
{
    Linux_Platform_Internal* platform_internal = (Linux_Platform_Internal*)platform;

    xcb_disconnect(platform_internal->connection);
}

void file_read(File_Attrib* file_attrib, Region_Alloc* region, const char* file_path,
               const char* operation)
{
    FILE* file = fopen(file_path, operation);

    if (file == NULL) SY_ERROR(file_path);

    fseek(file, 0, SEEK_END);
    file_attrib->size = (u32)ftell(file);
    rewind(file);

    if (region)
    {
        file_attrib->buffer = region_malloc(region, file_attrib->size, u8);
    }
    else
    {
        file_attrib->buffer = (u8*)malloc(file_attrib->size);
    }

    if (fread(file_attrib->buffer, 1, file_attrib->size, file) != file_attrib->size)
    {
        SY_ERROR(file_path);
    }
    fclose(file);
}

void file_write(const char* file_path, const char* content)
{
    FILE* file = fopen(file_path, "w");

    if (file == NULL) SY_ERROR(file_path);

    fseek(file, 0, SEEK_END);
    fwrite(content, 1, strlen(content), file);
    fclose(file);
}

void file_write_entire(const char* file_path, const char* content, u32 size)
{
    FILE* file = fopen(file_path, "w");

    if (file == NULL) SY_ERROR(file_path);

    fwrite(content, 1, (size_t)size, file);
    fclose(file);
}

u32 executable_directory(char* file, u32 size)
{
    u32 len = (u32)readlink("/proc/self/exe", file, size - 1);
    assert(len != -1);
    file[len] = '\0';
    return len;
}

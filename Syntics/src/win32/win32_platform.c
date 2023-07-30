
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

typedef void Win32_Platform;

typedef struct Win32_Platform_Internal
{
    WNDCLASS window_class;
    HWND win;
    HINSTANCE instance;

    Callbacks callback_handler;

    u16 width;
    u16 height;
    u16 caps_on;
    u16 shift_down;

    HCURSOR cursors[TOTAL_CURSORS];

    b8 initialized;
} Win32_Platform_Internal;

global i16 POS_X_WIN32PLATFORM = 0;
global i16 POS_Y_WIN32PLATFORM = 0;
global i16 SAVED_X_WIN32PLATFORM = 0;
global i16 SAVED_Y_WIN32PLATFORM = 0;
global u16 current_cursor = SYNT_NORMAL_CURSOR;

void* thread_create(void* data, unsigned long (*thread_function)(void* data),
                    unsigned long creation_flag, unsigned long* thread_id)
{
    return CreateThread(0, 0, thread_function, data, creation_flag, thread_id);
}

void close_handle(void* handle)
{
    CloseHandle(handle);
}

void error_msg(const char* msg)
{
    MessageBoxA(NULL, msg, "Error", MB_OK);
}

HWND platform_window_get(Win32_Platform* platform)
{
    assert(platform);
    return ((Win32_Platform_Internal*)platform)->win;
}

#if 0
    char temp[10] = {};
    sprintf(temp, "%u\n", key);
    OutputDebugString(temp);
#endif

void get_rect(long* left, long* top, long* right, long* bottom);

LRESULT msg_handler(HWND win, UINT msg, WPARAM w_param, LPARAM l_param)
{
    Win32_Platform_Internal* platform =
        (Win32_Platform_Internal*)GetWindowLongPtrA(win, GWLP_USERDATA);

    LRESULT res = 0;
    switch (msg)
    {
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        {

            if (platform)
            {
                u8 button = (u8)w_param;
                platform->callback_handler.on_button_pressed(button);
            }
            break;
        }
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        {
            if (platform)
            {
                u8 button = (u8)w_param;
                platform->callback_handler.on_button_released(button);
            }
            break;
        }
        case WM_MOUSEMOVE:
        {
            POS_X_WIN32PLATFORM = LOWORD(l_param);
            POS_Y_WIN32PLATFORM = HIWORD(l_param);
            if (platform)
            {
                platform->callback_handler.on_mouse_move(POS_X_WIN32PLATFORM,
                                                         POS_Y_WIN32PLATFORM);
            }
            break;
        }
        case WM_MOUSEWHEEL:
        {
            if (platform)
            {
                i16 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);

                platform->callback_handler.on_mouse_wheel(z_delta);
            }
            break;
        }
        case WM_SIZE:
        {
            if (platform)
            {
                platform->width = LOWORD(l_param);
                platform->height = HIWORD(l_param);
                platform->callback_handler.on_window_resize(platform->width,
                                                            platform->height);
            }
            break;
        }
        // TODO: mouse leave and enter and focus;
        case WM_MOVE:
        {
            break;
        }
        case WM_SETCURSOR:
        {
            if (platform)
            {
                SetCursor(platform->cursors[current_cursor]);
            }
            break;
        }
        case WM_DESTROY:
        {
            break;
        }
        case WM_QUIT:
        {
            break;
        }
        case WM_ACTIVATEAPP:
        {
            break;
        }
        default:
        {
            res = DefWindowProc(win, msg, w_param, l_param);
            return res;
        }
    }
    if (!platform)
    {
        res = DefWindowProc(win, msg, w_param, l_param);
    }
    return res;
}

// From Raymond Chen
// Source: https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
//
WINDOWPLACEMENT WIN32PLATFORM_window_placement = { sizeof(
    WIN32PLATFORM_window_placement) };

global b8 fullscreen2_WIN32PLATFORM = false;
global b8 maximize_WIN32PLATFORM = false;
global b8 WIN32PLATFORM_fullscreen = false;
global u16 WIN32PLATFORM_WIDTH = 0;
global u16 WIN32PLATFORM_HEIGHT = 0;
static void sy_fullscreen(HWND window)
{
    DWORD window_style = GetWindowLong(window, GWL_STYLE);
    if (!WIN32PLATFORM_fullscreen)
    {
        MONITORINFO monitor_info = { sizeof(monitor_info) };
        if (GetWindowPlacement(window, &WIN32PLATFORM_window_placement) &&
            GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY),
                           &monitor_info))
        {
            SetWindowLong(window, GWL_STYLE, window_style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(window, HWND_TOP, monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.top,
                         monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

            WIN32PLATFORM_WIDTH = (u16)monitor_info.rcMonitor.right;
            WIN32PLATFORM_HEIGHT = (u16)monitor_info.rcMonitor.bottom;
        }
        WIN32PLATFORM_fullscreen = true;
        fullscreen2_WIN32PLATFORM = true;
    }
    else
    {
        SetWindowLong(window, GWL_STYLE, window_style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(window, &WIN32PLATFORM_window_placement);
        SetWindowPos(window, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER |
                         SWP_FRAMECHANGED);
        WIN32PLATFORM_fullscreen = false;
        fullscreen2_WIN32PLATFORM = false;
        maximize_WIN32PLATFORM = false;
    }
}

void platform_init(Region_Alloc* region, const char* title, u16* width, u16* height,
                   b32 full_screen, Win32_Platform** platform)
{
    assert(!(*platform));
    Win32_Platform_Internal* platform_internal =
        region_calloc(region, 1, Win32_Platform_Internal);

    platform_internal->cursors[SYNT_NORMAL_CURSOR] =
        LoadCursor(platform_internal->instance, IDC_ARROW);
    platform_internal->cursors[SYNT_HAND_CURSOR] =
        LoadCursor(platform_internal->instance, IDC_HAND);
    platform_internal->cursors[SYNT_RESIZE_H_CURSOR] =
        LoadCursor(platform_internal->instance, IDC_SIZEWE);
    platform_internal->cursors[SYNT_RESIZE_V_CURSOR] =
        LoadCursor(platform_internal->instance, IDC_SIZENS);
    platform_internal->cursors[SYNT_RESIZE_NW_CURSOR] =
        LoadCursor(platform_internal->instance, IDC_SIZENWSE);
    platform_internal->cursors[SYNT_MOVE_CURSOR] =
        LoadCursor(platform_internal->instance, IDC_SIZEALL);
    platform_internal->cursors[SYNT_HIDDEN_CURSOR] = NULL;

    platform_internal->window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    platform_internal->window_class.lpfnWndProc = msg_handler;
    platform_internal->window_class.hInstance = GetModuleHandle(0);
    platform_internal->window_class.lpszClassName = "Syn_win_c";
    platform_internal->window_class.hCursor =
        platform_internal->cursors[SYNT_NORMAL_CURSOR];

    platform_internal->instance = platform_internal->window_class.hInstance;

    ATOM res = RegisterClass(&platform_internal->window_class);
    if (!res)
    {
        SY_ERROR("RegisterClass");
    }

    platform_internal->win =
        CreateWindowEx(0, platform_internal->window_class.lpszClassName, title,
                       WS_OVERLAPPEDWINDOW | WS_VISIBLE, 10, 10, *width, *height, 0,
                       0, platform_internal->window_class.hInstance, 0);

#if 0
    // Windows is nuts, probaly should just use popupwindow
    DWORD window_style = GetWindowLong(platform_internal->win, GWL_STYLE);
    SetWindowLong(platform_internal->win, GWL_STYLE, (window_style & ~WS_OVERLAPPEDWINDOW));
    SetWindowPos(platform_internal->win, HWND_TOP, 10, 10, width, height, SWP_FRAMECHANGED);
#endif

    platform_internal->width = *width;
    platform_internal->height = *height;

    if (platform_internal->win == NULL)
    {
        SY_ERROR("CreateWindowEx");
    }

    if (full_screen)
    {
        sy_fullscreen(platform_internal->win);

        platform_internal->width = WIN32PLATFORM_WIDTH;
        platform_internal->height = WIN32PLATFORM_HEIGHT;
        *width = WIN32PLATFORM_WIDTH;
        *height = WIN32PLATFORM_HEIGHT;
    }

    SetWindowLongPtrA(platform_internal->win, GWLP_USERDATA,
                      (LONG_PTR)platform_internal);

    *platform = (Win32_Platform*)platform_internal;
}

void platform_event_set_callbacks(
    Win32_Platform* platform, void (*on_key_pressed)(u16 key, u16 op),
    void (*on_key_released)(u16 key), void (*on_button_pressed)(u8 key),
    void (*on_button_released)(u8 key), void (*on_mouse_move)(i16 pos_x, i16 pos_y),
    void (*on_mouse_wheel)(i16 z_delta), void (*on_window_focused)(b8 focused),
    void (*on_enter_leave)(b8 e_l), void (*on_window_resize)(u16 width, u16 height))
{
    Win32_Platform_Internal* platform_internal = (Win32_Platform_Internal*)platform;
    assert(platform_internal);

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

b8 is_fullscreen(void)
{
    return fullscreen2_WIN32PLATFORM;
}

b8 is_maximized(void)
{
    return maximize_WIN32PLATFORM;
}

void sy_toggle_fullscreen(HWND win)
{
    sy_fullscreen(win);
}

void sy_toggle_maximize(HWND win)
{
    maximize_WIN32PLATFORM = true;
    sy_fullscreen(win);
    fullscreen2_WIN32PLATFORM = false;
}

void window_move(HWND win, i32 x, i32 y, i32 w, i32 h)
{
    SetWindowPos(win, NULL, x, y, w, h, SWP_FRAMECHANGED);
}

#define SYNT_KEY_CAPS 20
#define SYNT_KEY_SHIFT 16

void event_fire(void)
{
    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        Win32_Platform_Internal* platform =
            (Win32_Platform_Internal*)GetWindowLongPtrA(msg.hwnd, GWLP_USERDATA);

        if (!platform)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {

            switch (msg.message)
            {
                case WM_SYSKEYDOWN:
                case WM_KEYDOWN:
                {
                    u16 key = (u16)msg.wParam;
                    platform->caps_on = (GetKeyState(VK_CAPITAL)) & 0xFF;

                    b32 was_alt_down = (msg.lParam & (1 << 29));
                    if (was_alt_down && key == VK_RETURN)
                    {
                        sy_fullscreen(msg.hwnd);
                    }
                    // TODO: FIX this mess
                    if (key == SYNT_KEY_SHIFT)
                    {
                        platform->shift_down = 1;
                    }
                    if (platform->shift_down)
                    {
                        platform->caps_on = platform->caps_on >= 1 ? 0 : 1;
                    }
                    platform->callback_handler.on_key_pressed(key,
                                                              platform->caps_on);
                    break;
                }
                case WM_SYSKEYUP:
                case WM_KEYUP:
                {
                    u16 key = (u16)msg.wParam;
                    if (key == SYNT_KEY_SHIFT)
                    {
                        platform->shift_down = 0;
                    }
                    if (!platform->shift_down)
                    {
                        platform->caps_on = platform->caps_on >= 1 ? 0 : 1;
                    }
                    platform->callback_handler.on_key_released(key);
                    break;
                }
                default:
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                    break;
                }
            }
        }
    }
}

void platform_window_get_size(const Win32_Platform* platform, u16* width,
                              u16* height)
{
    const Win32_Platform_Internal* wpi = (const Win32_Platform_Internal*)platform;
    *width = wpi->width;
    *height = wpi->height;
}

void screen_get_pos(i32* x, i32* y)
{
    POINT point;
    GetCursorPos(&point);
    *x = (i32)point.x;
    *y = (i32)point.y;
}

static void platform_cursor_set_pos(const Win32_Platform* platform, i16 x, i16 y)
{
    const Win32_Platform_Internal* wpi = (const Win32_Platform_Internal*)platform;

    POINT point;
    point.x = x;
    point.y = y;
    ClientToScreen(wpi->win, &point);
    SetCursorPos(point.x, point.y);
    SetCursor(wpi->cursors[current_cursor]);
}

static b8 MOUSE_HIDDEN = false;
void platform_cursor_hide(const Win32_Platform* platform)
{
    const Win32_Platform_Internal* wpi = (const Win32_Platform_Internal*)platform;

    if (!MOUSE_HIDDEN)
    {
        current_cursor = SYNT_HIDDEN_CURSOR;
        SetCursor(wpi->cursors[current_cursor]);
        SAVED_X_WIN32PLATFORM = POS_X_WIN32PLATFORM;
        SAVED_Y_WIN32PLATFORM = POS_Y_WIN32PLATFORM;
    }
    MOUSE_HIDDEN = true;
}

void platform_cursor_show(const Win32_Platform* platform)
{
    const Win32_Platform_Internal* wpi = (const Win32_Platform_Internal*)platform;

    if (MOUSE_HIDDEN)
    {
        current_cursor = SYNT_NORMAL_CURSOR;
        SetCursor(wpi->cursors[current_cursor]);
    }
    MOUSE_HIDDEN = false;
}

void platform_mouse_set_pos(const Win32_Platform* platform, i16 pos_x, i16 pos_y)
{
    POS_X_WIN32PLATFORM = pos_x;
    POS_Y_WIN32PLATFORM = pos_y;
    platform_cursor_set_pos(platform, pos_x, pos_y);
}

void platform_cursor_show_centered(const Win32_Platform* platform)
{
    const Win32_Platform_Internal* wpi = (const Win32_Platform_Internal*)platform;

    if (MOUSE_HIDDEN)
    {
        platform_mouse_set_pos(platform, wpi->width / 2, wpi->height / 2);
    }
    platform_cursor_show(platform);
    MOUSE_HIDDEN = false;
}

void platform_mouse_set_last_pos(const Win32_Platform* platform)
{
    POS_X_WIN32PLATFORM = SAVED_X_WIN32PLATFORM;
    POS_Y_WIN32PLATFORM = SAVED_Y_WIN32PLATFORM;
    platform_cursor_set_pos(platform, POS_X_WIN32PLATFORM, POS_Y_WIN32PLATFORM);
}

void platform_cursor_show_last_pos(const Win32_Platform* platform)
{
    if (MOUSE_HIDDEN)
    {
        platform_mouse_set_last_pos(platform);
    }
    platform_cursor_show(platform);
    MOUSE_HIDDEN = false;
}

void platform_cursor_change(const Win32_Platform* platform, u32 cursor_id)
{
    const Win32_Platform_Internal* wpi = (const Win32_Platform_Internal*)platform;

    if (current_cursor != cursor_id && !MOUSE_HIDDEN)
    {
        if (cursor_id < TOTAL_CURSORS)
        {
            current_cursor = (u16)cursor_id;
            SetCursor(wpi->cursors[current_cursor]);
        }
        else
        {
            sy_print("WARNING: trying to change to a cursor that doesn't exist.");
        }
    }
}

void platform_mouse_get_pos(i16* pos_x, i16* pos_y)
{
    *pos_x = POS_X_WIN32PLATFORM;
    *pos_y = POS_Y_WIN32PLATFORM;
}

double platform_get_time(void)
{
#if 0
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER time;
    time.LowPart = ft.dwLowDateTime;
    time.HighPart = ft.dwHighDateTime;
    return time.QuadPart * 0.0000001;
#else
    struct timespec now;
    timespec_get(&now, TIME_UTC);
    return now.tv_sec + (now.tv_nsec * 0.000000001);
#endif
}

void platform_sleep(u64 milli)
{
    Sleep((DWORD)milli);
}

void platform_shut_down(Win32_Platform* platform)
{
    Win32_Platform_Internal* wpi = (Win32_Platform_Internal*)platform;

    if (WIN32PLATFORM_fullscreen)
    {
        sy_fullscreen(wpi->win);
    }
    DestroyWindow(wpi->win);
}

HANDLE file_get_handle(LPCSTR file_path, DWORD operation, DWORD share_mode,
                       DWORD creation)
{
    HANDLE file = CreateFile(file_path, operation, share_mode, 0, creation, 0, 0);
    assert(file != INVALID_HANDLE_VALUE);
    return file;
}

u32 file_get_size(HANDLE file)
{
#if 0
    LARGE_INTEGER file_size;
    assert(GetFileSizeEx(file, &file_size));
    return (u32)file_size.QuadPart;
#else
    u32 size = GetFileSize(file, NULL);
    return size;
#endif
}

void file_read_bytes(File_Attrib* file_attrib, HANDLE file)
{
    DWORD bytes_read;
    assert(ReadFile(file, file_attrib->buffer, file_attrib->size, &bytes_read, 0) &&
           file_attrib->size == bytes_read);

    CloseHandle(file);
}

void file_read(File_Attrib* file_attrib, Region_Alloc* region, const char* file_path,
               const char* operation)
{
    HANDLE file =
        file_get_handle(file_path, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);

    file_attrib->size = file_get_size(file);

    if (region)
    {
        file_attrib->buffer =
            region_malloc(region, file_attrib->size, unsigned char);
    }
    else
    {
        file_attrib->buffer = (unsigned char*)malloc(file_attrib->size);
    }
    file_attrib->current_pos = 0;
    file_read_bytes(file_attrib, file);
}

void file_write(const char* file_path, const char* content)
{
    HANDLE file =
        file_get_handle(file_path, FILE_GENERIC_WRITE, FILE_SHARE_READ, OPEN_ALWAYS);

    SetFilePointer(file, 0, NULL, FILE_END);

    DWORD bytes_written = 0;
    WriteFile(file, content, (DWORD)strlen(content), &bytes_written, 0);
    CloseHandle(file);
}

void file_write_entire(const char* file_path, const char* content, u32 size)
{
    HANDLE file =
        file_get_handle(file_path, GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS);

    DWORD bytes_written = 0;
    WriteFile(file, content, (DWORD)size, &bytes_written, 0);
    CloseHandle(file);
}

#include "win32_platform.h"
#include "logging.h"
#include "ansi_keycodes.h"
#include <time.h>
#include <tchar.h>
#include <Windows.h>

typedef struct Callbacks
{
    void (*on_key_pressed)(u16 key, u16 op);
    void (*on_key_released)(u16 key, u16 op);
    void (*on_button_pressed)(u8 key, u16 op);
    void (*on_button_released)(u8 key, u16 op);
    void (*on_mouse_move)(i16 pos_x, i16 pos_y, u16 op);
    void (*on_mouse_wheel)(i16 z_delta);
    void (*on_window_focused)(b8 focused, u16 op);
    void (*on_enter_leave)(b8 e_l, u16 op);
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

typedef struct Win32_Platform
{
    WNDCLASS window_class;
    HWND win;
    HINSTANCE instance;

    u16 width;
    u16 height;

    HCURSOR cursors[TOTAL_CURSORS];
} Win32_Platform;

static Callbacks callback_handler;
static Win32_Platform platform;

static b8 INITIALIZED = 0;

static i16 POS_X = 0;
static i16 POS_Y = 0;

static i16 SAVED_X = 0;
static i16 SAVED_Y = 0;

static u16 current_cursor = SYNT_NORMAL_CURSOR;

void error_msg(const char* msg)
{
    MessageBoxA(NULL, msg, "Error", MB_OK);
}

HWND get_win()
{
    return platform.win;
}

#if 0
    char temp[10] = {};
    sprintf(temp, "%u\n", key);
    OutputDebugString(temp);
#endif

void get_rect(long* left, long* top, long* right, long* bottom);

#define SYNT_KEY_CAPS 20
#define SYNT_KEY_SHIFT 16
static u16 _CAPS_ON = 0;
static u16 shift_down = 0;

LRESULT msg_handler(HWND win, UINT msg, WPARAM w_param, LPARAM l_param)
{
    LRESULT res = 0;
    switch (msg)
    {
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        {
            u8 button = (u8)w_param;
            callback_handler.on_button_pressed(button, 0);
            break;
        }
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        {
            u8 button = (u8)w_param;
            callback_handler.on_button_released(button, 0);
            break;
        }
        case WM_MOUSEMOVE:
        {
            POS_X = LOWORD(l_param);
            POS_Y = HIWORD(l_param);
            callback_handler.on_mouse_move(POS_X, POS_Y, 0);
            break;
        }
        case WM_MOUSEWHEEL:
        {
            i16 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
            callback_handler.on_mouse_wheel(z_delta);
            break;
        }
        case WM_SIZE:
        {
            platform.width = LOWORD(l_param);
            platform.height = HIWORD(l_param);
            callback_handler.on_window_resize(platform.width, platform.height);
            break;
        }
        // TODO: mouse leave and enter and focus;
        case WM_MOVE:
        {
            break;
        }
        case WM_SETCURSOR:
        {
            SetCursor(platform.cursors[current_cursor]);
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
#if 0 // Windows api is absolute garbage, (snapping)
        case WM_NCHITTEST:
        {
            synt_LOG_Term("Hellp\n");
            POINT point = { LOWORD(l_param), HIWORD(l_param) };
            // Map the point to client coordinates.
            MapWindowPoints(nullptr, win, &point, 1);
            // If the point is in your maximize button then return HTMAXBUTTON
            RECT r;
            r.left;
            get_rect(&r.left, &r.top, &r.right, &r.bottom);
            if (PtInRect(&r, point))
            {
                return HTMAXBUTTON;
            }
            else
            {
                res = DefWindowProc(win, msg, w_param, l_param);
            }
            break;
        }
#endif
        default:
        {
            res = DefWindowProc(win, msg, w_param, l_param);
            break;
        }
    }
    return res;
}

void init_platform(const char* title, b8 fullscreen, u16 width, u16 height)
{
    if (INITIALIZED)
    {
        SY_ERROR("platform already initialized");
    }

    platform.cursors[SYNT_NORMAL_CURSOR] = LoadCursor(platform.instance, IDC_ARROW);
    platform.cursors[SYNT_HAND_CURSOR] = LoadCursor(platform.instance, IDC_HAND);
    platform.cursors[SYNT_RESIZE_H_CURSOR] =
        LoadCursor(platform.instance, IDC_SIZEWE);
    platform.cursors[SYNT_RESIZE_V_CURSOR] =
        LoadCursor(platform.instance, IDC_SIZENS);
    platform.cursors[SYNT_RESIZE_NW_CURSOR] =
        LoadCursor(platform.instance, IDC_SIZENWSE);
    platform.cursors[SYNT_MOVE_CURSOR] = LoadCursor(platform.instance, IDC_SIZEALL);
    platform.cursors[SYNT_HIDDEN_CURSOR] = NULL;

    platform.window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    platform.window_class.lpfnWndProc = msg_handler;
    platform.window_class.hInstance = GetModuleHandle(0);
    platform.window_class.lpszClassName = "Syn_win_c";
    platform.window_class.hCursor = platform.cursors[SYNT_NORMAL_CURSOR];

    platform.instance = platform.window_class.hInstance;

    ATOM res = RegisterClass(&platform.window_class);
    if (!res)
    {
        SY_ERROR("RegisterClass");
    }

    platform.win = CreateWindowEx(0, platform.window_class.lpszClassName, title,
                                  WS_OVERLAPPEDWINDOW | WS_VISIBLE, 10, 10, width,
                                  height, 0, 0, platform.window_class.hInstance, 0);

#if 0
    // Windows is nuts, probaly should just use popupwindow
    DWORD window_style = GetWindowLong(platform.win, GWL_STYLE);
    SetWindowLong(platform.win, GWL_STYLE, (window_style & ~WS_OVERLAPPEDWINDOW));
    SetWindowPos(platform.win, HWND_TOP, 10, 10, width, height, SWP_FRAMECHANGED);
#endif

    platform.width = width;
    platform.height = height;

    if (platform.win == NULL)
    {
        SY_ERROR("CreateWindowEx");
    }

#if 0 // Tried to get system temp but like everything in windows, too fucking hard.
    HKEY hKey;
    LONG lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                             "Hardware\\Description\\System\\CentralProcessor\\0", 0,
                             KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS)
    {
        SY_ERROR("RegOpenKeyEx");
    }

    DWORD dwSize = 0;
    lRes = RegQueryValueEx(hKey, "ThermalPerformance", NULL, NULL, NULL, &dwSize);
    if (lRes != ERROR_SUCCESS)
    {
        LPVOID lpMsgBuf;
        DWORD dwRet = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, lRes, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf,
            0, NULL);
        RegCloseKey(hKey);

        SY_ERROR((const char*)lpMsgBuf);
    }

    DWORD dwTemperature = 0;
    lRes = RegQueryValueEx(hKey, "Temperature", NULL, NULL, (LPBYTE)&dwTemperature,
                           &dwSize);
    if (lRes != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        SY_ERROR("RegQueryValueEx2");
    }

    synt_LOG_Term("Temperature: %d\n", dwTemperature);
    RegCloseKey(hKey);
#endif

    INITIALIZED = true;
}

void set_event_callbacks(void (*on_key_pressed)(u16 key, u16 op),
                         void (*on_key_released)(u16 key, u16 op),
                         void (*on_button_pressed)(u8 key, u16 op),
                         void (*on_button_released)(u8 key, u16 op),
                         void (*on_mouse_move)(i16 pos_x, i16 pos_y, u16 op),
                         void (*on_mouse_wheel)(i16 z_delta),
                         void (*on_window_focused)(b8 focused, u16 op),
                         void (*on_enter_leave)(b8 e_l, u16 op),
                         void (*on_window_resize)(u16 width, u16 height))
{
    callback_handler.on_key_pressed = on_key_pressed;
    callback_handler.on_key_released = on_key_released;
    callback_handler.on_button_pressed = on_button_pressed;
    callback_handler.on_button_released = on_button_released;
    callback_handler.on_mouse_move = on_mouse_move;
    callback_handler.on_mouse_wheel = on_mouse_wheel;
    callback_handler.on_window_focused = on_window_focused;
    callback_handler.on_enter_leave = on_enter_leave;
    callback_handler.on_window_resize = on_window_resize;
}

// From Raymond Chen
// Source: https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
//
WINDOWPLACEMENT window_placement = { sizeof(window_placement) };

static b8 fullscreen2 = false;
static b8 maximize = false;
static void sy_fullscreen(HWND window)
{
    static b8 fullscreen = false;
    DWORD window_style = GetWindowLong(window, GWL_STYLE);
    if (!fullscreen)
    {
        MONITORINFO monitor_info = { sizeof(monitor_info) };
        if (GetWindowPlacement(window, &window_placement) &&
            GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY),
                           &monitor_info))
        {
            SetWindowLong(window, GWL_STYLE, window_style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(window, HWND_TOP, monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.top,
                         monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
        fullscreen = true;
        fullscreen2 = true;
    }
    else
    {
        SetWindowLong(window, GWL_STYLE, window_style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(window, &window_placement);
        SetWindowPos(window, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER |
                         SWP_FRAMECHANGED);
        fullscreen = false;
        fullscreen2 = false;
        maximize = false;
    }
}

b8 is_fullscreen()
{
    return fullscreen2;
}

b8 is_maximized()
{
    return maximize;
}

void sy_toggle_fullscreen()
{
    sy_fullscreen(get_win());
}

// TODO: this should call its own function
void sy_toggle_maximize()
{
    maximize = true;
    sy_fullscreen(get_win());
    fullscreen2 = false;
}

void sy_move_window(i32 x, i32 y, i32 w, i32 h)
{
    HWND win = get_win();

    SetWindowPos(win, NULL, x, y, w, h, SWP_FRAMECHANGED);
}

void event_fire()
{
    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        switch (msg.message)
        {
            case WM_SYSKEYDOWN:
            case WM_KEYDOWN:
            {
                u16 key = (u16)msg.wParam;
                _CAPS_ON = (GetKeyState(VK_CAPITAL)) & 0xFF;

                b32 was_alt_down = (msg.lParam & (1 << 29));
                if (was_alt_down && key == VK_RETURN)
                {
                    sy_fullscreen(msg.hwnd);
                }
                // TODO: FIX this mess
                if (key == SYNT_KEY_SHIFT)
                {
                    shift_down = 1;
                }
                if (shift_down)
                {
                    _CAPS_ON = _CAPS_ON >= 1 ? 0 : 1;
                }
                callback_handler.on_key_pressed(key, _CAPS_ON);
                break;
            }
            case WM_SYSKEYUP:
            case WM_KEYUP:
            {
                u16 key = (u16)msg.wParam;
                if (key == SYNT_KEY_SHIFT)
                {
                    shift_down = 0;
                }
                if (!shift_down)
                {
                    _CAPS_ON = _CAPS_ON >= 1 ? 0 : 1;
                }
                callback_handler.on_key_released(key, _CAPS_ON);
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

void change_title(const char* title, u32 len)
{
}

void get_window_size(u16* width, u16* height)
{
    *width = platform.width;
    *height = platform.height;
}

void get_screen_pos(i32* x, i32* y)
{
    POINT point;
    GetCursorPos(&point);
    *x = (i32)point.x;
    *y = (i32)point.y;
}

static void set_cursor_pos(i16 x, i16 y)
{
    POINT point;
    point.x = x;
    point.y = y;
    ClientToScreen(platform.win, &point);
    SetCursorPos(point.x, point.y);
    SetCursor(platform.cursors[current_cursor]);
}

static b8 MOUSE_HIDDEN = false;
void hide_cursor()
{
    if (!MOUSE_HIDDEN)
    {
        current_cursor = SYNT_HIDDEN_CURSOR;
        SetCursor(platform.cursors[current_cursor]);
        SAVED_X = POS_X;
        SAVED_Y = POS_Y;
    }
    MOUSE_HIDDEN = true;
}

void show_cursor()
{
    if (MOUSE_HIDDEN)
    {
        current_cursor = SYNT_NORMAL_CURSOR;
        SetCursor(platform.cursors[current_cursor]);
    }
    MOUSE_HIDDEN = false;
}
void set_mouse_pos(i16 pos_x, i16 pos_y);

void show_cursor_centered()
{
    if (MOUSE_HIDDEN)
    {
        set_mouse_pos(platform.width / 2, platform.height / 2);
    }
    show_cursor();
    MOUSE_HIDDEN = false;
}

void set_mouse_last_pos();

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
    if (current_cursor != cursor_id && !MOUSE_HIDDEN)
    {
        if (cursor_id < TOTAL_CURSORS)
        {
            current_cursor = (u16)cursor_id;
            SetCursor(platform.cursors[current_cursor]);
        }
        else
        {
            synt_LOG_Term(
                "WARNING: trying to change to a cursor that doesn't exist.");
        }
    }
}

void set_mouse_pos(i16 pos_x, i16 pos_y)
{
    POS_X = pos_x;
    POS_Y = pos_y;
    set_cursor_pos(pos_x, pos_y);
}

void set_mouse_last_pos()
{
    POS_X = SAVED_X;
    POS_Y = SAVED_Y;
    set_cursor_pos(POS_X, POS_Y);
}

void get_pos(i16* pos_x, i16* pos_y)
{
    *pos_x = POS_X;
    *pos_y = POS_Y;
}

double get_time()
{
    struct timespec now;
    timespec_get(&now, TIME_UTC);
    return now.tv_sec + (now.tv_nsec * 0.000000001);
}

void platform_sleep(u64 milli)
{
    Sleep((DWORD)milli);
}

void shut_down_platform()
{
    DestroyWindow(platform.win);
}


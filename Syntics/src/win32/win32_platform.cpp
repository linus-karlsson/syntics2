#include "win32_platform.h"
#include "logging.h"
#include <time.h>
#include "ansi_keycodes.h"

typedef struct Callbacks
{
    void (*on_key_pressed)(uint16 key, uint16 op);
    void (*on_key_released)(uint16 key, uint16 op);
    void (*on_button_pressed)(uint8 key, uint16 op);
    void (*on_button_released)(uint8 key, uint16 op);
    void (*on_mouse_move)(int16 pos_x, int16 pos_y, uint16 op);
    void (*on_mouse_wheel)(int16 z_delta);
    void (*on_window_focused)(bool focused, uint16 op);
    void (*on_enter_leave)(bool e_l, uint16 op);
    void (*on_window_resize)(uint16 width, uint16 height);
} Callbacks;

#define SYNT_NORMAL_CURSOR 0
#define SYNT_HAND_CURSOR 1
#define SYNT_RESIZE_H_CURSOR 2
#define SYNT_RESIZE_V_CURSOR 3
#define SYNT_MOVE_CURSOR 4
#define SYNT_HIDDEN_CURSOR 5

#define TOTAL_CURSORS 6

typedef struct Win32_Platform
{
    WNDCLASS window_class;
    HWND win;
    HINSTANCE instance;

    uint16 width;
    uint16 height;

    HCURSOR cursors[TOTAL_CURSORS];
} Win32_Platform;

static Callbacks callback_handler;
static Win32_Platform platform;

static bool INITIALIZED = 0;

static int16 POS_X = 0;
static int16 POS_Y = 0;

static int16 SAVED_X = 0;
static int16 SAVED_Y = 0;

static uint16 current_cursor = SYNT_NORMAL_CURSOR;

HWND get_win()
{
    return platform.win;
}

#if 0
    char temp[10] = {};
    sprintf(temp, "%u\n", key);
    OutputDebugString(temp);
#endif

#define SYNT_KEY_CAPS 20
static uint16 _CAPS_ON = 0;

LRESULT msg_handler(HWND win, UINT msg, WPARAM w_param, LPARAM l_param)
{
    LRESULT res = 0;
    switch (msg)
    {
        case WM_KEYDOWN:
        {
            uint16 key = (uint16)w_param;
            _CAPS_ON = (GetKeyState(VK_CAPITAL)) & 0xFF;
            callback_handler.on_key_pressed(key, _CAPS_ON);
            break;
        }
        case WM_KEYUP:
        {
            uint16 key = (uint16)w_param;
            callback_handler.on_key_released(key, _CAPS_ON);
            break;
        }
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        {
            uint8 button = (uint8)w_param;
            callback_handler.on_button_pressed(button, 0);
            break;
        }
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        {
            uint8 button = (uint8)w_param;
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
            int16 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
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
        default:
        {
            res = DefWindowProc(win, msg, w_param, l_param);
            break;
        }
    }
    return res;
}

void init_platform(const char* title, uint16 width, uint16 height)
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
    platform.width = width;
    platform.height = height;

    if (platform.win == NULL)
    {
        SY_ERROR("CreateWindowEx");
    }

    INITIALIZED = true;
}

void set_event_callbacks(void (*on_key_pressed)(uint16 key, uint16 op),
                         void (*on_key_released)(uint16 key, uint16 op),
                         void (*on_button_pressed)(uint8 key, uint16 op),
                         void (*on_button_released)(uint8 key, uint16 op),
                         void (*on_mouse_move)(int16 pos_x, int16 pos_y, uint16 op),
                         void (*on_mouse_wheel)(int16 z_delta),
                         void (*on_window_focused)(bool focused, uint16 op),
                         void (*on_enter_leave)(bool e_l, uint16 op),
                         void (*on_window_resize)(uint16 width, uint16 height))
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

void event_fire()
{
    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            return;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void change_title(const char* title, uint32 len)
{
}

void get_window_size(uint16& width, uint16& height)
{
    width = platform.width;
    height = platform.height;
}

static void set_cursor_pos(int16 x, int16 y)
{
    POINT point;
    point.x = x;
    point.y = y;
    ClientToScreen(platform.win, &point);
    SetCursorPos(point.x, point.y);
    SetCursor(platform.cursors[current_cursor]);
}

static bool MOUSE_HIDDEN = false;
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
void set_mouse_pos(int16 pos_x, int16 pos_y);

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

void change_cursor(uint32 cursor_id)
{
    if (current_cursor != cursor_id && !MOUSE_HIDDEN)
    {
        if (cursor_id < TOTAL_CURSORS)
        {
            current_cursor = cursor_id;
            SetCursor(platform.cursors[current_cursor]);
        }
        else
        {
            synt_LOG("WARNING: trying to change to a cursor that doesn't exist.");
        }
    }
}

void set_mouse_pos(int16 pos_x, int16 pos_y)
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

void get_pos(int16& pos_x, int16& pos_y)
{
    pos_x = POS_X;
    pos_y = POS_Y;
}

double get_time()
{
    struct timespec now;
    timespec_get(&now, TIME_UTC);
    return now.tv_sec + (now.tv_nsec * 0.000000001);
}

void platform_sleep(uint64 milli)
{
    Sleep(milli);
}

void shut_down_platform()
{
    DestroyWindow(platform.win);
}


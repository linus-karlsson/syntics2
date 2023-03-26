extern "C"
{

#define WM_NULL 0x0000
#define WM_CREATE 0x0001
#define WM_DESTROY 0x0002
#define WM_MOVE 0x0003
#define WM_SIZE 0x0005
#define WM_QUIT 0x0012
#define WM_ACTIVATEAPP 0x001C

#define WM_SETCURSOR 0x0020

#define WM_LBUTTONDOWN 0x0201
#define WM_LBUTTONUP 0x0202
#define WM_RBUTTONDOWN 0x0204
#define WM_RBUTTONUP 0x0205
#define WM_MOUSEMOVE 0x0200
#define WM_MOUSEWHEEL 0x020A

#define DECLARE_HANDLE(name)                                                             \
    struct name##__                                                                      \
    {                                                                                    \
        int unused;                                                                      \
    };                                                                                   \
    typedef struct name##__* name

    typedef struct tagWNDCLASSA
    {
        UINT style;
        WNDPROC lpfnWndProc;
        int cbClsExtra;
        int cbWndExtra;
        HINSTANCE hInstance;
        HICON hIcon;
        HCURSOR hCursor;
        HBRUSH hbrBackground;
        LPCSTR lpszMenuName;
        LPCSTR lpszClassName;
    } WNDCLASSA, *PWNDCLASSA, NEAR *NPWNDCLASSA, FAR *LPWNDCLASSA;

    typedef struct tagWNDCLASSW
    {
        UINT style;
        WNDPROC lpfnWndProc;
        int cbClsExtra;
        int cbWndExtra;
        HINSTANCE hInstance;
        HICON hIcon;
        HCURSOR hCursor;
        HBRUSH hbrBackground;
        LPCWSTR lpszMenuName;
        LPCWSTR lpszClassName;
    } WNDCLASSW, *PWNDCLASSW, NEAR *NPWNDCLASSW, FAR *LPWNDCLASSW;

#ifdef UNICODE
    typedef WNDCLASSW WNDCLASS;
    typedef PWNDCLASSW PWNDCLASS;
    typedef NPWNDCLASSW NPWNDCLASS;
    typedef LPWNDCLASSW LPWNDCLASS;
#else
    typedef WNDCLASSA WNDCLASS;
    typedef PWNDCLASSA PWNDCLASS;
    typedef NPWNDCLASSA NPWNDCLASS;
    typedef LPWNDCLASSA LPWNDCLASS;
#endif // UNICODE

    DECLARE_HANDLE(HINSTANCE);
    DECLARE_HANDLE(HCURSOR);
    DECLARE_HANDLE(HWND);

    typedef __int64 INT_PTR, *PINT_PTR;
    typedef unsigned __int64 UINT_PTR, *PUINT_PTR;

    typedef __int64 LONG_PTR, *PLONG_PTR;
    typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;

#define __int3264 __int64

    typedef unsigned int UINT;

    typedef UINT_PTR WPARAM;
    typedef LONG_PTR LPARAM;
    typedef LONG_PTR LRESULT;

    typedef char* LPSTR;

    typedef unsigned long DWORD;

#define WINAPI __stdcall

    int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
                       int nShowCmd);

    typedef const wchar_t* LPCWSTR;

    typedef HINSTANCE HMODULE;

    HMODULE WINAPI GetModuleHandle(LPCWSTR lpModuleName);

    void __stdcall ExitProcess(unsigned int uExitCode);

    WINUSERAPI
    HCURSOR
    WINAPI
    LoadCursor(_In_opt_ HINSTANCE hInstance, _In_ LPCSTR lpCursorName);

    WINUSERAPI
    int WINAPI MessageBoxA(_In_opt_ HWND hWnd, _In_opt_ LPCSTR lpText,
                           _In_opt_ LPCSTR lpCaption, _In_ UINT uType);

    WINUSERAPI
    HCURSOR
    WINAPI
    SetCursor(_In_opt_ HCURSOR hCursor);

    WINUSERAPI
    LRESULT
    CALLBACK
    DefWindowProcA(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam);

#define DefWindowProc DefWindowProcA

    WINUSERAPI
    LONG WINAPI GetWindowLongA(_In_ HWND hWnd, _In_ int nIndex);
    
    184
}

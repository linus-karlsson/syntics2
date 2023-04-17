#include "syntic_app.h"
#include "file_reading.h"


#define DECLARE_HANDLE(name)                                                             \
    struct name##__                                                                      \
    {                                                                                    \
        int unused;                                                                      \
    };                                                                                   \
    typedef struct name##__* name

DECLARE_HANDLE(HINSTANCE);

typedef char* LPSTR;

#define WINAPI __stdcall

#ifdef LINUX
int main(int argc, char* argv[])
{
    run_app();
    return 0;
}
#else
int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line,
                   int show_cmd)
{
    run_app();
    return 0;
}
#endif

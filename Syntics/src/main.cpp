#include "syntic_app.h"
#include "file_reading.h"
#include <Windows.h>

#ifdef LINUX
int main(int argc, char* argv[])
{
    run_app();
    return 0;
}
#else
INT WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line,
                   int show_cmd)
{
    run_app();
    return 0;
}
#endif

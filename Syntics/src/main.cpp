#include "syntic_app.h"
#include <Windows.h>

#ifdef LINUX
int main(int argc, char* argv[])
{
    synt::run_app();
    return 0;
}
#else
INT WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line,
                   int show_cmd)
{
    synt::run_app();
    return 0;
}
#endif

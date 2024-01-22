#ifndef SY_UNIT_BUILD
#include "win32/sy_windows.h"
#include "syntics_app.h"
#include "notebook_app.h"
#endif

#ifdef LINUX
int main(int argc, char* argv[])
{
#if 0
    run_app();
#else
    run_notebook_app();
#endif

    return 0;
}
#else


int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line,
                   int show_cmd)
{
#if 1
    //run_app();
    printf("Size: %zd\n", sizeof(Ui_Window_Render));
#else
    run_notebook_app();
#endif
    return 0;
}
#endif

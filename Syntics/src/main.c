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
    U32_Array array = {0};
    u32_array_create(NULL, &array, 10);

    array_push(&array, 10);
    array_push(&array, 10);
    array_push(&array, 10);

    for(u32 i = 0; i < array.size; i++)
    {
        printf("Hell");
    }

    run_app();
#else
    run_notebook_app();
#endif
    return 0;
}
#endif

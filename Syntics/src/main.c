#ifndef SY_UNIT_BUILD
#include "win32/sy_windows.h"
#include "hash_table.h"
#include "math/syntics_math.h"
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

typedef struct Node_Char_U32 Node_Char_U32;
struct Node_Char_U32
{
    Node node;

    u32 value;
    const char* key;
};

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line,
                   int show_cmd)
{
#if 1
    run_app();
#else
    // run_notebook_app();
#endif
    return 0;
}
#endif

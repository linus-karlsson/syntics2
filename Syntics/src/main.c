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
    HASH_TABLE_NODE_HEADER

    u32 value;
    const char* key;
};

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line,
                   int show_cmd)
{
#if 1

    Node_Char_U32 node = {0};

    Hash_Table table =
        hash_table_create(NULL, 10, 10, hash_murmur, KEY_CHAR, Node_Char_U32);

    {
        char* buffer[] = { "Hello1", "Hello2", "Hello3", "Hello4", "Hello5",
                           "Hello6", "Hello7", "Hello8", "Hello9", "Hello11" };
        for (u32 i = 0; i < sy_SIZE(buffer); i++)
        {
            hash_table_insert(&table, buffer[i], &i);
        }
    }
    {
        char* buffer[] = { "Hello1", "Hello2", "Hello3", "Hello4", "Hello5",
                           "Hello6", "Hello7", "Hello8", "Hello9", "Hello11" };
        
        for (u32 i = 0; i < sy_SIZE(buffer); i++)
        {
            u32* val = hash_table_get(&table, buffer[i]);
            if (val)
            {
                printf("%u\n", *val);
            }
        }
    }

    run_app();
#else
    // run_notebook_app();
#endif
    return 0;
}
#endif

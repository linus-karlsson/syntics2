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

    u32 capacity = 100000;
    Hash_Table table1 =
        hash_table_create(NULL, capacity * 10, (u32)((f32)capacity * 0.1f),
                          hash_murmur, STRUCT, Node_Vertex_U32);

    Vertex_Array array0 = { 0 };
    vertex_array_create(NULL, &array0, capacity);

    U32_Array array1 = { 0 };
    u32_array_create(NULL, &array1, capacity);

    u32 seed = 0;
    for (u32 i = 0; i < capacity; i++)
    {
        Vertex vertex =
            vertex_create(v3_random(seed++, -100.0f, 100.0f),
                          v3_random(seed++, -100.0f, 100.0f),
                          v2_random(seed++, -100.0f, 100.0f), v4i(1.0f),
                          random_f32s(seed++, -1.0f, 1.0f));
        array_push(&array0, vertex);

        u32 value = random_u32ss(seed++, 0, 10000);
        array_push(&array1, value);
    }

    f64 start = platform_get_time();

    for (u32 i = 0; i < capacity; i++)
    {
        hash_table_insert(&table1, &array0.data[i], &array1.data[i]);
    }

    u32 count = 0;
    for (u32 i = 0; i < capacity; i++)
    {
       if(hash_table_get(&table1, &array0.data[i]))
       {
           count++;
       }
    }

    f64 duration = platform_get_time() - start;
    printf("Duration: %lf\n", duration);
    printf("Count, capacity: %u, %u\n", count, capacity);

#if 0
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
#endif
#else
    // run_notebook_app();
#endif
    return 0;
}
#endif

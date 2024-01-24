#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#endif

typedef struct Node_U32 Node_U32;
struct Node_U32
{
    Node_U32* next;
    Vertex key;
    u32 value;
    u32 active;
};

typedef struct Hash_Table_U32
{
    Node_U32* values;
    u32 capacity;

    Node_U32* collision_buffer;
    u32 collision_buffer_size;
    u32 collision_buffer_capacity;

    /* Maybe have two different insert and get based on the hash function to
     * eliminate the function pointer */
    u64 (*hash_function)(const void* key, u32 len, u64 seed);
} Hash_Table_U32;

Hash_Table_U32 hash_table_u32_create(Region_Alloc* region, u32 capacity,
                                     u32 collision_buffer_capacity,
                                     u64 (*hash_function)(const void* key,
                                                          u32 len, u64 seed));
;
void insert_value_u32(Hash_Table_U32* table, Vertex key, u32 value);
u32* get_value_u32(Hash_Table_U32* table, Vertex key);

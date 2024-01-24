#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#endif

typedef struct Node Node;
struct Node
{
    Node* next;
    u32 active;
};

typedef struct Node_U32 Node_U32;
struct Node_U32
{
    Node_U32* next;
    u32 active;
    u32 value;
    Vertex key;
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

typedef struct Hash_Table
{
    void* values;
    u32 capacity;

    void* collision_buffer;
    u32 collision_buffer_size;
    u32 collision_buffer_capacity;

    /* Maybe have two different insert and get based on the hash function to
     * eliminate the function pointer */
    u64 (*hash_function)(const void* key, u32 len, u64 seed);
} Hash_Table;

#define hash_table_create(region, table, table_capacity, collision_capacity,   \
                          hash_function, node_type)                            \
    do                                                                         \
    {                                                                          \
        *(table) = hash_table_create_(table_capacity, collision_capacity,      \
                                      hash_function);                          \
        if (region)                                                            \
        {                                                                      \
            (table)->values =                                                  \
                (void*)region_calloc(region, capacity, node_type);             \
            (table)->collision_buffer = (void*)region_calloc(                  \
                region, collision_buffer_capacity, node_type);                 \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            (table)->values = calloc(capacity, sizeof(node_type));             \
            (table)->collision_buffer =                                        \
                calloc(collision_buffer_capacity, sizeof(node_type));          \
        }                                                                      \
    } while (0)

Hash_Table hash_table_create_(u32 capacity, u32 collision_buffer_capacity,
                              u64 (*hash_function)(const void* key, u32 len,
                                                   u64 seed));

#define hash_table_insert_value(table, key, value, node_type)                  \
    do                                                                         \
    {                                                                          \
        node_type* node = (node_type*)hash_get_node_(                          \
            table, &(key), sizeof(key), sizeof(node_type));                    \
        if (node->active)                                                      \
        {                                                                      \
            if (memcmp(&node->key, &(key), sizeof(node->key)))                 \
            {                                                                  \
                while (node->next && node->next->active)                       \
                {                                                              \
                    node = node->next;                                         \
                    if (!memcmp(&node->key, &(key), sizeof(node->key)))        \
                    {                                                          \
                        goto add_node;                                         \
                    }                                                          \
                }                                                              \
                node->next = next_node_(table, sizeof(node_type));             \
                node = node->next;                                             \
            }                                                                  \
        }                                                                      \
        node->active = true;                                                   \
        node->key = key;                                                       \
    add_node:                                                                  \
        node->value = value;                                                   \
    } while (0)

void hash_table_insert_value_(Hash_Table* table, Vertex key, u32 value);

#define hash_table_get_value(table, key, node_type)                            \
    hash_table_get_value_(table, &(key), (u32)sizeof(node_type),               \
                          (u32)sizeof(key), offsetof(node_type, key),          \
                          offsetof(node_type, value));

void* hash_table_get_value_(Hash_Table* table, void* key, u32 node_type,
                            u32 key_size, u32 key_offset, u32 value_offset);

void* hash_get_node_(Hash_Table* table, void* key, u32 key_size,
                     u32 node_type) void* next_node_(Hash_Table* table,
                                                     u32 node_size)


#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#endif

/*****************************************************************************
 * typedef struct Node_Char_U32 Node_Char_U32;
 * struct Node_Char_U32
 * {
 *     HASH_TABLE_NODE_HEADER
 *
 *     u32 value;
 *     const char* key;
 * };
 *
 *****************************************************************************
 * If only key is a char*
 *
 * Hash_Table table = hash_table_create(NULL, 100, 10, hash_murmur, KEY_CHAR,
 *                                       Node_Char_U32);
 *
 * hash_table_insert_constant(&table, "Hello world!", 10, u32);
 * hash_table_insert_constant(&table, "Hello world!", 10, u32);
 * hash_table_insert_constant(&table, "FuzzBizzish", 3876, u32);
 *
 * u32* val = hash_table_get(&table, "Hello world!");
 * u32* val2 = hash_table_get(&table, "FuzzBizzish");
 *
 *****************************************************************************
 * If value is a char*
 *
 * Hash_Table table = hash_table_create(NULL, 100, 10, hash_murmur, VALUE_CHAR,
 *                                       Node_U32_Char);
 *
 *****************************************************************************
 * If value and key is a char*
 *
 * Hash_Table table = hash_table_create(NULL, 100, 10, hash_murmur,
 *                                      KEY_CHAR | VALUE_CHAR, Node_Char_Char);
 *
 ******************************************************************************
 * Custom hash table
 *
 * internal HASH_TABLE_KEY_SIZE(key_size_char)
 * {
 *     return (u32)strlen((const char*)key);
 * }
 *
 * internal HASH_TABLE_KEY_EQUALS(key_equals_char)
 * {
 *     return !strcmp(*((const char**)key1), (const char*)key2);
 * }
 *
 * internal HASH_TABLE_KEY_COPY(key_copy_char)
 * {
 *     *((const char**)dist) = src;
 * }
 *
 * internal HASH_TABLE_VALUE_COPY(value_copy_u32)
 * {
 *     *((u32*)dist) = *((u32*)src);
 * }
 *
 * Hash_Table table = hash_table_custom_create(
 *       NULL, 100, 10, hash_murmur, key_size_char, key_equals_char,
 *       key_copy_char, value_copy_u32, 0, Node_Char_U32);
 *
 * hash_table_insert_constant(&table, "Hello world!", 10, u32);
 * hash_table_insert_constant(&table, "Hello world!", 10, u32);
 *
 * u32* val = hash_table_get(&table, "Hello world!");
 *
 *****************************************************************************/

// If not defined the table uses open addressing.
#define HASH_TABLE_LINKED_LIST

#define HASH_TABLE_NODE_HEADER                                                 \
    Node* next;                                                                \
    b8 active;

typedef struct Node Node;
struct Node
{
    HASH_TABLE_NODE_HEADER
};

typedef enum Key_Value_Type
{
    STRUCT = 0,
    KEY_CHAR = BIT_1,
    VALUE_CHAR = BIT_2,
} Key_Value_Type;

#ifdef HASH_TABLE_LINKED_LIST
typedef struct Collision_Chunk Collision_Chunk;
struct Collision_Chunk
{
    u8* buffer;
    u32 size;
    u32 capacity;
    Collision_Chunk* next;
};
#endif

#define HASH_TABLE_KEY_SIZE(name) u32 name(const void* key, u32 size)
#define HASH_TABLE_KEY_EQUALS(name)                                            \
    b8 name(const void* key1, const void* key2, u32 size)
#define HASH_TABLE_COPY(name) void name(void* dist, const void* src, u32 size)

typedef struct Functions
{
    u32 (*key_size)(const void* key, u32 size);
    b8 (*key_equals)(const void* key1, const void* key2, u32 size);
    void (*key_copy)(void* dist, const void* src, u32 size);
    // b8 (*value_equals)(const void* key1, const void* key2);
    void (*value_copy)(void* dist, const void* src, u32 size);
}Functions;

typedef struct Hash_Table
{
    u8* values;
    u32 node_size;
    u32 key_offset;
    u32 value_offset;
    u32 key_size_bytes;
    u32 value_size_bytes;

    u32 capacity;

#ifdef HASH_TABLE_LINKED_LIST
    Collision_Chunk* current;
    Collision_Chunk collision_chunk;
#endif

    /* Maybe have two different insert and get based on the hash function to
     * eliminate the function pointer */
    u64 (*hash_function)(const void* key, u32 len, u64 seed);
    Functions f; 
} Hash_Table;

#define hash_table_create(region, table_capacity, collision_capacity,          \
                          hash_function, key_value_type, node_type)            \
    hash_table_create_(                                                        \
        region, table_capacity, collision_capacity, key_value_type,            \
        sizeof(node_type), _Alignof(node_type), sizeof(((node_type*)0)->key),  \
        offsetof(node_type, key), sizeof(((node_type*)0)->value),              \
        offsetof(node_type, value), hash_function, NULL);

Hash_Table hash_table_create_(
    Region_Alloc* region, u32 capacity, u32 collision_buffer_capacity,
    u8 key_value_type, u32 node_size, u32 node_alignment, u32 key_size_bytes,
    u32 key_offset, u32 value_size_bytes, u32 value_offset,
    u64 (*hash_function)(const void* key, u32 len, u64 seed), const Functions* functions);

#define hash_table_insert_constant(table, key, value, type)                    \
    do                                                                         \
    {                                                                          \
        type AHDINE_HLSAOPNE_KUU_76401721897890321 = value;                    \
        hash_table_insert(table, key, &AHDINE_HLSAOPNE_KUU_76401721897890321); \
    } while (0)

void hash_table_insert(Hash_Table* table, const void* key, const void* value);
void* hash_table_get(Hash_Table* table, const void* key);

///////////////////////////////////////////////////////////////////////////////

#define hash_table_custom_create(region, table_capacity, collision_capacity,   \
                                 hash_function, functions, node_type)                    \
    hash_table_create_(                                                        \
        region, table_capacity, collision_capacity, key_value_type,            \
        sizeof(node_type), _Alignof(node_type), sizeof(((node_type*)0)->key),  \
        offsetof(node_type, key), sizeof(((node_type*)0)->value),              \
        offsetof(node_type, value), hash_function, functions);

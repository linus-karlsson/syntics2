#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#endif


/*****************************************************************************
 * typedef struct Node_Char_U32 Node_Char_U32;
 * struct Node_Char_U32
 * {
 *     Node node;
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
 * Hash_Table_Custom table = hash_table_custom_create(
 *       NULL, 100, 10, hash_murmur, key_size_char, key_equals_char,
 *       key_copy_char, value_copy_u32, Node_Char_U32);
 *
 * hash_table_custom_insert_constant(&table, "Hello world!", 10, u32);
 * hash_table_custom_insert_constant(&table, "Hello world!", 10, u32);
 *
 * u32* val = hash_table_custom_get(&table, "Hello world!");
 *
 *****************************************************************************/

// If not defined the table uses open addressing.
#define HASH_TABLE_LINKED_LIST

typedef struct Node Node;
struct Node
{
#ifdef HASH_TABLE_LINKED_LIST
    Node* next;
    u32 padding;
#endif
    u32 active;
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
    void* buffer;
    u32 size;
    u32 capacity;
    Collision_Chunk* next;
};
#endif

typedef struct Hash_Table
{
    void* values;
    u32 node_size;
    u32 key_offset;
    u32 key_size;
    u32 value_offset;
    u32 value_size;

    u32 capacity;

#ifdef HASH_TABLE_LINKED_LIST
    Collision_Chunk* current;
    Collision_Chunk collision_chunk;
#endif

    /* Maybe have two different insert and get based on the hash function to
     * eliminate the function pointer */
    u64 (*hash_function)(const void* key, u32 len, u64 seed);

    u8 key_value_type : 2;
} Hash_Table;

#define HASH_TABLE_KEY_SIZE(name) u32 name(const void* key)
#define HASH_TABLE_KEY_EQUALS(name) b8 name(const void* key1, const void* key2)
#define HASH_TABLE_KEY_COPY(name) void name(void* dist, const void* src)
#define HASH_TABLE_VALUE_COPY(name) void name(void* dist, const void* src)

typedef struct Hash_Table_Custom
{
    void* values;
    u32 node_size;
    u32 key_offset;
    u32 value_offset;

    u32 capacity;

#ifdef HASH_TABLE_LINKED_LIST
    Collision_Chunk* current;
    Collision_Chunk collision_chunk;
#endif

    u64 (*hash_function)(const void* key, u32 len, u64 seed);

    u32 (*key_size)(const void* key);
    b8 (*key_equals)(const void* key1, const void* key2);
    void (*key_copy)(void* dist, const void* src);
    // b8 (*value_equals)(const void* key1, const void* key2);
    void (*value_copy)(void* dist, const void* src);
} Hash_Table_Custom;

#define hash_table_create(region, table_capacity, collision_capacity,          \
                          hash_function, key_value_type, node_type)            \
    hash_table_create_(region, table_capacity, collision_capacity,             \
                       key_value_type, sizeof(node_type), _Alignof(node_type), \
                       sizeof(((node_type*)0)->key), offsetof(node_type, key), \
                       sizeof(((node_type*)0)->value),                         \
                       offsetof(node_type, value), hash_function);

Hash_Table hash_table_create_(Region_Alloc* region, u32 capacity,
                              u32 collision_buffer_capacity, u8 key_value_type,
                              u32 node_size, u32 node_alignment, u32 key_size,
                              u32 key_offset, u32 value_size, u32 value_offset,
                              u64 (*hash_function)(const void* key, u32 len,
                                                   u64 seed));

#define hash_table_insert_constant(table, key, value, type)                    \
    do                                                                         \
    {                                                                          \
        type AHDINE_HLSAOPNE_KUU_76401721897890321 = value;                    \
        hash_table_insert(table, key, &AHDINE_HLSAOPNE_KUU_76401721897890321); \
    } while (0)

void hash_table_insert(Hash_Table* table, const void* key, const void* value);
void* hash_table_get(Hash_Table* table, const void* key);

///////////////////////////////////////////////////////////////////////////////

#define hash_table_custom_create(                                              \
    region, table_capacity, collision_capacity, hash_function,                 \
    key_size_function, key_equals_function, copy_key, copy_value, node_type)   \
    hash_table_custom_create_(                                                 \
        region, table_capacity, collision_capacity, sizeof(node_type),         \
        _Alignof(node_type), offsetof(node_type, key),                         \
        offsetof(node_type, value), hash_function, key_size_function,          \
        key_equals_function, copy_key, copy_value);

Hash_Table_Custom hash_table_custom_create_(
    Region_Alloc* region, u32 capacity, u32 collision_buffer_capacity,
    u32 node_size, u32 node_alignment, u32 key_offset, u32 value_offset,
    u64 (*hash_function)(const void* key, u32 len, u64 seed),
    u32 (*key_size)(const void* key),
    b8 (*key_equals)(const void* key1, const void* key2),
    void (*key_copy)(void* dist, const void* src),
    void (*value_copy)(void* dist, const void* src));

#define hash_table_custom_insert_constant(table, key, value, type)             \
    do                                                                         \
    {                                                                          \
        type AHDINE_HLSAOPNE_KUU_76401721897890321 = value;                    \
        hash_table_custom_insert(table, key,                                   \
                                 &AHDINE_HLSAOPNE_KUU_76401721897890321);      \
    } while (0)

void hash_table_custom_insert(Hash_Table_Custom* table, const void* key,
                              const void* value);
void* hash_table_custom_get(Hash_Table_Custom* table, const void* key);


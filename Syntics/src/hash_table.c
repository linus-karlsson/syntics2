#ifndef SY_UNIT_BUILD
#include "hash_table.h"
#include "hash.h"
#include "region_alloc.h"
#include "math/syntics_math.h"
#endif

internal void* hash_table_node_get_key(const Node* node, u32 key_offset)
{
    u8* key = (u8*)node;
    return key + key_offset;
}

internal void* hash_table_node_get_value(const Node* node, u32 value_offset)
{
    u8* value = (u8*)node;
    return value + value_offset;
}

internal Node* hash_table_get_node_(void* values, u64 hashed_index,
                                    u32 node_size)
{
    return (Node*)((u8*)values + (hashed_index * node_size));
}

internal Node* hash_table_node_advance(void* values, u64* hashed_index,
                                       u32 capacity, u32 node_size)
{
    *hashed_index = ((*hashed_index) + 1) % capacity;
    return hash_table_get_node_(values, *hashed_index, node_size);
}

#ifdef HASH_TABLE_LINKED_LIST
internal void* hash_table_next_collision_node_(Collision_Chunk* chunk,
                                               u32 node_size)
{
    assert(chunk->size < chunk->capacity);
    return ((u8*)chunk->buffer + (chunk->size++ * node_size));
}
#endif

Hash_Table hash_table_create_(Region_Alloc* region, u32 capacity,
                              u32 collision_buffer_capacity, u8 key_value_type,
                              u32 node_size, u32 node_alignment, u32 key_size,
                              u32 key_offset, u32 value_size, u32 value_offset,
                              u64 (*hash_function)(const void* key, u32 len,
                                                   u64 seed))
{
    Hash_Table out = {
        .key_value_type = key_value_type,
        .capacity = capacity,
        .hash_function = hash_function,
        .node_size = node_size,
        .key_size = key_size,
        .key_offset = key_offset,
        .value_size = value_size,
        .value_offset = value_offset,
        .key_value_type = key_value_type,
        .values = (region)
                      ? i_region_calloc(region, (u32)(capacity * node_size),
                                        node_alignment)
                      : calloc(capacity, node_size),
    };
#ifdef HASH_TABLE_LINKED_LIST
    out.current = &out.collision_chunk;
    out.collision_chunk.capacity = collision_buffer_capacity;

    if (region)
    {
        out.collision_chunk.buffer = i_region_calloc(
            region, (u32)(collision_buffer_capacity * node_size),
            node_alignment);
    }
    else
    {
        out.collision_chunk.buffer =
            calloc(collision_buffer_capacity, node_size);
    }
#endif

    return out;
}

internal u32 hash_table_key_size(Hash_Table* table, const void* key)
{
    if (table->key_value_type & KEY_CHAR)
    {
        return (u32)strlen((const char*)key);
    }
    return table->key_size;
}

internal b8 hash_table_equals(u8 key_value_type, Key_Value_Type type_to_compare,
                              const void* first, const void* second, u32 size)
{
    if (key_value_type & type_to_compare)
    {
        return !strcmp(*((const char**)first), (const char*)second);
    }
    return !memcmp(first, second, size);
}

internal void hash_table_copy(u8 key_value_type, Key_Value_Type type_to_compare,
                              void* dist, const void* src, u32 size)
{
    if (key_value_type & type_to_compare)
    {
        *((const char**)dist) = src;
    }
    else
    {
        memcpy(dist, src, size);
    }
}

void hash_table_insert(Hash_Table* table, const void* key, const void* value)
{
    u64 hashed_index =
        table->hash_function(key, hash_table_key_size(table, key), 0) %
        table->capacity;

    Node* node =
        hash_table_get_node_(table->values, hashed_index, table->node_size);
    void* node_key = hash_table_node_get_key(node, table->key_offset);
    if (node->active)
    {
        sy_print("Collision!\n");
        if (!hash_table_equals(table->key_value_type, KEY_CHAR, node_key, key,
                               table->key_size))
        {
#ifdef HASH_TABLE_LINKED_LIST
            while (node->next && node->next->active)
            {
                node = node->next;
                node_key = hash_table_node_get_key(node, table->key_offset);
                if (hash_table_equals(table->key_value_type, KEY_CHAR, node_key,
                                      key, table->key_size))
                {
                    goto add_node;
                }
            }
            node->next = hash_table_next_collision_node_(
                &table->collision_chunk, table->node_size);
            node = node->next;
#else
            node = hash_table_node_advance(table->values, &hashed_index,
                                           table->capacity, table->node_size);
            u32 n = 0;
            while (node->active && n++ < table->capacity)
            {
                node_key = hash_table_node_get_key(node, table->key_offset);
                if (hash_table_equals(table->key_value_type, KEY_CHAR, node_key,
                                      key, table->key_size))
                {
                    goto add_node;
                }
                node =
                    hash_table_node_advance(table->values, &hashed_index,
                                            table->capacity, table->node_size);
            }
#endif
            node_key = hash_table_node_get_key(node, table->key_offset);
        }
        else
        {
            goto add_node;
        }
    }
    node->active = true;
    hash_table_copy(table->key_value_type, KEY_CHAR, node_key, key,
                    table->key_size);
add_node:
    hash_table_copy(table->key_value_type, VALUE_CHAR,
                    hash_table_node_get_value(node, table->value_offset), value,
                    table->value_size);
}

void* hash_table_get(Hash_Table* table, const void* key)
{
    u64 hashed_index =
        table->hash_function(key, hash_table_key_size(table, key), 0) %
        table->capacity;

    Node* node =
        hash_table_get_node_(table->values, hashed_index, table->node_size);
    if (node->active)
    {
        void* node_key = hash_table_node_get_key(node, table->key_offset);
        if (hash_table_equals(table->key_value_type, KEY_CHAR, node_key, key,
                              table->key_size))
        {
            return hash_table_node_get_value(node, table->value_offset);
        }
#ifdef HASH_TABLE_LINKED_LIST
        while (node->next && node->next->active)
        {
            node = node->next;
            node_key = hash_table_node_get_key(node, table->key_offset);
            if (hash_table_equals(table->key_value_type, KEY_CHAR, node_key,
                                  key, table->key_size))
            {
                return hash_table_node_get_value(node, table->value_offset);
            }
        }
#else
        node = hash_table_node_advance(table->values, &hashed_index,
                                       table->capacity, table->node_size);
        u32 n = 0;
        while (node->active && n++ < table->capacity)
        {
            node_key = hash_table_node_get_key(node, table->key_offset);
            if (hash_table_equals(table->key_value_type, KEY_CHAR, node_key,
                                  key, table->key_size))
            {
                return hash_table_node_get_value(node, table->value_offset);
            }
            node = hash_table_node_advance(table->values, &hashed_index,
                                           table->capacity, table->node_size);
        }
#endif
    }
    return NULL;
}

Hash_Table_Custom hash_table_custom_create_(
    Region_Alloc* region, u32 capacity, u32 collision_buffer_capacity,
    u32 node_size, u32 node_alignment, u32 key_offset, u32 value_offset,
    u64 (*hash_function)(const void* key, u32 len, u64 seed),
    u32 (*key_size)(const void* key),
    b8 (*key_equals)(const void* key1, const void* key2),
    void (*key_copy)(void* dist, const void* src),
    void (*value_copy)(void* dist, const void* src))
{
    Hash_Table_Custom out = {
        .capacity = capacity,
        .node_size = node_size,
        .key_offset = key_offset,
        .value_offset = value_offset,
        .hash_function = hash_function,
        .key_size = key_size,
        .key_equals = key_equals,
        .key_copy = key_copy,
        .value_copy = value_copy,
        .values = (region)
                      ? i_region_calloc(region, (u32)(capacity * node_size),
                                        node_alignment)
                      : calloc(capacity, node_size),
    };
#ifdef HASH_TABLE_LINKED_LIST
    out.current = &out.collision_chunk;
    out.collision_chunk.capacity = collision_buffer_capacity;

    if (region)
    {
        out.collision_chunk.buffer = i_region_calloc(
            region, (u32)(collision_buffer_capacity * node_size),
            node_alignment);
    }
    else
    {
        out.collision_chunk.buffer =
            calloc(collision_buffer_capacity, node_size);
    }
#endif
    return out;
}

void hash_table_custom_insert(Hash_Table_Custom* table, const void* key,
                              const void* value)
{
    u64 hashed_index =
        table->hash_function(key, table->key_size(key), 0) % table->capacity;

    Node* node =
        hash_table_get_node_(table->values, hashed_index, table->node_size);
    void* node_key = hash_table_node_get_key(node, table->key_offset);
    if (node->active)
    {
        sy_print("Collision!\n");
        if (!table->key_equals(node_key, key))
        {
#ifdef HASH_TABLE_LINKED_LIST
            while (node->next && node->next->active)
            {
                node = node->next;
                node_key = hash_table_node_get_key(node, table->key_offset);
                if (table->key_equals(node_key, key))
                {
                    goto add_node;
                }
            }
            node->next = hash_table_next_collision_node_(
                &table->collision_chunk, table->node_size);
            node = node->next;
#else
            node = hash_table_node_advance(table->values, &hashed_index,
                                           table->capacity, table->node_size);
            u32 n = 0;
            while (node->active && n++ < table->capacity)
            {
                node_key = hash_table_node_get_key(node, table->key_offset);
                if (table->key_equals(node_key, key))
                {
                    goto add_node;
                }
                node =
                    hash_table_node_advance(table->values, &hashed_index,
                                            table->capacity, table->node_size);
            }
#endif
            node_key = hash_table_node_get_key(node, table->key_offset);
        }
        else
        {
            goto add_node;
        }
    }
    node->active = true;
    table->key_copy(node_key, key);
add_node:
    table->value_copy(hash_table_node_get_value(node, table->value_offset),
                      value);
}

void* hash_table_custom_get(Hash_Table_Custom* table, void* key)
{
    u64 hashed_index =
        table->hash_function(key, table->key_size(key), 0) % table->capacity;

    Node* node =
        hash_table_get_node_(table->values, hashed_index, table->node_size);
    if (node->active)
    {
        void* node_key = hash_table_node_get_key(node, table->key_offset);
        if (table->key_equals(node_key, key))
        {
            return hash_table_node_get_value(node, table->value_offset);
        }
#ifdef HASH_TABLE_LINKED_LIST
        while (node->next && node->next->active)
        {
            node = node->next;
            node_key = hash_table_node_get_key(node, table->key_offset);
            if (table->key_equals(node_key, key))
            {
                return hash_table_node_get_value(node, table->value_offset);
            }
        }
#else
        node = hash_table_node_advance(table->values, &hashed_index,
                                       table->capacity, table->node_size);
        u32 n = 0;
        while (node->active && n++ < table->capacity)
        {
            node_key = hash_table_node_get_key(node, table->key_offset);
            if (table->key_equals(node_key, key))
            {
                return hash_table_node_get_value(node, table->value_offset);
            }
            node = hash_table_node_advance(table->values, &hashed_index,
                                           table->capacity, table->node_size);
        }
#endif
    }
    return NULL;
}


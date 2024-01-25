#ifndef SY_UNIT_BUILD
#include "hash_table.h"
#include "hash.h"
#include "region_alloc.h"
#include "math/syntics_math.h"
#endif

Hash_Table_Custom hash_table_custom_create_(
    Region_Alloc* region, u32 capacity, u32 collision_buffer_capacity,
    u32 node_size, u32 node_alignment, u32 key_offset, u32 value_offset,
    u64 (*hash_function)(const void* key, u32 len, u64 seed),
    u32 (*key_size)(void* key), b8 (*key_equals)(void* key1, void* key2),
    void (*key_copy)(void* dist, void* src),
    void (*value_copy)(void* dist, void* src))
{
    Hash_Table_Custom out = {
        .capacity = capacity,
        .collision_buffer_capacity = collision_buffer_capacity,
        .node_size = node_size,
        .key_offset = key_offset,
        .value_offset = value_offset,
        .key_size = key_size,
        .key_equals = key_equals,
        .key_copy = key_copy,
        .value_copy = value_copy,
    };
    out.hash_function = (hash_function) ? hash_function : hash_murmur;

    if (region)
    {
        out.values = i_region_calloc(region, (u32)(capacity * node_size),
                                     node_alignment);
        out.collision_buffer = i_region_calloc(
            region, (u32)(collision_buffer_capacity * node_size),
            node_alignment);
    }
    else
    {
        out.values = calloc(capacity, node_size);
        out.collision_buffer = calloc(collision_buffer_capacity, node_size);
    }

    return out;
}

internal void* hash_table_custom_next_node_(Hash_Table_Custom* table)
{
    assert(table->collision_buffer_size < table->collision_buffer_capacity);
    return ((u8*)table->collision_buffer +
            (table->collision_buffer_size++ * table->node_size));
}

internal void* hash_table_custom_get_node_(Hash_Table_Custom* table, void* key)
{
    return (u8*)table->values +
           ((table->hash_function(key, table->key_size(key), 0) %
             table->capacity) *
            table->node_size);
}

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

void hash_table_custom_insert(Hash_Table_Custom* table, void* key, void* value)
{
    Node* node = (Node*)hash_table_custom_get_node_(table, key);
    void* node_key = hash_table_node_get_key(node, table->key_offset);
    if (node->active)
    {
        sy_print("Collision!\n");
        if (!table->key_equals(node_key, key))
        {
            while (node->next && node->next->active)
            {
                node = node->next;
                node_key = hash_table_node_get_key(node, table->key_offset);
                if (table->key_equals(node_key, key))
                {
                    goto add_node;
                }
            }
            node->next = hash_table_custom_next_node_(table);
            node = node->next;
        }
        else
        {
            return;
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
    Node* node = (Node*)hash_table_custom_get_node_(table, key);
    if (node->active)
    {
        void* node_key = hash_table_node_get_key(node, table->key_offset);
        if (table->key_equals(node_key, key))
        {
            return hash_table_node_get_value(node, table->value_offset);
        }
        while (node->next && node->next->active)
        {
            node = node->next;
            node_key = hash_table_node_get_key(node, table->key_offset);
            if (table->key_equals(node_key, key))
            {
                return hash_table_node_get_value(node, table->value_offset);
            }
        }
    }
    return NULL;
}

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
        .collision_buffer_capacity = collision_buffer_capacity,
        .hash_function = hash_function,
        .node_size = node_size,
        .key_size = key_size,
        .key_offset = key_offset,
        .value_size = value_size,
        .value_offset = value_offset,
        .key_value_type = key_value_type,
    };

    if (region)
    {
        out.values = i_region_calloc(region, (u32)(capacity * node_size),
                                     node_alignment);
        out.collision_buffer = i_region_calloc(
            region, (u32)(collision_buffer_capacity * node_size),
            node_alignment);
    }
    else
    {
        out.values = calloc(capacity, node_size);
        out.collision_buffer = calloc(collision_buffer_capacity, node_size);
    }

    return out;
}

internal void* hash_table_next_node_(Hash_Table* table)
{
    assert(table->collision_buffer_size < table->collision_buffer_capacity);
    return ((u8*)table->collision_buffer +
            (table->collision_buffer_size++ * table->node_size));
}

internal u32 hash_table_key_size(Hash_Table* table, void* key)
{
    if (table->key_value_type & KEY_CHAR)
    {
        return (u32)strlen((const char*)key);
    }
    return table->key_size;
}

internal b8 hash_table_equals(u8 key_value_type, Key_Value_Type type_to_compare,
                              void* first, void* second, u32 size)
{
    if (key_value_type & type_to_compare)
    {
        return !strcmp(*((const char**)first), (const char*)second);
    }
    return !memcmp(first, second, size);
}

internal void hash_table_copy(u8 key_value_type, Key_Value_Type type_to_compare,
                              void* dist, void* src, u32 size)
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

internal void* hash_table_get_node_(Hash_Table* table, void* key)
{
    return (u8*)table->values +
           ((table->hash_function(key, hash_table_key_size(table, key), 0) %
             table->capacity) *
            table->node_size);
}

void hash_table_insert(Hash_Table* table, void* key, void* value)
{
    Node* node = (Node*)hash_table_get_node_(table, key);
    void* node_key = hash_table_node_get_key(node, table->key_offset);
    if (node->active)
    {
        sy_print("Collision!\n");
        if (!hash_table_equals(table->key_value_type, KEY_CHAR, node_key, key,
                               table->key_size))
        {
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
            node->next = hash_table_next_node_(table);
            node = node->next;
        }
        else
        {
            return;
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

void* hash_table_get(Hash_Table* table, void* key)
{
    Node* node = (Node*)hash_table_get_node_(table, key);
    if (node->active)
    {
        void* node_key = hash_table_node_get_key(node, table->key_offset);
        if (hash_table_equals(table->key_value_type, KEY_CHAR, node_key, key,
                              table->key_size))
        {
            return hash_table_node_get_value(node, table->value_offset);
        }
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
    }
    return NULL;
}

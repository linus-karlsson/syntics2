#ifndef SY_UNIT_BUILD
#include "hash_table.h"
#include "hash.h"
#include "region_alloc.h"
#include "math/syntics_math.h"
#endif

global u64 HASH_COLLISION_COUNT = 0;

void reset_collision_count()
{
    HASH_COLLISION_COUNT = 0;
}

void print_collision_count()
{
    printf("Collision count: %llu\n", HASH_COLLISION_COUNT);
    sy_print("Collision count: %llu\n", HASH_COLLISION_COUNT);
}

internal inline void* hash_table_node_get_key(const Node* node, u32 key_offset)
{
    u8* key = (u8*)node;
    return key + key_offset;
}

internal inline void* hash_table_node_get_value(const Node* node,
                                                u32 value_offset)
{
    u8* value = (u8*)node;
    return value + value_offset;
}

internal inline Node* hash_table_get_node_(u8* values, u64 hashed_index,
                                           u32 node_size)
{
    return (Node*)(values + (hashed_index * node_size));
}

internal inline Node* hash_table_get_first_node(const Hash_Table* table,
                                                u64* hashed_index,
                                                const void* key, u32 key_size)
{
    *hashed_index = table->hash_function(key, key_size, 0) % table->capacity;
    return hash_table_get_node_(table->values, *hashed_index, table->node_size);
}

internal inline Node* hash_table_node_advance(u8* values, u64* hashed_index,
                                              u32 capacity, u32 node_size)
{
    *hashed_index = ((*hashed_index) + 1) % capacity;
    return hash_table_get_node_(values, *hashed_index, node_size);
}

internal inline Node* hash_table_next_collision_node_(Collision_Chunk* chunk,
                                                      u32 node_size)
{
    assert(chunk->size < chunk->capacity);
    return (Node*)(chunk->buffer + (chunk->size++ * node_size));
}

internal HASH_TABLE_KEY_SIZE(hash_table_key_size_struct)
{
    return size;
}

internal HASH_TABLE_KEY_SIZE(hash_table_key_size_char)
{
    return (u32)strlen((const char*)key);
}

internal HASH_TABLE_KEY_EQUALS(hash_table_equals_struct)
{
    return !memcmp(key1, key2, size);
}

internal HASH_TABLE_KEY_EQUALS(hash_table_equals_char)
{
    return !strcmp(*((const char**)key1), (const char*)key2);
}

internal HASH_TABLE_COPY(hash_table_copy_struct)
{
    memcpy(dist, src, size);
}

internal HASH_TABLE_COPY(hash_table_copy_char)
{
    *((const char**)dist) = src;
}

internal b8 set_next_node_linked_list(Hash_Table* table, Node** result,
                                      const void* key, u64 hashed_index)
{
    HASH_COLLISION_COUNT++;
    Node* current = *result;
    while (current->next && current->next->active)
    {
        current = current->next;
        void* node_key = hash_table_node_get_key(current, table->key_offset);
        if (table->f.key_equals(node_key, key, table->key_size_bytes))
        {
            *result = current;
            return true;
        }
        HASH_COLLISION_COUNT++;
    }
    current->next = hash_table_next_collision_node_(&table->collision_chunk,
                                                    table->node_size);
    *result = current->next;
    return false;
}

internal void* get_next_node_linked_list(Hash_Table* table, Node* node,
                                         const void* key, u64 hashed_index)
{
    while (node->next && node->next->active)
    {
        node = node->next;
        void* node_key = hash_table_node_get_key(node, table->key_offset);
        if (table->f.key_equals(node_key, key, table->key_size_bytes))
        {
            return hash_table_node_get_value(node, table->value_offset);
        }
    }
    return NULL;
}

internal b8 set_next_node_open_addressing(Hash_Table* table, Node** result,
                                          const void* key, u64 hashed_index)
{
    Node* current = *result;
    current = hash_table_node_advance(table->values, &hashed_index,
                                      table->capacity, table->node_size);
    u32 n = 0;
    while (current->active && n++ < table->capacity)
    {
        void* node_key = hash_table_node_get_key(current, table->key_offset);
        if (table->f.key_equals(node_key, key, table->key_size_bytes))
        {
            *result = current;
            return true;
        }
        current = hash_table_node_advance(table->values, &hashed_index,
                                          table->capacity, table->node_size);
    }
    *result = current;
    return false;
}

internal void* get_next_node_open_addressing(Hash_Table* table, Node* node,
                                             const void* key, u64 hashed_index)
{
    node = hash_table_node_advance(table->values, &hashed_index,
                                   table->capacity, table->node_size);
    u32 n = 0;
    while (node->active && n++ < table->capacity)
    {
        void* node_key = hash_table_node_get_key(node, table->key_offset);
        if (table->f.key_equals(node_key, key, table->key_size_bytes))
        {
            return hash_table_node_get_value(node, table->value_offset);
        }
        node = hash_table_node_advance(table->values, &hashed_index,
                                       table->capacity, table->node_size);
    }
    return NULL;
}

Hash_Table hash_table_create_(
    Region_Alloc* region, u32 capacity, u32 collision_buffer_capacity,
    u8 key_value_type, Hash_Mode hash_mode, u32 node_size, u32 node_alignment,
    u32 key_size_bytes, u32 key_offset, u32 value_size_bytes, u32 value_offset,
    u64 (*hash_function)(const void* key, u32 len, u64 seed),
    const Functions* functions)
{
    Hash_Table out = {
        .values = (region)
                      ? i_region_calloc(region, (u32)(capacity * node_size),
                                        node_alignment)
                      : calloc(capacity, node_size),

        .node_size = node_size,
        .key_offset = key_offset,
        .value_offset = value_offset,
        .capacity = capacity,

        .key_size_bytes = key_size_bytes,
        .value_size_bytes = value_size_bytes,

        .hash_function = hash_function,
    };
    b8 key_char = key_value_type & KEY_CHAR;
    if (!functions)
    {
        out.f.key_size =
            (key_char) ? hash_table_key_size_char : hash_table_key_size_struct;
        out.f.key_equals =
            (key_char) ? hash_table_equals_char : hash_table_equals_struct;
        out.f.key_copy =
            (key_char) ? hash_table_copy_char : hash_table_copy_struct;
        out.f.value_copy = (key_value_type & VALUE_CHAR)
                               ? hash_table_copy_char
                               : hash_table_copy_struct;
    }
    else
    {
        out.f = *functions;
        if (!out.f.key_size)
        {
            out.f.key_size = (key_char) ? hash_table_key_size_char
                                        : hash_table_key_size_struct;
        }
        if (!out.f.key_equals)
        {
            out.f.key_equals =
                (key_char) ? hash_table_equals_char : hash_table_equals_struct;
        }
        if (!out.f.key_copy)
        {
            out.f.key_copy =
                (key_char) ? hash_table_copy_char : hash_table_copy_struct;
        }
        if (!out.f.value_copy)
        {
            out.f.value_copy = (key_value_type & VALUE_CHAR)
                                   ? hash_table_copy_char
                                   : hash_table_copy_struct;
        }
    }

    if (hash_mode == LINKED_LIST)
    {
        out.set_next_node = set_next_node_linked_list;
        out.get_next_node = get_next_node_linked_list;

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
    }
    else
    {
        out.set_next_node = set_next_node_open_addressing;
        out.get_next_node = get_next_node_open_addressing;
    }

    return out;
}

void hash_table_insert(Hash_Table* table, const void* key, const void* value)
{
    u64 hashed_index;
    Node* node = hash_table_get_first_node(
        table, &hashed_index, key,
        table->f.key_size(key, table->key_size_bytes));

    void* node_key = hash_table_node_get_key(node, table->key_offset);
    if (node->active)
    {
        // sy_print("Collision!\n");
        if (!table->f.key_equals(node_key, key, table->key_size_bytes))
        {
            if (table->set_next_node(table, &node, key, hashed_index))
            {
                goto add_node;
            }
            node_key = hash_table_node_get_key(node, table->key_offset);
        }
        else
        {
            goto add_node;
        }
    }
    node->active = true;
    table->f.key_copy(node_key, key, table->key_size_bytes);
add_node:
    table->f.value_copy(hash_table_node_get_value(node, table->value_offset),
                        value, table->value_size_bytes);
}

void* hash_table_get(Hash_Table* table, const void* key)
{
    u64 hashed_index;
    Node* node = hash_table_get_first_node(
        table, &hashed_index, key,
        table->f.key_size(key, table->key_size_bytes));
    if (node->active)
    {
        void* node_key = hash_table_node_get_key(node, table->key_offset);
        if (table->f.key_equals(node_key, key, table->key_size_bytes))
        {
            return hash_table_node_get_value(node, table->value_offset);
        }
        return table->get_next_node(table, node, key, hashed_index);
    }
    return NULL;
}

void hash_table_remove(Hash_Table* table, const void* key)
{
}

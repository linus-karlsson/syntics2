#ifndef SY_UNIT_BUILD
#include "hash_table.h"
#include "hash.h"
#include "region_alloc.h"
#include "math/syntics_math.h"
#endif

#if 1

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

#ifdef HASH_TABLE_LINKED_LIST
internal inline void* hash_table_next_collision_node_(Collision_Chunk* chunk,
                                                      u32 node_size)
{
    assert(chunk->size < chunk->capacity);
    return (chunk->buffer + (chunk->size++ * node_size));
}
#endif

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

Hash_Table hash_table_create_(
    Region_Alloc* region, u32 capacity, u32 collision_buffer_capacity,
    u8 key_value_type, u32 node_size, u32 node_alignment, u32 key_size_bytes,
    u32 key_offset, u32 value_size_bytes, u32 value_offset,
    u64 (*hash_function)(const void* key, u32 len, u64 seed),
    u32 (*key_size)(const void* key, u32 size),
    b8 (*key_equals)(const void* key1, const void* key2, u32 size),
    void (*key_copy)(void* dist, const void* src, u32 size),
    void (*value_copy)(void* dist, const void* src, u32 size))
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

        .key_size = key_size,
        .key_equals = key_equals,
        .key_copy = key_copy,
        .value_copy = value_copy,
        .hash_function = hash_function,
    };
    b8 key_char = key_value_type & KEY_CHAR;
    if (!key_size)
    {
        out.key_size =
            (key_char) ? hash_table_key_size_char : hash_table_key_size_struct;
    }
    if (!key_equals)
    {
        out.key_equals =
            (key_char) ? hash_table_equals_char : hash_table_equals_struct;
    }
    if (!key_copy)
    {
        out.key_copy =
            (key_char) ? hash_table_copy_char : hash_table_copy_struct;
    }
    if (!value_copy)
    {
        out.value_copy = (key_value_type & VALUE_CHAR) ? hash_table_copy_char
                                                       : hash_table_copy_struct;
    }

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

void hash_table_insert(Hash_Table* table, const void* key, const void* value)
{
    u64 hashed_index;
    Node* node = hash_table_get_first_node(
        table, &hashed_index, key, table->key_size(key, table->key_size_bytes));

    void* node_key = hash_table_node_get_key(node, table->key_offset);
    if (node->active)
    {
        // sy_print("Collision!\n");
        if (!table->key_equals(node_key, key, table->key_size_bytes))
        {
#ifdef HASH_TABLE_LINKED_LIST
            while (node->next && node->next->active)
            {
                node = node->next;
                node_key = hash_table_node_get_key(node, table->key_offset);
                if (table->key_equals(node_key, key, table->key_size_bytes))
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
                if (key_equals(node_key, key_equals_data))
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
    table->key_copy(node_key, key, table->key_size_bytes);
add_node:
    table->value_copy(hash_table_node_get_value(node, table->value_offset),
                      value, table->value_size_bytes);
}

void* hash_table_get(Hash_Table* table, const void* key)
{
    u64 hashed_index;
    Node* node = hash_table_get_first_node(
        table, &hashed_index, key, table->key_size(key, table->key_size_bytes));
    if (node->active)
    {
        void* node_key = hash_table_node_get_key(node, table->key_offset);
        if (table->key_equals(node_key, key, table->key_size_bytes))
        {
            return hash_table_node_get_value(node, table->value_offset);
        }
#ifdef HASH_TABLE_LINKED_LIST
        while (node->next && node->next->active)
        {
            node = node->next;
            node_key = hash_table_node_get_key(node, table->key_offset);
            if (table->key_equals(node_key, key, table->key_size_bytes))
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
            if (key_equals(node_key, key_equals_data))
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

#elif 1

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

internal inline Node* hash_table_get_first_node(const Hash_Header* header,
                                                u64* hashed_index,
                                                const void* key, u32 key_size)
{
    *hashed_index = header->hash_function(key, key_size, 0) % header->capacity;
    return hash_table_get_node_(header->values, *hashed_index,
                                header->node_size);
}

internal inline Node* hash_table_node_advance(u8* values, u64* hashed_index,
                                              u32 capacity, u32 node_size)
{
    *hashed_index = ((*hashed_index) + 1) % capacity;
    return hash_table_get_node_(values, *hashed_index, node_size);
}

#ifdef HASH_TABLE_LINKED_LIST
internal inline void* hash_table_next_collision_node_(Collision_Chunk* chunk,
                                                      u32 node_size)
{
    assert(chunk->size < chunk->capacity);
    return (chunk->buffer + (chunk->size++ * node_size));
}
#endif

internal void hash_table_insert_internal(
    Hash_Header* header, const void* key, const void* value,
    const void* key_size_data, const void* key_equals_data,
    const void* key_copy_data, const void* value_copy_data,
    u32 (*key_size)(const void* key),
    b8 (*key_equals)(const void* key1, const void* key2),
    void (*key_copy)(void* dist, const void* src),
    void (*value_copy)(void* dist, const void* src))
{
    u64 hashed_index;
    Node* node = hash_table_get_first_node(header, &hashed_index, key,
                                           key_size(key_size_data));
    void* node_key = hash_table_node_get_key(node, header->key_offset);
    if (node->active)
    {
        // sy_print("Collision!\n");
        if (!key_equals(node_key, key_equals_data))
        {
#ifdef HASH_TABLE_LINKED_LIST
            while (node->next && node->next->active)
            {
                node = node->next;
                node_key = hash_table_node_get_key(node, header->key_offset);
                if (key_equals(node_key, key_equals_data))
                {
                    goto add_node;
                }
            }
            node->next = hash_table_next_collision_node_(
                &header->collision_chunk, header->node_size);
            node = node->next;
#else
            node = hash_table_node_advance(header->values, &hashed_index,
                                           header->capacity, header->node_size);
            u32 n = 0;
            while (node->active && n++ < header->capacity)
            {
                node_key = hash_table_node_get_key(node, header->key_offset);
                if (key_equals(node_key, key_equals_data))
                {
                    goto add_node;
                }
                node = hash_table_node_advance(header->values, &hashed_index,
                                               header->capacity,
                                               header->node_size);
            }
#endif
            node_key = hash_table_node_get_key(node, header->key_offset);
        }
        else
        {
            goto add_node;
        }
    }
    node->active = true;
    key_copy(node_key, key_copy_data);
add_node:
    value_copy(hash_table_node_get_value(node, header->value_offset),
               value_copy_data);
}

void* hash_table_get_internal(
    Hash_Header* header, const void* key, const void* key_size_data,
    const void* key_equals_data, u32 (*key_size)(const void* key),
    b8 (*key_equals)(const void* key1, const void* key2))
{
    u64 hashed_index;
    Node* node = hash_table_get_first_node(header, &hashed_index, key,
                                           key_size(key_size_data));
    if (node->active)
    {
        void* node_key = hash_table_node_get_key(node, header->key_offset);
        if (key_equals(node_key, key_equals_data))
        {
            return hash_table_node_get_value(node, header->value_offset);
        }
#ifdef HASH_TABLE_LINKED_LIST
        while (node->next && node->next->active)
        {
            node = node->next;
            node_key = hash_table_node_get_key(node, header->key_offset);
            if (key_equals(node_key, key_equals_data))
            {
                return hash_table_node_get_value(node, header->value_offset);
            }
        }
#else
        node = hash_table_node_advance(header->values, &hashed_index,
                                       header->capacity, header->node_size);
        u32 n = 0;
        while (node->active && n++ < header->capacity)
        {
            node_key = hash_table_node_get_key(node, header->key_offset);
            if (key_equals(node_key, key_equals_data))
            {
                return hash_table_node_get_value(node, header->value_offset);
            }
            node = hash_table_node_advance(header->values, &hashed_index,
                                           header->capacity, header->node_size);
        }
#endif
    }
    return NULL;
}

typedef struct Table_Key_Size
{
    Hash_Table* table;
    const void* key;
} Table_Key_Size;

typedef struct Table_Key_Equal
{
    u8 key_value_type;
    Key_Value_Type type_to_compare;
    const void* second;
    u32 size;
} Table_Key_Equal;

typedef struct Table_Copy
{
    u8 key_value_type;
    Key_Value_Type type_to_compare;
    const void* src;
    u32 size;
} Table_Copy;

internal inline u32 hash_table_key_size(const void* data)
{
    Table_Key_Size* t_k = (Table_Key_Size*)data;
    if (t_k->table->key_value_type & KEY_CHAR)
    {
        return (u32)strlen((const char*)t_k->key);
    }
    return t_k->table->key_size;
}

internal inline b8 hash_table_equals(const void* first, const void* data)
{
    Table_Key_Equal* k_e = (Table_Key_Equal*)data;
    if (k_e->key_value_type & k_e->type_to_compare)
    {
        return !strcmp(*((const char**)first), (const char*)k_e->second);
    }
    return !memcmp(first, k_e->second, k_e->size);
}

internal inline void hash_table_copy(void* dist, const void* data)
{
    Table_Copy* t_c = (Table_Copy*)data;
    if (t_c->key_value_type & t_c->type_to_compare)
    {
        *((const char**)dist) = t_c->src;
    }
    else
    {
        memcpy(dist, t_c->src, t_c->size);
    }
}

Hash_Table hash_table_create_(Region_Alloc* region, u32 capacity,
                              u32 collision_buffer_capacity, u8 key_value_type,
                              u32 node_size, u32 node_alignment, u32 key_size,
                              u32 key_offset, u32 value_size, u32 value_offset,
                              u64 (*hash_function)(const void* key, u32 len,
                                                   u64 seed))
{
    Hash_Table out = {
        .header.values =
            (region) ? i_region_calloc(region, (u32)(capacity * node_size),
                                       node_alignment)
                     : calloc(capacity, node_size),
        .header.node_size = node_size,
        .header.key_offset = key_offset,
        .header.value_offset = value_offset,
        .header.capacity = capacity,
        .header.hash_function = hash_function,

        .key_value_type = key_value_type,
        .key_size = key_size,
        .value_size = value_size,
        .key_value_type = key_value_type,
    };
#ifdef HASH_TABLE_LINKED_LIST
    out.header.current = &out.header.collision_chunk;
    out.header.collision_chunk.capacity = collision_buffer_capacity;

    if (region)
    {
        out.header.collision_chunk.buffer = i_region_calloc(
            region, (u32)(collision_buffer_capacity * node_size),
            node_alignment);
    }
    else
    {
        out.header.collision_chunk.buffer =
            calloc(collision_buffer_capacity, node_size);
    }
#endif

    return out;
}

void hash_table_insert(Hash_Table* table, const void* key, const void* value)
{
    Table_Key_Size k_s = {
        .table = table,
        .key = key,
    };
    Table_Key_Equal k_e = {
        .key_value_type = table->key_value_type,
        .type_to_compare = KEY_CHAR,
        .second = key,
        .size = table->key_size,
    };
    Table_Copy t_c_key = {
        .key_value_type = table->key_value_type,
        .type_to_compare = KEY_CHAR,
        .src = key,
        .size = table->key_size,
    };
    Table_Copy t_c_value = {
        .key_value_type = table->key_value_type,
        .type_to_compare = VALUE_CHAR,
        .src = value,
        .size = table->value_size,
    };
    hash_table_insert_internal(&table->header, key, value, &k_s, &k_e, &t_c_key,
                               &t_c_value, hash_table_key_size,
                               hash_table_equals, hash_table_copy,
                               hash_table_copy);
}

void* hash_table_get(Hash_Table* table, const void* key)
{
    Table_Key_Size k_s = {
        .table = table,
        .key = key,
    };
    Table_Key_Equal k_e = {
        .key_value_type = table->key_value_type,
        .type_to_compare = KEY_CHAR,
        .second = key,
        .size = table->key_size,
    };
    return hash_table_get_internal(&table->header, key, &k_s, &k_e,
                                   hash_table_key_size, hash_table_equals);
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
        .header.values =
            (region) ? i_region_calloc(region, (u32)(capacity * node_size),
                                       node_alignment)
                     : calloc(capacity, node_size),
        .header.capacity = capacity,
        .header.node_size = node_size,
        .header.key_offset = key_offset,
        .header.value_offset = value_offset,
        .header.hash_function = hash_function,
        .key_size = key_size,
        .key_equals = key_equals,
        .key_copy = key_copy,
        .value_copy = value_copy,
    };
#ifdef HASH_TABLE_LINKED_LIST
    out.header.current = &out.header.collision_chunk;
    out.header.collision_chunk.capacity = collision_buffer_capacity;

    if (region)
    {
        out.header.collision_chunk.buffer = i_region_calloc(
            region, (u32)(collision_buffer_capacity * node_size),
            node_alignment);
    }
    else
    {
        out.header.collision_chunk.buffer =
            calloc(collision_buffer_capacity, node_size);
    }
#endif
    return out;
}

void hash_table_custom_insert(Hash_Table_Custom* table, const void* key,
                              const void* value)
{
    hash_table_insert_internal(&table->header, key, value, key, key, key, value,
                               table->key_size, table->key_equals,
                               table->key_copy, table->value_copy);
}

void* hash_table_custom_get(Hash_Table_Custom* table, void* key)
{
    return hash_table_get_internal(&table->header, key, key, key,
                                   table->key_size, table->key_equals);
}

#else

Hash_Table hash_table_create_(Region_Alloc* region, u32 capacity,
                              u32 collision_buffer_capacity, u8 key_value_type,
                              u32 node_size, u32 node_alignment, u32 key_size,
                              u32 key_offset, u32 value_size, u32 value_offset,
                              u64 (*hash_function)(const void* key, u32 len,
                                                   u64 seed))
{
    Hash_Table out = {
        .header.values =
            (region) ? i_region_calloc(region, (u32)(capacity * node_size),
                                       node_alignment)
                     : calloc(capacity, node_size),
        .header.node_size = node_size,
        .header.key_offset = key_offset,
        .header.value_offset = value_offset,
        .header.capacity = capacity,
        .header.hash_function = hash_function,

        .key_value_type = key_value_type,
        .key_size = key_size,
        .value_size = value_size,
        .key_value_type = key_value_type,
    };
#ifdef HASH_TABLE_LINKED_LIST
    out.header.current = &out.header.collision_chunk;
    out.header.collision_chunk.capacity = collision_buffer_capacity;

    if (region)
    {
        out.header.collision_chunk.buffer = i_region_calloc(
            region, (u32)(collision_buffer_capacity * node_size),
            node_alignment);
    }
    else
    {
        out.header.collision_chunk.buffer =
            calloc(collision_buffer_capacity, node_size);
    }
#endif

    return out;
}

internal inline u32 hash_table_key_size(Hash_Table* table, const void* key)
{
    if (table->key_value_type & KEY_CHAR)
    {
        return (u32)strlen((const char*)key);
    }
    return table->key_size;
}

internal inline b8 hash_table_equals(u8 key_value_type,
                                     Key_Value_Type type_to_compare,
                                     const void* first, const void* second,
                                     u32 size)
{
    if (key_value_type & type_to_compare)
    {
        return !strcmp(*((const char**)first), (const char*)second);
    }
    return !memcmp(first, second, size);
}

internal inline void hash_table_copy(u8 key_value_type,
                                     Key_Value_Type type_to_compare, void* dist,
                                     const void* src, u32 size)
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
    u64 hashed_index;
    Node* node = hash_table_get_first_node(&table->header, &hashed_index, key,
                                           hash_table_key_size(table, key));
    void* node_key = hash_table_node_get_key(node, table->header.key_offset);
    if (node->active)
    {
        // sy_print("Collision!\n");
        if (!hash_table_equals(table->key_value_type, KEY_CHAR, node_key, key,
                               table->key_size))
        {
#ifdef HASH_TABLE_LINKED_LIST
            while (node->next && node->next->active)
            {
                node = node->next;
                node_key =
                    hash_table_node_get_key(node, table->header.key_offset);
                if (hash_table_equals(table->key_value_type, KEY_CHAR, node_key,
                                      key, table->key_size))
                {
                    goto add_node;
                }
            }
            node->next = hash_table_next_collision_node_(
                &table->header.collision_chunk, table->header.node_size);
            node = node->next;
#else
            node = hash_table_node_advance(table->header.values, &hashed_index,
                                           table->header.capacity,
                                           table->header.node_size);
            u32 n = 0;
            while (node->active && n++ < table->header.capacity)
            {
                node_key =
                    hash_table_node_get_key(node, table->header.key_offset);
                if (hash_table_equals(table->key_value_type, KEY_CHAR, node_key,
                                      key, table->key_size))
                {
                    goto add_node;
                }
                node = hash_table_node_advance(
                    table->header.values, &hashed_index, table->header.capacity,
                    table->header.node_size);
            }
#endif
            node_key = hash_table_node_get_key(node, table->header.key_offset);
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
                    hash_table_node_get_value(node, table->header.value_offset),
                    value, table->value_size);
}

void* hash_table_get(Hash_Table* table, const void* key)
{
    u64 hashed_index;
    Node* node = hash_table_get_first_node(&table->header, &hashed_index, key,
                                           hash_table_key_size(table, key));
    if (node->active)
    {
        void* node_key =
            hash_table_node_get_key(node, table->header.key_offset);
        if (hash_table_equals(table->key_value_type, KEY_CHAR, node_key, key,
                              table->key_size))
        {
            return hash_table_node_get_value(node, table->header.value_offset);
        }
#ifdef HASH_TABLE_LINKED_LIST
        while (node->next && node->next->active)
        {
            node = node->next;
            node_key = hash_table_node_get_key(node, table->header.key_offset);
            if (hash_table_equals(table->key_value_type, KEY_CHAR, node_key,
                                  key, table->key_size))
            {
                return hash_table_node_get_value(node,
                                                 table->header.value_offset);
            }
        }
#else
        node = hash_table_node_advance(table->header.values, &hashed_index,
                                       table->header.capacity,
                                       table->header.node_size);
        u32 n = 0;
        while (node->active && n++ < table->header.capacity)
        {
            node_key = hash_table_node_get_key(node, table->header.key_offset);
            if (hash_table_equals(table->key_value_type, KEY_CHAR, node_key,
                                  key, table->key_size))
            {
                return hash_table_node_get_value(node,
                                                 table->header.value_offset);
            }
            node = hash_table_node_advance(table->header.values, &hashed_index,
                                           table->header.capacity,
                                           table->header.node_size);
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
        .header.values =
            (region) ? i_region_calloc(region, (u32)(capacity * node_size),
                                       node_alignment)
                     : calloc(capacity, node_size),
        .header.capacity = capacity,
        .header.node_size = node_size,
        .header.key_offset = key_offset,
        .header.value_offset = value_offset,
        .header.hash_function = hash_function,
        .key_size = key_size,
        .key_equals = key_equals,
        .key_copy = key_copy,
        .value_copy = value_copy,
    };
#ifdef HASH_TABLE_LINKED_LIST
    out.header.current = &out.header.collision_chunk;
    out.header.collision_chunk.capacity = collision_buffer_capacity;

    if (region)
    {
        out.header.collision_chunk.buffer = i_region_calloc(
            region, (u32)(collision_buffer_capacity * node_size),
            node_alignment);
    }
    else
    {
        out.header.collision_chunk.buffer =
            calloc(collision_buffer_capacity, node_size);
    }
#endif
    return out;
}

void hash_table_custom_insert(Hash_Table_Custom* table, const void* key,
                              const void* value)
{
    u64 hashed_index;
    Node* node = hash_table_get_first_node(&table->header, &hashed_index, key,
                                           table->key_size(key));
    void* node_key = hash_table_node_get_key(node, table->header.key_offset);
    if (node->active)
    {
        // sy_print("Collision!\n");
        if (!table->key_equals(node_key, key))
        {
#ifdef HASH_TABLE_LINKED_LIST
            while (node->next && node->next->active)
            {
                node = node->next;
                node_key =
                    hash_table_node_get_key(node, table->header.key_offset);
                if (table->key_equals(node_key, key))
                {
                    goto add_node;
                }
            }
            node->next = hash_table_next_collision_node_(
                &table->header.collision_chunk, table->header.node_size);
            node = node->next;
#else
            node = hash_table_node_advance(table->header.values, &hashed_index,
                                           table->header.capacity,
                                           table->header.node_size);
            u32 n = 0;
            while (node->active && n++ < table->header.capacity)
            {
                node_key =
                    hash_table_node_get_key(node, table->header.key_offset);
                if (table->key_equals(node_key, key))
                {
                    goto add_node;
                }
                node = hash_table_node_advance(
                    table->header.values, &hashed_index, table->header.capacity,
                    table->header.node_size);
            }
#endif
            node_key = hash_table_node_get_key(node, table->header.key_offset);
        }
        else
        {
            goto add_node;
        }
    }
    node->active = true;
    table->key_copy(node_key, key);
add_node:
    table->value_copy(
        hash_table_node_get_value(node, table->header.value_offset), value);
}

void* hash_table_custom_get(Hash_Table_Custom* table, void* key)
{
    u64 hashed_index;
    Node* node = hash_table_get_first_node(&table->header, &hashed_index, key,
                                           table->key_size(key));
    if (node->active)
    {
        void* node_key =
            hash_table_node_get_key(node, table->header.key_offset);
        if (table->key_equals(node_key, key))
        {
            return hash_table_node_get_value(node, table->header.value_offset);
        }
#ifdef HASH_TABLE_LINKED_LIST
        while (node->next && node->next->active)
        {
            node = node->next;
            node_key = hash_table_node_get_key(node, table->header.key_offset);
            if (table->key_equals(node_key, key))
            {
                return hash_table_node_get_value(node,
                                                 table->header.value_offset);
            }
        }
#else
        node = hash_table_node_advance(table->header.values, &hashed_index,
                                       table->header.capacity,
                                       table->header.node_size);
        u32 n = 0;
        while (node->active && n++ < table->header.capacity)
        {
            node_key = hash_table_node_get_key(node, table->header.key_offset);
            if (table->key_equals(node_key, key))
            {
                return hash_table_node_get_value(node,
                                                 table->header.value_offset);
            }
            node = hash_table_node_advance(table->header.values, &hashed_index,
                                           table->header.capacity,
                                           table->header.node_size);
        }
#endif
    }
    return NULL;
}

#endif


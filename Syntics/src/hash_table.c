#ifndef SY_UNIT_BUILD
#include "hash_table.h"
#include "hash.h"
#include "region_alloc.h"
#include "math/syntics_math.h"
#endif

Hash_Table_U32 hash_table_u32_create(Region_Alloc* region, u32 capacity,
                                     u32 collision_buffer_capacity,
                                     u64 (*hash_function)(const void* key,
                                                          u32 len, u64 seed))
{
    Hash_Table_U32 out = { 0 };
    out.capacity = capacity;
    out.collision_buffer_capacity = collision_buffer_capacity;
    if (region)
    {
        out.values = region_calloc(region, capacity, Node_U32);
        out.collision_buffer =
            region_calloc(region, collision_buffer_capacity, Node_U32);
    }
    else
    {
        out.values = (Node_U32*)calloc(capacity, sizeof(Node_U32));
        out.collision_buffer =
            (Node_U32*)calloc(collision_buffer_capacity, sizeof(Node_U32));
    }
    out.hash_function = hash_function;
    return out;
}

internal Node_U32* next_node_u32(Hash_Table_U32* table)
{
    assert(table->collision_buffer_size < table->collision_buffer_capacity);
    return table->collision_buffer + table->collision_buffer_size++;
}

void insert_value_u32(Hash_Table_U32* table, Vertex key, u32 value)
{
    Node_U32* node =
        table->values +
        (table->hash_function(&key, sizeof(Vertex), 0) % table->capacity);
    if (node->active)
    {
        sy_print("Collision!\n");
        if (vertex_equal(&node->key, &key))
        {
            if (node->value == value)
            {
                return;
            }
            goto add_node;
        }
        else
        {
            while (node->next)
            {
                node = node->next;
                if (node->active && vertex_equal(&node->key, &key))
                {
                    if (node->value == value)
                    {
                        return;
                    }
                    goto add_node;
                }
            }
            node->next = next_node_u32(table);
            node = node->next;
        }
    }
    node->key = key;
    node->active = true;
add_node:
    node->value = value;
}

u32* get_value_u32(Hash_Table_U32* table, Vertex key)
{
    Node_U32* node =
        table->values +
        (table->hash_function(&key, sizeof(Vertex), 0) % table->capacity);

    if (node->active)
    {
        if (vertex_equal(&node->key, &key))
        {
            return &node->value;
        }
        while (node->next)
        {
            node = node->next;
            if (node->active && vertex_equal(&node->key, &key))
            {
                return &node->value;
            }
        }
    }
    return NULL;
}

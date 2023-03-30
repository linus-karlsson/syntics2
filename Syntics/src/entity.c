#include "entity.h"
#include "defines.h"
#include "logging.h"
#include "region_alloc.h"

#define MAX_ENTITIES 1000

typedef struct Table_Row
{
    u32 index;
    u32 ref_value;
} Table_Row;

typedef struct Lookup_Table
{
    Table_Row* d_entries;
    Table_Row* s_entries;
} Lookup_Table;

typedef struct Internal_S_Entity
{
    Static_Entity* entities;
} Internal_S_Entity;

typedef struct Internal_D_Entity
{
    Dynamic_Entity_2D* entities;
} Internal_D_Entity;

static Internal_S_Entity g_s_in = { 0 };
static Internal_D_Entity g_d_in = { 0 };
static Lookup_Table g_l_t = { 0 };

static u32* g_free_indices = NULL;

// First spot is always empty
static u32 num_entities = 1;

void init_entity(Region_Alloc* region)
{
    g_free_indices = dyn_array_calloc(region, MAX_ENTITIES, u32, PERM_ARRAY);

    g_s_in.entities = dyn_arrayP(region, MAX_ENTITIES, Static_Entity);
    g_d_in.entities = dyn_arrayP(region, MAX_ENTITIES, Dynamic_Entity_2D);

    g_l_t.d_entries = dyn_array_calloc(region, MAX_ENTITIES, Table_Row, PERM_ARRAY);
    g_l_t.s_entries = dyn_array_calloc(region, MAX_ENTITIES, Table_Row, PERM_ARRAY);
}

void update_dyn_etities()
{
}

Lookup_Key add_dyn_entity()
{
    ASSERT(num_entities < MAX_ENTITIES, "add_dyn_entity");

    Dynamic_Entity_2D new = { 0 };
    Lookup_Key out = { 0 };

    u32 free_size = size_arr(g_free_indices);
    if (free_size)
    {
        new.id = synt_pop(g_free_indices);
    }
    else
    {
        new.id = num_entities;
    }
    out._table_index = new.id;
    out._ref_value = g_l_t.d_entries[new.id].ref_value;
    g_l_t.d_entries[new.id].index = num_entities;
    g_d_in.entities[num_entities++] = new;

    return out;
}

void remove_dyn_entity(Lookup_Key e)
{
    ASSERT(e._table_index < MAX_ENTITIES, "remove_dyn_entitiy e._table_index");
    if (e._table_index == 0) return;

    Table_Row* current_row = g_l_t.d_entries + e._table_index;
    if (current_row->ref_value == e._ref_value)
    {
        current_row->ref_value++;
        ASSERT(current_row->ref_value < U32_MAX - 10, "ref_value is to large");
        synt_push(g_free_indices, e._table_index);

        if (current_row->index != num_entities - 1)
        {
            Dynamic_Entity_2D* update_pos = g_d_in.entities + current_row->index;
            *update_pos = g_d_in.entities[num_entities - 1];
            g_l_t.d_entries[update_pos->id].index = current_row->index;
        }
        num_entities--;
        current_row->index = 0;
    }
}

Lookup_Key ref_dyn_entity(Lookup_Key e)
{
    ASSERT(e._table_index < MAX_ENTITIES, "remove_dyn_entitiy e._table_index");
    Lookup_Key out = { 0 };
    if (e._table_index == 0)
    {
        return out;
    }
    Table_Row* current_row = g_l_t.d_entries + e._table_index;
    if (current_row->ref_value == e._ref_value)
    {
        out._table_index = e._table_index;
        out._ref_value = e._ref_value;
    }
    return out;
}

Dynamic_Entity_2D* iterate_entities(u32* i)
{
    Dynamic_Entity_2D* out = NULL;
    if (++(*i) < num_entities)
    {
        out = g_d_in.entities + (*i);
    }
    return out;
}

Dynamic_Entity_2D* access_dyn_entity(Lookup_Key e)
{
    ASSERT(e._table_index < MAX_ENTITIES, "remove_dyn_entitiy e._table_index");
    Dynamic_Entity_2D* out = NULL;
    Table_Row* current_row = g_l_t.d_entries + e._table_index;
    if (current_row->index != 0 && current_row->ref_value == e._ref_value)
    {
        out = g_d_in.entities + current_row->index;
    }
    return out;
}

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
    Table_Row* dyn_entries;
    Table_Row* static_entries;
} Lookup_Table;

typedef struct Internal_S_Entity
{
    Static_Entity* entities;
} Internal_S_Entity;

typedef struct Internal_D_Entity
{
    Entity_Movement* movements;
    Entity_Misc* miscs;
} Internal_D_Entity;

static Internal_S_Entity g_s_in = { 0 };
static Internal_D_Entity g_d_in = { 0 };
static Lookup_Table g_l_t = { 0 };

static u32* g_free_indices = NULL;

// First spot is always empty
static u32 num_entities = 1;

internal Dynamic_Entity_2D construct_entity(Entity_Movement* move, Entity_Misc* misc)
{
    Dynamic_Entity_2D out;
    out.movement = move;
    out.misc = misc;
    return out;
}

void init_entity(Region_Alloc* region)
{
    g_free_indices = dyn_array_calloc(region, MAX_ENTITIES, u32, PERM_ARRAY);

    g_s_in.entities = dyn_arrayP(region, MAX_ENTITIES, Static_Entity);

    g_d_in.movements = dyn_arrayP(region, MAX_ENTITIES, Entity_Movement);
    g_d_in.miscs = dyn_arrayP(region, MAX_ENTITIES, Entity_Misc);

    g_l_t.dyn_entries =
        dyn_array_calloc(region, MAX_ENTITIES, Table_Row, PERM_ARRAY);

    g_l_t.static_entries =
        dyn_array_calloc(region, MAX_ENTITIES, Table_Row, PERM_ARRAY);
}

void update_dyn_etities()
{
}

Lookup_Key add_dyn_entity()
{
    ASSERT(num_entities < MAX_ENTITIES, "add_dyn_entity");

    Entity_Movement new_move = { 0 };
    Entity_Misc new_misc = { 0 };
    Lookup_Key out = { 0 };

    u32 free_size = size_arr(g_free_indices);
    if (free_size)
    {
        new_misc.id = synt_pop(g_free_indices);
    }
    else
    {
        new_misc.id = num_entities;
    }
    out._table_index = new_misc.id;
    out._ref_value = g_l_t.dyn_entries[new_misc.id].ref_value;
    g_l_t.dyn_entries[new_misc.id].index = num_entities;
    g_d_in.movements[num_entities] = new_move;
    g_d_in.miscs[num_entities++] = new_misc;

    return out;
}

void remove_dyn_entity(Lookup_Key e)
{
    ASSERT(e._table_index < MAX_ENTITIES, "remove_dyn_entitiy e._table_index");
    if (e._table_index == 0) return;

    Table_Row* current_row = g_l_t.dyn_entries + e._table_index;
    if (current_row->ref_value == e._ref_value)
    {
        current_row->ref_value++;
        ASSERT(current_row->ref_value < U32_MAX - 10, "ref_value is to large");
        synt_push(g_free_indices, e._table_index);

        if (current_row->index != num_entities - 1)
        {
            Entity_Movement* update_pos_move = g_d_in.movements + current_row->index;
            Entity_Misc* update_pos_misc = g_d_in.miscs + current_row->index;
            *update_pos_move = g_d_in.movements[num_entities - 1];
            *update_pos_misc = g_d_in.miscs[num_entities - 1];
            g_l_t.dyn_entries[update_pos_misc->id].index = current_row->index;
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
    Table_Row* current_row = g_l_t.dyn_entries + e._table_index;
    if (current_row->ref_value == e._ref_value)
    {
        out._table_index = e._table_index;
        out._ref_value = e._ref_value;
    }
    return out;
}

Dynamic_Entity_2D iterate_entities(u32* i)
{
    Dynamic_Entity_2D out = { 0 };
    if (++(*i) < num_entities)
    {
        out = construct_entity(g_d_in.movements + (*i), g_d_in.miscs + (*i));
    }
    return out;
}

Entity_Movement* iterate_entity_movement(u32* i)
{
    Entity_Movement* out = NULL ;
    if (++(*i) < num_entities)
    {
        out = g_d_in.movements + (*i);
    }
    return out;
}

Entity_Movement* access_dyn_entity_movement(Lookup_Key e)
{
    ASSERT(e._table_index < MAX_ENTITIES, "remove_dyn_entitiy e._table_index");
    Entity_Movement* out = NULL;
    Table_Row* current_row = g_l_t.dyn_entries + e._table_index;
    if (current_row->index != 0 && current_row->ref_value == e._ref_value)
    {
        out = g_d_in.movements + current_row->index;
    }
    return out;
}

Dynamic_Entity_2D access_dyn_entity(Lookup_Key e)
{
    ASSERT(e._table_index < MAX_ENTITIES, "remove_dyn_entitiy e._table_index");
    Dynamic_Entity_2D out = { 0 };
    Table_Row* current_row = g_l_t.dyn_entries + e._table_index;
    if (current_row->index != 0 && current_row->ref_value == e._ref_value)
    {
        Entity_Movement* move = g_d_in.movements + current_row->index;
        Entity_Misc* misc = g_d_in.miscs + current_row->index;
        out = construct_entity(move, misc);
    }
    return out;
}

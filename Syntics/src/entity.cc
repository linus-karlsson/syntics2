/*
#include "entity.h"
#include "defines.h"
#include "logging.h"
#include "region_alloc.h"
#include "lookup_table.h"
*/

#define MAX_ENTITIES 1000

typedef struct Internal_S_Entity
{
    Static_Entity* entities;
} Internal_S_Entity;

typedef struct Internal_D_Entity
{
    Entity_Movement* movements;
    Entity_Misc* miscs;
} Internal_D_Entity;

global Internal_S_Entity g_s_in = { };
global Internal_D_Entity g_d_in = {  };
global Lookup_Table* g_l_t = NULL;

// First spot is always empty
global u32 num_entities = 1;

internal Dynamic_Entity_2D construct_entity(Entity_Movement* move, Entity_Misc* misc)
{
    Dynamic_Entity_2D out;
    out.movement = move;
    out.misc = misc;
    return out;
}

void init_entity(Region_Alloc* region)
{
    g_s_in.entities = dyn_arrayP(region, MAX_ENTITIES, Static_Entity);

    g_d_in.movements = dyn_arrayP(region, MAX_ENTITIES, Entity_Movement);
    g_d_in.miscs = dyn_arrayP(region, MAX_ENTITIES, Entity_Misc);

    g_l_t = region_mallocP(region, 1, Lookup_Table);
    *g_l_t = Lookup_Table(region, MAX_ENTITIES);
}

void update_dyn_etities()
{
}

Lookup_Key add_dyn_entity()
{
    ASSERT(num_entities < MAX_ENTITIES, "add_dyn_entity");

    Entity_Movement new_move = {};
    Entity_Misc new_misc = {};

    Lookup_Key out = g_l_t->add_entry(num_entities);
    new_misc.id = out.table_index();
    g_d_in.movements[num_entities] = new_move;
    g_d_in.miscs[num_entities++] = new_misc;

    return out;
}

void remove_dyn_entity(Lookup_Key e)
{
    u32 index = g_l_t->remove_entry(e);
    if(index == 0)
    {
        return;
    }
    if (index != num_entities - 1)
    {
        Entity_Movement* update_pos_move = g_d_in.movements + index;
        Entity_Misc* update_pos_misc = g_d_in.miscs + index;
        *update_pos_move = g_d_in.movements[num_entities - 1];
        *update_pos_misc = g_d_in.miscs[num_entities - 1];
        g_l_t->cange_entry_index(update_pos_misc->id, index);
    }
    num_entities--;
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
    Entity_Movement* out = NULL;
    if (++(*i) < num_entities)
    {
        out = g_d_in.movements + (*i);
    }
    return out;
}

Entity_Movement* access_dyn_entity_movement(Lookup_Key e)
{
    Entity_Movement* out = NULL;
    u32 index = g_l_t->index(e);
    if(index != 0)
    {
        out = g_d_in.movements + index;
    }
    return out;
}

Dynamic_Entity_2D access_dyn_entity(Lookup_Key e)
{
    Dynamic_Entity_2D out = { 0 };
    u32 index = g_l_t->index(e);
    if(index != 0)
    {
        Entity_Movement* move = g_d_in.movements + index;
        Entity_Misc* misc = g_d_in.miscs + index;
        out = construct_entity(move, misc);
    }
    return out;
}

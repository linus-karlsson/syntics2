
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

global Internal_S_Entity s_in_ENTITY = {0};
global Internal_D_Entity d_in_ENTITY = {0};
global Lookup_Table* l_t_ENTITY = NULL;

// First spot is always empty
global u32 num_entities = 1;

internal Dynamic_Entity_2D entity_construct(Entity_Movement* move, Entity_Misc* misc)
{
    Dynamic_Entity_2D out;
    out.movement = move;
    out.misc = misc;
    return out;
}

void entity_init(Region_Alloc* region)
{
    s_in_ENTITY.entities = region_arrayP(region, MAX_ENTITIES, Static_Entity);

    d_in_ENTITY.movements = region_arrayP(region, MAX_ENTITIES, Entity_Movement);
    d_in_ENTITY.miscs = region_arrayP(region, MAX_ENTITIES, Entity_Misc);

    l_t_ENTITY = region_mallocP(region, 1, Lookup_Table);
    *l_t_ENTITY = lookup_table_create(region, MAX_ENTITIES);
}

Lookup_Key dyn_entity_add(void)
{
    ASSERT(num_entities < MAX_ENTITIES, "add_dyn_entity");

    Entity_Movement new_move = {0};
    Entity_Misc new_misc = {0};

    Lookup_Key out = add_entry(l_t_ENTITY, num_entities);
    new_misc.id = out._row.index;
    d_in_ENTITY.movements[num_entities] = new_move;
    d_in_ENTITY.miscs[num_entities++] = new_misc;

    return out;
}

void dyn_entity_remove(Lookup_Key e)
{
    u32 index = remove_entry(l_t_ENTITY, e);
    if (index == 0)
    {
        return;
    }
    if (index != num_entities - 1)
    {
        Entity_Movement* update_pos_move = d_in_ENTITY.movements + index;
        Entity_Misc* update_pos_misc = d_in_ENTITY.miscs + index;
        *update_pos_move = d_in_ENTITY.movements[num_entities - 1];
        *update_pos_misc = d_in_ENTITY.miscs[num_entities - 1];
        entry_index_cange(l_t_ENTITY, update_pos_misc->id, index);
    }
    num_entities--;
}

Dynamic_Entity_2D entities_iterate(u32* i)
{
    Dynamic_Entity_2D out = { 0 };
    if (++(*i) < num_entities)
    {
        out =
            entity_construct(d_in_ENTITY.movements + (*i), d_in_ENTITY.miscs + (*i));
    }
    return out;
}

Entity_Movement* entities_movement_iterate(u32* i)
{
    Entity_Movement* out = NULL;
    if (++(*i) < num_entities)
    {
        out = d_in_ENTITY.movements + (*i);
    }
    return out;
}

Entity_Movement* dyn_entity_movement_access(Lookup_Key e)
{
    Entity_Movement* out = NULL;
    u32 index = table_index(l_t_ENTITY, e);
    if (index != 0)
    {
        out = d_in_ENTITY.movements + index;
    }
    return out;
}

Dynamic_Entity_2D dyn_entity_access(Lookup_Key e)
{
    Dynamic_Entity_2D out = { 0 };
    u32 index = table_index(l_t_ENTITY,e);
    if (index != 0)
    {
        Entity_Movement* move = d_in_ENTITY.movements + index;
        Entity_Misc* misc = d_in_ENTITY.miscs + index;
        out = entity_construct(move, misc);
    }
    return out;
}

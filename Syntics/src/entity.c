#ifndef SY_UNIT_BUILD
#include "entity.h"
#include "defines.h"
#include "logging.h"
#include "region_alloc.h"
#include "lookup_table.h"
#endif

Dynamic_Entity_2D entity_2d_construct(Entity_Movement_2D* move,
                                      Entity_Misc_2D* misc)
{
    Dynamic_Entity_2D out;
    out.movement = move;
    out.misc = misc;
    return out;
}

void entity_2d_init(Region_Alloc* region, u32 max_static_entities,
                    u32 max_dynamic_entities, Entity_State_2D* entity_state)
{
    if (max_static_entities)
    {
        entity_state->static_table =
            lookup_table_create(region, max_static_entities);
        entity_state->static_entities =
            region_array_calloc(region, max_static_entities, Static_Entity_2D);
    }
    if (max_dynamic_entities)
    {
        entity_state->dynamic_table =
            lookup_table_create(region, max_dynamic_entities);
        entity_state->movement = region_array_calloc(
            region, max_dynamic_entities, Entity_Movement_2D);
        entity_state->misc =
            region_array_calloc(region, max_dynamic_entities, Entity_Misc_2D);
    }
}

Lookup_Key entity_dynamic_2d_add(Entity_State_2D* state)
{
    Array_Head* head = array_head(state->movement);
    assert(head->size < head->capacity);

    Entity_Movement_2D new_move = { 0 };
    Entity_Misc_2D new_misc = { 0 };

    Lookup_Key out = entry_add(&state->dynamic_table, head->size);
    new_misc.id = out._row.index;

    array_push(state->movement, new_move);
    array_push(state->misc, new_misc);

    return out;
}

void entity_dynamic_2d_remove(Entity_State_2D* state, Lookup_Key key)
{
    u32 index = entry_remove(&state->dynamic_table, key);
    if (index-- == 0)
    {
        return;
    }
    Array_Head* head = array_head(state->movement);
    if (index != head->size - 1)
    {
        Entity_Movement_2D* update_pos_move =
            array_val_ptr(state->movement, index);
        Entity_Misc_2D* update_pos_misc = array_val_ptr(state->misc, index);
        *update_pos_move = array_val(state->movement, head->size - 1);
        *update_pos_misc = array_val(state->misc, head->size - 1);
        entry_index_change(&state->dynamic_table, update_pos_misc->id, index);
    }
    head->size--;
}

Dynamic_Entity_2D entity_dynamic_2d_iterate(Entity_State_2D* state, u32* i)
{
    Dynamic_Entity_2D out = { 0 };
    u32 size = array_size(state->movement);
    if (++(*i) < size)
    {
        out = entity_2d_construct(state->movement + (*i), state->misc + (*i));
    }
    return out;
}

Entity_Movement_2D* entity_movement_2d_iterate(Entity_State_2D* state, u32* i)
{
    Entity_Movement_2D* out = NULL;
    u32 size = array_size(state->movement);
    if (++(*i) < size)
    {
        out = state->movement + (*i);
    }
    return out;
}

Entity_Movement_2D* entity_movement_2d_access(Entity_State_2D* state,
                                              Lookup_Key key)
{
    Entity_Movement_2D* out = NULL;
    u32 index = table_index(&state->dynamic_table, key);
    if (index-- != 0)
    {
        out = state->movement + index;
    }
    return out;
}

Dynamic_Entity_2D entity_dynamic_2d_access(Entity_State_2D* state,
                                           Lookup_Key key)
{
    Dynamic_Entity_2D out = { 0 };
    u32 index = table_index(&state->dynamic_table, key);
    if (index-- != 0)
    {
        Entity_Movement_2D* move = state->movement + index;
        Entity_Misc_2D* misc = state->misc + index;
        out = entity_2d_construct(move, misc);
    }
    return out;
}

Dynamic_Entity_3D entity_3d_construct(Entity_Movement_3D* move,
                                      Entity_Animation_3D* animation,
                                      Entity_Misc_3D* misc)
{
    Dynamic_Entity_3D out;
    out.movement = move;
    out.animation = animation;
    out.misc = misc;
    return out;
}

void entity_3d_init(Region_Alloc* region, u32 max_static_entities,
                    u32 max_dynamic_entities, Entity_State_3D* entity_state)
{
    if (max_static_entities)
    {
        entity_state->static_table =
            lookup_table_create(region, max_static_entities);
        entity_state->static_entities =
            region_array_calloc(region, max_static_entities, Static_Entity_3D);
    }
    if (max_dynamic_entities)
    {
        entity_state->dynamic_table =
            lookup_table_create(region, max_dynamic_entities);
        entity_state->movements = region_array_calloc(
            region, max_dynamic_entities, Entity_Movement_3D);
        entity_state->animations = region_array_calloc(
            region, max_dynamic_entities, Entity_Animation_3D);
        entity_state->miscs =
            region_array_calloc(region, max_dynamic_entities, Entity_Misc_3D);
    }
}

Lookup_Key entity_dynamic_3d_add(Entity_State_3D* state,
                                 Dynamic_Entity_3D* enity)
{
    Array_Head* head = array_head(state->movements);
    assert(head->size < head->capacity);

    Entity_Movement_3D new_move = { 0 };
    Entity_Animation_3D new_ani = { 0 };
    Entity_Misc_3D new_misc = { 0 };
    if (enity)
    {
        new_move = *enity->movement;
        new_ani = *enity->animation;
        new_misc = *enity->misc;
    }
    Lookup_Key out = entry_add(&state->dynamic_table, head->size);
    new_misc.id = out._row.index;

    array_push(state->movements, new_move);
    array_push(state->animations, new_ani);
    array_push(state->miscs, new_misc);

    return out;
}

void entity_dynamic_3d_remove(Entity_State_3D* state, Lookup_Key key)
{
    u32 index = entry_remove(&state->dynamic_table, key);
    if (index-- == 0)
    {
        return;
    }
    if (index != array_size(state->movements) - 1)
    {
        Entity_Movement_3D* update_pos_move =
            array_val_ptr(state->movements, index);
        Entity_Animation_3D* update_pos_ani =
            array_val_ptr(state->animations, index);
        Entity_Misc_3D* update_pos_misc = array_val_ptr(state->miscs, index);
        *update_pos_move = array_pop(state->movements);
        *update_pos_ani = array_pop(state->animations);
        *update_pos_misc = array_pop(state->miscs);
        entry_index_change(&state->dynamic_table, update_pos_misc->id, index);
    }
    else
    {
        array_head(state->movements)->size--;
        array_head(state->animations)->size--;
        array_head(state->miscs)->size--;
    }
}

Dynamic_Entity_3D entity_dynamic_3d_iterate(Entity_State_3D* state, u32 i)
{
    Dynamic_Entity_3D out = { 0 };
    u32 size = array_size(state->movements);
    if (i < size)
    {
        out = entity_3d_construct(state->movements + i, state->animations + i,
                                  state->miscs + i);
    }
    return out;
}

Entity_Movement_3D* entity_movement_3d_iterate(Entity_State_3D* state, u32 i)
{
    Entity_Movement_3D* out = NULL;
    u32 size = array_size(state->movements);
    if (i < size)
    {
        out = state->movements + i;
    }
    return out;
}

Entity_Animation_3D* entity_animation_3d_iterate(Entity_State_3D* state, u32 i)
{
    Entity_Animation_3D* out = NULL;
    u32 size = array_size(state->animations);
    if (i < size)
    {
        out = state->animations + i;
    }
    return out;
}

Entity_Movement_3D* entity_movement_3d_access(Entity_State_3D* state,
                                              Lookup_Key key)
{
    Entity_Movement_3D* out = NULL;
    u32 index = table_index(&state->dynamic_table, key);
    if (index-- != 0)
    {
        out = state->movements + index;
    }
    return out;
}

Dynamic_Entity_3D entity_dynamic_3d_access(Entity_State_3D* state,
                                           Lookup_Key key)
{
    Dynamic_Entity_3D out = { 0 };
    u32 index = table_index(&state->dynamic_table, key);
    if (index-- != 0)
    {
        Entity_Movement_3D* move = state->movements + index;
        Entity_Animation_3D* ani = state->animations + index;
        Entity_Misc_3D* misc = state->miscs + index;
        out = entity_3d_construct(move, ani, misc);
    }
    return out;
}

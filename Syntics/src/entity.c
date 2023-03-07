#include "entity.h"
#include "defines.h"
#include "logging.h"
#include "region_alloc.h"
#include <string.h>

#define MAX_ENTITIES 1000

#if 0
typedef struct Table_Row
{
    u32 id;
    u32 index;
} Table_Row;

typedef struct Look_Up_Table
{
    Table_Row* d_entries;
    Table_Row* s_entries;
} Look_Up_Table;
#endif

typedef struct Internal_Entity
{
    b32 should_update;
    u32* index_array;
} Internal_Entity;

typedef struct Internal_S_Entity
{
    Static_Entity* enities;
} Internal_S_Entity;

typedef struct Internal_D_Entity
{
    Dynamic_Entity_2D* enities;
} Internal_D_Entity;

static Internal_Entity g_index_a = { 0 };

// Keeps track of entities that have a active reference to the entity. The idea is
// that after a while all entities that ref anothor will unref it before it gets
// reused. This will change when i have more time to think about a better solution.
// Good enough for now.
static u32* g_ref_count = NULL;

static u32* g_free_indices = NULL;

static Internal_S_Entity g_s_in = { 0 };
static Internal_D_Entity g_d_in = { 0 };
static u32 num_entities = 0;
static u32 end_point = 0;

void init_entity(Region_Alloc* region)
{
    g_index_a.should_update = true;
    g_index_a.index_array = dyn_arrayP(region, MAX_ENTITIES, u32);
    memset(g_index_a.index_array, 0, MAX_ENTITIES * sizeof(*g_index_a.index_array));

    g_free_indices = dyn_arrayP(region, MAX_ENTITIES, u32);
    memset(g_free_indices, 0, MAX_ENTITIES * sizeof(*g_free_indices));

    g_s_in.enities = dyn_arrayP(region, MAX_ENTITIES, Static_Entity);
    g_d_in.enities = dyn_arrayP(region, MAX_ENTITIES, Dynamic_Entity_2D);
    g_ref_count = dyn_arrayP(region, MAX_ENTITIES, u32);
    memset(g_ref_count, 0, MAX_ENTITIES * sizeof(*g_ref_count));
}

void update_dyn_etities()
{
}

Dynamic_Entity_2D* add_dyn_entity()
{
    ASSERT(end_point < MAX_ENTITIES, "add_dyn_entity");

    Dynamic_Entity_2D new = { 0 };
    Dynamic_Entity_2D* out = NULL;

    u32 free_size = size_arr(g_free_indices);
    b32 nothing_is_free = true;
    if (free_size)
    {
        for_range(i, free_size)
        {
            u32 free_idx = g_free_indices[i];
            if (g_ref_count[free_idx] == 0)
            {
                new.alive = true;
                new.id = true;
                g_d_in.enities[free_idx] = new;
                out = &g_d_in.enities[free_idx];
                nothing_is_free = false;
                get_head(g_free_indices)->size--;
                break;
            }
        }
    }
    if (nothing_is_free)
    {
        new.alive = true;
        new.id = num_entities;
        g_d_in.enities[end_point] = new;
        out = &g_d_in.enities[end_point++];
    }
    if (out)
    {
        num_entities++;
    }
    return out;
}

void remove_dyn_entity(Dynamic_Entity_2D* e)
{
    ASSERT(e, "remove_dyn_entity");

    if (e)
    {
        synt_push(g_free_indices, e->id);
        e->alive = false;
        g_index_a.should_update = true;
        num_entities--;
    }
}

u32 ref_dyn_entity(Dynamic_Entity_2D* e)
{
    ASSERT(e && e->alive, "Dyn_Entity_Ref");

    if (e && e->alive)
    {
        g_ref_count[e->id]++;
    }
    return e->id;
}

void drop_dyn_entity_ref(Dynamic_Entity_2D* e)
{
    g_ref_count[e->id]--;
}

Dynamic_Entity_2D* access_dyn_entity(u32 key)
{
    Dynamic_Entity_2D* out = NULL;
    Dynamic_Entity_2D* e = &g_d_in.enities[key];
    if (e->alive)
    {
        out = e;
    }
    else
    {
        drop_dyn_entity_ref(e);
    }
    return out;
}

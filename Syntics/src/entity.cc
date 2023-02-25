#include "entity.h"
#include "region_alloc.h"

#define MAX_ENTITIES 10000

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
    Dynamic_Entity* enities;
} Internal_D_Entity;

static Internal_Entity g_in = {};
static Internal_S_Entity g_S_in = {};
static Internal_D_Entity g_D_in = {};

void init_entity(Region_Alloc* region)
{
    g_in.should_update = true; 
    g_in.index_array = dyn_arrayP(region, MAX_ENTITIES, u32);
    g_S_in.enities = dyn_arrayP(region, MAX_ENTITIES, Static_Entity);
    g_D_in.enities = dyn_arrayP(region, MAX_ENTITIES, Dynamic_Entity);
}

void update_etities()
{
}

void add_entity()
{
}

void remove_entity()
{
}

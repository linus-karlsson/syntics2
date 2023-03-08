#pragma once
#include "math/vectors.h"

typedef struct Static_Entity
{
    V3 pos;
    V2 size;
    u32 id;
    b32 alive;
} Static_Entity;

typedef struct Dynamic_Entity_2D
{
    V2 pos;
    V2 size;
    V2 vel;
    f32 z;
    f32 speed; // TODO: why are you here!?!?
    b32 alive;
    u32 id;
} Dynamic_Entity_2D;

void init_entity(Region_Alloc* region);
void update_dyn_etities();
Dynamic_Entity_2D* add_dyn_entity();
void remove_dyn_entity(Dynamic_Entity_2D* e);
void drop_dyn_entity_ref(Dynamic_Entity_2D* e);
u32 ref_dyn_entity(Dynamic_Entity_2D* e);

Dynamic_Entity_2D* iterate_entities(u32* i);

Dynamic_Entity_2D* access_dyn_entity(u32 key);

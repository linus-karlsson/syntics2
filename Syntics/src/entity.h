#pragma once
#include "math/vectors.h"

typedef struct Static_Entity
{
    V3 pos;
    V2 size;
    u32 id;
} Static_Entity;

typedef struct Entity_Movement
{
    V2 pos;
    V2 vel;
} Entity_Movement;

typedef struct Entity_Misc
{
    V2 size;
    f32 z;
    f32 speed;
    u32 id;
} Entity_Misc;

typedef struct Dynamic_Entity_2D
{
    Entity_Movement* movement;
    Entity_Misc* misc;
} Dynamic_Entity_2D;


typedef struct Lookup_Key {
    u32 _table_index;
    u32 _ref_value; 
} Lookup_Key;

void init_entity(Region_Alloc* region);
void update_dyn_etities(void);
Lookup_Key add_dyn_entity(void);
void remove_dyn_entity(Lookup_Key e);
Lookup_Key ref_dyn_entity(Lookup_Key e);

Dynamic_Entity_2D iterate_entities(u32* i);

Entity_Movement* iterate_entity_movement(u32* i);

Entity_Movement* access_dyn_entity_movement(Lookup_Key e);

Dynamic_Entity_2D access_dyn_entity(Lookup_Key e);


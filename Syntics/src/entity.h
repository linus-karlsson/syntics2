#pragma once
#include "math/vectors.h"

typedef struct Region_Alloc Region_Alloc;
// TODO: Will impl when i need this
typedef struct Static_Entity
{
    V3 pos;
    V2 size;
} Static_Entity;

typedef struct Dynamic_Entity
{
    V3 pos;
    V3 vel;
    f32 speed;
} Dynamic_Entity;

void init_entity(Region_Alloc* region);
void update_etities();
void add_entity();
void remove_entity();

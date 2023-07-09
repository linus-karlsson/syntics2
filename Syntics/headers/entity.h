#pragma once

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

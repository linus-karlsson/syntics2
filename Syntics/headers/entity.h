#pragma once

typedef struct Static_Entity_2D
{
    V2 pos;
    V2 size;
    u32 id;
} Static_Entity_2D;

typedef struct Entity_Movement_2D
{
    V2 pos;
    V2 vel;
} Entity_Movement_2D;

typedef struct Entity_Misc_2D
{
    V2 size;
    f32 speed;
    u32 id;
} Entity_Misc_2D;

typedef struct Dynamic_Entity_2D
{
    Entity_Movement_2D* movement;
    Entity_Misc_2D* misc;
} Dynamic_Entity_2D;

typedef struct Entity_State_2D
{
    Lookup_Table static_table;
    Lookup_Table dynamic_table;

    Static_Entity_2D* static_entities;

    Entity_Movement_2D* movement;
    Entity_Misc_2D* misc;

} Entity_State_2D;

typedef struct Static_Entity_3D
{
    V3 pos;
    V3 size;
    u32 id;
} Static_Entity_3D;

typedef struct Entity_Movement_3D
{
    V3 pos;
    V3 vel;
} Entity_Movement_3D;

typedef struct Entity_Misc_3D
{
    V3 size;
    f32 speed;
    u32 id;
} Entity_Misc_3D;

typedef struct Dynamic_Entity_3D
{
    Entity_Movement_3D* movement;
    Entity_Misc_3D* misc;
} Dynamic_Entity_3D;

typedef struct Entity_State_3D
{
    Lookup_Table static_table;
    Lookup_Table dynamic_table;
    Static_Entity_3D* static_entities;

    Entity_Movement_3D* movement;
    Entity_Misc_3D* misc;
} Entity_State_3D;


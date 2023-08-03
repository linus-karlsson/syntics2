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

typedef struct Entity_Animation_3D
{
    M4 dude_models[3];
    f32 left_leg_rotation_angle;
    f32 right_leg_rotation_angle;
    f32 dude_rotation_angle;
    f32 leg_rotation_speed;
    f32 dude_rotation_speed;
    f32 stop_animation_sec;
    f32 angle;
    f32 sec_off_ground;
    b32 off_the_ground;
    b32 reset;
}Entity_Animation_3D;

typedef struct Entity_Movement_3D
{
    V3 pos;
    V3 vel;
    V3 acc;
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
    Entity_Animation_3D* animation;
    Entity_Misc_3D* misc;
} Dynamic_Entity_3D;

typedef struct Entity_State_3D
{
    Lookup_Table static_table;
    Lookup_Table dynamic_table;
    Static_Entity_3D* static_entities;

    Entity_Movement_3D* movements;
    Entity_Animation_3D* animations;
    Entity_Misc_3D* miscs;
} Entity_State_3D;


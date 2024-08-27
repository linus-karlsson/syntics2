#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#include "math/syntics_math.h"
#include "lookup_table.h"
#endif

typedef struct Static_Entity_2D
{
    V2 position;
    V2 size;
    u32 id;
} Static_Entity_2D;

typedef struct Entity_Movement_2D
{
    V2 position;
    V2 velocity;
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
    V3 position;
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
} Entity_Animation_3D;

typedef struct Entity_Movement_3D
{
    V3 position;
    V3 velocity;
    V3 acceleration;
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

void                 entity_2d_init(Region_Alloc* region, u32 max_static_entities, u32 max_dynamic_entities, Entity_State_2D* entity_state);
Dynamic_Entity_2D    entity_2d_construct(Entity_Movement_2D* move, Entity_Misc_2D* misc);

Lookup_Key           entity_2d_dynamic_add(Entity_State_2D* state);
Dynamic_Entity_2D    entity_2d_dynamic_access(Entity_State_2D* state, Lookup_Key key);
Dynamic_Entity_2D    entity_2d_dynamic_iterate(Entity_State_2D* state, u32* i);
void                 entity_2d_dynamic_remove(Entity_State_2D* state, Lookup_Key key);

Entity_Movement_2D*  entity_2d_movement_access(Entity_State_2D* state, Lookup_Key key);
Entity_Movement_2D*  entity_2d_movement_iterate(Entity_State_2D* state, u32* i);

void                 entity_3d_init(Region_Alloc* region, u32 max_static_entities, u32 max_dynamic_entities, Entity_State_3D* entity_state);
Dynamic_Entity_3D    entity_3d_construct(Entity_Movement_3D* move, Entity_Animation_3D* animation, Entity_Misc_3D* misc);

Lookup_Key           entity_3d_dynamic_add(Entity_State_3D* state, Dynamic_Entity_3D* enity);
Dynamic_Entity_3D    entity_3d_dynamic_access(Entity_State_3D* state, Lookup_Key key);
Dynamic_Entity_3D    entity_3d_dynamic_iterate(Entity_State_3D* state, u32 i);
void                 entity_3d_dynamic_remove(Entity_State_3D* state, Lookup_Key key);

Entity_Movement_3D*  entity_3d_movement_access(Entity_State_3D* state, Lookup_Key key);
Entity_Movement_3D*  entity_3d_movement_iterate(Entity_State_3D* state, u32 i);
Entity_Animation_3D* entity_3d_animation_iterate(Entity_State_3D* state, u32 i);


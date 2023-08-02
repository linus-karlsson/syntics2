#pragma once

#define TOTAL_ENTITY_TYPES 10

typedef struct Entity_Function
{
    void (*entity_update)(void* data);
} Entity_Function;

typedef struct Entity
{
    u32 id;
    V3 pos;
    V3 velocity;
} Entity;

typedef struct Blob
{
    Entity entity;
    f32 fluid_varient;
} Blob;

typedef struct Entity_Array
{
    Entity_Function functions[TOTAL_ENTITY_TYPES];
    u32 size;
    Entity entities[10];
} Entity_Array;

typedef struct AABB_Representation
{
    AABB_3D aabb;
} AABB_Representation;

typedef struct Thread_Attrib_Terrain
{
    u32 index;
    Vertex* verts;
} Thread_Attrib_Terrain;

typedef struct Thread_Attrib_Grass
{
    u32 index;
    u32 seed;
    V3* positions;
    Vertex_Array vertex_array;
    Vertex_Array model_vertices;
    U32_Array indices_array;
    U32_Array model_indices;
} Thread_Attrib_Grass;

typedef struct Thread_Attrib_Gui 
{
    HANDLE start_semaphore;
    HANDLE end_semaphore;
    Gui_Context* ctx;
    const Application_State* app_state;
    V2 dimensions;
    f32 dt;
    u32 semaphore_idx;
}Thread_Attrib_Gui;

typedef struct Game_State
{
    Graphic_Pipeline triangle_strip_pipeline;
    Graphic_Pipeline triangle_list_pipeline;
    Graphic_Pipeline line_list_pipeline;

    Graphic_Pipeline grass_pipeline;

    Vertex_Index_Buffer terrain_vert_idx;
    Vertex_Index_Buffer road_vert_idx;
    Vertex_Index_Buffer road_line_vert_idx;
    Vertex_Index_Buffer car_vert_idx;
    Vertex_Index_Buffer particles_vert_idx;
    Vertex_Index_Buffer aabb_rep;
    Vertex_Index_Buffer tree_vert_idx;

    Vertex_Index_Buffer grass_vert_idx;
    V3* grass_pos_offset_cache;
    u32 grass_vert_count;
    f32 offset_p;

    AABB_Representation car_aabb;
    Rect3D* rects;

    Entity_State_3D entity_state;

    Camera_3D cam;
    M4 global_model;

    M4 grass_model;

    V3 road_pos;
    M4 road_model;

    Lookup_Key dude;
    M4 dude_models[3];

    Texture* textures;
    Font font;
    Events* mouse_evt;

    Particles_3D particles;
    u32 particle_vert_offset;

    Gui_Context gui_ctx;
    Thread_Attrib_Gui gui_thread; 
    HANDLE gui_thread_handle;
    Window_Handle* win_handles;

} Game_State;

typedef struct Cube
{
    Vertex verts[8];
} Cube;


typedef struct Cubic_Bezier_Curve
{
    V3 p[4];
    u32 points_indices[4];
    u32 vertex_offset;
} Cubic_Bezier_Curve;

typedef struct Bezier_Spline
{
    Cubic_Bezier_Curve* bc;
    u32 n_curves;
    u32 splitt;
} Bezier_Spline;

typedef struct Bezier_Spline_3D
{
    Cubic_Bezier_Curve* bc[2];
    u32 n_curves;
    u32 splitt;
} Bezier_Spline_3D;

// Hash Table
typedef struct Node_U32 Node_U32;
struct Node_U32
{
    Node_U32* next;
    V3 key;
    u32 value;
    u32 active;
};

typedef struct Hash_Table_U32
{
    Node_U32* values;
    u32 capacity;

    Node_U32* collision_buffer;
    u32 collision_buffer_size;
    u32 collision_buffer_capacity;
} Hash_Table_U32;

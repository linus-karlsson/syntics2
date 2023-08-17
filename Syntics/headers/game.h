#pragma once

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
    u32 grass_count;
    u32 vertex_offset;

    Vertex* vertex_array;
    u32* indices_array;

    const V2* positions;
    const Vertex_Array* model_vertices;
    const U32_Array* model_indices;
} Thread_Attrib_Grass;

typedef struct Index_Offset_Render
{
    u32 idx;
    u32 idx_size;
}Index_Offset_Render;

typedef struct Float_Gui
{
    const char* name;
    f32* value;
    f32 min;
    f32 max;
}Float_Gui;

typedef struct Game_State
{
    VkPipelineLayout pipeline_layout;
    VkDescriptorSetLayout descriptor_set_layout;

    Buffer* uniform_buffers;
    Descriptors descriptors;

    VkPipeline triangle_strip_pipeline;
    VkPipeline triangle_list_pipeline;
    VkPipeline line_list_pipeline;
    VkPipeline grass_pipeline;

    Vertex_Index_Buffer vert_idx_buffer;

    Index_Offset_Render terrain_offsets;
    Index_Offset_Render dude_offsets;
    Index_Offset_Render tree_offsets;
    Index_Offset_Render sign_offsets;
    Index_Offset_Render grass_offsets;

    Index_Offset_Render particles_offsets;
    Buffer particles_staging_buffer;
    Vertex_Array particles_vert_array;
    
    f32* particles_arc_offsets;

#if 0
    Vertex_Index_Buffer terrain_vert_idx;
    Vertex_Index_Buffer car_vert_idx;
    Vertex_Index_Buffer tree_vert_idx;
    Vertex_Index_Buffer sign_vert_idx;
    Vertex_Index_Buffer grass_vert_idx;
#endif

    Vertex_Index_Buffer road_vert_idx;
    Vertex_Index_Buffer road_line_vert_idx;
    Vertex_Index_Buffer aabb_rep;

    Float_Gui* float_guis;

    V3* grass_pos_offset_cache;
    u32 grass_vert_count;
    f32 offset_p;

    Window_Handle* win_handles;

    AABB_Representation car_aabb;
    Rect3D* rects;

    Entity_State_3D entity_state;

    Camera_3D cam;
    M4 global_model;

    Push_Constant* sign_constants;

    M4 grass_model;

    V3 road_pos;
    M4 road_model;

    Lookup_Key dude;
    Lookup_Key dude2;

    Cubic_Bezier_Curve boom_curve;

    Texture* textures;
    Font font;
    Events* mouse_evt;
    Events* wheel_evt;

    Particles_3D particles;
    u32 particle_vert_offset;

    AABB_3D sign_aabb;
    AABB_3D sign_aabb_text;
    AABB_3D sign_aabb_yes;
    AABB_3D sign_aabb_no;

    V2 dimensions;

    b32 should_update;
    u32 aabb_count;
    u32 aabb_indices_count;

} Game_State;

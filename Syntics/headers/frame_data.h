#pragma once

typedef struct Render_Task
{
    void (*callback)(void* data, VkCommandBuffer command_buffer,
                          u32 semaphore_idx);
    void* data;
} Render_Task;

typedef struct Frame_Data
{
    Semaphore render_counter;

    Region_Alloc frame_region;

    Render_Task* copy_tasks;
    Render_Task* render_tasks;

    u32 id;

    V2 dimensions;
    u32 semaphore_idx;
    f32 dt;

    f32 game_offset_p_grass;
    VP game_cam_vp;
    Push_Constant* game_sign_constants;
    M4** game_dude_models;
    M4 game_arc_model;

    u32 game_aabb_count;
    u32 game_aabb_indices_count;

    VkPipelineLayout game_pipeline_layout;
    VkDescriptorSetLayout game_descriptor_set_layout;
    Buffer* game_uniform_buffers;
    Descriptors* game_descriptors;

    VkPipeline game_triangle_strip_pipeline;
    VkPipeline game_triangle_list_pipeline;
    VkPipeline game_line_list_pipeline;
    VkPipeline game_grass_pipeline;

    Vertex_Index_Buffer game_vert_idx_buffer;

    Vertex_Index_Buffer game_road_vert_idx;
    Vertex_Index_Buffer game_road_line_vert_idx;
    Vertex_Index_Buffer game_aabb_rep;

    Index_Offset_Render game_terrain_offsets;
    Index_Offset_Render game_dude_offsets;
    Index_Offset_Render game_tree_offsets;
    Index_Offset_Render game_sign_offsets;
    Index_Offset_Render game_grass_offsets;

    Index_Offset_Render game_particles_offsets;
    u32 game_particle_count;
    Buffer game_particles_staging_buffer;

    VP gui_cam_vp;

    Ui_Window_Render* gui_windows;
    Terminal_Render gui_terminal;
    u32 gui_blue_rects_index_offset;
    u32 gui_docking_display_quad_count;

    VkPipelineLayout gui_pipeline_layout;
    VkDescriptorSetLayout gui_descriptor_set_layout;
    Buffer* gui_uniform_buffers;
    Descriptors* gui_descriptors;

    VkPipeline gui_triangle_list_pipeline;

    Buffer gui_main_vert_staging_buffer;
    Buffer gui_terminal_vert_staging_buffer;

    Vertex_Index_Buffer gui_main_vert_idx;
    Vertex_Index_Buffer gui_terminal_vert_idx;
}Frame_Data;

typedef struct Game_Logic {

    Application_State* app_state;
    Gui_Context* gui_ctx;
    Game_State* game_state;
    Frame_Data* frame;
}Game_Logic;

typedef struct Render_Logic {

    Application_State* app_state;
    Render_State* render_state;
    Frame_Data* frame;
}Render_Logic;

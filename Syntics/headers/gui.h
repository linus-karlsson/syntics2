#pragma once

#define TOTAL_DOCK_HIT_GUI 3

typedef void* Window_Handle;

typedef struct Input
{
    u32 curr_index;
    u32 buffer_size;
    u32 frames_moved;

    f32 max;
    f32 min;
    f32 time;

    b8 presist_clicked;
    b8 presist_hold;
    b8 highlight_on;
} Input;

typedef struct Input_Text
{
    Input input;
    char text[100];
    char last_text[100];
} Input_Text;

typedef struct Input_Float
{
    Input input;
    char text[15];
    char last_text[15]; 
} Input_Float;

typedef struct Gridd
{
    f32 dimensions[2];
} Gridd;

typedef struct Hover_Clicked_Index
{
    u32 hover;
    u32 clicked;
} Hover_Clicked_Index;

typedef struct Hover_Clicked
{
    b8 clicked;
    b8 hover;
} Hover_Clicked;

typedef struct Gui_Context Gui_Context;

typedef struct Ui_Window_Render 
{
    VkRect2D scissor;
    u32 index_offset;
    u32 num_indices;
    b32 win_show;
    b32 win_terminal;
}Ui_Window_Render;

typedef struct Terminal_Render
{
    VkRect2D scissor;
    u32 num_indices;
}Terminal_Render;

typedef struct Ui_Window
{
    f32* translucentcy;
    V4 font_color;

    const Gui_Context* _const_gui_ctx;
    Vertex_Array _vertex_array;
    
    AABB_2D* _aabbs;

    Input_Float _input_floats[20];
    Input_Text _input_texts[10];

    VkRect2D _scissor;
    Gridd _gridd;

    u32 _id;
    u32 _window_index;

    u32 _input_f32_index;
    u32 _input_text_index;
    u32 _index_offset;
    u32 _num_indices;
    u32 _extra_hight;
    u32 _highest_high;

    V2 _dimensions;
    V2 _g;
    V2 _start;
    V2 _offset;
    V2 _presist_offset;
    V2 _size_cache;

    f32 _biggest_wide;
    f32 _last_button_width;

    b8 _flags;
    b8 _is_holding;
    b8 _docked;
    b8 _recreate;
    b8 _show;
    b8 _active;
} Ui_Window;

struct Gui_Context
{
    f32 dt;
    f32 translucentcy;

    V2 dimensions;
    V2 mouse_pos;
    V4 font_color;
    Font font;

    Region_Alloc* region;
    Events* mouse_evt;
    Events* wheel_evt;
    Events* key_evt;

    const Swap_Chain_Attrib* _const_swap_chain;
    const Platform* _const_platform;
    VkDevice _device;

    VkPipelineLayout pipeline_layout;
    VkDescriptorSetLayout descriptor_set_layout;
    Buffer* uniform_buffers;
    Descriptors descriptors;

    VkPipeline _triangle_list_pipeline;

    Vertex_Array _main_vert_array;
    Vertex_Array _terminal_vert_array;

    //Vertex_Index_Buffer _main_vert_idx;
    //Vertex_Index_Buffer _graph_vert_idx;
    //Vertex_Index_Buffer _terminal_vert_idx;

    Camera_3D _cam;

    Vertex_Array _docking_display_vertex_array;

    u32 _wins_count;
    u32 _num_wins_frame;
    u32 _win_hold_idx;
    u32 _win_dock_hit_idx;
    u32 _blue_rects_index_offset;
    u32 _resize_idx;
    u32 _window_event_index;
    u32 _docking_display_quad_count;
    u32 _entity_open_idx;

    Hover_Clicked_Index _hover_clicked_index;

    b8 _top_bar_presist_hold;
    b8 _dock_hit[TOTAL_DOCK_HIT_GUI];

    Texture* _textures;

    Ui_Window* _ui_wins;
    u32* _render_order;

    Lookup_Table* _lookup_table;
    Lookup_Key* _win_handles;
    u32* _free_handles;

    AABB_2D _blue_rects[TOTAL_DOCK_HIT_GUI];
    AABB_2D _dock_resized_rect;

    VkRect2D _graph_scissor;
};

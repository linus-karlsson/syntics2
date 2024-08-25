#pragma once
#ifndef SY_UNIT_BUILD
#include "vulkan_types.h"
#include "font.h"
#include "camera.h"
#include "logging.h"
#include "lookup_table.h"
#include "collision.h"
#endif

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
    b8 clicked : 1;
    b8 hover : 1;
} Hover_Clicked;

typedef struct Gui_Context Gui_Context;

typedef struct Ui_Window_Render
{
    VkRect2D scissor;
    u32 index_offset;
    u32 num_indices;
    b8 win_show : 1;
    b8 win_terminal : 1;
} Ui_Window_Render;

typedef struct Terminal_Render
{
    VkRect2D scissor;
    u32 num_indices;
} Terminal_Render;

typedef struct Ui_Window
{
    f32* translucentcy;
    V4 font_color;

    const Gui_Context* p_const_gui_ctx;
    Vertex_Array p_vertex_array;

    AABB_2D* p_aabbs;

    Input_Float p_input_floats[20];
    Input_Text p_input_texts[10];

    VkRect2D p_scissor;
    Gridd p_gridd;

    u64 id_pressed;

    u32 p_id;
    u32 p_window_index;

    u32 p_input_f32_index;
    u32 p_input_text_index;

    u32 p_index_offset;
    u32 p_num_indices;

    u32 p_extra_hight;
    u32 p_highest_high;

    V2 p_dimensions;
    V2 p_g;
    V2 p_start;
    V2 p_offset;
    V2 p_presist_offset;
    V2 p_size_cache;

    f32 p_biggest_wide;
    f32 p_last_button_width;

    b8 p_win_retracted : 1;
    b8 p_win_first : 1;
    b8 p_win_gridd_start : 1;
    b8 p_win_dyn_resize : 1;
    b8 p_win_presist_hold : 1;
    b8 p_win_resize_hold : 1;
    b8 p_win_term : 1;
    b8 p_win_graph : 1;

    b8 p_is_holding : 1;
    b8 p_docked : 1;
    b8 p_recreate : 1;
    b8 p_show : 1;
    b8 p_active : 1;
} Ui_Window;

struct Gui_Context
{
    f32 dt;
    f32 translucentcy;

    V2 dimensions;
    V2 mouse_pos;
    V4 font_color;
    Font_TTF font;

    Region_Alloc* region;
    Events* mouse_evt;
    Events* wheel_evt;
    Events* key_evt;

    const Swap_Chain_Attrib* p_const_swap_chain;
    const Platform* p_const_platform;
    VkDevice p_device;

    VkPipelineLayout pipeline_layout;
    VkDescriptorSetLayout descriptor_set_layout;
    Buffer* uniform_buffers;
    Descriptors descriptors;

    VkPipeline p_triangle_list_pipeline;

    Vertex_Array p_main_vert_array;
    Vertex_Array p_terminal_vert_array;

    Camera_3D p_cam;

    Vertex_Array p_docking_display_vertex_array;

    u32 p_wins_count;
    u32 p_num_wins_frame;
    u32 p_win_hold_idx;
    u32 p_win_dock_hit_idx;
    u32 p_blue_rects_index_offset;
    u32 p_resize_idx;
    u32 p_window_event_index;
    u32 p_docking_display_quad_count;
    u32 p_entity_open_idx;

    Hover_Clicked_Index p_hover_clicked_index;

    b8 p_dock_hit[TOTAL_DOCK_HIT_GUI];
    b8 p_top_bar_presist_hold;

    Texture* p_textures;

    Ui_Window* p_ui_wins;
    u32* p_render_order;

    Lookup_Table* p_lookup_table;
    Lookup_Key* p_win_handles;
    u32* p_free_handles;

    AABB_2D p_blue_rects[TOTAL_DOCK_HIT_GUI];
    AABB_2D p_dock_resized_rect;

    VkRect2D p_graph_scissor;
};

#define gui_window_button_add(win, text) gui_window_button_add_(win, 0, __FILE__, __LINE__, text)
#define gui_window_button_add_id(win, id, text) gui_window_button_add_(win, id, __FILE__, __LINE__, text)

#define gui_window_float_input_add(win, input, min, max, speed) gui_window_float_input_add_(win, 0, __FILE__, __LINE__, input, min, max, speed)
#define gui_window_float_input_add_d(win, input, min, max) gui_window_float_input_add_(win, 0, __FILE__, __LINE__, input, min, max, (max - min) * 0.4f)
#define gui_window_float_input_add_id(win, id, input, min, max, speed) gui_window_float_input_add_(win, id, __FILE__, __LINE__, input, min, max, speed)

#define gui_window_text_input_add(win, ptr_to_text, size) gui_window_text_input_add_(win, 0, __FILE__, __LINE__, ptr_to_text, size)
#define gui_window_text_input_add_id(win, id, ptr_to_text, size) gui_window_text_input_add_(win, id, __FILE__, __LINE__, ptr_to_text, size)

b8   gui_is_focus(void);
void gui_binary_file_save(const Gui_Context* ctx);
void gui_init_frames(VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool, VkQueue graphic_queue, Gui_Frame* frames, u32 frame_count, u32 total_num_wins);
void gui_init(Region_Alloc* region, VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool, VkQueue graphic_queue, const Swap_Chain_Attrib* swap_chain, const Platform* platform, u32 num_semaphores, u32 total_num_wins, b32 use_save, Gui_Context* ctx);
void gui_update_begin(Gui_Context* ctx, V2 dimensions, u32 semaphore_idx, f32 delta);
void gui_update_end(Gui_Context* ctx, Gui_Frame* frame, Render_Task* copy_tasks, Render_Task* render_tasks, Region_Alloc* frame_region);

Window_Handle 
     gui_window_create(Gui_Context* ctx);
Ui_Window* 
     gui_window_begin(Gui_Context* ctx, Window_Handle handle, const char* title, V2 pos);
void gui_window_free(Gui_Context* ctx, Window_Handle handle);

void gui_window_end(Ui_Window** win);
void gui_window_gridd_begin(Ui_Window* win, u32 x, u32 y);
void gui_window_gridd_end(Ui_Window* win);

b8   gui_window_button_add_(Ui_Window* win, u64 id, const char* file, int line, const char* text);
b8   gui_window_float_input_add_(Ui_Window* win, u64 id, const char* file, int line, f32* input, f32 min, f32 max, f32 speed);
b8   gui_window_text_input_add_(Ui_Window* win, u64 id, const char* file, int line, char* ptr_to_text, u32* size);
void gui_window_text_add(Ui_Window* win, const char* text);

void gui_terminal_add(Gui_Context* ctx, Terminal_Attrib* term, Ui_Window* win, f32 width, f32 height);
void gui_destroy(Gui_Context* ctx, VkDevice device, u32 num_semaphores);


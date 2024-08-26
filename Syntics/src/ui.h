#pragma once
#ifndef SY_UNIT_BUILD
#include "vulkan_types.h"
#include "font.h"
#include "logging.h"
#include "lookup_table.h"
#include "collision.h"
#endif

#define UI_DEFAULT_TEXTURE 0
#define UI_FONT_TEXTURE 1
#define UI_COLOR_PICKER_TEXTURE 2

#define DOCK_SIDE_RIGHT 0
#define DOCK_SIDE_LEFT 1

#define DOCK_SIDE_BOTTOM 0
#define DOCK_SIDE_TOP 1

#define RESIZE_NONE 0
#define RESIZE_RIGHT BIT_1
#define RESIZE_LEFT BIT_2
#define RESIZE_BOTTOM BIT_3
#define RESIZE_TOP BIT_4

#define SCROLL_BAR_NONE 0
#define SCROLL_BAR_HEIGHT BIT_1
#define SCROLL_BAR_WIDTH BIT_2

#define UI_WINDOW_NONE 0
#define UI_WINDOW_TOP_BAR BIT_1
#define UI_WINDOW_RESIZEABLE BIT_2
#define UI_WINDOW_OVERLAY BIT_3
#define UI_WINDOW_FROSTED_GLASS BIT_4
#define UI_WINDOW_DOCKED BIT_5
#define UI_WINDOW_HIDE BIT_6
#define UI_WINDOW_AREA_HIT BIT_7
#define UI_WINDOW_CLOSING BIT_8

typedef struct Index_Array
{
    u32 size;
    u32 capacity;
    u32* data;
} Index_Array;

typedef struct Char_Array
{
    u32 size;
    u32 capacity;
    char* data;
} Char_Array;

typedef struct Char_Ptr_Array
{
    u32 size;
    u32 capacity;
    char** data;
} Char_Ptr_Array;

typedef struct Hover_Clicked_Index
{
    i32 index;
    b8 hover;
    b8 pressed;
    b8 clicked;
    b8 double_clicked;
} Hover_Clicked_Index;

typedef enum Node_Type
{
    NODE_ROOT,
    NODE_PARENT,
    NODE_LEAF
} Node_Type;

typedef enum Split_Axis
{
    SPLIT_NONE,
    SPLIT_HORIZONTAL,
    SPLIT_VERTICAL
} Split_Axis;

typedef struct Dock_Node
{
    Node_Type type;
    Split_Axis split_axis;
    U32_Array windows;
    struct Dock_Node* children[2];
    AABB_2D aabb;

    u32 window_in_focus;
    f32 size_ratio;
} Dock_Node;

typedef struct Ui_Window
{
    Char_Array title;
    Dock_Node* dock_node;

    u32 id;
    V2 size;
    V2 position;

    f32 end_scroll_offset;
    f32 start_scroll_offset;
    f32 scroll_x;
    f32 current_scroll_offset;

    f32 end_scroll_offset_width;
    f32 current_scroll_offset_width;

    f32 scroll_bar_mouse_pointer_offset;
    f32 alpha;

    u8 scroll_bar_dragging;
    u8 flags;
} Ui_Window;

typedef struct Input_Buffer
{
    Char_Array buffer;
    Selection_Character_Array chars;
    Selection_Character_Array chars_selected;

    i32 start_selection_index;
    i32 end_selection_index;

    f32 time;
    f32 selected_pivot_point;
    i32 input_index;
    b8 selected;
    b8 active;
} Input_Buffer;

typedef struct UU_32
{
    u32 first;
    u32 second;
} UU_32;

typedef struct Color_Picker
{
    V2 at;
    f32 spectrum_at;
    b8 hold;
    b8 spectrum_hold;
} Color_Picker;

typedef struct Theme_Color_Picker
{
    Color_Picker secondary_color;
    Color_Picker clear_color;
    Color_Picker text_color;
    Color_Picker tab_color;
    Color_Picker bar_top_color;
    Color_Picker bar_bottom_color;
    Color_Picker border_color;
    Color_Picker scroll_bar_color;
} Theme_Color_Picker;

typedef struct Ui_Layout
{
    V2 at;
    f32 start_x;
    f32 row_height;
    f32 column_width;
    f32 padding;
} Ui_Layout;

Ui_Layout ui_layout_create(V2 at);
void ui_layout_row(Ui_Layout* layout);
void ui_layout_column(Ui_Layout* layout);
void ui_layout_reset_column(Ui_Layout* layout);

Input_Buffer ui_input_buffer_create(void);
void ui_input_buffer_delete(Input_Buffer* input);
void ui_input_buffer_clear_selection(Input_Buffer* input);
char* ui_input_buffer_get_selection_as_string(Input_Buffer* input);
void ui_input_buffer_copy_selection_to_clipboard(Input_Buffer* input);
void ui_input_buffer_erase_from_selection(Input_Buffer* input);

void ui_context_create(VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool, VkQueue graphic_queue, const Swap_Chain_Attrib* swap_chain, const Platform* platform, u32 num_semaphores);
void ui_context_begin(const V2 dimensions, const AABB_2D* dock_space, const f64 delta_time, const b8 check_collisions);
void ui_context_end(Render_Task* copy_tasks, Render_Task* render_tasks);
void ui_context_destroy(void);

void ui_context_set_window_in_focus(const u32 window_id);

f32 ui_get_big_icon_size(void);
void ui_set_big_icon_size(f32 new_size);
void ui_set_big_icon_min_size(f32 new_min);
void ui_set_big_icon_max_size(f32 new_max);
V2 ui_get_big_icon_min_max(void);
void ui_set_list_padding(const f32 padding);
f32 ui_get_list_padding(void);
void ui_set_frosted_glass(b8 on);
f32 ui_get_frosted_blur_amount(void);
void ui_set_frosted_blur_amount(const f32 new_blur_amount);

const Font_TTF* ui_context_get_font(void);
f32 ui_context_get_font_pixel_height(void);
void ui_context_change_font_pixel_height(const f32 pixel_height);
const char* ui_context_get_font_path(void);
void ui_context_set_font_path(const char* new_path);

void ui_context_set_animation(b8 on);
void ui_context_set_highlight_focused_window(b8 on);

void ui_context_set_window_top_color(V4 color);
void ui_context_set_window_bottom_color(V4 color);

u32 ui_window_create(void);
const Ui_Window* ui_window_get(const u32 window_id);
u32 ui_window_in_focus(void);
b8 ui_window_begin(u32 window_id, const char* title, u8 flags);
b8 ui_window_end(b8 reset_textures);

b8 ui_window_is_hit(const u32 window_id);

void ui_window_set_end_scroll_offset(const u32 window_id, const f32 offest);
void ui_window_set_current_scroll_offset(const u32 window_id, const f32 offset);

void ui_window_close(u32 window_id);
void ui_window_close_current(void);

void ui_window_set_size(u32 window_id, const V2 size);
void ui_window_set_position(u32 window_id, const V2 position);

void ui_window_row_begin(const f32 padding);
f32 ui_window_row_end(void);
void ui_window_column_begin(const f32 padding);
f32 ui_window_column_end(void);

void ui_window_start_size_animation(const u32 window_id, const V2 end_size);
void ui_window_start_position_animation(const u32 window_id, const V2 end_position);
void ui_window_dock_space_size(const u32 window_id, const V2 end_size);
void ui_window_dock_space_min(const u32 window_id, const V2 end_position);
void ui_window_set_animation_x(const u32 window_id, const f32 x);

void ui_window_set_alpha(const u32 window_id, const f32 alpha);

b8 ui_window_add_icon_button(V2 position, const V2 size, const V4 hover_color, const V4 texture_coordinates, const f32 texture_index, const b8 disable, Ui_Layout* layout);
V2 ui_window_get_button_dimensions(V2 dimensions, const char* text, f32* x_advance_out);
b8 ui_window_add_button(V2 position, V2* dimensions, const V4* color, const char* text, Ui_Layout* layout);
b8 ui_window_add_input_field(V2 position, const V2 size, Input_Buffer* input, Ui_Layout* layout);
void ui_window_add_text(V2 position, const char* text, b8 scrolling, Ui_Layout* layout);
void ui_window_add_text_c(V2 position, V4 color, const char* text, b8 scrolling, Ui_Layout* layout);
void ui_window_add_text_colored(V2 position, const Colored_Character_Array* text, b8 scrolling, Ui_Layout* layout);
void ui_window_add_image(V2 position, V2 image_dimensions, u32 image, Ui_Layout* layout);
i32 ui_window_add_menu_bar(Char_Ptr_Array* values, V2* position_of_clicked_item);
void ui_window_add_icon(V2 position, const V2 size, const V4 texture_coordinates, const f32 texture_index, Ui_Layout* layout);
V2 ui_window_get_switch_size(void);
void ui_window_add_switch(V2 position, b8* selected, f32* x, Ui_Layout* layout);

f32 ui_window_add_slider(V2 position, V2 size, const f32 min_value, const f32 max_value, f32 value, b8* pressed, Ui_Layout* layout);
V4 ui_window_add_color_picker(V2 position, V2 size, Color_Picker* picker, Ui_Layout* layout);
void ui_window_add_border(V2 position, const V2 size, const V4 color, const f32 thickness);
void ui_window_add_rectangle(V2 position, const V2 size, const V4 color, Ui_Layout* layout);
void ui_window_add_radio_button(V2 position, const V2 size, b8* selected, Ui_Layout* layout);

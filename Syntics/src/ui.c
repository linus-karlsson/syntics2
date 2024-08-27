#ifndef SY_UNIT_BUILD
#include "ui.h"
#include "defines.h"
#include "math/syntics_math.h"
#include "syntics_vulkan.h"
#include "event_system.h"
#include "font.h"
#include "region_alloc.h"
#include "camera.h"
#include "collision.h"
#include "file_reading.h"
#include "ansi_keycodes.h"
#include "noise.h"
#include "render_util.h"
#include "entity.h"
#include "lookup_table.h"
#include "syntics_platform.h"
#include "frame_data.h"
#include "hash.h"
#include "globals.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#endif

global u32 random_seed = 0;

#define MAX_PATH 260

typedef struct Ui_Window_Array
{
    u32 size;
    u32 capacity;
    Ui_Window* data;
} Ui_Window_Array;

typedef struct Hover_Clicked_Index_Array
{
    u32 size;
    u32 capacity;
    Hover_Clicked_Index* data;
} Hover_Clicked_Index_Array;

typedef struct AABB_2D_Array_Array
{
    u32 size;
    u32 capacity;
    AABB_2D_Array* data;
} AABB_2D_Array_Array;

typedef struct Dock
{
    u32 id;
    AABB_2D aabb;
} Dock;

typedef struct Window_Render_Data
{
    u32 id;
    AABB_2D aabb;
    u32 index_offset;
    u32 index_count;
} Window_Render_Data;

typedef struct Window_Render_Data_Array
{
    u32 size;
    u32 capacity;
    Window_Render_Data* data;
} Window_Render_Data_Array;

typedef struct F32_Array
{
    u32 size;
    u32 capacity;
    f32* data;
} F32_Array;

typedef struct Texture_Array
{
    u32 size;
    u32 capacity;
    Texture* data;
} Texture_Array;

typedef struct UU_32_Array
{
    u32 size;
    u32 capacity;
    UU_32* data;
} UU_32_Array;

typedef struct Ui_Context
{
    VP vp;

    Ui_Window* pressed_window;
    V2 window_pressed_offset;
    V2 window_pressed_size_offset;

    u32 window_in_focus;
    u32 current_window_id;

    F32_Array animation_x;

    Window_Render_Data_Array last_frame_windows;
    Window_Render_Data_Array current_frame_windows;

    Window_Render_Data_Array last_frame_docked_windows;
    Window_Render_Data_Array current_frame_docked_windows;

    Window_Render_Data_Array last_frame_overlay_windows;
    Window_Render_Data_Array current_frame_overlay_windows;

    UU_32_Array docked_index_offsets_and_counts;
    UU_32_Array floating_index_offsets_and_counts;
    u32 overlay_index_offset;
    u32 overlay_index_count;

    Texture_Array textures;

    VkPipelineLayout pipeline_layout;
    VkDescriptorSetLayout descriptor_set_layout;
    Buffer* uniform_buffers;
    Descriptors descriptors;

    VkPipeline triangle_list_pipeline;

    VkDevice device;
    VkPhysicalDevice physical_device;
    VkCommandPool command_pool;
    VkQueue graphic_queue;

    Vertex_2D_Buffer main_vertex_buffer;
    Index_Buffer main_index_buffer;

    // Rendering_Properties frosted_render;
    f32 frosted_blur_amount;
    i32 frosted_samples;
    i32 frosted_blur_amount_location;
    i32 frosted_samples_location;

    U32_Array id_to_index;
    U32_Array free_indices;
    Ui_Window_Array windows;
    AABB_2D_Array_Array window_aabbs;
    Hover_Clicked_Index_Array window_hover_clicked_indices;

    // Rendering_Properties render;
    u32 current_index_offset;
    u32 default_textures_offset;

    // NOTE: this makes it possible to drastically reduce Ui_Window size
    u32 current_window_index_count;
    u32 current_window_texture_offset;
    V2 current_window_first_item_position;
    f32 current_window_total_height;
    f32 current_window_total_width;
    V4 current_window_top_color;
    V4 current_window_bottom_color;

    u32 extra_index_offset;
    u32 extra_index_count;

    i32 dock_side_hit;

    Dock_Node* dock_tree;
    Dock_Node* dock_hit_node;

    AABB_2D dock_resize_aabb;
    AABB_2D dock_space;

    V2 mouse_drag_box_point;
    AABB_2D mouse_drag_box;

    Font_TTF font;

    V4 docking_color;

    char font_path[MAX_PATH];

    V2 dimensions;
    f64 delta_time;

    // TODO: Better solution?
    b8 any_window_top_bar_hold;
    b8 any_window_hold;

    b8 dock_resize;
    b8 dock_resize_hover;
    b8 check_collisions;

    b8 animation_off;
    b8 highlight_fucused_window_off;

    b8 non_docked_window_hover;

    b8 mouse_box_is_dragging;

    u8 window_pressed_resize_dragging;
    b8 window_pressed;
    b8 window_pressed_release_from_dock_space;
} Ui_Context;

global Ui_Context ui_context = { 0 };
global f32 ui_big_icon_size = 84.0f;
global f32 ui_list_padding = 0.0f;
global V2 ui_big_icon_min_max = { .min = 64.0f, .max = 256.0f };
global b8 ui_frosted_glass = true;
global const V4 color_picker_spectrum_colors[7] = {
    { .r = 1.0f, .g = 0, .b = 0, .a = 1.0f }, { .r = 1.0f, .g = 1.0f, .b = 0, .a = 1.0f },
    { .r = 0, .g = 1.0f, .b = 0, .a = 1.0f }, { .r = 0, .g = 1.0f, .b = 1.0f, .a = 1.0f },
    { .r = 0, .g = 0, .b = 1.0f, .a = 1.0f }, { .r = 1.0f, .g = 0, .b = 1.0f, .a = 1.0f },
    { .r = 1.0f, .g = 0, .b = 0, .a = 1.0f },
};

f32 ui_get_big_icon_size(void)
{
    return ui_big_icon_size;
}

void ui_set_big_icon_min_size(const f32 new_min)
{
    ui_big_icon_min_max.min = new_min;
}

void ui_set_big_icon_max_size(const f32 new_max)
{
    ui_big_icon_min_max.max = new_max;
}

V2 ui_get_big_icon_min_max(void)
{
    return ui_big_icon_min_max;
}

f32 ui_get_list_padding(void)
{
    return ui_list_padding;
}

void ui_set_list_padding(const f32 padding)
{
    ui_list_padding = padding;
}

void ui_set_big_icon_size(const f32 new_size)
{
    ui_big_icon_size = new_size;
}

void ui_set_frosted_glass(const b8 on)
{
    ui_frosted_glass = on;
}

f32 ui_get_frosted_blur_amount(void)
{
    return ui_context.frosted_blur_amount;
}

void ui_set_frosted_blur_amount(const f32 new_blur_amount)
{
    ui_context.frosted_blur_amount = new_blur_amount;
}

Ui_Layout ui_layout_create(V2 at)
{
    Ui_Layout result = {
        .at = at,
        .start_x = at.x,
    };
    return result;
}

void ui_layout_row(Ui_Layout* layout)
{
    layout->at.y += layout->row_height + layout->padding;
    layout->row_height = 0.0f;
}

void ui_layout_column(Ui_Layout* layout)
{
    layout->at.x += layout->column_width + layout->padding;
}

void ui_layout_reset_column(Ui_Layout* layout)
{
    layout->at.x = layout->start_x;
}

internal void ui_window_start_size_animation_(Ui_Window* window, const V2 end_size)
{
    ui_context.animation_x.data[window->id] = 0.0f;
    window->dock_node->aabb.size = end_size;
}

internal void ui_window_start_position_animation_(Ui_Window* window, const V2 end_position)
{
    ui_context.animation_x.data[window->id] = 0.0f;
    window->dock_node->aabb.min = end_position;
}

internal Ui_Window* ui_window_get_(const u32 window_id)
{
    return ui_context.windows.data + ui_context.id_to_index.data[window_id];
}

internal void ui_layout_set_width_and_height(Ui_Layout* layout, const f32 width, const f32 height)
{
    if (layout)
    {
        layout->column_width = width;
        layout->row_height = sy_max(layout->row_height, height);
    }
}

internal f32 set_scroll_offset(const f32 total_height, const f32 area_y, f32 offset)
{
    const f32 y_offset = event_get_mouse_wheel_event()->y_offset * 80.0f;
    offset += y_offset;
    offset = sy_clamp_high(offset, 0.0f);

    f32 low = area_y - total_height;
    low = sy_clamp_high(low, 0.0f);
    return sy_clamp_low(offset, low);
}

internal void smooth_scroll(Ui_Window* window, const f64 delta_time, const f32 speed)
{
    window->scroll_x += (f32)delta_time * speed;
    window->scroll_x += 1.0f * ui_context.animation_off;
    window->scroll_x = sy_min(window->scroll_x, 1.0f);

    window->current_scroll_offset = sy_lerp(window->start_scroll_offset, window->end_scroll_offset,
                                            ease_out_cubic(window->scroll_x));
}

internal void push_window_to_back(Window_Render_Data_Array* window_array, const u32 index)
{
    if ((!window_array->size) || (index == (window_array->size - 1)) ||
        (index >= window_array->size))
    {
        return;
    }
    Window_Render_Data temp = window_array->data[index];
    for (u32 i = index; i < window_array->size - 1; ++i)
    {
        window_array->data[i] = window_array->data[i + 1];
    }
    window_array->data[window_array->size - 1] = temp;
}

internal u32 generate_id(const u32 index, U32_Array* free_ids, U32_Array* id_to_index)
{
    u32 id = 0;
    if (free_ids->size)
    {
        id = *array_back(free_ids);
        free_ids->size--;
        id_to_index->data[id] = index;
    }
    else
    {
        id = id_to_index->size;
        array_push(id_to_index, index);
    }
    return id;
}

internal AABB_2D add_quad_(u32* index_count, const V2 position, const V2 size, const V4 color,
                           const f32 texture_index)
{
    AABB_2D result =
        quad(&ui_context.main_vertex_buffer.array, position, size, color, texture_index);
    *index_count += 6;
    return result;
}

internal AABB_2D add_border_(u32* index_count, const V2 position, const V2 size, const V4 color)
{
    return border_add(&ui_context.main_vertex_buffer.array, index_count, position, size, color,
                      1.0f, UI_DEFAULT_TEXTURE);
}

internal AABB_2D add_default_quad_(u32* index_count, const V2 position, const V2 size,
                                   const V4 color)
{
    return add_quad_(index_count, position, size, color, UI_DEFAULT_TEXTURE);
}

internal AABB_2D add_default_quad_aabb_(u32* index_count, const AABB_2D* aabb, const V4 color)
{
    return add_quad_(index_count, aabb->min, aabb->size, color, UI_DEFAULT_TEXTURE);
}

internal AABB_2D add_quad_co(const V2 position, const V2 size, const V4 color,
                             const V4 texture_coordinates, const f32 texture_index)
{
    AABB_2D result = quad_co(&ui_context.main_vertex_buffer.array, position, size, color,
                             texture_coordinates, texture_index);
    ui_context.current_window_index_count += 6;
    return result;
}

internal AABB_2D add_quad(const V2 position, const V2 size, const V4 color, const f32 texture_index)
{
    return add_quad_(&ui_context.current_window_index_count, position, size, color, texture_index);
}

internal AABB_2D add_quad_aabb(const AABB_2D* aabb, const V4 color, const f32 texture_index)
{
    return add_quad(aabb->min, aabb->size, color, texture_index);
}

internal AABB_2D add_default_quad(const V2 position, const V2 size, const V4 color)
{
    return add_default_quad_(&ui_context.current_window_index_count, position, size, color);
}

internal AABB_2D add_default_quad_aabb(const AABB_2D* aabb, const V4 color)
{
    return add_default_quad_aabb_(&ui_context.current_window_index_count, aabb, color);
}

internal AABB_2D add_border(const V2 position, const V2 size, const V4 color)
{
    return border_add(&ui_context.main_vertex_buffer.array, &ui_context.current_window_index_count,
                      position, size, color, 1.0f, 0.0f);
}

internal V4 add_window_alpha(const Ui_Window* window, const V4 color)
{
    return v4a(color, window->alpha);
}

internal f32 add_text_selection_chars(Ui_Window* window, const V2 position, const char* buffer,
                                      Selection_Character_Array* selection_chars)
{
    f32 x_advance = 0.0f;
    ui_context.current_window_index_count += text_generation_color(
        ui_context.font.chars, buffer, UI_FONT_TEXTURE, position, 1.0f, ui_context.font.line_height,
        add_window_alpha(window, global_get_text_color()), NULL, &x_advance, selection_chars,
        &ui_context.main_vertex_buffer.array);
    return x_advance;
}

internal f32 add_text(Ui_Window* window, const V2 position, const char* buffer)
{
    return add_text_selection_chars(window, position, buffer, NULL);
}

internal V2 get_text_position(const V2 position)
{
    return v2f(position.x, position.y + ui_context.font.pixel_height - 2.0f);
}

internal V2 add_first_item_offset(V2 relative_position)
{
    return v2_add(relative_position, ui_context.current_window_first_item_position);
}

internal V2 add_scroll_offset(Ui_Window* window, const V2 position)
{
    return v2f(position.x + window->current_scroll_offset_width,
               position.y + window->current_scroll_offset);
}

internal b8 set_docking(const V2 contracted_position, const V2 contracted_size,
                        const V2 expanded_position, const V2 expanded_size, u32* index_count)
{
    AABB_2D aabb = {
        .min = contracted_position,
        .size = contracted_size,
    };

    b8 result = false;
    if (collision_point_in_aabb_2d(event_get_mouse_position(), &aabb))
    {
        quad(&ui_context.main_vertex_buffer.array, expanded_position, expanded_size,
             ui_context.docking_color, 0.0f);
        *index_count += 6;
        result = true;
    }
    border_add_rounded(&ui_context.main_vertex_buffer.array, index_count, aabb.min, aabb.size,
                       global_get_secondary_color(), 2.0f, 0.4f, 3, 0.0f);
    return result;
}

internal f32 middle(const f32 area_size, const f32 object_size)
{
    return (area_size * 0.5f) - (object_size * 0.5f);
}

internal i32 display_docking(const AABB_2D* dock_aabb, const V2 top_position,
                             const V2 right_position, const V2 bottom_position,
                             const V2 left_position, const b8 should_have_middle)
{
    const V2 docking_size_top_bottom = v2f(50.0f, 30.0f);
    const V2 docking_size_left_right = v2f(30.0f, 50.0f);

    const V2 expanded_size_top_bottom = v2f(dock_aabb->size.width, dock_aabb->size.height * 0.5f);
    const V2 expanded_size_left_right = v2f(dock_aabb->size.width * 0.5f, dock_aabb->size.height);

    i32 index = -1;
    if (set_docking(top_position, docking_size_top_bottom, dock_aabb->min, expanded_size_top_bottom,
                    &ui_context.extra_index_count))
    {
        index = 0;
    }
    if (set_docking(right_position, docking_size_left_right,
                    v2f(dock_aabb->min.x + expanded_size_left_right.width, dock_aabb->min.y),
                    expanded_size_left_right, &ui_context.extra_index_count))
    {
        index = 1;
    }
    if (set_docking(bottom_position, docking_size_top_bottom,
                    v2f(dock_aabb->min.x, dock_aabb->min.y + expanded_size_top_bottom.height),
                    expanded_size_top_bottom, &ui_context.extra_index_count))
    {
        index = 2;
    }
    if (set_docking(left_position, docking_size_left_right, dock_aabb->min,
                    expanded_size_left_right, &ui_context.extra_index_count))
    {
        index = 3;
    }
    if (should_have_middle)
    {
        const V2 docking_size_middle = v2i(50.0f);
        const f32 x = middle((right_position.x + docking_size_left_right.width) - left_position.x,
                             docking_size_middle.width);
        const f32 y = middle((bottom_position.y + docking_size_top_bottom.height) - top_position.y,
                             docking_size_middle.height);

        const V2 middle_position = v2f(left_position.x + x, top_position.y + y);
        if (set_docking(middle_position, docking_size_middle, dock_aabb->min, dock_aabb->size,
                        &ui_context.extra_index_count))
        {
            index = 4;
        }
    }
    return index;
}

internal void update_window_dock_nodes(Dock_Node* dock_node)
{
    for (u32 i = 0; i < dock_node->windows.size; ++i)
    {
        ui_window_get_(dock_node->windows.data[i])->dock_node = dock_node;
    }
}

internal Dock_Node* dock_node_create_(Node_Type type, Split_Axis split_axis)
{
    Dock_Node* node = (Dock_Node*)calloc(1, sizeof(Dock_Node));
    node->type = type;
    node->split_axis = split_axis;
    node->size_ratio = 0.5f;
    return node;
}

internal Dock_Node* dock_node_create(Node_Type type, Split_Axis split_axis, i32 window)
{
    Dock_Node* node = dock_node_create_(type, split_axis);
    if (window != -1)
    {
        array_create(&node->windows, 2);
        array_push(&node->windows, window);
    }
    return node;
}

internal Dock_Node* dock_node_create_multiple_windows(Node_Type type, Split_Axis split_axis,
                                                      U32_Array windows)
{
    Dock_Node* node = dock_node_create_(type, split_axis);
    node->windows = windows;
    return node;
}

internal void dock_node_set_split(Dock_Node* split_node, Dock_Node* parent, Split_Axis split_axis,
                                  b8 animate_window)
{
    if (split_axis == SPLIT_HORIZONTAL)
    {
        f32 split_size = parent->aabb.size.height * split_node->size_ratio;

        split_node->children[0]->aabb.size.width = parent->aabb.size.width;
        split_node->children[0]->aabb.size.height = split_size;
        split_node->children[1]->aabb.size.width = parent->aabb.size.width;
        split_node->children[1]->aabb.size.height = parent->aabb.size.height - split_size;

        split_node->children[0]->aabb.min = parent->aabb.min;
        split_node->children[1]->aabb.min.x = parent->aabb.min.x;
        split_node->children[1]->aabb.min.y = parent->aabb.min.y + split_size;
    }
    else if (split_axis == SPLIT_VERTICAL)
    {
        f32 split_size = parent->aabb.size.width * split_node->size_ratio;

        split_node->children[0]->aabb.size.width = split_size;
        split_node->children[0]->aabb.size.height = parent->aabb.size.height;
        split_node->children[1]->aabb.size.width = parent->aabb.size.width - split_size;
        split_node->children[1]->aabb.size.height = parent->aabb.size.height;

        split_node->children[0]->aabb.min = parent->aabb.min;
        split_node->children[1]->aabb.min.x = parent->aabb.min.x + split_size;
        split_node->children[1]->aabb.min.y = parent->aabb.min.y;
    }
    Dock_Node* left = split_node->children[0];
    for (u32 i = 0; i < left->windows.size; ++i)
    {
        Ui_Window* left_window = ui_window_get_(left->windows.data[i]);
        if (ui_context.dock_resize || !animate_window)
        {
            left_window->position = left->aabb.min;
            left_window->size = left->aabb.size;
        }
        else
        {
            ui_window_start_position_animation_(left_window, left->aabb.min);
            ui_window_start_size_animation_(left_window, left->aabb.size);
        }
    }
    Dock_Node* right = split_node->children[1];
    for (u32 i = 0; i < right->windows.size; ++i)
    {
        Ui_Window* right_window = ui_window_get_(right->windows.data[i]);
        if (ui_context.dock_resize || !animate_window)
        {
            right_window->position = right->aabb.min;
            right_window->size = right->aabb.size;
        }
        else
        {
            ui_window_start_position_animation_(right_window, right->aabb.min);
            ui_window_start_size_animation_(right_window, right->aabb.size);
        }
    }
}

internal void dock_node_resize_traverse(Dock_Node* split_node, b8 animate_window)
{
    dock_node_set_split(split_node, split_node, split_node->split_axis, animate_window);
    Dock_Node* left = split_node->children[0];
    Dock_Node* right = split_node->children[1];
    if (left->type == NODE_PARENT)
    {
        dock_node_resize_traverse(left, animate_window);
    }
    if (right->type == NODE_PARENT)
    {
        dock_node_resize_traverse(right, animate_window);
    }
}

internal void dock_node_dock_window(Dock_Node* root, Dock_Node* window, Split_Axis split_axis,
                                    u8 where)
{
    Dock_Node* copy_node = root->children[0];
    Dock_Node* split_node = root;
    if (root->type == NODE_ROOT)
    {
        if (copy_node == NULL)
        {
            for (u32 i = 0; i < window->windows.size; ++i)
            {
                Ui_Window* ui_window = ui_window_get_(window->windows.data[i]);
                ui_window->position = root->aabb.min;
                ui_window->size = root->aabb.size;
                ui_window->dock_node->aabb.min = ui_window->position;
                ui_window->dock_node->aabb.size = ui_window->size;
            }
            root->children[0] = window;
            return;
        }
        // TODO: might be a memory leak
        split_node = dock_node_create(NODE_PARENT, split_axis, -1);
    }
    else if (root->type == NODE_LEAF)
    {
        copy_node = dock_node_create_multiple_windows(NODE_LEAF, SPLIT_NONE, root->windows);
        copy_node->window_in_focus = root->window_in_focus;
        update_window_dock_nodes(copy_node);
    }
    else
    {
        assert(false);
    }

    // where == 1 Left/Top
    split_node->children[!where] = window;
    split_node->children[where] = copy_node;

    dock_node_set_split(split_node, root, split_axis, true);
    if (copy_node->type == NODE_PARENT)
    {
        dock_node_resize_traverse(copy_node, true);
    }
    if (window->type == NODE_PARENT)
    {
        dock_node_resize_traverse(window, true);
    }

    if (root->type == NODE_ROOT)
    {
        split_node->aabb = root->aabb;
        root->children[0] = split_node;
        root->children[1] = NULL;
    }
    else
    {
        root->type = NODE_PARENT;
    }
    root->split_axis = split_axis;
    root->windows.size = 0;
    root->windows.data = NULL;
}

internal void dock_node_resize_from_root(Dock_Node* root, const AABB_2D* aabb, b8 animate_window)
{
    root->aabb = *aabb;
    Dock_Node* left = root->children[0];
    if (left != NULL)
    {
        left->aabb = root->aabb;
        for (u32 i = 0; i < left->windows.size; ++i)
        {
            Ui_Window* left_window = ui_window_get_(left->windows.data[i]);
            left_window->position = left->aabb.min;
            left_window->size = left->aabb.size;
        }
        if (left->type == NODE_PARENT)
        {
            dock_node_resize_traverse(left, animate_window);
        }
    }
}

internal Dock_Node* find_node(Dock_Node* root, Dock_Node* node_before, Dock_Node* node)
{
    // TODO: i believe we can check the adress of the first data
    if ((root->type == NODE_LEAF) && (root->windows.data == node->windows.data))
    {
        return node_before;
    }
    if (root->type != NODE_LEAF)
    {
        for (int i = 0; i < 2; i++)
        {
            if (root->children[i])
            {
                Dock_Node* found = find_node(root->children[i], root, node);
                if (found)
                {
                    return found;
                }
            }
        }
    }
    return NULL;
}

internal void dock_node_remove_node(Dock_Node* root, Dock_Node* node_to_remove)
{
    if (root->children[0]->type == NODE_LEAF &&
        root->children[0]->windows.data == node_to_remove->windows.data)
    {
        root->children[0] = NULL;
        return;
    }
    Dock_Node* node_before = find_node(root->children[0], root, node_to_remove);
    Dock_Node* left = node_before->children[0];
    Dock_Node* right = node_before->children[1];
    if (left->type == NODE_LEAF && left->windows.data == node_to_remove->windows.data)
    {
        node_before->type = right->type;
        node_before->split_axis = right->split_axis;
        node_before->windows = right->windows;
        node_before->window_in_focus = right->window_in_focus;
        node_before->children[0] = right->children[0];
        node_before->children[1] = right->children[1];
        node_before->size_ratio = right->size_ratio;
        if (right->type == NODE_LEAF)
        {
            update_window_dock_nodes(node_before);
        }
        free(right);
    }
    else if (right->type == NODE_LEAF && right->windows.data == node_to_remove->windows.data)
    {
        node_before->type = left->type;
        node_before->split_axis = left->split_axis;
        node_before->windows = left->windows;
        node_before->window_in_focus = left->window_in_focus;
        node_before->children[0] = left->children[0];
        node_before->children[1] = left->children[1];
        node_before->size_ratio = left->size_ratio;
        if (left->type == NODE_LEAF)
        {
            update_window_dock_nodes(node_before);
        }
        free(left);
    }
    dock_node_resize_from_root(root, &root->aabb, true);
}

internal u32 remove_window_from_shared_dock_space(const u32 window_index,
                                                  Ui_Window* window_to_remove,
                                                  Dock_Node* dock_space_to_change)
{
    b8 focused_window = window_index == dock_space_to_change->window_in_focus;

    for (u32 i = window_index; i < dock_space_to_change->windows.size - 1; ++i)
    {
        dock_space_to_change->windows.data[i] = dock_space_to_change->windows.data[i + 1];
        if ((i + 1) == dock_space_to_change->window_in_focus)
        {
            dock_space_to_change->window_in_focus = i;
        }
    }
    dock_space_to_change->windows.size--;

    set_bit(window_to_remove->flags, UI_WINDOW_CLOSING);
    unset_bit(window_to_remove->flags, UI_WINDOW_HIDE);
    unset_bit(window_to_remove->flags, UI_WINDOW_DOCKED);
    window_to_remove->dock_node = dock_node_create(NODE_LEAF, SPLIT_NONE, window_to_remove->id);

    if (focused_window)
    {
        dock_space_to_change->window_in_focus = 0;
        Ui_Window* window_to_show = ui_window_get_(
            dock_space_to_change->windows.data[dock_space_to_change->window_in_focus]);
        window_to_show->position = window_to_remove->position;
        window_to_show->size = window_to_remove->size;
        unset_bit(window_to_show->flags, UI_WINDOW_HIDE);
    }
    return dock_space_to_change->windows.data[dock_space_to_change->window_in_focus];
}

internal i32 root_display_docking(Dock_Node* dock_node)
{
    const V2 docking_size_top_bottom = v2f(50.0f, 30.0f);
    const V2 docking_size_left_right = v2f(30.0f, 50.0f);

    const V2 middle_top_bottom = v2f(
        dock_node->aabb.min.x + middle(dock_node->aabb.size.width, docking_size_top_bottom.width),
        dock_node->aabb.min.y +
            middle(dock_node->aabb.size.height, docking_size_top_bottom.height));

    const V2 middle_left_right = v2f(
        dock_node->aabb.min.x + middle(dock_node->aabb.size.width, docking_size_left_right.width),
        dock_node->aabb.min.y +
            middle(dock_node->aabb.size.height, docking_size_left_right.height));

    const V2 top_position = v2f(middle_top_bottom.x, dock_node->aabb.min.y + 10.0f);

    const V2 right_position = v2f(
        dock_node->aabb.size.width - (docking_size_left_right.width + 10.0f), middle_left_right.y);

    const V2 bottom_position =
        v2f(middle_top_bottom.x, (dock_node->aabb.min.y + dock_node->aabb.size.height) -
                                     (docking_size_top_bottom.height + 10.0f));

    const V2 left_position = v2f(dock_node->aabb.min.x + 10.0f, middle_left_right.y);

    i32 index = display_docking(&dock_node->aabb, top_position, right_position, bottom_position,
                                left_position, false);
    if (index != -1) ui_context.dock_hit_node = dock_node;
    return index;
}

internal i32 leaf_display_docking(Dock_Node* dock_node)
{
    const V2 docking_size_top_bottom = v2f(50.0f, 30.0f);
    const V2 docking_size_left_right = v2f(30.0f, 50.0f);

    const V2 middle_top_bottom = v2f(
        dock_node->aabb.min.x + middle(dock_node->aabb.size.width, docking_size_top_bottom.width),
        dock_node->aabb.min.y +
            middle(dock_node->aabb.size.height, docking_size_top_bottom.height));

    const V2 middle_left_right = v2f(
        dock_node->aabb.min.x + middle(dock_node->aabb.size.width, docking_size_left_right.width),
        dock_node->aabb.min.y +
            middle(dock_node->aabb.size.height, docking_size_left_right.height));

    const V2 top_position =
        v2f(middle_top_bottom.x, middle_top_bottom.y - (docking_size_top_bottom.height + 20.0f));

    const V2 right_position =
        v2f(middle_left_right.x + (docking_size_left_right.width + 20.0f), middle_left_right.y);

    const V2 bottom_position =
        v2f(middle_top_bottom.x, middle_top_bottom.y + (docking_size_top_bottom.height + 20.0f));

    const V2 left_position =
        v2f(middle_left_right.x - (docking_size_left_right.width + 20.0f), middle_left_right.y);

    i32 index = display_docking(&dock_node->aabb, top_position, right_position, bottom_position,
                                left_position, true);
    if (index != -1) ui_context.dock_hit_node = dock_node;
    return index;
}

internal i32 look_for_docking_collisions(Dock_Node* parent)
{
    const V2 mouse_position = event_get_mouse_position();

    Dock_Node* left = parent->children[0];
    Dock_Node* right = parent->children[1];

    if (left->type == NODE_LEAF)
    {
        if (collision_point_in_aabb_2d(mouse_position, &left->aabb))
        {
            return leaf_display_docking(left);
        }
    }
    if (right->type == NODE_LEAF)
    {
        if (collision_point_in_aabb_2d(mouse_position, &right->aabb))
        {
            return leaf_display_docking(right);
        }
    }

    if (left->type == NODE_PARENT)
    {
        i32 hit = look_for_docking_collisions(left);
        if (hit != -1) return hit;
    }
    if (right->type == NODE_PARENT)
    {
        i32 hit = look_for_docking_collisions(right);
        if (hit != -1) return hit;
    }
    return -1;
}

internal i32 dock_node_docking_display_traverse(Dock_Node* root)
{
    if (root->children[0] && root->children[0]->type == NODE_PARENT)
    {
        return look_for_docking_collisions(root->children[0]);
    }
    return -1;
}

internal AABB_2D dock_node_set_resize_aabb(Dock_Node* node)
{
    Dock_Node* right = node->children[1];
    AABB_2D resize_aabb = { .min = right->aabb.min };
    if (node->split_axis == SPLIT_HORIZONTAL)
    {
        resize_aabb.min.y -= 3.0f;
        resize_aabb.size.width = right->aabb.size.width;
        resize_aabb.size.height = 5.0f;
    }
    else // SPLIT_VERTICAL
    {
        resize_aabb.min.x -= 3.0f;
        resize_aabb.size.width = 5.0f;
        resize_aabb.size.height = right->aabb.size.height;
    }
    return resize_aabb;
}

internal b8 look_for_resize_collisions(Dock_Node* parent)
{
    const V2 mouse_position = event_get_mouse_position();

    AABB_2D resize_aabb = dock_node_set_resize_aabb(parent);
    if (collision_point_in_aabb_2d(mouse_position, &resize_aabb))
    {
        ui_context.dock_resize_aabb = resize_aabb;
        ui_context.dock_hit_node = parent;
        return true;
    }

    Dock_Node* left = parent->children[0];
    if (left->type == NODE_PARENT)
    {
        b8 hit = look_for_resize_collisions(left);
        if (hit) return hit;
    }
    Dock_Node* right = parent->children[1];
    if (right->type == NODE_PARENT)
    {
        b8 hit = look_for_resize_collisions(right);
        if (hit) return hit;
    }
    return false;
}

internal b8 dock_node_resize_collision_traverse(Dock_Node* root)
{
    if (root->children[0] && root->children[0]->type == NODE_PARENT)
    {
        return look_for_resize_collisions(root->children[0]);
    }
    return false;
}

void ui_window_start_size_animation(const u32 window_id, const V2 end_size)
{
    ui_window_start_size_animation_(ui_window_get_(window_id), end_size);
}

void ui_window_start_position_animation(const u32 window_id, const V2 end_position)
{
    ui_window_start_position_animation_(ui_window_get_(window_id), end_position);
}

void ui_window_dock_space_size(const u32 window_id, const V2 end_size)
{
    ui_window_get_(window_id)->dock_node->aabb.size = end_size;
}

void ui_window_dock_space_min(const u32 window_id, const V2 end_position)
{
    ui_window_get_(window_id)->dock_node->aabb.min = end_position;
}

void ui_window_set_animation_x(const u32 window_id, const f32 x)
{
    ui_context.animation_x.data[window_id] = x;
}

Input_Buffer ui_input_buffer_create(void)
{
    Input_Buffer input = {
        .input_index = -1,
        .time = 0.4f,
    };
    array_create(&input.buffer, 20);
    array_create(&input.chars, 20);
    array_create(&input.chars_selected, 20);
    return input;
}

void ui_input_buffer_delete(Input_Buffer* input)
{
    array_free(&input->buffer);
    array_free(&input->chars);
    array_free(&input->chars_selected);
}

void ui_input_buffer_clear_selection(Input_Buffer* input)
{
    input->chars_selected.size = 0;
    input->start_selection_index = -1;
    input->end_selection_index = -1;
}

char* ui_input_buffer_get_selection_as_string(Input_Buffer* input)
{
    char* text = (char*)calloc(input->chars_selected.size + 1, sizeof(char));
    for (u32 i = 0; i < input->chars_selected.size; ++i)
    {
        text[i] = input->chars_selected.data[i].character;
    }
    return text;
}

void ui_input_buffer_copy_selection_to_clipboard(Input_Buffer* input)
{
    char* text = ui_input_buffer_get_selection_as_string(input);
    // window_set_clipboard(text);
    free(text);
}

void ui_input_buffer_erase_from_selection(Input_Buffer* input)
{
    for (u32 i = input->start_selection_index, j = input->end_selection_index;
         j < input->buffer.size; ++i, ++j)
    {
        input->buffer.data[i] = input->buffer.data[j];
    }
    input->input_index = input->start_selection_index;
    input->buffer.size -= input->end_selection_index - input->start_selection_index;

    ui_input_buffer_clear_selection(input);
    input->buffer.data[input->buffer.size] = '\0';
}

internal void insert_window(Dock_Node* node, const u32 id, const b8 docked)
{
    Ui_Window window = {
        .position = v2f(200.0f, 200.0f),
        .id = id,
        .size = v2f(200.0f, 200.0f),
        .dock_node = node,
        .alpha = 1.0f,
    };
    window.dock_node->aabb.min = window.position;
    window.dock_node->aabb.size = window.size;
    set_bit(window.flags, UI_WINDOW_DOCKED * docked);
    array_push(&ui_context.windows, window);
    array_push(&ui_context.animation_x, 1.0f);

    AABB_2D_Array aabbs = { 0 };
    array_create(&aabbs, 10);
    array_push(&ui_context.window_aabbs, aabbs);

    Hover_Clicked_Index hover_clicked_index = { .index = -1 };
    array_push(&ui_context.window_hover_clicked_indices, hover_clicked_index);
}

internal void write_node(FILE* file, Dock_Node* node)
{
    if (node == NULL)
    {
        b8 is_null = true;
        fwrite(&is_null, sizeof(is_null), 1, file);
        return;
    }

    b8 is_null = false;
    fwrite(&is_null, sizeof(is_null), 1, file);
    fwrite(&node->type, sizeof(node->type), 1, file);
    fwrite(&node->split_axis, sizeof(node->split_axis), 1, file);
    fwrite(&node->window_in_focus, sizeof(node->window_in_focus), 1, file);
    fwrite(&node->windows.size, sizeof(node->windows.size), 1, file);
    for (u32 i = 0; i < node->windows.size; ++i)
    {
        fwrite(&node->windows.data[i], sizeof(node->windows.data[i]), 1, file);
    }
    fwrite(&node->size_ratio, sizeof(node->size_ratio), 1, file);

    write_node(file, node->children[0]);
    write_node(file, node->children[1]);
}

internal void save_layout(void)
{
    char full_path_buffer[MAX_PATH] = { 0 };
    path_extend2("saved/ui_layout.txt", full_path_buffer);
    FILE* file = fopen(full_path_buffer, "wb");
    if (file == NULL)
    {
        return;
    }
    write_node(file, ui_context.dock_tree);
    fclose(file);
}

Dock_Node* read_node(FILE* file)
{
    b8 is_null = false;
    size_t count = fread(&is_null, sizeof(uint8_t), 1, file);
    if (is_null || !count)
    {
        return NULL;
    }

    Dock_Node* node = (Dock_Node*)calloc(1, sizeof(Dock_Node));
    assert(fread(&node->type, sizeof(node->type), 1, file));
    assert(fread(&node->split_axis, sizeof(node->split_axis), 1, file));
    assert(fread(&node->window_in_focus, sizeof(node->window_in_focus), 1, file));
    assert(fread(&node->windows.size, sizeof(node->windows.size), 1, file));

    if (node->windows.size)
    {
        u32 size = node->windows.size;
        array_create(&node->windows, 2);
        for (u32 i = 0; i < size; ++i)
        {
            u32 window_id = 0;
            assert(fread(&window_id, sizeof(window_id), 1, file));
            array_push(&node->windows, window_id);
            insert_window(node, window_id, true);
            if (i != node->window_in_focus)
            {
                set_bit(array_back(&ui_context.windows)->flags, UI_WINDOW_HIDE);
            }
        }
    }
    else
    {
        node->windows.capacity = 0;
        node->windows.data = NULL;
    }

    assert(fread(&node->size_ratio, sizeof(node->size_ratio), 1, file));

    node->children[0] = read_node(file);
    node->children[1] = read_node(file);

    return node;
}

internal Dock_Node* load_layout(void)
{
    char full_path_buffer[MAX_PATH] = { 0 };
    path_extend2("saved/ui_layout.txt", full_path_buffer);
    FILE* file = fopen(full_path_buffer, "rb");
    if (file == NULL)
    {
        return NULL;
    }

    Dock_Node* root = read_node(file);
    fclose(file);
    return root;
}

internal void get_window_resize_aabbs(Ui_Window* window, AABB_2D* left, AABB_2D* top,
                                      AABB_2D* right, AABB_2D* bottom)
{
    const f32 size = 6.0f;
    const f32 half_size = size * 0.5f;
    *left = (AABB_2D){
        .min = v2f(window->position.x - half_size, window->position.y - half_size),
        .size = v2f(size, window->size.height + size),
    };
    *right = (AABB_2D){
        .min = v2f(window->position.x + window->size.width - half_size,
                   window->position.y - half_size),
        .size = left->size,
    };
    *top = (AABB_2D){
        .min = v2f(window->position.x - half_size, window->position.y - half_size),
        .size = v2f(window->size.width + size, size),
    };
    *bottom = (AABB_2D){
        .min = v2f(window->position.x - half_size,
                   window->position.y + window->size.height - half_size),
        .size = top->size,
    };
}

internal void set_resize_cursor(u8 resize)
{

    if ((check_bit(resize, RESIZE_LEFT) && check_bit(resize, RESIZE_TOP)) ||
        (check_bit(resize, RESIZE_RIGHT) && check_bit(resize, RESIZE_BOTTOM)))
    {
        // window_set_cursor(window_get_current(), FTIC_RESIZE_NWSE_CURSOR);
    }
    else if ((check_bit(resize, RESIZE_LEFT) && check_bit(resize, RESIZE_BOTTOM)) ||
             (check_bit(resize, RESIZE_RIGHT) && check_bit(resize, RESIZE_TOP)))
    {
        // window_set_cursor(window_get_current(), FTIC_RESIZE_NESW_CURSOR);
    }
    else if (check_bit(resize, RESIZE_LEFT) || check_bit(resize, RESIZE_RIGHT))
    {
        // window_set_cursor(window_get_current(), FTIC_RESIZE_H_CURSOR);
    }
    else if (check_bit(resize, RESIZE_TOP) || check_bit(resize, RESIZE_BOTTOM))
    {
        // window_set_cursor(window_get_current(), FTIC_RESIZE_V_CURSOR);
    }
    else
    {
        // window_set_cursor(window_get_current(), FTIC_NORMAL_CURSOR);
    }
}

internal Texture load_font_texture(const f32 pixel_height, VkDevice device,
                                   VkPhysicalDevice physical_device, VkCommandPool command_pool,
                                   VkQueue graphic_queue)
{
    ui_context.font = (Font_TTF){ 0 };
    const i32 width_atlas = 256;
    const i32 height_atlas = 256;
    const u32 bitmap_size = width_atlas * height_atlas;
    u8* font_bitmap_temp = (u8*)calloc(bitmap_size, sizeof(u8));
    init_ttf_atlas(width_atlas, height_atlas, pixel_height, 96, 32, ui_context.font_path,
                   font_bitmap_temp, &ui_context.font);

    u8* font_bitmap = (u8*)malloc(bitmap_size * 4 * sizeof(u8));
    memset(font_bitmap, UINT8_MAX, bitmap_size * 4 * sizeof(u8));
    for (u32 i = 0, j = 3; i < bitmap_size; ++i, j += 4)
    {
        font_bitmap[j] = font_bitmap_temp[i];
    }
    free(font_bitmap_temp);

    Texture texture = {
        .mip_map_lvl = 1,
        .width = width_atlas,
        .height = height_atlas,
        .size_bytes = (u32)(width_atlas * height_atlas * 4),
    };

    vulkan_texture_buffer_create(device, physical_device, command_pool, graphic_queue,
                                 VK_FORMAT_R8G8B8A8_SRGB, VK_FILTER_NEAREST, font_bitmap, &texture);

    free(font_bitmap);
    return texture;
}

void ui_context_set_font_path(const char* new_path)
{
    memset(ui_context.font_path, 0, sizeof(ui_context.font_path));
    memcpy(ui_context.font_path, new_path, strlen(new_path));
}

#if 0

void ui_context_change_font_pixel_height(const f32 pixel_height)
{
    if (closed_interval(12.0f, pixel_height, 26.0f))
    {
        texture_delete(ui_context.render.render.textures.data[1]);
        ui_context.render.render.textures.data[(u32)UI_FONT_TEXTURE] =
            load_font_texture(pixel_height);
    }
}
#endif

const char* ui_context_get_font_path(void)
{
    return ui_context.font_path;
}

void ui_context_create(VkDevice device, VkPhysicalDevice physical_device,
                       VkCommandPool command_pool, VkQueue graphic_queue, VkRenderPass render_pass,
                       VkSampleCountFlagBits sample_count, const Platform* platform,
                       u32 num_semaphores)
{
    array_create(&ui_context.id_to_index, 100);
    array_create(&ui_context.free_indices, 100);
    array_create(&ui_context.animation_x, 100);

    array_create(&ui_context.last_frame_windows, 10);
    array_create(&ui_context.current_frame_windows, 10);

    array_create(&ui_context.last_frame_docked_windows, 10);
    array_create(&ui_context.current_frame_docked_windows, 10);

    array_create(&ui_context.last_frame_overlay_windows, 10);
    array_create(&ui_context.current_frame_overlay_windows, 10);

    array_create(&ui_context.windows, 10);
    array_create(&ui_context.window_aabbs, 10);
    array_create(&ui_context.window_hover_clicked_indices, 10);

    array_create(&ui_context.docked_index_offsets_and_counts, 10);
    array_create(&ui_context.floating_index_offsets_and_counts, 10);

    Dock_Node* saved_tree = load_layout();
    if (saved_tree)
    {
        ui_context.dock_tree = saved_tree;
    }
    else
    {
        ui_context.dock_tree = dock_node_create(NODE_ROOT, SPLIT_NONE, -1);
    }
    ui_context.dock_side_hit = -1;

    const char* texture_paths[] = {
        [UI_DEFAULT_TEXTURE] = "Syntics/res/default.png",
    };
    array_create(&ui_context.textures, static_array_size(texture_paths) + 2);
    vulkan_texture_multiple_path_create(device, physical_device, command_pool, graphic_queue,
                                        VK_FILTER_LINEAR, false, static_array_size(texture_paths),
                                        texture_paths, ui_context.textures.data);
    ui_context.textures.size = static_array_size(texture_paths);

    char* font_path = "C:/Windows/Fonts/arial.ttf";
    memset(ui_context.font_path, 0, sizeof(ui_context.font_path));
    memcpy(ui_context.font_path, font_path, strlen(font_path));

    array_push(&ui_context.textures,
               load_font_texture(16, device, physical_device, command_pool, graphic_queue));

    vulkan_descriptor_set_layout_create(device, ui_context.textures.size,
                                        &ui_context.descriptor_set_layout);
    vulkan_graphic_pipeline_layout_create(device, ui_context.descriptor_set_layout,
                                          &ui_context.pipeline_layout);

    vulkan_uniforms_descriptors_init(NULL, device, physical_device, &ui_context.uniform_buffers,
                                     &ui_context.descriptors, ui_context.descriptor_set_layout,
                                     num_semaphores, ui_context.textures.data,
                                     ui_context.textures.size);

    { // Triangle list
        Graphic_Pipeline_Attrib g_p_info =
            gp_default2(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT);
        vulkan_graphic_pipeline_create_deluxe_2d(device, ui_context.pipeline_layout, &g_p_info,
                                                 "Syntics/res/shaders/spv/ui.vert.spv",
                                                 "Syntics/res/shaders/spv/ui.frag.spv", render_pass,
                                                 sample_count, &ui_context.triangle_list_pipeline);
    }

    { // Main
        const u32 start_count = 100;

        Index_Buffer* index_buffer = &ui_context.main_index_buffer;
        Vertex_2D_Buffer* vertex_buffer = &ui_context.main_vertex_buffer;

        array_create(&vertex_buffer->array, start_count * 4);
        vertex_buffer->buffer.size_bytes = vertex_buffer->array.capacity * sizeof(Vertex);
        vulkan_vertex_buffer_create_visible(device, physical_device, &vertex_buffer->buffer, NULL);

        array_create(&index_buffer->array, start_count * 6);
        generate_indicies(&index_buffer->array, 0, start_count);

        index_buffer->buffer.size_bytes = index_buffer->array.size * sizeof(u32);
        vulkan_index_buffer_create_local(device, physical_device, command_pool, graphic_queue,
                                         index_buffer);
    }

    ui_context.device = device;
    ui_context.physical_device = physical_device;
    ui_context.command_pool = command_pool;
    ui_context.graphic_queue = graphic_queue;
}

internal Window_Render_Data render_data_create(const Ui_Window* window)
{
    AABB_2D window_aabb = { .min = window->position, .size = window->size };
    Window_Render_Data render_data = {
        .id = window->id,
        .aabb = window_aabb,
        .index_offset = ui_context.current_index_offset,
        .index_count = ui_context.current_window_index_count,
    };
    return render_data;
}

internal u8 look_for_window_resize(Ui_Window* window)
{
    AABB_2D left, top, right, bottom;
    get_window_resize_aabbs(window, &left, &top, &right, &bottom);

    const V2 mouse_position = event_get_mouse_position();
    const b8 mouse_button_pressed_once = event_is_mouse_button_pressed_once(SYNT_LEFT_BUTTON);

    u8 result = RESIZE_NONE;
    if (collision_point_in_aabb_2d(mouse_position, &left))
    {
        if (mouse_button_pressed_once)
        {
            ui_context.window_pressed_resize_dragging |= RESIZE_LEFT;
        }
        result |= RESIZE_LEFT;
    }
    else if (collision_point_in_aabb_2d(mouse_position, &right))
    {
        if (mouse_button_pressed_once)
        {
            ui_context.window_pressed_resize_dragging |= RESIZE_RIGHT;
        }
        result |= RESIZE_RIGHT;
    }

    if (collision_point_in_aabb_2d(mouse_position, &top))
    {
        if (mouse_button_pressed_once)
        {
            ui_context.window_pressed_resize_dragging |= RESIZE_TOP;
        }
        result |= RESIZE_TOP;
    }
    else if (collision_point_in_aabb_2d(mouse_position, &bottom))
    {
        if (mouse_button_pressed_once)
        {
            ui_context.window_pressed_resize_dragging |= RESIZE_BOTTOM;
        }
        result |= RESIZE_BOTTOM;
    }
    return result;
}

internal b8 check_window_collisions(const Window_Render_Data* render_data)
{
    const b8 mouse_button_clicked = event_is_mouse_button_clicked(SYNT_LEFT_BUTTON);
    const Mouse_Button_Event* mouse_button_event = event_get_mouse_button_event();
    const b8 mouse_button_pressed = event_is_mouse_button_pressed_once(SYNT_LEFT_BUTTON);
    const V2 mouse_position = event_get_mouse_position();
    const u32 window_index = ui_context.id_to_index.data[render_data->id];
    const AABB_2D_Array* aabbs = ui_context.window_aabbs.data + window_index;

    b8 area_hit = false;
    if (aabbs->size)
    {
        if (!collision_point_in_aabb_2d(mouse_position, &render_data->aabb))
        {
            return false;
        }
        else
        {
            area_hit = true;
        }
    }

    for (i32 aabb_index = ((i32)aabbs->size) - 1; aabb_index >= 0; --aabb_index)
    {
        const AABB_2D* aabb = aabbs->data + aabb_index;
        if (collision_point_in_aabb_2d(mouse_position, aabb))
        {
            Hover_Clicked_Index* hover_clicked_index =
                ui_context.window_hover_clicked_indices.data + window_index;
            hover_clicked_index->index = aabb_index;
            hover_clicked_index->hover = true;
            hover_clicked_index->double_clicked = mouse_button_event->double_clicked;
            if (mouse_button_clicked || mouse_button_pressed ||
                event_is_mouse_button_clicked(SYNT_LEFT_BUTTON))
            {
                hover_clicked_index->pressed = mouse_button_pressed;
                hover_clicked_index->clicked = mouse_button_clicked;
                ui_context.window_in_focus = window_index;
            }
            return true;
        }
    }
    return area_hit;
}

f32 ui_context_get_font_pixel_height(void)
{
    return ui_context.font.pixel_height;
}

const Font_TTF* ui_context_get_font(void)
{
    return &ui_context.font;
}

internal void handle_window_top_bar_events(Ui_Window* window, V2 top_bar_offset)
{
    const f32 top_bar_height = ui_context.font.pixel_height + 6.0f;
    if (ui_context.any_window_top_bar_hold)
    {
        window->position = v2_add(event_get_mouse_position(), top_bar_offset);

        window->position.y =
            sy_clamp_high(window->position.y, ui_context.dimensions.height - window->size.height);
        window->position.y = sy_clamp_low(window->position.y, 0.0f);

        window->position.x =
            sy_clamp_high(window->position.x, ui_context.dimensions.width - window->size.width);
        window->position.x = sy_clamp_low(window->position.x, 0.0f);

        window->dock_node->aabb.min = window->position;
        // window->dock_node->aabb.size = window->size;
    }
    else
    {
        if (v2_distance(top_bar_offset, event_get_mouse_position()) >= 10.0f)
        {
            const V2 mouse_position = event_get_mouse_position();
            if (ui_context.window_pressed_release_from_dock_space)
            {
                Dock_Node* dock_node = window->dock_node;
                if (dock_node->windows.size > 1)
                {
                    u32 i = 0;
                    for (; i < dock_node->windows.size; ++i)
                    {
                        if (window->id == dock_node->windows.data[i]) break;
                    }
                    remove_window_from_shared_dock_space(i, window, dock_node);
                    unset_bit(window->flags, UI_WINDOW_CLOSING);
                    ui_window_start_size_animation_(window, v2i(200.0f));
                }
                ui_context.window_pressed_release_from_dock_space = false;
                ui_context.window_in_focus = ui_context.id_to_index.data[window->id];
            }

            if (check_bit(window->flags, UI_WINDOW_DOCKED))
            {
                Dock_Node* dock_node = window->dock_node;
                for (u32 i = 0; i < dock_node->windows.size; ++i)
                {
                    u32 id = dock_node->windows.data[i];
                    Ui_Window* window_to_undock = ui_window_get_(id);

                    unset_bit(window_to_undock->flags, UI_WINDOW_DOCKED);
                }
                dock_node_remove_node(ui_context.dock_tree, window->dock_node);

                ui_window_start_size_animation_(window, v2i(200.0f));
            }
            window->position.y = mouse_position.y - (top_bar_height * 0.5f);
            window->dock_node->aabb.min.y = window->position.y;

            ui_context.any_window_top_bar_hold = true;
            ui_context.any_window_hold = true;
            ui_context.window_pressed_offset = v2_sub(window->position, mouse_position);
        }
    }
}

internal void resize_where_position_change(const f32 mouse_position, const f32 size_offset,
                                           const f32 position_offset, f32* position, f32* size)
{
    *size = size_offset + position_offset;
    if (*size > 200.0f)
    {
        *position = mouse_position;
    }
    else
    {
        *position = mouse_position - (200.0f - (*size));
        *size = 200.0f;
    }
}

internal void resize_where_only_size_change(const f32 size_offset, const f32 position_offset,
                                            f32* size)
{
    *size = size_offset - position_offset;
    if (*size < 200.0f)
    {
        *size = 200.0f;
    }
}

internal void handle_window_resize_events(Ui_Window* window)
{
    const V2 mouse_position = event_get_mouse_position();
    const V2 offset = v2_sub(ui_context.window_pressed_offset, mouse_position);
    const V2 size_offset = ui_context.window_pressed_size_offset;
    if (check_bit(ui_context.window_pressed_resize_dragging, RESIZE_LEFT))
    {
        resize_where_position_change(mouse_position.x, size_offset.width, offset.x,
                                     &window->position.x, &window->size.width);
    }
    else if (check_bit(ui_context.window_pressed_resize_dragging, RESIZE_RIGHT))
    {
        resize_where_only_size_change(size_offset.width, offset.x, &window->size.width);
    }

    if (check_bit(ui_context.window_pressed_resize_dragging, RESIZE_TOP))
    {
        resize_where_position_change(mouse_position.y, size_offset.height, offset.y,
                                     &window->position.y, &window->size.height);
    }
    else if (check_bit(ui_context.window_pressed_resize_dragging, RESIZE_BOTTOM))
    {
        resize_where_only_size_change(size_offset.height, offset.y, &window->size.height);
    }
    set_resize_cursor(ui_context.window_pressed_resize_dragging);

    window->dock_node->aabb.size = window->size;
    window->dock_node->aabb.min = window->position;
}

internal void reset_last_frame_windows(Window_Render_Data_Array* last_frame)
{
    for (u32 i = 0; i < last_frame->size; ++i)
    {
        u32 window_index = ui_context.id_to_index.data[last_frame->data[i].id];
        Ui_Window* window = ui_context.windows.data + window_index;
        ui_context.current_window_index_count = 0;
        unset_bit(window->flags, UI_WINDOW_AREA_HIT);
    }
}

void ui_context_set_window_top_color(V4 color)
{
    ui_context.current_window_top_color = color;
}

void ui_context_set_window_bottom_color(V4 color)
{
    ui_context.current_window_bottom_color = color;
}

void ui_context_begin(const V2 dimensions, const AABB_2D* dock_space, const f64 delta_time,
                      const b8 check_collisions)
{
    if (!aabb_2d_equal(&ui_context.dock_space, dock_space))
    {
        dock_node_resize_from_root(ui_context.dock_tree, dock_space, false);
    }
    if (event_get_mouse_button_event()->action == SYNT_RELEASE)
    {
        ui_context.window_pressed = false;
        ui_context.any_window_top_bar_hold = false;
        ui_context.window_pressed_release_from_dock_space = false;
        ui_context.window_pressed_resize_dragging = RESIZE_NONE;
        ui_context.any_window_hold = false;
    }

    ui_context.current_window_top_color = global_get_clear_color();
    ui_context.current_window_bottom_color = global_get_clear_color();

    ui_context.docking_color = v4a(global_get_secondary_color(), 0.4f);

    if (!ui_context.any_window_hold && ui_context.dock_resize)
    {
        const V2 relative_mouse_position =
            v2_sub(event_get_mouse_position(), ui_context.dock_hit_node->aabb.min);

        f32 new_ratio = 0.0f;
        if (ui_context.dock_hit_node->split_axis == SPLIT_HORIZONTAL)
        {
            new_ratio = relative_mouse_position.y / ui_context.dock_hit_node->aabb.size.height;
        }
        else // SPLIT_VERTICAL
        {
            new_ratio = relative_mouse_position.x / ui_context.dock_hit_node->aabb.size.width;
        }
        if (closed_interval(0.1f, new_ratio, 0.9f))
        {
            ui_context.dock_hit_node->size_ratio = new_ratio;
            dock_node_resize_traverse(ui_context.dock_hit_node, true);
        }
        ui_context.dock_resize_aabb = dock_node_set_resize_aabb(ui_context.dock_hit_node);
    }

    if (ui_context.window_pressed && !ui_context.mouse_box_is_dragging)
    {
        handle_window_top_bar_events(ui_context.pressed_window, ui_context.window_pressed_offset);
    }
    else if (ui_context.window_pressed_resize_dragging)
    {
        handle_window_resize_events(ui_context.pressed_window);
    }

    ui_context.check_collisions = check_collisions;
    ui_context.dock_space = *dock_space;
    ui_context.dimensions = dimensions;

    const f64 clamp_high = sy_clamp_high(delta_time, 0.5);
    ui_context.delta_time = sy_clamp_low(clamp_high, 0.0);

    ui_context.vp.proj = ortho(0.0f, dimensions.x, 0.0f, dimensions.y, -1.0f, 1.0f);
    ui_context.vp.view = m4i(1.0f);

    ui_context.main_vertex_buffer.array.size = 0;
    ui_context.current_index_offset = 0;

    ui_context.non_docked_window_hover = false;

    for (u32 i = 0; i < ui_context.window_hover_clicked_indices.size; ++i)
    {
        ui_context.window_hover_clicked_indices.data[i] = (Hover_Clicked_Index){ .index = -1 };
    }

    reset_last_frame_windows(&ui_context.last_frame_docked_windows);
    reset_last_frame_windows(&ui_context.last_frame_windows);
    reset_last_frame_windows(&ui_context.last_frame_overlay_windows);
    if (ui_context.pressed_window && ui_context.any_window_hold)
    {
        set_bit(ui_context.pressed_window->flags, UI_WINDOW_AREA_HIT);
    }
    if (check_collisions && !ui_context.dock_resize && !ui_context.any_window_top_bar_hold &&
        !ui_context.any_window_hold)
    {
        // window_set_cursor(window_get_current(), FTIC_NORMAL_CURSOR);

        for (i32 i = ((i32)ui_context.last_frame_overlay_windows.size) - 1; i >= 0; --i)
        {
            const Window_Render_Data* render_data = ui_context.last_frame_overlay_windows.data + i;
            if (check_window_collisions(render_data))
            {
                ui_context.non_docked_window_hover = true;
                set_bit(ui_window_get_(render_data->id)->flags, UI_WINDOW_AREA_HIT);
                goto collision_check_done;
            }
        }
        for (i32 i = ((i32)ui_context.last_frame_windows.size) - 1; i >= 0; --i)
        {
            const Window_Render_Data* render_data = ui_context.last_frame_windows.data + i;
            Ui_Window* window = ui_window_get_(render_data->id);
            if (check_bit(window->flags, UI_WINDOW_RESIZEABLE) &&
                !ui_context.window_pressed_resize_dragging)
            {
                u8 any_collision = look_for_window_resize(window);
                if (ui_context.window_pressed_resize_dragging)
                {
                    ui_context.window_pressed_offset = event_get_mouse_position();
                    ui_context.window_pressed_size_offset = window->size;
                    ui_context.pressed_window = window;
                    ui_context.window_pressed = false;
                    ui_context.any_window_hold = true;
                }
                set_resize_cursor(any_collision);
                if (any_collision)
                {
                    set_bit(window->flags, UI_WINDOW_AREA_HIT);
                    goto collision_check_done;
                }
                ui_context.non_docked_window_hover = true;
            }
            const u32 window_in_focus = ui_context.window_in_focus;
            if (check_window_collisions(render_data))
            {
                set_bit(window->flags, UI_WINDOW_AREA_HIT);
                if (ui_context.window_in_focus != window_in_focus)
                {
                    push_window_to_back(&ui_context.last_frame_windows, i);
                }
                goto collision_check_done;
            }
        }
        for (i32 i = ((i32)ui_context.last_frame_docked_windows.size) - 1; i >= 0; --i)
        {
            const Window_Render_Data* render_data = ui_context.last_frame_docked_windows.data + i;
            if (check_window_collisions(render_data))
            {
                set_bit(ui_window_get_(render_data->id)->flags, UI_WINDOW_AREA_HIT);
                goto collision_check_done;
            }
        }
    }
collision_check_done:;
}

internal void check_if_window_should_be_docked(void)
{
    b8 any_hit = false;
    // TODO: This could cause issues if we have an overlay window, it makes it
    // the window_in_focus. Solution, save the window that is dragging.
    Ui_Window* focused_window = ui_context.pressed_window;
    if (ui_context.dock_side_hit == 0)
    {
        dock_node_dock_window(ui_context.dock_hit_node, focused_window->dock_node, SPLIT_HORIZONTAL,
                              DOCK_SIDE_TOP);
        any_hit = true;
    }
    else if (ui_context.dock_side_hit == 1)
    {
        dock_node_dock_window(ui_context.dock_hit_node, focused_window->dock_node, SPLIT_VERTICAL,
                              DOCK_SIDE_RIGHT);
        any_hit = true;
    }
    else if (ui_context.dock_side_hit == 2)
    {
        dock_node_dock_window(ui_context.dock_hit_node, focused_window->dock_node, SPLIT_HORIZONTAL,
                              DOCK_SIDE_BOTTOM);
        any_hit = true;
    }
    else if (ui_context.dock_side_hit == 3)
    {
        dock_node_dock_window(ui_context.dock_hit_node, focused_window->dock_node, SPLIT_VERTICAL,
                              DOCK_SIDE_LEFT);
        any_hit = true;
    }
    else if (ui_context.dock_side_hit == 4)
    {
        Dock_Node* dock_node = focused_window->dock_node;
        for (u32 i = 0; i < dock_node->windows.size; ++i)
        {
            u32 window_id = dock_node->windows.data[i];
            array_push(&ui_context.dock_hit_node->windows, window_id);

            Ui_Window* window = ui_window_get_(window_id);
            window->dock_node = ui_context.dock_hit_node;
            window->position = ui_context.dock_hit_node->aabb.min;
            window->size = ui_context.dock_hit_node->aabb.size;
            set_bit(window->flags, UI_WINDOW_DOCKED);
        }
        for (u32 i = 0; i < ui_context.dock_hit_node->windows.size; ++i)
        {
            Dock_Node* node = ui_context.dock_hit_node;
            u32 window_id = node->windows.data[i];
            Ui_Window* window = ui_window_get_(window_id);
            if (window_id == focused_window->id)
            {
                unset_bit(window->flags, UI_WINDOW_HIDE);
                node->window_in_focus = i;
                ui_context.window_in_focus = ui_context.id_to_index.data[window->id];
            }
            else
            {
                set_bit(window->flags, UI_WINDOW_HIDE);
            }
        }
        free(dock_node->windows.data);
        dock_node->windows.data = NULL;
        free(dock_node);
    }
    if (any_hit)
    {
        Dock_Node* dock_node = focused_window->dock_node;
        for (u32 i = 0; i < dock_node->windows.size; ++i)
        {
            u32 id = dock_node->windows.data[i];
            Ui_Window* window_to_dock = ui_window_get_(id);
            set_bit(window_to_dock->flags, UI_WINDOW_DOCKED);
        }
    }
    ui_context.dock_side_hit = -1;
}

internal void check_dock_space_resize(void)
{
    const Mouse_Button_Event* event = event_get_mouse_button_event();
    if (event->action == SYNT_RELEASE)
    {
        ui_context.dock_resize = false;
    }
    ui_context.dock_resize_hover = false;

    if (ui_context.check_collisions && !event_get_key_event()->alt_pressed)
    {
        if (!ui_context.dock_resize)
        {
            if (dock_node_resize_collision_traverse(ui_context.dock_tree))
            {
                ui_context.dock_resize_hover = true;
                if (event->activated && event->action == SYNT_PRESS &&
                    event->button == SYNT_LEFT_BUTTON)
                {

                    ui_context.dock_resize = true;
                }

                // FTicWindow* ftic_window = window_get_current();
                if (ui_context.dock_hit_node->split_axis == SPLIT_HORIZONTAL)
                {
                    // window_set_cursor(ftic_window, FTIC_RESIZE_V_CURSOR);
                }
                else
                {
                    // window_set_cursor(ftic_window, FTIC_RESIZE_H_CURSOR);
                }
                add_default_quad_aabb_(&ui_context.extra_index_count, &ui_context.dock_resize_aabb,
                                       global_get_secondary_color());
            }
            else
            {
                // window_set_cursor(window_get_current(), FTIC_NORMAL_CURSOR);
            }
        }

        if (ui_context.dock_resize)
        {
            add_default_quad_aabb_(&ui_context.extra_index_count, &ui_context.dock_resize_aabb,
                                   global_get_secondary_color());
        }
    }
}

internal void check_and_display_mouse_drag_box(void)
{
    if (event_is_mouse_button_pressed_once(SYNT_LEFT_BUTTON))
    {
        ui_context.mouse_drag_box_point = event_get_mouse_position();
    }

    if (event_is_mouse_button_pressed(SYNT_LEFT_BUTTON) && event_get_key_event()->alt_pressed)
    {
        const V2 mouse_position = event_get_mouse_position();
        const V2 min_point = v2f(sy_min(ui_context.mouse_drag_box_point.x, mouse_position.x),
                                 sy_min(ui_context.mouse_drag_box_point.y, mouse_position.y));
        const V2 max_point = v2f(sy_max(ui_context.mouse_drag_box_point.x, mouse_position.x),
                                 sy_max(ui_context.mouse_drag_box_point.y, mouse_position.y));

        AABB_2D mouse_drag_box = {
            .min = min_point,
            .size = v2_sub(max_point, min_point),
        };

        add_default_quad_aabb_(&ui_context.extra_index_count, &mouse_drag_box,
                               ui_context.docking_color);

        V4 drag_border_color = ui_context.docking_color;
        drag_border_color.a = 0.9f;
        add_border_(&ui_context.extra_index_count, mouse_drag_box.min, mouse_drag_box.size,
                    drag_border_color);

        ui_context.mouse_drag_box = mouse_drag_box;
        ui_context.mouse_box_is_dragging = true;
    }
    else
    {
        ui_context.mouse_drag_box = (AABB_2D){ 0 };
        ui_context.mouse_box_is_dragging = false;
    }
}

internal void sync_current_frame_windows(Window_Render_Data_Array* current_frame,
                                         Window_Render_Data_Array* last_frame)
{
    // TODO: can be very expensive. Consider a more efficient way.
    for (u32 i = 0; i < last_frame->size; ++i)
    {
        u32 id = last_frame->data[i].id;
        b8 exist = false;
        for (u32 j = 0; j < current_frame->size; ++j)
        {
            if (current_frame->data[j].id == id)
            {
                last_frame->data[i] = current_frame->data[j];
                push_window_to_back(current_frame, j--);
                current_frame->size--;
                exist = true;
                break;
            }
        }
        if (!exist)
        {
            push_window_to_back(last_frame, i--);
            last_frame->size--;
        }
    }

    for (u32 i = 0; i < current_frame->size; ++i)
    {
        array_push(last_frame, current_frame->data[i]);
    }
    current_frame->size = 0;
}

internal void string_swap(char* first, char* second)
{
    char temp[4] = { 0 };
    const size_t length = sizeof(temp);
    memcpy(temp, first, length);
    memcpy(first, second, length);
    memcpy(second, temp, length);
}

internal u32 display_text_and_truncate_if_necissary(const V2 position, const f32 total_width,
                                                    const f32 alpha, char* text)
{
    const u32 text_len = (u32)strlen(text);
    const i32 i =
        text_check_length_within_boundary(ui_context.font.chars, text, text_len, 1.0f, total_width);
    const b8 too_long = i >= 3;
    char saved_name[4] = "...";
    if (too_long)
    {
        i32 j = i - 3;
        string_swap(text + j, saved_name); // Truncate
    }
    u32 index_count =
        text_generation_color(ui_context.font.chars, text, UI_FONT_TEXTURE, position, 1.0f,
                              ui_context.font.pixel_height, v4a(global_get_text_color(), alpha),
                              NULL, NULL, NULL, &ui_context.main_vertex_buffer.array);
    if (too_long)
    {
        memcpy(text + (i - 3), saved_name, sizeof(saved_name));
    }
    return index_count;
}

typedef struct TabChange
{
    Dock_Node* dock_space;
    Ui_Window* window_to_hide;
    Ui_Window* window_to_show;
    u32 window_focus_index;
    b8 close_tab;
} TabChange;

internal TabChange update_tabs(const Window_Render_Data_Array* windows,
                               UU_32_Array* dock_spaces_index_offsets_and_counts)
{
    TabChange tab_change = { 0 };
    const V2 mouse_position = event_get_mouse_position();
    const b8 should_check_collision =
        ui_context.check_collisions && !ui_context.dock_resize_hover && !ui_context.dock_resize &&
        !ui_context.any_window_top_bar_hold && !ui_context.any_window_hold;

    const f32 top_bar_height = ui_context.font.pixel_height + 6.0f;

    b8 any_tab_hit = false;
    b8 any_hit = false;
    for (i32 i = ((i32)windows->size) - 1; i >= 0; --i)
    {
        Ui_Window* window = ui_window_get_(windows->data[i].id);
        Dock_Node* dock_space = window->dock_node;
        UU_32 index_offset_and_count = {
            .first = ui_context.current_index_offset,
        };
        const b8 in_focus = ui_context.id_to_index.data[window->id] == ui_context.window_in_focus &&
                            !ui_context.highlight_fucused_window_off;
        V4 window_border_color = global_get_border_color();

        const f32 min_tab_width = window->size.width / dock_space->windows.size;
        if (in_focus)
        {
            window_border_color = global_get_secondary_color();
        }
        if (check_bit(window->flags, UI_WINDOW_TOP_BAR))
        {
            const V2 top_bar_dimensions = v2f(window->size.width, top_bar_height);
            const V4 top_bar_color = v4a(v4_s_multi(global_get_clear_color(), 1.2f), 1.0f);
            const AABB_2D aabb = quad_gradiant_t_b(&ui_context.main_vertex_buffer.array,
                                                   window->position, top_bar_dimensions,
                                                   top_bar_color, global_get_clear_color(), 0.0f);
            index_offset_and_count.second += 6;

            V2 tab_position = window->position;
            for (i32 j = 0; j < (i32)dock_space->windows.size; ++j)
            {
                Ui_Window* tab_window = ui_window_get_(dock_space->windows.data[j]);

                const V2 button_size = v2i(top_bar_dimensions.height - 4.0f);

                const f32 tab_padding = 8.0f;
                V2 tab_dimensions = v2f(100.0f + (tab_padding * 2.0f) + button_size.width + 8.0f,
                                        top_bar_dimensions.height);

                tab_dimensions.width = sy_min(tab_dimensions.width, min_tab_width);

                AABB_2D tab_aabb = {
                    .min = tab_position,
                    .size = tab_dimensions,
                };

                V4 tab_color = global_get_tab_color();
                if (j == (i32)dock_space->window_in_focus)
                {
                    tab_color = v4a(v4_s_multi(tab_color, 1.2f), 1.0f);
                }
                b8 tab_collided = false;
                if (should_check_collision && check_bit(window->flags, UI_WINDOW_AREA_HIT) &&
                    !tab_change.close_tab && !any_tab_hit &&
                    collision_point_in_aabb_2d(mouse_position, &tab_aabb))
                {
                    const b8 window_in_focus = j == (i32)dock_space->window_in_focus;
                    if (!window_in_focus)
                    {
                        if (event_is_mouse_button_clicked(SYNT_LEFT_BUTTON))
                        {
                            tab_change.dock_space = dock_space;
                            tab_change.window_focus_index = j;
                            tab_change.window_to_hide = window;
                            tab_change.window_to_show = tab_window;
                        }
                        tab_color = v4ic(0.3f);
                    }
                    if (event_is_mouse_button_pressed_once(SYNT_LEFT_BUTTON))
                    {
                        ui_context.window_pressed_release_from_dock_space = true;
                        ui_context.pressed_window = tab_window;
                        ui_context.window_pressed_offset = event_get_mouse_position();
                        ui_context.window_pressed = true;
                    }

                    any_hit = true;

                    tab_collided = true;
                }
                add_default_quad_(&index_offset_and_count.second, tab_position, tab_dimensions,
                                  tab_color);

                V2 button_position =
                    v2f(tab_position.x + tab_dimensions.width - button_size.width - 5.0f,
                        tab_position.y + 2.0f);

                AABB_2D button_aabb = {
                    .min = button_position,
                    .size = button_size,
                };

                V2 text_position = v2f(tab_position.x + tab_padding,
                                       tab_position.y + ui_context.font.pixel_height);
                if (tab_window->title.size)
                {
                    index_offset_and_count.second += display_text_and_truncate_if_necissary(
                        text_position, button_position.x - text_position.x, tab_window->alpha,
                        tab_window->title.data);
                }

                b8 collided = collision_point_in_aabb_2d(mouse_position, &button_aabb);

                if (should_check_collision && collided &&
                    check_bit(window->flags, UI_WINDOW_AREA_HIT))
                {
                    if (event_is_mouse_button_pressed_once(SYNT_LEFT_BUTTON))
                    {
                        tab_change.close_tab = true;
                        tab_change.dock_space = dock_space;
                        tab_change.window_focus_index = j;
                        tab_change.window_to_hide = tab_window;
                    }
                }

                V4 button_color = tab_color;
                if (collided && event_get_mouse_button_event()->action != SYNT_PRESS)
                {
                    button_color = v4ic(0.5f);
                }
                else if (tab_collided)
                {
                    button_color = v4ic(0.4f);
                }
                add_default_quad_(&index_offset_and_count.second, button_position, button_size,
                                  button_color);

                tab_position.x += tab_dimensions.width;
            }
            add_border_(&index_offset_and_count.second, window->position, top_bar_dimensions,
                        global_get_border_color());

            if (should_check_collision && check_bit(window->flags, UI_WINDOW_AREA_HIT) &&
                !tab_change.close_tab && !any_tab_hit && !any_hit &&
                collision_point_in_aabb_2d(mouse_position, &aabb) &&
                event_is_mouse_button_pressed_once(SYNT_LEFT_BUTTON))
            {
                ui_context.window_pressed_release_from_dock_space = false;
                ui_context.pressed_window = window;
                ui_context.window_pressed_offset = event_get_mouse_position();
                ui_context.window_pressed = true;
                any_hit = true;
            }
        }
        add_border_(&index_offset_and_count.second, window->position, window->size,
                    window_border_color);

        dock_spaces_index_offsets_and_counts->data[i] = index_offset_and_count;
        ui_context.current_index_offset =
            index_offset_and_count.first + index_offset_and_count.second;
    }
    return tab_change;
}

#if 0
internal void add_frosted_background(V2 position, const V2 size, const u32 frosted_texture_index)
{
    V2 saved_position = position;
    position.y = ui_context.dimensions.y - (position.y + size.height);

    Texture_Coordinates texture_coordinates;
    texture_coordinates.coordinates[0] =
        v2f(position.x / ui_context.dimensions.width,
            (position.y + size.height) / ui_context.dimensions.height);
    texture_coordinates.coordinates[1] =
        v2f(position.x / ui_context.dimensions.width, position.y / ui_context.dimensions.height);
    texture_coordinates.coordinates[2] =
        v2f((position.x + size.width) / ui_context.dimensions.width,
            position.y / ui_context.dimensions.height);
    texture_coordinates.coordinates[3] =
        v2f((position.x + size.width) / ui_context.dimensions.width,
            (position.y + size.height) / ui_context.dimensions.height);

    set_up_verticies(&ui_context.frosted_render.vertices, saved_position, size, v4ic(1.0f),
                     (f32)frosted_texture_index, texture_coordinates);
}
#endif

#if 1
internal VkRect2D get_window_scissor(const AABB_2D* window_aabb)
{
    VkRect2D scissor = { 0 };
    scissor.offset.x = (i32)round_f32(window_aabb->min.x);
    scissor.offset.y = (i32)round_f32(window_aabb->min.y);
    scissor.extent.width = (u32)round_f32(window_aabb->size.width);
    scissor.extent.height = (u32)round_f32(window_aabb->size.height);

    scissor.offset.x = sy_clamp_low(scissor.offset.x, 0);
    scissor.offset.y = sy_clamp_low(scissor.offset.y, 0);
    return scissor;
}
#else
internal VkRect2D get_window_scissor(const AABB_2D* window_aabb)
{
    VkRect2D scissor = { 0 };
    scissor.offset.x = (int32_t)window_aabb->min.x;
    scissor.offset.y =
        (int32_t)(ui_context.dimensions.y - (window_aabb->min.y + window_aabb->size.height));
    scissor.extent.width = (u32)window_aabb->size.width;
    scissor.extent.height = (u32)window_aabb->size.height;
    return scissor;
}
#endif

internal void handle_tab_change_or_close(TabChange tab_change)
{
    u32 focused_window_id = 0;
    if (tab_change.close_tab)
    {
        ui_window_close(tab_change.window_to_hide->id);
    }
    else
    {
        tab_change.dock_space->window_in_focus = tab_change.window_focus_index;
        set_bit(tab_change.window_to_hide->flags, UI_WINDOW_HIDE);
        tab_change.window_to_show->position = tab_change.window_to_hide->position;
        tab_change.window_to_show->size = tab_change.window_to_hide->size;
        unset_bit(tab_change.window_to_show->flags, UI_WINDOW_HIDE);
        focused_window_id = tab_change.window_to_show->id;
    }
    ui_context.window_in_focus = ui_context.id_to_index.data[focused_window_id];
}

internal void ui_draw(VkCommandBuffer command_buffer, const VkRect2D* scissor, u32 index_offset,
                      u32 num_indices)
{
    vkCmdSetScissor(command_buffer, 0, 1, scissor);
    vkCmdDrawIndexed(command_buffer, num_indices, 1, index_offset, 0, 0);
}

internal void ui_render(void* data, VkCommandBuffer command_buffer, u32 semaphore_idx)
{
    vulkan_buffer_copy_data(&ui_context.uniform_buffers[semaphore_idx], &ui_context.vp,
                            sizeof(ui_context.vp));

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            ui_context.pipeline_layout, 0, 1,
                            &ui_context.descriptors.desc_sets[semaphore_idx], 0, NULL);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      ui_context.triangle_list_pipeline);

    M4 model_matrix = m4i(1.0f);
    vulkan_push_constant(command_buffer, ui_context.pipeline_layout, &model_matrix,
                         sizeof(model_matrix));

    vulkan_vertex_index_buffer_bind(command_buffer, &ui_context.main_vertex_buffer.buffer,
                                    &ui_context.main_index_buffer);

    VkViewport view_port = { 0 };
    view_port.width = ui_context.dimensions.width;
    view_port.height = ui_context.dimensions.height;
    view_port.maxDepth = 1.0f;

    vkCmdSetViewport(command_buffer, 0, 1, &view_port);

    Window_Render_Data_Array* docked_windows = &ui_context.last_frame_docked_windows;
    Window_Render_Data_Array* floating_windows = &ui_context.last_frame_windows;
    Window_Render_Data_Array* overlay_windows = &ui_context.last_frame_overlay_windows;

    for (u32 i = 0; i < docked_windows->size; ++i)
    {
        const Window_Render_Data* render_data = docked_windows->data + i;
        const VkRect2D scissor = get_window_scissor(&render_data->aabb);
        ui_draw(command_buffer, &scissor, render_data->index_offset, render_data->index_count);
        UU_32 index_offset_and_count = ui_context.docked_index_offsets_and_counts.data[i];
        ui_draw(command_buffer, &scissor, index_offset_and_count.first,
                index_offset_and_count.second);
    }

    VkRect2D full_screen_scissor = {
        .extent.width = (u32)ui_context.dimensions.width,
        .extent.height = (u32)ui_context.dimensions.height,
    };
    ui_draw(command_buffer, &full_screen_scissor, ui_context.extra_index_offset,
            ui_context.extra_index_count);

    for (u32 i = 0; i < floating_windows->size; ++i)
    {
        const Window_Render_Data* render_data = floating_windows->data + i;
        const VkRect2D scissor = get_window_scissor(&render_data->aabb);
        ui_draw(command_buffer, &scissor, render_data->index_offset, render_data->index_count);
        UU_32 index_offset_and_count = ui_context.floating_index_offsets_and_counts.data[i];
        ui_draw(command_buffer, &scissor, index_offset_and_count.first,
                index_offset_and_count.second);
    }

    for (u32 i = 0; i < overlay_windows->size; ++i)
    {
        const Window_Render_Data* render_data = overlay_windows->data + i;
        const VkRect2D scissor = get_window_scissor(&render_data->aabb);
        ui_draw(command_buffer, &scissor, render_data->index_offset, render_data->index_count);
        ui_draw(command_buffer, &scissor,
                ui_context.overlay_index_count + (ui_context.overlay_index_count * i),
                ui_context.overlay_index_count);
    }
}

internal void check_and_grow_vertex_buffer(VkDevice device, VkPhysicalDevice physical_device,
                                           Vertex_2D_Buffer* buffer)
{
    const u64 current_size_in_bytes = buffer->array.size * sizeof(Vertex_2D);
    if (current_size_in_bytes > buffer->buffer.size_bytes)
    {
        vulkan_buffer_destroy(device, buffer->buffer);
        buffer->buffer.size_bytes = current_size_in_bytes;
        vulkan_vertex_buffer_create_visible(device, physical_device, &buffer->buffer,
                                            buffer->array.data);
    }
}

internal void check_and_grow_index_buffer(VkDevice device, VkPhysicalDevice physical_device,
                                          VkCommandPool command_pool, VkQueue graphic_queue,
                                          Index_Buffer* buffer, const u32 current_size)
{
    if (current_size > buffer->array.size)
    {
        vulkan_buffer_destroy(device, buffer->buffer);
        buffer->array.size = 0;
        generate_indicies(&buffer->array, 0, current_size / 6);
        assert(buffer->array.size == current_size);

        buffer->buffer.size_bytes = buffer->array.size * sizeof(u32);
        vulkan_index_buffer_create_local(device, physical_device, command_pool, graphic_queue,
                                         buffer);
    }
}

void ui_context_end(Render_Task* copy_tasks, Render_Task* render_tasks)
{
    ui_context.extra_index_offset = ui_context.current_index_offset;
    ui_context.extra_index_count = 0;

    if (ui_context.any_window_top_bar_hold)
    {
        i32 hit = root_display_docking(ui_context.dock_tree);
        ui_context.dock_side_hit = hit;
        hit = dock_node_docking_display_traverse(ui_context.dock_tree);
        if (hit != -1) ui_context.dock_side_hit = hit;
    }
    else if (!ui_context.any_window_hold)
    {
        if (ui_context.dock_side_hit != -1)
        {
            check_if_window_should_be_docked();
        }
        if (!ui_context.non_docked_window_hover)
        {
            check_dock_space_resize();
        }
    }
    // check_and_display_mouse_drag_box();

    ui_context.current_index_offset = ui_context.extra_index_offset + ui_context.extra_index_count;

    Window_Render_Data_Array* docked_windows = &ui_context.last_frame_docked_windows;
    Window_Render_Data_Array* floating_windows = &ui_context.last_frame_windows;
    Window_Render_Data_Array* overlay_windows = &ui_context.last_frame_overlay_windows;
    sync_current_frame_windows(&ui_context.current_frame_docked_windows, docked_windows);
    sync_current_frame_windows(&ui_context.current_frame_windows, floating_windows);
    sync_current_frame_windows(&ui_context.current_frame_overlay_windows, overlay_windows);

    if (ui_context.dimensions.y == 0.0f) return;

    ui_context.docked_index_offsets_and_counts.size = 0;
    ui_context.floating_index_offsets_and_counts.size = 0;

    TabChange tab_change_docked =
        update_tabs(docked_windows, &ui_context.docked_index_offsets_and_counts);
    TabChange tab_change =
        update_tabs(floating_windows, &ui_context.floating_index_offsets_and_counts);

    ui_context.overlay_index_offset = ui_context.current_index_offset;
    ui_context.overlay_index_count = 0;
    for (u32 i = 0; i < ui_context.last_frame_overlay_windows.size; ++i)
    {
        const AABB_2D* window_aabb = &ui_context.last_frame_overlay_windows.data[i].aabb;
        ui_context.overlay_index_count = 0;
        add_border(window_aabb->min, window_aabb->size, global_get_secondary_color());
    }

    ui_context.current_index_offset +=
        ui_context.overlay_index_count * ui_context.last_frame_overlay_windows.size;

    check_and_grow_vertex_buffer(ui_context.device, ui_context.physical_device,
                                 &ui_context.main_vertex_buffer);
    check_and_grow_index_buffer(ui_context.device, ui_context.physical_device,
                                ui_context.command_pool, ui_context.graphic_queue,
                                &ui_context.main_index_buffer, ui_context.current_index_offset);

    vulkan_buffer_copy_data(&ui_context.main_vertex_buffer.buffer,
                            ui_context.main_vertex_buffer.array.data,
                            ui_context.main_vertex_buffer.array.size * sizeof(Vertex_2D));

    Render_Task task = { .callback = ui_render, .data = NULL };
    region_array_push(render_tasks, task);

    if (tab_change_docked.dock_space)
    {
        handle_tab_change_or_close(tab_change_docked);
    }
    if (tab_change.dock_space)
    {
        handle_tab_change_or_close(tab_change);
    }
}

void ui_context_destroy(void)
{
    save_layout();

    array_free(&ui_context.id_to_index);
    array_free(&ui_context.free_indices);
    array_free(&ui_context.animation_x);

    array_free(&ui_context.last_frame_windows);
    array_free(&ui_context.current_frame_windows);

    array_free(&ui_context.last_frame_docked_windows);
    array_free(&ui_context.current_frame_docked_windows);

    array_free(&ui_context.last_frame_overlay_windows);
    array_free(&ui_context.current_frame_overlay_windows);

    array_free(&ui_context.windows);
    array_free(&ui_context.window_aabbs);
    array_free(&ui_context.window_hover_clicked_indices);
}

void ui_context_set_window_in_focus(const u32 window_id)
{
    ui_context.window_in_focus = ui_context.id_to_index.data[window_id];
}

void ui_context_set_animation(b8 on)
{
    ui_context.animation_off = !on;
}

void ui_context_set_highlight_focused_window(b8 on)
{
    ui_context.highlight_fucused_window_off = !on;
}

u32 ui_window_create(void)
{
    u32 id =
        generate_id(ui_context.windows.size, &ui_context.free_indices, &ui_context.id_to_index);

    for (u32 i = 0; i < ui_context.windows.size; ++i)
    {
        Ui_Window* window = ui_context.windows.data + i;
        if (window->id == id)
        {
            ui_context.id_to_index.data[id] = i;
            return id;
        }
    }
    Dock_Node* node = dock_node_create(NODE_LEAF, SPLIT_NONE, id);
    insert_window(node, id, false);
    return id;
}

const Ui_Window* ui_window_get(const u32 window_id)
{
    return ui_window_get_(window_id);
}

void ui_window_set_end_scroll_offset(const u32 window_id, const f32 offset)
{
    ui_window_get_(window_id)->end_scroll_offset = offset;
}

void ui_window_set_current_scroll_offset(const u32 window_id, const f32 offset)
{
    ui_window_get_(window_id)->current_scroll_offset = offset;
}

u32 ui_window_in_focus(void)
{
    return ui_context.windows.data[ui_context.window_in_focus].id;
}

void ui_window_set_alpha(const u32 window_id, const f32 alpha)
{
    ui_window_get_(window_id)->alpha = alpha;
}

internal void window_animate(Ui_Window* window)
{
    const f64 animation_speed = 1.0;
#if 1
    f32* x = ui_context.animation_x.data + window->id;
    if (*x <= 1.0f && !ui_context.animation_off)
    {
        f32 value = ease_out_cubic(*x);
        value = sy_clamp_high(value, 1.0f);
        window->size = v2_lerp(window->size, window->dock_node->aabb.size, value);
        window->position = v2_lerp(window->position, window->dock_node->aabb.min, value);
        if (window == ui_context.pressed_window && ui_context.any_window_top_bar_hold)
        {
            const V2 mouse_position = event_get_mouse_position();
            window->position.x = mouse_position.x - (window->size.width * 0.5f);
            window->dock_node->aabb.min.x = window->position.x;
            ui_context.window_pressed_offset = v2_sub(window->position, mouse_position);
        }
        *x += (f32)(ui_context.delta_time * animation_speed);
    }
    else
    {
        window->size = window->dock_node->aabb.size;
        window->position = window->dock_node->aabb.min;
    }

#else
    const V2 size_diff = v2_sub(window->dock_node->aabb.size, window->size);
    window->size = v2_add(window->size, v2_s_multi(size_diff, delta));

    const V2 position_diff = v2_sub(window->dock_node->aabb.min, window->position);
    window->position = v2_add(window->position, v2_s_multi(position_diff, delta));

    if (window == ui_context.pressed_window && ui_context.any_window_top_bar_hold)
    {
        const V2 mouse_position = event_get_mouse_position();
        window->position.x = mouse_position.x - (window->size.width * 0.5f);
        window->dock_node->aabb.min.x = window->position.x;
        ui_context.window_pressed_offset = v2_sub(window->position, mouse_position);
    }
#endif
}

b8 ui_window_begin(u32 window_id, const char* title, b8 draw_back, u8 flags)
{
    ui_context.current_window_id = window_id;

    const u32 window_index = ui_context.id_to_index.data[window_id];
    Ui_Window* window = ui_context.windows.data + window_index;
    AABB_2D_Array* aabbs = ui_context.window_aabbs.data + window_index;
    aabbs->size = 0;

    window->flags = flags | (window->flags & (UI_WINDOW_DOCKED | UI_WINDOW_HIDE |
                                              UI_WINDOW_AREA_HIT | UI_WINDOW_CLOSING));

    // ui_context.current_window_texture_offset = ui_context.render.render.textures.size;

    if (event_get_mouse_button_event()->action == SYNT_RELEASE)
    {
        window->scroll_bar_dragging = SCROLL_BAR_NONE;
    }
    if (title)
    {
        const u32 title_length = (u32)strlen(title);
        if (!window->title.data)
        {
            array_create(&window->title, (u32)strlen(title));
        }
        window->title.size = 0;
        for (u32 i = 0; i < title_length; ++i)
        {
            array_push(&window->title, title[i]);
        }
        array_push(&window->title, '\0');
        array_push(&window->title, '\0');
        array_push(&window->title, '\0');
    }
    else
    {
        window->title.size = 0;
    }

    window_animate(window);

    const f32 top_bar_height = ui_context.font.pixel_height + 6.0f;

    if (check_bit(window->flags, UI_WINDOW_HIDE)) return false;

    ui_context.current_window_total_height = 0.0f;
    ui_context.current_window_total_width = 0.0f;

    ui_context.current_window_first_item_position = window->position;
    ui_context.current_window_first_item_position.y +=
        top_bar_height * check_bit(window->flags, UI_WINDOW_TOP_BAR);

    ui_context.current_window_index_count = 0;

    // window->position = round_v2(window->position);
    // window->size = round_v2(window->size);

    V4 top_color = ui_context.current_window_top_color;
    V4 bottom_color = ui_context.current_window_bottom_color;
    if (check_bit(window->flags, UI_WINDOW_FROSTED_GLASS))
    {
        const f32 alpha = 0.8f + (0.2f * !ui_frosted_glass);
        top_color.a = alpha;
        bottom_color.a = alpha;
    }
    AABB_2D aabb = { .min = window->position, .size = window->size };
    array_push(aabbs, aabb);
    if (draw_back)
    {
        quad_gradiant_t_b(&ui_context.main_vertex_buffer.array, window->position, window->size,
                          top_color, bottom_color, 0.0f);
        ui_context.current_window_index_count += 6;
    }

    return true;
}

internal void add_scroll_bar_height(Ui_Window* window, AABB_2D_Array* aabbs,
                                    Hover_Clicked_Index hover_clicked_index)
{
    const f32 scroll_bar_width = 8.0f;
    V2 position = v2f(window->position.x + window->size.width,
                      ui_context.current_window_first_item_position.y);
    position.x -= scroll_bar_width;

    const f32 area_height = window->size.height;
    const f32 total_height = ui_context.current_window_total_height;
    if (area_height < total_height)
    {
        quad(&ui_context.main_vertex_buffer.array, position, v2f(scroll_bar_width, area_height),
             global_get_tab_color(), 0.0f);
        ui_context.current_window_index_count += 6;

        const f32 initial_y = position.y;
        const f32 high = 0.0f;
        const f32 low = area_height - total_height;
        const f32 p = (window->current_scroll_offset - low) / (high - low);
        const V2 scroll_bar_dimensions =
            v2f(scroll_bar_width, area_height * (area_height / total_height));

        const f32 lower_end = window->position.y + window->size.height;
        position.y = sy_lerp(lower_end - scroll_bar_dimensions.height, initial_y, p);

        V2 mouse_position = event_get_mouse_position();
        b8 collided = hover_clicked_index.index == (i32)aabbs->size;

        AABB_2D scroll_bar_aabb = {
            .min = position,
            .size = scroll_bar_dimensions,
        };
        array_push(aabbs, scroll_bar_aabb);

        if (collided || check_bit(window->scroll_bar_dragging, SCROLL_BAR_HEIGHT))
        {
            quad(&ui_context.main_vertex_buffer.array, position, scroll_bar_dimensions,
                 v4a(v4_s_multi(global_get_scroll_bar_color(), 1.4f), 1.0f), 0.0f);
            ui_context.current_window_index_count += 6;
        }
        else
        {
            quad(&ui_context.main_vertex_buffer.array, position, scroll_bar_dimensions,
                 v4a(v4_s_multi(global_get_scroll_bar_color(), 1.2f), 1.0f), 0.0f);
            ui_context.current_window_index_count += 6;
        }

        if (hover_clicked_index.pressed && collided)
        {
            ui_context.any_window_hold = true;
            window->scroll_bar_dragging = SCROLL_BAR_HEIGHT;
            window->scroll_bar_mouse_pointer_offset = scroll_bar_aabb.min.y - mouse_position.y;
        }

        if (hover_clicked_index.clicked)
        {
            window->scroll_bar_dragging = SCROLL_BAR_NONE;
        }

        if (check_bit(window->scroll_bar_dragging, SCROLL_BAR_HEIGHT))
        {
            const f32 end_position_y = lower_end - scroll_bar_dimensions.height;

            f32 new_y = mouse_position.y + window->scroll_bar_mouse_pointer_offset;

            // Clipping
            if (new_y < initial_y)
            {
                window->scroll_bar_mouse_pointer_offset = scroll_bar_aabb.min.y - mouse_position.y;
                new_y = initial_y;
            }
            if (new_y > end_position_y)
            {
                window->scroll_bar_mouse_pointer_offset = scroll_bar_aabb.min.y - mouse_position.y;
                new_y = end_position_y;
            }

            const f32 offset_p = (new_y - initial_y) / (end_position_y - initial_y);

            f32 lerp = sy_lerp(high, low, offset_p);

            lerp = sy_clamp_high(lerp, high);
            lerp = sy_clamp_low(lerp, low);
            window->current_scroll_offset = lerp;
            window->end_scroll_offset = lerp;
        }
    }
}

internal void add_scroll_bar_width(Ui_Window* window, AABB_2D_Array* aabbs,
                                   Hover_Clicked_Index hover_clicked_index)
{
    const f32 scroll_bar_height = 8.0f;
    V2 position = v2f(window->position.x, window->position.y + window->size.y);
    position.y -= scroll_bar_height;

    const f32 area_width = window->size.width;
    const f32 total_width = ui_context.current_window_total_width;
    if (area_width < total_width)
    {
        quad(&ui_context.main_vertex_buffer.array, position, v2f(area_width, scroll_bar_height),
             global_get_highlight_color(), 0.0f);
        ui_context.current_window_index_count += 6;

        const f32 initial_x = position.x;
        const f32 high = 0.0f;
        const f32 low = area_width - total_width;
        const f32 p = (window->current_scroll_offset_width - low) / (high - low);
        const V2 scroll_bar_dimensions =
            v2f(area_width * (area_width / total_width), scroll_bar_height);

        const f32 lower_end = window->position.x + window->size.width;
        position.x = sy_lerp(lower_end - scroll_bar_dimensions.width, initial_x, p);

        V2 mouse_position = event_get_mouse_position();
        b8 collided = hover_clicked_index.index == (i32)aabbs->size;

        AABB_2D scroll_bar_aabb = {
            .min = position,
            .size = scroll_bar_dimensions,
        };
        array_push(aabbs, scroll_bar_aabb);

        if (collided || check_bit(window->scroll_bar_dragging, SCROLL_BAR_WIDTH))
        {
            quad(&ui_context.main_vertex_buffer.array, position, scroll_bar_dimensions,
                 global_get_bright_color(), 0.0f);
            ui_context.current_window_index_count += 6;
        }
        else
        {
            quad_gradiant_t_b(&ui_context.main_vertex_buffer.array, position, scroll_bar_dimensions,
                              global_get_lighter_color(), v4ic(0.45f), 0.0f);
            ui_context.current_window_index_count += 6;
        }

        if (hover_clicked_index.pressed && collided)
        {
            ui_context.any_window_hold = true;
            window->scroll_bar_dragging = SCROLL_BAR_WIDTH;
            window->scroll_bar_mouse_pointer_offset = scroll_bar_aabb.min.x - mouse_position.x;
        }

        if (window->scroll_bar_dragging)
        {
            const f32 end_position_x = lower_end - scroll_bar_dimensions.width;

            f32 new_x = mouse_position.x + window->scroll_bar_mouse_pointer_offset;

            // Clipping
            if (new_x < initial_x)
            {
                window->scroll_bar_mouse_pointer_offset = scroll_bar_aabb.min.x - mouse_position.x;
                new_x = initial_x;
            }
            if (new_x > end_position_x)
            {
                window->scroll_bar_mouse_pointer_offset = scroll_bar_aabb.min.x - mouse_position.x;
                new_x = end_position_x;
            }

            const f32 offset_p = (new_x - initial_x) / (end_position_x - initial_x);

            f32 lerp = sy_lerp(high, low, offset_p);

            lerp = sy_clamp_high(lerp, high);
            lerp = sy_clamp_low(lerp, low);
            window->current_scroll_offset_width = lerp;
            window->end_scroll_offset_width = lerp;
        }
    }
}

b8 ui_window_end(b8 reset_textures)
{
    const u32 window_index = ui_context.id_to_index.data[ui_context.current_window_id];
    Ui_Window* window = ui_context.windows.data + window_index;

    AABB_2D_Array* aabbs = ui_context.window_aabbs.data + window_index;

    Hover_Clicked_Index hover_clicked_index =
        ui_context.window_hover_clicked_indices.data[window_index];

    add_scroll_bar_height(window, aabbs, hover_clicked_index);
    add_scroll_bar_width(window, aabbs, hover_clicked_index);

    if (!window->scroll_bar_dragging)
    {
        if (event_get_mouse_wheel_event()->activated &&
            check_bit(window->flags, UI_WINDOW_AREA_HIT))
        {
            window->scroll_x = 0.0f;
            window->start_scroll_offset = window->current_scroll_offset;
            window->end_scroll_offset = set_scroll_offset(
                ui_context.current_window_total_height, window->size.y, window->end_scroll_offset);
        }
        if (ui_context.delta_time < 0.5f)
        {
            smooth_scroll(window, ui_context.delta_time, 4.0f);
        }
    }

    const V2 size_diff = v2_sub(window->dock_node->aabb.size, window->size);
    const V2 position_diff = v2_sub(window->dock_node->aabb.min, window->position);
    const b8 size_animation_on =
        !closed_interval(-1.0f, size_diff.x, 1.0f) && !closed_interval(-1.0f, size_diff.y, 1.0f);
    const b8 position_animation_on = !closed_interval(-1.0f, position_diff.x, 1.0f) &&
                                     !closed_interval(-1.0f, position_diff.y, 1.0f);

    if (!check_bit(window->flags, UI_WINDOW_HIDE) &&
        (!check_bit(window->flags, UI_WINDOW_CLOSING) || size_animation_on ||
         position_animation_on))
    {
        Window_Render_Data render_data = render_data_create(window);
        if (check_bit(window->flags, UI_WINDOW_OVERLAY))
        {
            array_push(&ui_context.current_frame_overlay_windows, render_data);
        }
        else if (check_bit(window->flags, UI_WINDOW_DOCKED))
        {
            array_push(&ui_context.current_frame_docked_windows, render_data);
        }
        else
        {
            array_push(&ui_context.current_frame_windows, render_data);
        }
    }
    ui_context.current_index_offset += ui_context.current_window_index_count;
    ui_context.current_window_index_count = 0;
    ui_context.current_window_bottom_color = global_get_clear_color();
    ui_context.current_window_top_color = global_get_clear_color();

    if (check_bit(window->flags, UI_WINDOW_CLOSING) && !size_animation_on && !position_animation_on)
    {
        unset_bit(window->flags, UI_WINDOW_CLOSING);
        // ui_context.render.render.textures.size = ui_context.current_window_texture_offset;
        return true;
    }
    if (check_bit(window->flags, UI_WINDOW_OVERLAY))
    {
        b8 closing = !check_bit(window->flags, UI_WINDOW_AREA_HIT) &&
                     (event_is_mouse_button_pressed_once(SYNT_LEFT_BUTTON) ||
                      event_is_mouse_button_pressed_once(SYNT_RIGHT_BUTTON));
        if (closing)
        {
            if (reset_textures)
            {
                // ui_context.render.render.textures.size =
                // ui_context.current_window_texture_offset;
            }
            return true;
        }
    }
    return false;
}

b8 ui_window_is_hit(const u32 window_id)
{
    const u32 window_index = ui_context.id_to_index.data[window_id];
    Ui_Window* window = ui_context.windows.data + window_index;
    return check_bit(window->flags, UI_WINDOW_AREA_HIT);
}

b8 ui_window_add_icon_button(V2 position, const V2 size, const V4 hover_color,
                             const V4 texture_coordinates, const f32 texture_index,
                             const b8 disable, Ui_Layout* layout)
{
    const u32 window_index = ui_context.id_to_index.data[ui_context.current_window_id];
    AABB_2D_Array* aabbs = ui_context.window_aabbs.data + window_index;
    Hover_Clicked_Index hover_clicked_index =
        ui_context.window_hover_clicked_indices.data[window_index];

    position = add_first_item_offset(position);

    V4 button_color = global_get_text_color();
    b8 hover = false;
    b8 clicked = false;
    if (disable)
    {
        button_color = global_get_border_color();
    }
    else
    {
        if (hover_clicked_index.index == (i32)aabbs->size)
        {
            hover = hover_clicked_index.hover;
            clicked = hover_clicked_index.clicked;
        }
    }

    AABB_2D button_aabb = {
        .min = position,
        .size = size,
    };
    array_push(aabbs, button_aabb);

    if (hover && event_get_mouse_button_event()->action == SYNT_RELEASE)
    {
        add_default_quad_aabb(&button_aabb, hover_color);
    }

    V2 icon_size = v2_s_multi(button_aabb.size, 0.7f);
    V2 icon_position = v2_add(
        position, v2f(middle(size.width, icon_size.width), middle(size.height, icon_size.height)));

    add_quad_co(icon_position, icon_size, button_color, texture_coordinates, texture_index);

    ui_layout_set_width_and_height(layout, button_aabb.size.width, button_aabb.size.height);
    return clicked;
}

internal void input_buffer_select_char(Input_Buffer* input, u32 index)
{
    if (input->start_selection_index == -1)
    {
        input->start_selection_index = index;
    }
    input->end_selection_index = index + 1;
    array_push(&input->chars_selected, input->chars.data[index]);
}

internal void render_input(Ui_Window* window, const f64 delta_time, const V2 text_position,
                           const b8 hit, Input_Buffer* input)
{
    // Blinking cursor
    if (input->active && input->chars_selected.size == 0)
    {
        if (input->input_index < 0)
        {
            input->input_index = input->buffer.size;
        }
        else
        {
            input->input_index = sy_min((i32)input->buffer.size, input->input_index);
        }
        if (event_is_key_pressed(SYNT_KEY_LEFT))
        {
            input->input_index = sy_max(input->input_index - 1, 0);
            if (event_get_key_event()->ctrl_pressed)
            {
                for (--input->input_index; input->input_index >= 0; --input->input_index)
                {
                    char current_char = input->buffer.data[input->input_index];
                    if (current_char == '.' || current_char == '\\' || current_char == '-')
                    {
                        input->input_index++;
                        break;
                    }
                }
            }
            input->input_index = sy_max(input->input_index, 0);
            input->time = 0.4f;
        }
        if (event_is_key_pressed(SYNT_KEY_RIGHT))
        {
            input->input_index = sy_min(input->input_index + 1, (i32)input->buffer.size);
            if (event_get_key_event()->ctrl_pressed)
            {
                for (++input->input_index; input->input_index < (i32)input->buffer.size;
                     ++input->input_index)
                {
                    char current_char = input->buffer.data[input->input_index];
                    if (current_char == '.' || current_char == '\\' || current_char == '-')
                    {
                        break;
                    }
                }
            }
            input->input_index = sy_min(input->input_index, (i32)input->buffer.size);
            input->time = 0.4f;
        }
        input->time += (f32)delta_time;
        if (input->time >= 0.4f)
        {
            const f32 x_advance =
                text_x_advance(ui_context.font.chars, input->buffer.data, input->input_index, 1.0f);

            add_default_quad(v2f(text_position.x + x_advance + 1.0f, text_position.y + 1.0f),
                             v2f(1.0f, ui_context.font.pixel_height), v4i(1.0f));

            input->time = input->time >= 0.8f ? 0 : input->time;
        }
    }
    input->chars.size = 0;
    if (input->buffer.size)
    {
        add_text_selection_chars(window, get_text_position(text_position), input->buffer.data,
                                 &input->chars);
    }
    if (hit && event_is_mouse_button_pressed(SYNT_LEFT_BUTTON))
    {
        const V2 mouse_position = event_get_mouse_position();
        if (event_get_mouse_button_event()->activated)
        {
            // This happens only once when the button is pressed.
            input->selected_pivot_point = mouse_position.x;
            input->selected = true;
            input->time = 0.4f;
        }
        ui_input_buffer_clear_selection(input);
        if (input->active && input->chars.size)
        {
            for (u32 i = 0; i < input->chars.size; ++i)
            {
                const AABB_2D* current_aabb = &input->chars.data[i].aabb;
                const f32 middle_point = current_aabb->min.x + (current_aabb->size.width * 0.5f);

                if (mouse_position.x > middle_point)
                {
                    input->input_index = i + 1;
                    if (mouse_position.x > input->selected_pivot_point &&
                        middle_point > input->selected_pivot_point)
                    {
                        input_buffer_select_char(input, i);
                    }
                }
                else if (mouse_position.x < input->selected_pivot_point &&
                         middle_point < input->selected_pivot_point)
                {
                    input_buffer_select_char(input, i);
                }
            }
        }
    }
    else
    {
        input->selected = false;
    }
    if (input->chars_selected.size)
    {
        if (event_is_ctrl_and_key_pressed(SYNT_KEY_C))
        {
            ui_input_buffer_copy_selection_to_clipboard(input);
        }
        const f32 position_x = input->chars_selected.data[0].aabb.min.x;
        const AABB_2D* last_char = &array_back(&input->chars_selected)->aabb;
        const f32 size_width = (last_char->min.x + last_char->size.width) - position_x;
        add_default_quad(v2f(position_x, text_position.y),
                         v2f(size_width, ui_context.font.pixel_height + 5.0f),
                         ui_context.docking_color);
    }
}

internal b8 add_from_key_buffer(const f32 width, Input_Buffer* input)
{
    b8 key_typed = false;
    const Key_Buffer key_buffer = event_get_key_buffer();
    for (u32 i = 0; i < key_buffer.size; ++i)
    {
        char current_char = key_buffer.buffer[i];
        if (closed_interval(0, (current_char - 32), 96))
        {
            array_push(&input->buffer, current_char);
            f32 x_advance =
                text_x_advance(ui_context.font.chars, input->buffer.data, input->buffer.size, 1.0f);

            input->buffer.data[--input->buffer.size] = '\0';
            if (x_advance >= width)
            {
                return key_typed;
            }
            array_push(&input->buffer, '\0');
            for (i32 j = input->buffer.size - 1; j > input->input_index; --j)
            {
                input->buffer.data[j] = input->buffer.data[j - 1];
            }
            input->buffer.data[input->input_index++] = current_char;
            array_push(&input->buffer, '\0');
            array_push(&input->buffer, '\0');
            array_push(&input->buffer, '\0');
            input->buffer.size -= 3;
            key_typed = true;
        }
    }
    return key_typed;
}

internal b8 erase_char(Input_Buffer* input)
{
    const Key_Event* key_event = event_get_key_event();
    if (key_event->activated && key_event->action == SYNT_PRESS &&
        (key_event->key == SYNT_KEY_BACKSPACE ||
         (key_event->ctrl_pressed && key_event->key == SYNT_KEY_H)))
    {
        if (input->input_index > 0)
        {
            if (input->chars_selected.size)
            {
                ui_input_buffer_erase_from_selection(input);
            }
            else
            {
                input->input_index--;
                for (u32 i = input->input_index; i < input->buffer.size; ++i)
                {
                    input->buffer.data[i] = input->buffer.data[i + 1];
                }
                input->buffer.data[--input->buffer.size] = '\0';
            }
            return true;
        }
    }
    return false;
}

b8 ui_window_add_input_field(V2 position, const V2 size, Input_Buffer* input, Ui_Layout* layout)
{
    const u32 window_index = ui_context.id_to_index.data[ui_context.current_window_id];
    Ui_Window* window = ui_context.windows.data + window_index;
    AABB_2D_Array* aabbs = ui_context.window_aabbs.data + window_index;
    Hover_Clicked_Index hover_clicked_index =
        ui_context.window_hover_clicked_indices.data[window_index];

    position = add_first_item_offset(position);

    b8 hit = hover_clicked_index.index == (i32)aabbs->size;
    if (event_is_mouse_button_pressed_once(SYNT_LEFT_BUTTON) ||
        event_is_mouse_button_pressed_once(SYNT_RIGHT_BUTTON))
    {
        input->active = hit;
    }
    array_push(aabbs, add_default_quad(position, size, global_get_clear_color()));

    b8 typed = false;
    if (input->active)
    {
        typed = erase_char(input);
        typed |= add_from_key_buffer(size.width - 10.0f, input);

        if (event_is_ctrl_and_key_pressed(SYNT_KEY_V))
        {
            /*
            const char* clip_board = window_get_clipboard();
            if (clip_board)
            {
                const u32 clip_board_length = (u32)strlen(clip_board);
                for (u32 i = 0; i < clip_board_length; ++i)
                {
                    array_push(&input->buffer, clip_board[i]);
                    for (i32 j = ((i32)input->buffer.size) - 1; j > input->input_index; --j)
                    {
                        input->buffer.data[j] = input->buffer.data[j - 1];
                    }
                    input->buffer.data[input->input_index++] = clip_board[i];
                }
                array_push(&input->buffer, '\0');
                input->buffer.size--;
                typed = true;
            }
            */
        }
        else if (event_is_ctrl_and_key_pressed(SYNT_KEY_X))
        {
            /*
            if (input->chars_selected.size)
            {
                ui_input_buffer_copy_selection_to_clipboard(input);
                ui_input_buffer_erase_from_selection(input);
                typed = true;
            }
            */
        }
    }

    V2 text_position =
        v2f(position.x + 5.0f, position.y + (middle(size.y, ui_context.font.pixel_height) * 0.8f));
    render_input(window, ui_context.delta_time, text_position, hit, input);

    add_border(position, size, global_get_border_color());

    ui_layout_set_width_and_height(layout, size.width, size.height);
    return typed;
}

internal void text_set_scrolling_and_layout(Ui_Window* window, Ui_Layout* layout,
                                            const V2 relative_position, const u32 new_lines,
                                            const f32 x_advance, const b8 scrolling)
{
    const f32 height = new_lines * ui_context.font.line_height;
    if (scrolling)
    {
        ui_context.current_window_total_height =
            sy_max(ui_context.current_window_total_height, relative_position.y + height);
        ui_context.current_window_total_width =
            sy_max(ui_context.current_window_total_width, relative_position.x + x_advance + 20.0f);
    }
    ui_layout_set_width_and_height(layout, x_advance, height);
}

void ui_window_add_text_c(V2 position, V4 color, const char* text, b8 scrolling, Ui_Layout* layout)
{
    const u32 window_index = ui_context.id_to_index.data[ui_context.current_window_id];
    Ui_Window* window = ui_context.windows.data + window_index;

    V2 relative_position = position;
    position = add_scroll_offset(window, add_first_item_offset(position));

    u32 new_lines = 0;
    f32 x_advance = 0.0f;
    ui_context.current_window_index_count +=
        text_generation_color(ui_context.font.chars, text, UI_FONT_TEXTURE,
                              get_text_position(position), 1.0f, ui_context.font.line_height, color,
                              &new_lines, &x_advance, NULL, &ui_context.main_vertex_buffer.array);

    text_set_scrolling_and_layout(window, layout, relative_position, new_lines + 1, x_advance,
                                  scrolling);
}

void ui_window_add_text(V2 position, const char* text, b8 scrolling, Ui_Layout* layout)
{
    ui_window_add_text_c(position, global_get_text_color(), text, scrolling, layout);
}

void ui_window_add_text_colored(V2 position, const Colored_Character_Array* text, b8 scrolling,
                                Ui_Layout* layout)
{
    const u32 window_index = ui_context.id_to_index.data[ui_context.current_window_id];
    Ui_Window* window = ui_context.windows.data + window_index;

    V2 relative_position = position;
    position = add_scroll_offset(window, add_first_item_offset(position));

    u32 new_lines = 0;
    f32 x_advance = 0.0f;
    ui_context.current_window_index_count += text_generation_colored_char(
        ui_context.font.chars, text, UI_FONT_TEXTURE, get_text_position(position), 1.0f,
        ui_context.font.line_height, &new_lines, &x_advance, NULL,
        &ui_context.main_vertex_buffer.array);

    text_set_scrolling_and_layout(window, layout, relative_position, new_lines + 1, x_advance,
                                  scrolling);
}

/*
void ui_window_add_image(V2 position, V2 image_dimensions, u32 image, Ui_Layout* layout)
{
    const u32 window_index = ui_context.id_to_index.data[ui_context.current_window_id];
    Ui_Window* window = ui_context.windows.data + window_index;

    V2 relative_position = position;
    position = add_scroll_offset(window, add_first_item_offset(position));

    f32 texture_index = (f32)ui_context.render.render.textures.size;
    array_push(&ui_context.render.render.textures, image);

    add_quad(position, image_dimensions, v4ic(1.0f), texture_index);

    ui_context.current_window_total_height = sy_max(ui_context.current_window_total_height,
                                                    relative_position.y + image_dimensions.height);
    ui_context.current_window_total_width =
        sy_max(ui_context.current_window_total_width, relative_position.x + image_dimensions.width);

    ui_layout_set_width_and_height(layout, image_dimensions.width, image_dimensions.height);
}
*/

V2 ui_window_get_button_dimensions(V2 dimensions, const char* text, f32* x_advance_out)
{
    f32 x_advance = 20.0f;
    f32 pixel_height = 10.0f;
    if (text)
    {
        x_advance += text_x_advance(ui_context.font.chars, text, (u32)strlen(text), 1.0f);
        pixel_height += ui_context.font.pixel_height;

        if (x_advance_out)
        {
            *x_advance_out = x_advance - 20.0f;
        }
    }

    return v2f(sy_max(dimensions.width, x_advance), sy_max(dimensions.height, pixel_height));
}

b8 ui_window_add_button(V2 position, V2* dimensions, const V4* color, const char* text,
                        Ui_Layout* layout)
{
    const u32 window_index = ui_context.id_to_index.data[ui_context.current_window_id];
    AABB_2D_Array* aabbs = ui_context.window_aabbs.data + window_index;
    Hover_Clicked_Index hover_clicked_index =
        ui_context.window_hover_clicked_indices.data[window_index];

    position = add_first_item_offset(position);

    V2 end_dimensions;
    f32 x_advance;
    if (dimensions)
    {
        end_dimensions = ui_window_get_button_dimensions(*dimensions, text, &x_advance);
        *dimensions = end_dimensions;
    }
    else
    {
        end_dimensions = ui_window_get_button_dimensions(v2d(), text, &x_advance);
    }

    b8 collided = hover_clicked_index.index == (i32)aabbs->size;
    const V4 col = v4a(v4_s_multi(global_get_clear_color(), 1.5f), 1.0f);
    V4 button_color = color ? *color : col;
    if (collided && event_get_mouse_button_event()->action != SYNT_PRESS)
    {
        button_color = v4_s_multi(button_color, 1.5f);
        button_color.a = 1.0f;
        if (!color)
        {
            add_default_quad(position, end_dimensions, button_color);
        }
    }
    if (color)
    {
        add_default_quad(position, end_dimensions, button_color);
    }

    AABB_2D button_aabb = { .min = position, .size = end_dimensions };
    array_push(aabbs, button_aabb);

    if (text)
    {
        V2 text_position = v2f(position.x + middle(end_dimensions.width, x_advance),
                               position.y + ui_context.font.pixel_height + 2.0f);
        ui_context.current_window_index_count += text_generation(
            ui_context.font.chars, text, UI_FONT_TEXTURE, text_position, 1.0f,
            ui_context.font.line_height, NULL, NULL, NULL, &ui_context.main_vertex_buffer.array);
    }
    ui_layout_set_width_and_height(layout, button_aabb.size.width, button_aabb.size.height);

    return collided && hover_clicked_index.pressed;
}

i32 ui_window_add_menu_bar(Char_Ptr_Array* values, V2* position_of_clicked_item)
{
    const u32 window_index = ui_context.id_to_index.data[ui_context.current_window_id];
    Ui_Window* window = ui_context.windows.data + window_index;

    i32 index = -1;

    f32 pixel_height = 8.0f + ui_context.font.pixel_height;
    add_default_quad(window->position, v2f(ui_context.dimensions.width, pixel_height),
                     global_get_clear_color());

    Ui_Layout layout = ui_layout_create(window->position);
    for (u32 i = 0; i < values->size; i++)
    {
        V2 current_dimensions = v2d();
        if (ui_window_add_button(layout.at, &current_dimensions, NULL, values->data[i], &layout))
        {
            if (position_of_clicked_item)
            {
                *position_of_clicked_item =
                    v2f(layout.at.x, window->position.y + current_dimensions.height);
            }
            index = i;
        }
        ui_layout_column(&layout);
    }

    return index;
}

void ui_window_add_icon(V2 position, const V2 size, const V4 texture_coordinates,
                        const f32 texture_index, Ui_Layout* layout)
{
    position = add_first_item_offset(position);
    add_quad_co(position, size, v4ic(1.0f), texture_coordinates, texture_index);

    ui_layout_set_width_and_height(layout, size.width, size.height);
}

V2 ui_window_get_switch_size(void)
{
    return v2_s_add(v2f(24.0f, 4.0f), ui_context.font.pixel_height);
}

void ui_window_add_switch(V2 position, b8* selected, f32* x, Ui_Layout* layout)
{
    const u32 window_index = ui_context.id_to_index.data[ui_context.current_window_id];
    AABB_2D_Array* aabbs = ui_context.window_aabbs.data + window_index;
    Hover_Clicked_Index hover_clicked_index =
        ui_context.window_hover_clicked_indices.data[window_index];

    position = add_first_item_offset(position);

    b8 hover = hover_clicked_index.index == (i32)aabbs->size;
    if (hover && hover_clicked_index.clicked)
    {
        *selected ^= true;
        *x = 0.0f;
    }

    V2 size = ui_window_get_switch_size();
    AABB_2D aabb = { .min = position, .size = size };
    array_push(aabbs, add_default_quad_aabb(&aabb, global_get_highlight_color()));

    V4 color = *selected ? global_get_secondary_color() : global_get_border_color();
    V2 t_position = aabb.min;
    if (*selected)
    {
        t_position = v2_lerp(aabb.min, v2f(aabb.min.x + (aabb.size.width * 0.5f), aabb.min.y),
                             ease_out_cubic(*x));
        *x = sy_clamp_high(*x + (f32)ui_context.delta_time * 5.0f, 1.0f);
    }
    else
    {
        t_position = v2_lerp(v2f(aabb.min.x + (aabb.size.width * 0.5f), aabb.min.y), aabb.min,
                             ease_out_cubic(*x));
        *x = sy_clamp_high(*x + (f32)ui_context.delta_time * 5.0f, 1.0f);
    }
    add_default_quad(t_position, v2f(aabb.size.width * 0.5f, aabb.size.height), color);

    ui_layout_set_width_and_height(layout, size.width, size.height);
}

void ui_window_close(u32 window_id)
{
    const u32 window_index = ui_context.id_to_index.data[window_id];
    Ui_Window* window = ui_context.windows.data + window_index;
    Dock_Node* dock_space = window->dock_node;

    if (dock_space->windows.size == 1)
    {
        if (check_bit(window->flags, UI_WINDOW_DOCKED))
        {
            dock_node_remove_node(ui_context.dock_tree, dock_space);
        }
        set_bit(window->flags, UI_WINDOW_CLOSING);
        unset_bit(window->flags, UI_WINDOW_HIDE);
        unset_bit(window->flags, UI_WINDOW_DOCKED);
        V2 end_position = v2_add(window->position, v2_s_multi(window->size, 0.5f));
        ui_window_start_position_animation_(window, end_position);
        ui_window_start_size_animation_(window, v2d());
    }
    else
    {
        u32 i = 0;
        for (; i < dock_space->windows.size; ++i)
        {
            if (dock_space->windows.data[i] == window_id)
            {
                break;
            }
        }
        assert(i != dock_space->windows.size);
        u32 focused_window_id = remove_window_from_shared_dock_space(i, window, dock_space);
        ui_context.window_in_focus = ui_context.id_to_index.data[focused_window_id];
    }
}

void ui_window_close_current(void)
{
    set_bit(ui_window_get_(ui_context.current_window_id)->flags, UI_WINDOW_CLOSING);
}

void ui_window_set_size(u32 window_id, const V2 size)
{
    Ui_Window* window = ui_window_get_(window_id);
    window->size = round_v2(size);
    window->dock_node->aabb.size = window->size;
}

void ui_window_set_position(u32 window_id, const V2 position)
{
    Ui_Window* window = ui_window_get_(window_id);
    window->position = round_v2(position);
    window->dock_node->aabb.min = window->position;
}

f32 ui_window_add_slider(V2 position, V2 size, const f32 min_value, const f32 max_value, f32 value,
                         b8* pressed, Ui_Layout* layout)
{
    const u32 window_index = ui_context.id_to_index.data[ui_context.current_window_id];
    AABB_2D_Array* aabbs = ui_context.window_aabbs.data + window_index;
    Hover_Clicked_Index hover_clicked_index =
        ui_context.window_hover_clicked_indices.data[window_index];

    position = add_first_item_offset(position);

    b8 hit = hover_clicked_index.index == (i32)aabbs->size;
    b8 slider_hit = hover_clicked_index.index == (i32)aabbs->size + 1;

    V4 color = global_get_text_color();
    V4 slider_color = global_get_secondary_color();
    if (hit || slider_hit || *pressed)
    {
        color = v4a(v4_s_multi(color, 1.4f), 1.0f);
        slider_color = v4a(v4_s_multi(slider_color, 1.2f), 1.0f);
    }
    array_push(aabbs, add_default_quad(position, size, color));

    V2 slider_size = v2f(0.0f, size.height * 5.0f);
    slider_size.width = slider_size.height * 0.5f;

    if (slider_hit && hover_clicked_index.pressed)
    {
        *pressed = true;
    }
    if (event_get_mouse_button_event()->action == SYNT_RELEASE)
    {
        *pressed = false;
    }

    V2 slider_position = position;
    slider_position.y -= (slider_size.height - size.height) * 0.5f;

    const f32 start_x = position.x;
    const f32 end_x = position.x + (size.width - slider_size.width);

    if (*pressed || (hit && hover_clicked_index.pressed))
    {
        *pressed = true;
        f32 position_x = event_get_mouse_position().x - (slider_size.width * 0.5f);
        position_x = sy_clamp_low(position_x, start_x);
        position_x = sy_clamp_high(position_x, end_x);

        const f32 p = (position_x - start_x) / (end_x - start_x);
        value = sy_lerp(min_value, max_value, p);
    }

    value = sy_clamp_low(value, min_value);
    value = sy_clamp_high(value, max_value);

    const f32 p = (value - min_value) / (max_value - min_value);
    slider_position.x = sy_lerp(start_x, end_x, p);

    array_push(aabbs, add_default_quad(slider_position, slider_size, slider_color));
    ui_layout_set_width_and_height(layout, size.width, slider_size.height);

    return value;
}

internal V4 texture_interpolate_colors(const V2 p, const f32 width, const f32 height,
                                       const V4* colors)
{
    f32 texture_x = p.x * (width - 1);
    f32 texture_y = p.y * (height - 1);

    u32 x0 = (u32)texture_x;
    u32 y0 = (u32)texture_y;

    f32 x_fraction = texture_x - x0;
    f32 y_fraction = texture_y - y0;

    u32 index_tl = y0 * (u32)width + x0;
    u32 index_tr = y0 * (u32)width + x0 + 1;
    u32 index_bl = (y0 + 1) * (u32)width + x0;
    u32 index_br = (y0 + 1) * (u32)width + x0 + 1;

    if (x0 >= (u32)width - 1) x0 = (u32)width - 1;
    if (y0 >= (u32)height - 1) y0 = (u32)height - 1;
    if (index_tr >= (u32)width * (u32)height) index_tr = index_tl;
    if (index_bl >= (u32)width * (u32)height) index_bl = index_tl;
    if (index_br >= (u32)width * (u32)height) index_br = index_tl;

    V4 color_tl = colors[index_tl];
    V4 color_tr = colors[index_tr];
    V4 color_bl = colors[index_bl];
    V4 color_br = colors[index_br];

    V4 color_top = v4_lerp(color_tl, color_tr, x_fraction);
    V4 color_bottom = v4_lerp(color_bl, color_br, x_fraction);
    return v4_lerp(color_top, color_bottom, y_fraction);
}

#if 0
V4 ui_window_add_color_picker(V2 position, V2 size, Color_Picker* picker, Ui_Layout* layout)
{
    const u32 window_index = ui_context.id_to_index.data[ui_context.current_window_id];
    Ui_Window* window = ui_context.windows.data + window_index;
    AABB_2D_Array* aabbs = ui_context.window_aabbs.data + window_index;
    Hover_Clicked_Index hover_clicked_index =
        ui_context.window_hover_clicked_indices.data[window_index];

    position = add_first_item_offset(position);

    if (event_get_mouse_button_event()->action == SYNT_RELEASE)
    {
        picker->hold = false;
        picker->spectrum_hold = false;
    }

    b8 hover = hover_clicked_index.index == (i32)aabbs->size;

    const f32 padding = 10.0f;
    AABB_2D color_spectrum_aabb = {
        .min = v2f(position.x + size.x + padding, position.y),
        .size = v2f(30.0f, size.height),
    };
    array_push(aabbs, color_spectrum_aabb);

    if (hover && hover_clicked_index.pressed)
    {
        picker->spectrum_hold = true;
    }
    if (picker->spectrum_hold)
    {
        const f32 rel_mouse_position_y = event_get_mouse_position().y - position.y;
        picker->spectrum_at = rel_mouse_position_y;
        picker->spectrum_at = sy_clamp_high(picker->spectrum_at, size.height);
        picker->spectrum_at = sy_clamp_low(picker->spectrum_at, 0.0f);
    }

    const f32 position_p = picker->spectrum_at / size.height;
    f32 color_p = position_p * 6.0f;
    const u32 index_start = (u32)color_p;
    const u32 index_end = (index_start + 1) < 7 ? index_start + 1 : index_start;
    color_p -= index_start;
    V4 color_start = color_picker_spectrum_colors[index_start];
    V4 color_end = color_picker_spectrum_colors[index_end];
    V4 color = v4_lerp(color_start, color_end, color_p);

    const V4 colors[] = { v4ic(1.0f), color, v4ic(0.0f), v4ic(0.0f) };
    Texture_Properties texture_properties = {
        .bytes = calloc(16, sizeof(u8)),
        .channels = 4,
        .width = 2,
        .height = 2,
    };
    for (u32 i = 0, j = 0; i < 4; ++i)
    {
        texture_properties.bytes[j++] = (u8)(colors[i].r * 255.0f);
        texture_properties.bytes[j++] = (u8)(colors[i].g * 255.0f);
        texture_properties.bytes[j++] = (u8)(colors[i].b * 255.0f);
        texture_properties.bytes[j++] = (u8)(colors[i].a * 255.0f);
    }
    const u32 texture = texture_create(&texture_properties, GL_RGBA8, GL_RGBA, GL_LINEAR);
    free(texture_properties.bytes);
    array_push(&ui_context.generated_textures, texture);

    const f32 texture_index = (f32)ui_context.render.render.textures.size;
    array_push(&ui_context.render.render.textures, texture);

    hover = hover_clicked_index.index == (i32)aabbs->size;

    AABB_2D color_picker_aabb = {
        .min = position,
        .size = size,
    };
    array_push(aabbs, color_picker_aabb);

    if (hover && hover_clicked_index.pressed)
    {
        picker->hold = true;
    }

    if (picker->hold)
    {
        V2 rel_mouse_position = v2_sub(event_get_mouse_position(), position);
        rel_mouse_position = v2_clamp_high_low(rel_mouse_position, v2d(), size);
        picker->at = rel_mouse_position;
    }
    const V2 picker_p = v2_div(picker->at, size);
    const V4 picker_color = texture_interpolate_colors(picker_p, 2.0f, 2.0f, colors);

    const V2 cirle_size = v2i(12.0f + (8.0f * (f32)(picker->hold)));
    const V2 cirle_position = v2_sub(v2_add(position, picker->at), v2_s_multi(cirle_size, 0.5f));

    quad_co(&ui_context.render.vertices, color_picker_aabb.min, color_picker_aabb.size, v4ic(1.0f),
            quad_get_gradiant_texture_coordinates(), texture_index);
    ui_context.current_window_index_count += 6;

    add_circle(cirle_position, cirle_size, picker_color);
    quad_border_rounded(&ui_context.render.vertices, &ui_context.current_window_index_count,
                        v2_sub(cirle_position, v2i(2.0f)), v2_add(cirle_size, v2i(4.0f)),
                        v4ic(0.0f), 2.0f, 1.0f, 4, UI_DEFAULT_TEXTURE);

    add_quad_aabb(&color_spectrum_aabb, v4ic(1.0f), UI_COLOR_PICKER_TEXTURE);
    add_default_quad(v2f(color_spectrum_aabb.min.x, position.y + picker->spectrum_at),
                     v2f(color_spectrum_aabb.size.width, 3.0f), v4ic(0.0f));

    const AABB_2D color_display = {
        .min =
            v2f(color_spectrum_aabb.min.x + color_spectrum_aabb.size.width + padding, position.y),
        .size = v2i(40.0f),
    };
    add_default_quad_aabb(&color_display, picker_color);

    const V2 rgb_text_position =
        v2f(position.x, position.y + size.width + padding + ui_context.font.pixel_height);
    char buffer[64] = { 0 };
    value_to_string(buffer, "R: %.6f | G: %.6f | B: %.6f", picker_color.r, picker_color.g,
                    picker_color.b);
    f32 x_advance = add_text(window, rgb_text_position, buffer);

    const f32 width = color_picker_aabb.size.width + padding + color_spectrum_aabb.size.width +
                      padding + color_display.size.width;
    ui_layout_set_width_and_height(layout, sy_max(width, x_advance),
                                   size.height + padding + ui_context.font.pixel_height);
    return picker_color;
}
#endif

void ui_window_add_border(V2 position, const V2 size, const V4 color, const f32 thickness)
{
    position = add_first_item_offset(position);
    add_border(position, size, color);
}

void ui_window_add_rectangle(V2 position, const V2 size, const V4 color, Ui_Layout* layout)
{
    position = add_first_item_offset(position);
    add_default_quad(position, size, color);
    ui_layout_set_width_and_height(layout, size.width, size.height);
}

void ui_window_add_radio_button(V2 position, const V2 size, b8* selected, Ui_Layout* layout)
{
    const u32 window_index = ui_context.id_to_index.data[ui_context.current_window_id];
    AABB_2D_Array* aabbs = ui_context.window_aabbs.data + window_index;
    Hover_Clicked_Index hover_clicked_index =
        ui_context.window_hover_clicked_indices.data[window_index];

    position = add_first_item_offset(position);

    b8 hover = hover_clicked_index.index == (i32)aabbs->size;
    V4 color = global_get_secondary_color();
    if (hover)
    {
        color = v4a(v4_s_multi(color, 1.4f), 1.0f);
    }
    array_push(aabbs, add_default_quad(position, size, color));

    if (hover && hover_clicked_index.pressed)
    {
        *selected ^= true;
    }

    if (*selected)
    {
        V2 selected_size = v2_s_multi(size, 0.6f);
        V2 selected_position = v2f(position.x + middle(size.width, selected_size.width),
                                   position.y + middle(size.height, selected_size.height));
        add_default_quad(selected_position, selected_size, v4ic(0.0f));
    }
    ui_layout_set_width_and_height(layout, size.width, size.height);
}


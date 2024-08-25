#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#include "math/syntics_math.h"
#endif

typedef struct Texture_Coordinates
{
    V2 coordinates[4];
} Texture_Coordinates;

AABB_2D quad_co(Vertex_2D_Array* vert_array, V2 pos, V2 size, V4 color, V4 tex_coords, f32 tex_index);
AABB_2D quad(Vertex_2D_Array* vert_array, V2 pos, V2 size, V4 color, f32 tex_index);
AABB_2D quad_f(Vertex_2D_Array* vert_array, V2 pos, V2 size, V4 color, f32 tex_index);
AABB_2D quad_gradiant_l_r(Vertex_2D_Array* vert_array, V2 pos, V2 size, V4 left_color, V4 right_color, f32 tex_index);
AABB_2D quad_gradiant_t_b(Vertex_2D_Array* vert_array, V2 pos, V2 size, V4 top_color, V4 bottom_color, f32 tex_index);
AABB_2D border_add(Vertex_2D_Array* vert_array, u32* num_indices, V2 top_left, V2 size, V4 border_color, f32 thickness, f32 tex_index);
AABB_2D quad_with_border(Vertex_2D_Array* vert_array, u32* num_indices, V4 border_color, V2 top_left, V2 size, f32 thickness, f32 tex_index);
void quad_middle(Vertex_2D_Array* vert_array, V2 pos, V2 size, V4 color, f32 tex_index);
void polygon2D_draw_quads(Vertex_2D_Array* vert_array, Polygon2D poly, f32 z, V4 color, f32 line_width, f32 tex_index);
void polygon2D_draw_lines(Vertex_2D_Array* vert_array, U32_Array* idx_array, Polygon2D poly, f32 z, V4 color, f32 tex_index);
void generate_indicies(U32_Array* array, u32 offset, u32 indices_count);
u32 cube(Vertex_Array* vert_array, u32 offset, V3 pos, V3 size, V4 color, f32 tex_index);
void cube1(Vertex_Array* vert_array, V3 pos, V3 size, V4 color, f32 tex_index);
void cube_not_center1(Vertex_Array* vert_array, V3 pos, V3 size, V4 color, f32 tex_index);
u32 cube_not_center(Vertex_Array* vert_array, u32 offset, V3 pos, V3 size, V4 color, f32 tex_index);
void cube_indices_offset(U32_Array* indices, u32 offset, u32 how_many);
void cube_indices(U32_Array* indices, u32 offset, u32 how_many);
u32 gridd_using_line_list(Vertex_Array* vert_array, u32 vertex_offset, U32_Array* indices, u32 index_offset, V3 middle_pos, V2 spacing, u32 lines_width_count, u32 lines_height_count, V4 color, f32 tex_index);
AABB_2D border_add_rounded(Vertex_2D_Array* vertex_array, u32* num_indices, V2 top_left, V2 size, V4 color, f32 thickness, f32 roundness, u32 samples_per_side, f32 texture_index);

f32 ease_out_elastic(const f32 x);
f32 ease_out_sine(const f32 x);
f32 ease_out_cubic(const f32 x);

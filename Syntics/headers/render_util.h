#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#include "math/syntics_math.h"
#endif

#define quad_d0(vertices, rect_count, pos, size)                                    \
    quad(vertices, rect_count, pos, size, v4i(1.0f), 0.0f)
#define quad_d1(vertices, rect_count, pos, size, color)                             \
    quad(vertices, rect_count, pos, size, color, 0.0f)
#define quad_d2(vertices, rect_count, pos, size, tex_index)                         \
    quad(vertices, rect_count, pos, size, v4i(1.0f), tex_index)

#define quad_f_d0(vertices, rect_count, pos, size)                                  \
    quad_f(vertices, rect_count, pos, size, v4i(1.0f), 0.0f)
#define quad_f_d1(vertices, rect_count, pos, size, color)                           \
    quad_f(vertices, rect_count, pos, size, color, 0.0f)
#define quad_f_d2(vertices, rect_count, pos, size, tex_index)                       \
    quad_f(vertices, rect_count, pos, size, v4i(1.0f), tex_index)

#define quad_s_d0(vertices, rect_count, pos, size)                                  \
    quad_s(vertices, rect_count, pos, size, v4i(1.0f), 0.0f, 3.0f)
#define quad_s_d1(vertices, rect_count, pos, size, color)                           \
    quad_s(vertices, rect_count, pos, size, color, 0.0f, 3.0f)
#define quad_s_d2(vertices, rect_count, pos, size, color, tex_index)                \
    quad_s(vertices, rect_count, pos, size, color, tex_index, 3.0f)

#define quad_sl_d0(vertices, rect_count, pos, size)                                 \
    quad_sl(vertices, rect_count, pos, size, v4i(1.0f), 0.0f, 2.0f)
#define quad_sl_d1(vertices, rect_count, pos, size, color)                          \
    quad_sl(vertices, rect_count, pos, size, color, 0.0f, 2.0f)
#define quad_sl_d2(vertices, rect_count, pos, size, color, tex_index)               \
    quad_sl(vertices, rect_count, pos, size, color, tex_index, 2.0f)

#define quad_sl_gradiant_d0(vertices, rect_count, pos, size)                        \
    quad_sl_gradiant(vertices, rect_count, pos, size, v4i(1.0f), 0.0f, 2.0f)
#define quad_sl_gradiant_d1(vertices, rect_count, pos, size, color)                 \
    quad_sl_gradiant(vertices, rect_count, pos, size, color, 0.0f, 2.0f)
#define quad_sl_gradiant_d2(vertices, rect_count, pos, size, color, tex_index)      \
    quad_sl_gradiant(vertices, rect_count, pos, size, color, tex_index, 2.0f)

#define quad_s_gradiant_d0(vertices, rect_count, pos, size)                         \
    quad_s_gradiant(vertices, rect_count, pos, size, v4i(1.0f), 0.4f, 0.0f, 2.0f)
#define quad_s_gradiant_d1(vertices, rect_count, pos, size, color)                  \
    quad_s_gradiant(vertices, rect_count, pos, size, color, 0.4f, 0.0f, 2.0f)
#define quad_s_gradiant_d2(vertices, rect_count, pos, size, color, multiplier)      \
    quad_s_gradiant(vertices, rect_count, pos, size, color, multiplier, 0.0f, 2.0f)
#define quad_s_gradiant_d3(vertices, rect_count, pos, size, color, multiplier,      \
                           tex_index)                                               \
    quad_s_gradiant(vertices, rect_count, pos, size, color, multiplier, tex_index,  \
                    2.0f)

#define quad_gradiant_l_r_d0(vertices, rect_count, pos, size)                       \
    quad_gradiant_l_r(vertices, rect_count, pos, size, v4i(1.0f), v4i(1.0f), 0.0f)
#define quad_gradiant_l_r_d1(vertices, rect_count, pos, size, left_color)           \
    quad_gradiant_l_r(vertices, rect_count, pos, size, left_color, v4i(1.0f), 0.0f)
#define quad_gradiant_l_r_d2(vertices, rect_count, pos, size, left_color,           \
                             right_color)                                           \
    quad_gradiant_l_r(vertices, rect_count, pos, size, left_color, right_color, 0.0f)

#define quad_gradiant_t_b_d0(vertices, rect_count, pos, size)                       \
    quad_gradiant_t_b(vertices, rect_count, pos, size, v4i(1.0f), v4i(1.0f), 0.0f)
#define quad_gradiant_t_b_d1(vertices, rect_count, pos, size, top_color)            \
    quad_gradiant_t_b(vertices, rect_count, pos, size, top_color, v4i(1.0f), 0.0f)
#define quad_gradiant_t_b_d2(vertices, rect_count, pos, size, top_color,            \
                             bottom_color)                                          \
    quad_gradiant_t_b(vertices, rect_count, pos, size, top_color, bottom_color, 0.0f)

#define quad_s_gradiant_l_r_d0(vertices, rect_count, pos, size)                     \
    quad_s_gradiant_l_r(vertices, rect_count, pos, size, v4i(1.0f), v4i(1.0f),      \
                        0.0f, 2.0f)
#define quad_s_gradiant_l_r_d1(vertices, rect_count, pos, size, left_color)         \
    quad_s_gradiant_l_r(vertices, rect_count, pos, size, left_color, v4i(1.0f),     \
                        0.0f, 2.0f)
#define quad_s_gradiant_l_r_d2(vertices, rect_count, pos, size, left_color,         \
                               right_color)                                         \
    quad_s_gradiant_l_r(vertices, rect_count, pos, size, left_color, right_color,   \
                        0.0f, 2.0f)
#define quad_s_gradiant_l_r_d3(vertices, rect_count, pos, size, left_color,         \
                               right_color, tex_index)                              \
    quad_s_gradiant_l_r(vertices, rect_count, pos, size, left_color, right_color,   \
                        tex_index, 2.0f)

#define quad_s_gradiant_t_b_d0(vertices, rect_count, pos, size)                     \
    quad_s_gradiant_t_b(vertices, rect_count, pos, size, v4i(1.0f), v4i(1.0f),      \
                        0.0f, 2.0f)
#define quad_s_gradiant_t_b_d1(vertices, rect_count, pos, size, top_color)          \
    quad_s_gradiant_t_b(vertices, rect_count, pos, size, top_color, v4i(1.0f),      \
                        0.0f, 2.0f)
#define quad_s_gradiant_t_b_d2(vertices, rect_count, pos, size, top_color,          \
                               bottom_color)                                        \
    quad_s_gradiant_t_b(vertices, rect_count, pos, size, top_color, bottom_color,   \
                        0.0f, 2.0f)
#define quad_s_gradiant_t_b_d3(vertices, rect_count, pos, size, top_color,          \
                               bottom_color, tex_index)                             \
    quad_s_gradiant_t_b(vertices, rect_count, pos, size, top_color, bottom_color,   \
                        tex_index, 2.0f)

#define border_add_s_d0(data, num_indices, border_color, top_left, size)            \
    border_add_s(data, num_indices, border_color, top_left, size, 1.0f, 0.0f)
#define border_add_s_d1(data, num_indices, border_color, top_left, size, thickness) \
    border_add_s(data, num_indices, border_color, top_left, size, thickness, 0.0f)

#define border_add_d0(data, num_indices, border_color, top_left, size)              \
    border_add(data, num_indices, border_color, top_left, size, 1.0f, 0.0f)
#define border_add_d1(data, num_indices, border_color, top_left, size, thickness)   \
    border_add(data, num_indices, border_color, top_left, size, thickness, 0.0f)

typedef struct Tex_Coords
{
    V2 coords[4];
} Tex_Coords;

AABB_2D quad_co(Vertex_Array* vert_array, u32* rect_count, V3 pos, V2 size,
                V4 color, V4 tex_coords, f32 tex_index);
AABB_2D quad(Vertex_Array* vert_array, u32* rect_count, V3 pos, V2 size,
             V4 color, f32 tex_index);
AABB_2D quad_f(Vertex_Array* vert_array, u32* rect_count, V3 pos, V2 size,
               V4 color, f32 tex_index);
AABB_2D quad_gradiant_l_r(Vertex_Array* vert_array, u32* rect_count, V3 pos,
                          V2 size, V4 left_color, V4 right_color, f32 tex_index);
AABB_2D quad_gradiant_t_b(Vertex_Array* vert_array, u32* rect_count, V3 pos,
                          V2 size, V4 top_color, V4 bottom_color, f32 tex_index);
AABB_2D quad_s_gradiant_l_r(Vertex_Array* vert_array, u32* rect_count, V3 pos,
                            V2 size, V4 left_color, V4 right_color,
                            f32 tex_index, f32 shadow_offset);
AABB_2D quad_s_gradiant_t_b(Vertex_Array* vert_array, u32* rect_count, V3 pos,
                            V2 size, V4 top_color, V4 bottom_color,
                            f32 tex_index, f32 shadow_offset);
AABB_2D quad_s_gradiant(Vertex_Array* vert_array, u32* rect_count, V3 pos,
                        V2 size, V4 color, f32 multiplier, f32 tex_index,
                        f32 shadow_offset);
AABB_2D quad_s(Vertex_Array* vert_array, u32* rect_count, V3 pos, V2 size,
               V4 color, f32 tex_index, f32 shadow_offset);
AABB_2D quad_sl(Vertex_Array* vert_array, u32* rect_count, V3 pos, V2 size,
                V4 color, f32 tex_index, f32 shadow_offset);
AABB_2D quad_sl_gradiant(Vertex_Array* vert_array, u32* rect_count, V3 pos,
                         V2 size, V4 color, f32 tex_index, f32 shadow_offset);
AABB_2D border_add_s(Vertex_Array* vert_array, u32* num_indices,
                     V4 border_color, V3 top_left, V2 size, f32 thickness,
                     f32 tex_index);
AABB_2D border_add(Vertex_Array* vert_array, u32* num_indices, V4 border_color,
                   V3 top_left, V2 size, f32 thickness, f32 tex_index);
void quad_middle(Vertex_Array* vert_array, V3 pos, V2 size, V4 color,
                 f32 tex_index);
void polygon2D_draw_quads(Vertex_Array* vert_array, Polygon2D poly, f32 z,
                          V4 color, f32 line_width, f32 tex_index);
void polygon2D_draw_lines(Vertex_Array* vert_array, U32_Array* idx_array,
                          Polygon2D poly, f32 z, V4 color, f32 tex_index);
void square_rounded_corners(Vertex_Array* vert_array, U32_Array* idx_array,
                            u32 vertex_offset, V3 pos, V2 size, V4 color,
                            f32 seperation, u32 corner_vertices_count,
                            f32 tex_index, u32 index_index);
void square_rounded_corners_3d(Vertex_Array* vert_array, U32_Array* idx_array,
                               const u32 vertex_offset, V3 pos, V3 size,
                               V4 color, f32 seperation,
                               u32 corner_vertices_count, f32 tex_index);
void indices_generate(U32_Array* array, u32 offset, u32 indices_count);
u32 cube(Vertex_Array* vert_array, u32 offset, V3 pos, V3 size, V4 color,
         f32 tex_index);
void cube1(Vertex_Array* vert_array, V3 pos, V3 size, V4 color, f32 tex_index);
void cube_not_center1(Vertex_Array* vert_array, V3 pos, V3 size, V4 color,
                      f32 tex_index);
u32 cube_not_center(Vertex_Array* vert_array, u32 offset, V3 pos, V3 size,
                    V4 color, f32 tex_index);
void cube_indices_offset(U32_Array* indices, u32 offset, u32 how_many);
void cube_indices(U32_Array* indices, u32 offset, u32 how_many);
u32 gridd_using_line_list(Vertex_Array* vert_array, u32 vertex_offset,
                          U32_Array* indices, u32 index_offset, V3 middle_pos,
                          V2 spacing, u32 lines_width_count,
                          u32 lines_height_count, V4 color, f32 tex_index);

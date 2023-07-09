#pragma once

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

#define add_border_s_d0(data, num_indices, border_color, top_left, size)            \
    add_border_s(data, num_indices, border_color, top_left, size, 1.0f, 0.0f)
#define add_border_s_d1(data, num_indices, border_color, top_left, size, thickness) \
    add_border_s(data, num_indices, border_color, top_left, size, thickness, 0.0f)

#define add_border_d0(data, num_indices, border_color, top_left, size)              \
    add_border(data, num_indices, border_color, top_left, size, 1.0f, 0.0f)
#define add_border_d1(data, num_indices, border_color, top_left, size, thickness)   \
    add_border(data, num_indices, border_color, top_left, size, thickness, 0.0f)

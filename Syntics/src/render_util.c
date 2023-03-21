#include "render_util.h"
#include "region_alloc.h"
#include "math/transforms.h"
#include <math.h>

static V3 QUAD_VERTEX[4] = { { -0.5f, -0.5f, 0.0f },
                             { -0.5f, 0.5f, 0.0f },
                             { 0.5f, 0.5f, 0.0f },
                             { 0.5f, -0.5f, 0.0f } };

Rect2D quad(Vertex** vertices, u32* rect_count, V3 pos, V2 size, V4 color, f32 tex_index)
{
    Vertex verts[4] = {
        { { pos.x, pos.y, pos.z }, color, { 0.0f, 0.0f }, tex_index },
        { { pos.x, pos.y + size.y, pos.z }, color, { 0.0f, 1.0f }, tex_index },
        { { pos.x + size.x, pos.y + size.y, pos.z }, color, { 1.0f, 1.0f }, tex_index },
        { { pos.x + size.x, pos.y, pos.z }, color, { 1.0f, 0.0f }, tex_index }
    };

    for (u32 i = 0; i < 4; i++)
    {
        synt_push((*vertices), verts[i]);
    }

    if (rect_count)
    {
        *rect_count += 1;
    }
    Rect2D out;
    out.pos.x = pos.x;
    out.pos.y = pos.y;
    out.size = size;
    out.color = color;
    return out;
}

Rect2D quad_rect(Vertex** vertices, u32* rect_count, const Rect3D* rect)
{
    return quad(vertices, rect_count, rect->pos, rect->size, rect->color, (f32)rect->id);
}

Rect2D quad_gradiant_l_r(Vertex** vertices, u32* rect_count, V3 pos, V2 size,
                         V4 left_color, V4 right_color, f32 tex_index)
{
    Vertex verts[4] = {
        { { pos.x, pos.y, pos.z }, left_color, { 0.0f, 0.0f }, tex_index },
        { { pos.x, pos.y + size.y, pos.z }, left_color, { 0.0f, 1.0f }, tex_index },
        { { pos.x + size.x, pos.y + size.y, pos.z },
          right_color,
          { 1.0f, 1.0f },
          tex_index },
        { { pos.x + size.x, pos.y, pos.z }, right_color, { 1.0f, 0.0f }, tex_index }
    };

    for (u32 i = 0; i < 4; i++)
    {
        synt_push((*vertices), verts[i]);
    }
    if (rect_count)
    {
        *rect_count += 1;
    }
    Rect2D out;
    out.pos.x = pos.x;
    out.pos.y = pos.y;
    out.size = size;
    out.color = left_color;
    return out;
}
Rect2D quad_gradiant_t_b(Vertex** vertices, u32* rect_count, V3 pos, V2 size,
                         V4 top_color, V4 bottom_color, f32 tex_index)
{
    Vertex verts[4] = {
        { { pos.x, pos.y, pos.z }, top_color, { 0.0f, 0.0f }, tex_index },
        { { pos.x, pos.y + size.y, pos.z }, bottom_color, { 0.0f, 1.0f }, tex_index },
        { { pos.x + size.x, pos.y + size.y, pos.z },
          bottom_color,
          { 1.0f, 1.0f },
          tex_index },
        { { pos.x + size.x, pos.y, pos.z }, top_color, { 1.0f, 0.0f }, tex_index }
    };

    for (u32 i = 0; i < 4; i++)
    {
        synt_push((*vertices), verts[i]);
    }
    if (rect_count)
    {
        *rect_count += 1;
    }
    Rect2D out;
    out.pos.x = pos.x;
    out.pos.y = pos.y;
    out.size = size;
    out.color = top_color;
    return out;
}

Rect2D quad_s_gradiant_l_r(Vertex** vertices, u32* rect_count, V3 pos, V2 size,
                           V4 left_color, V4 right_color, f32 tex_index,
                           f32 shadow_offset)
{
    const V4 S_COLOR = v4f(0.0f, 0.0f, 0.0f, left_color.w - 0.1f);

    f32 s_pos_z = pos.z - 0.001f;
    V3 s_pos = v3f(pos.x + shadow_offset, pos.y + shadow_offset, s_pos_z);

    quad(vertices, rect_count, s_pos, size, S_COLOR, tex_index);

    return quad_gradiant_l_r(vertices, rect_count, pos, size, left_color, right_color,
                             tex_index);
}

Rect2D quad_s_gradiant_t_b(Vertex** vertices, u32* rect_count, V3 pos, V2 size,
                           V4 top_color, V4 bottom_color, f32 tex_index,
                           f32 shadow_offset)
{
    const V4 S_COLOR = v4f(0.0f, 0.0f, 0.0f, top_color.w - 0.1f);

    f32 s_pos_z = pos.z - 0.001f;
    V3 s_pos = v3f(pos.x + shadow_offset, pos.y + shadow_offset, s_pos_z);

    quad(vertices, rect_count, s_pos, size, S_COLOR, tex_index);
    return quad_gradiant_t_b(vertices, rect_count, pos, size, top_color, bottom_color,
                             tex_index);
}

Rect2D quad_s_gradiant(Vertex** vertices, u32* rect_count, V3 pos, V2 size, V4 color,
                       f32 multiplier, f32 tex_index, f32 shadow_offset)
{
    V4 bottom_color = v4_s_multi(color, multiplier);
    bottom_color.w = color.w;
    return quad_s_gradiant_t_b(vertices, rect_count, pos, size, color, bottom_color,
                               tex_index, shadow_offset);
}

Rect2D quad_s(Vertex** vertices, u32* rect_count, V3 pos, V2 size, V4 color,
              f32 tex_index, f32 shadow_offset)
{
    const V4 S_COLOR = v4f(0.0f, 0.0f, 0.0f, color.w - 0.1f);
    V4 f_color = v4f(color.x, color.y, color.z, color.w + 0.05f);

    f32 s_pos_z = pos.z - 0.001f;
    V3 s_pos = v3f(pos.x + shadow_offset, pos.y + shadow_offset, s_pos_z);

    quad(vertices, rect_count, s_pos, size, S_COLOR, tex_index);

    return quad(vertices, rect_count, pos, size, f_color, tex_index);
}

Rect2D quad_sl(Vertex** vertices, u32* rect_count, V3 pos, V2 size, V4 color,
               f32 tex_index, f32 shadow_offset)
{
    const V4 S_COLOR = v4f(0.0f, 0.0f, 0.0f, color.w - 0.1f);
    V4 f_color = v4f(color.x, color.y, color.z, color.w + 0.05f);

    V4 l_color = v4_s_multi(color, 2.0f);
    l_color.w = color.w;

    f32 s_pos_z = pos.z - 0.001f;
    f32 shadow_offset_2x = shadow_offset * 2.0f;
    pos.x += shadow_offset;
    V3 s_pos_h = v3f(pos.x - shadow_offset, pos.y + size.y, s_pos_z);
    V3 s_pos_v = v3f(pos.x + size.x, pos.y, s_pos_z);
    V3 l_pos_h = v3f(pos.x - shadow_offset, pos.y - shadow_offset, s_pos_z);
    V3 l_pos_v = v3f(pos.x - shadow_offset, pos.y, s_pos_z);
    V2 sl_size_h = v2f(size.x + shadow_offset_2x, shadow_offset);
    V2 sl_size_v = v2f(shadow_offset, size.y);

    quad(vertices, rect_count, s_pos_h, sl_size_h, S_COLOR, tex_index);
    quad(vertices, rect_count, s_pos_v, sl_size_v, S_COLOR, tex_index);
    sl_size_h.x -= shadow_offset;
    quad(vertices, rect_count, l_pos_h, sl_size_h, l_color, tex_index);
    quad(vertices, rect_count, l_pos_v, sl_size_v, l_color, tex_index);

    return quad(vertices, rect_count, pos, size, f_color, tex_index);
}

Rect2D quad_sl_gradiant(Vertex** vertices, u32* rect_count, V3 pos, V2 size, V4 color,
                        f32 tex_index, f32 shadow_offset)
{
    const V4 S_COLOR = v4f(0.0f, 0.0f, 0.0f, color.w - 0.1f);

    V4 l_color = v4_s_multi(color, 2.0f);
    l_color.w = color.w;

    f32 s_pos_z = pos.z - 0.001f;
    f32 shadow_offset_2x = shadow_offset * 2.0f;
    pos.x += shadow_offset;
    V3 s_pos_h = v3f(pos.x - shadow_offset, pos.y + size.y, s_pos_z);
    V3 s_pos_v = v3f(pos.x + size.x, pos.y, s_pos_z);
    V3 l_pos_h = v3f(pos.x - shadow_offset, pos.y - shadow_offset, s_pos_z);
    V3 l_pos_v = v3f(pos.x - shadow_offset, pos.y, s_pos_z);
    V2 sl_size_h = v2f(size.x + shadow_offset_2x, shadow_offset);
    V2 sl_size_v = v2f(shadow_offset, size.y);

    quad(vertices, rect_count, s_pos_h, sl_size_h, S_COLOR, tex_index);
    quad(vertices, rect_count, s_pos_v, sl_size_v, S_COLOR, tex_index);
    sl_size_h.x -= shadow_offset;
    quad(vertices, rect_count, l_pos_h, sl_size_h, l_color, tex_index);
    quad(vertices, rect_count, l_pos_v, sl_size_v, l_color, tex_index);

    V4 gr_color = v4_s_multi(color, 0.6f);
    gr_color.w = color.w;

    Vertex verts[4] = {
        { { pos.x, pos.y, pos.z }, color, { 0.0f, 0.0f }, tex_index },
        { { pos.x, pos.y + size.y, pos.z }, gr_color, { 0.0f, 1.0f }, tex_index },
        { { pos.x + size.x, pos.y + size.y, pos.z },
          gr_color,
          { 1.0f, 1.0f },
          tex_index },
        { { pos.x + size.x, pos.y, pos.z }, color, { 1.0f, 0.0f }, tex_index }
    };

    for (u32 i = 0; i < 4; i++)
    {
        synt_push((*vertices), verts[i]);
    }
    if (rect_count)
    {
        *rect_count += 1;
    }
    Rect2D out;
    out.pos.x = pos.x;
    out.pos.y = pos.y;
    out.size = size;
    out.color = color;
    return out;
}

Rect2D quad_r(Vertex** vertices, u32* rect_count, V3 pos, V2 size, V4 color,
              f32 tex_index, f32 rotation)
{
    // TODO: think translate is broken...
    Mat4f transform = m4_multi(
        m4_multi(m4_translate(m4i(1.0f), pos), m4_rotate(m4i(1.0f), rotation, Z)),
        m4_scale(m4i(1.0f), v3f(size.x, size.y, 1.0f)));

    V3 positions[4];
    positions[0] = m4_v3_multi(transform, QUAD_VERTEX[0]);
    positions[1] = m4_v3_multi(transform, QUAD_VERTEX[1]);
    positions[2] = m4_v3_multi(transform, QUAD_VERTEX[2]);
    positions[3] = m4_v3_multi(transform, QUAD_VERTEX[3]);

    Vertex verts[4] = { { positions[0], color, { 0.0f, 0.0f }, tex_index },
                        { positions[1], color, { 0.0f, 1.0f }, tex_index },
                        { positions[2], color, { 1.0f, 1.0f }, tex_index },
                        { positions[3], color, { 1.0f, 0.0f }, tex_index } };

#if 0
    glm::vec3 ved(pos.x, pos.y, pos.z);
    glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), ved) *
        glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));

    glm::vec4 positions[4] = { { transform * QUAD_VERTEX[0] },
                               { transform * QUAD_VERTEX[1] },
                               { transform * QUAD_VERTEX[2] },
                               { transform * QUAD_VERTEX[3] } };

    Vertex verts[4] = {
        { V4(positions[0].x, positions[0].y, positions[0].z, positions[0].w),
          color,
          { 0.0f, 0.0f },
          tex_index },
        { V4(positions[1].x, positions[1].y, positions[1].z, positions[1].w),
          color,
          { 0.0f, 1.0f },
          tex_index },
        { V4(positions[2].x, positions[2].y, positions[2].z, positions[2].w),
          color,
          { 1.0f, 1.0f },
          tex_index },
        { V4(positions[3].x, positions[3].y, positions[3].z, positions[3].w),
          color,
          { 1.0f, 0.0f },
          tex_index }
    };

#endif

    for (u32 i = 0; i < 4; i++)
    {
        synt_push((*vertices), verts[i]);
    }
    if (rect_count)
    {
        *rect_count += 1;
    }
    Rect2D out;
    out.pos.x = positions[0].x;
    out.pos.y = positions[0].y;
    out.size = size;
    out.color = color;
    return out;
}

Rect2D add_border_s(Vertex** data, u32* num_indices, V4 border_color, V3 top_left,
                    V2 size, f32 thickness, f32 tex_index)
{
    V2 h_size = v2f(size.x, thickness);
    V2 v_size = v2f(thickness, size.y);

    quad_s(data, num_indices, top_left, h_size, border_color, tex_index, 1.0f);

    quad_s(data, num_indices,
           v3f(top_left.x, top_left.y + v_size.y - thickness, top_left.z), h_size,
           border_color, tex_index, 1.0f);

    quad_s(data, num_indices, top_left, v_size, border_color, tex_index, 1.0f);

    quad_s(data, num_indices,
           v3f(top_left.x + h_size.x - thickness, top_left.y, top_left.z), v_size,
           border_color, tex_index, 1.0f);

    Rect2D out = { 0 };
    out.pos.x = top_left.x;
    out.pos.y = top_left.y;
    out.size = size;
    return out;
}

Rect2D add_border(Vertex** data, u32* num_indices, V4 border_color, V3 top_left, V2 size,
                  f32 thickness, f32 tex_index)
{
    V2 h_size = v2f(size.x, thickness);
    V2 v_size = v2f(thickness, size.y);

    quad(data, num_indices, top_left, h_size, border_color, tex_index);

    quad(data, num_indices,
         v3f(top_left.x, top_left.y + v_size.y - thickness, top_left.z), h_size,
         border_color, tex_index);

    quad(data, num_indices, top_left, v_size, border_color, tex_index);

    quad(data, num_indices,
         v3f(top_left.x + h_size.x - thickness, top_left.y, top_left.z), v_size,
         border_color, tex_index);

    Rect2D out = { 0 };
    out.pos.x = top_left.x;
    out.pos.y = top_left.y;
    out.size = size;
    return out;
}

void polygon2D_draw_quads(Vertex** data, Polygon2D poly, f32 z, V4 color, f32 line_width,
                          f32 tex_index)
{
    f32 scalars[] = { 2.0f, -2.0f };
    u32 s_i = 0;
    for_range(i, poly.n_sides)
    {
        Vertex vert = { 0 };
        V3 normal = v3_v2(poly.normals[i]);

        f32* first = scalars + s_i;
        f32* second = scalars + (s_i + 1) % 2;

        vert.color = color;
        vert.pos = v3f(poly.points[i].x, poly.points[i].y, z);
        vert.pos = v3_add(vert.pos, v3_s_multi(normal, *first));
        vert.tex_index = tex_index;
        synt_push(*data, vert);

        vert.pos = v3f(poly.points[i].x, poly.points[i].y, z);
        vert.pos = v3_add(vert.pos, v3_s_multi(normal, *second));
        synt_push(*data, vert);
        s_i++;
        s_i %= 2;
    }
}

void polygon2D_draw_lines(Vertex** data, u32** idx_data, Polygon2D poly, f32 z, V4 color,
                          f32 tex_index)
{
    Vertex vert = { 0 };
    u32 size = size_arr(*data);
    for_range(i, poly.n_sides)
    {
        vert.color = color;
        vert.pos = v3f(poly.points[i].x, poly.points[i].y, z);
        vert.tex_index = tex_index;
        synt_push(*data, vert);
    }
    for_range(i, poly.n_sides)
    {
        u32 j = (i + 1) % poly.n_sides;
        synt_push(*idx_data, size + i);
        synt_push(*idx_data, size + j);
    }
}

internal void insert_indices(u32** idx_data, u32 p_i, u32 added_val0, u32 added_val1)
{
    synt_push(*idx_data, p_i);
    synt_push(*idx_data, p_i + added_val0);
    synt_push(*idx_data, p_i + added_val1);
}

void square_rounded_corners(Vertex** data, u32** idx_data, V3 pos, V2 size, V4 color,
                            f32 seperation, u32 num_corner_vertices, f32 tex_index)
{
    stack_begin_scope();
    V2 pos_plus_size = v2_add(v2_v3(pos), size);

    V2 pivot_points[4];
    pivot_points[0] = v2f(pos.x + seperation, pos.y + seperation);
    pivot_points[1] = v2f(pos.x + seperation, pos_plus_size.y - seperation);
    pivot_points[2] = v2f(pos_plus_size.x - seperation, pos_plus_size.y - seperation);
    pivot_points[3] = v2f(pos_plus_size.x - seperation, pos.y + seperation);

    V2* vert_pos = stack_malloc(num_corner_vertices * 4, V2);

    f32 _90_d = PI / 2.0f;
    f32 _360_d = 2.0f * PI;
    f32 d_rad = _90_d / (num_corner_vertices - 1);

    u32 count = 0;
    for_range(corner, 4)
    {
        f32 extra_rad = _360_d - (_90_d * ((corner + 1) % 4));
        for_range(i, num_corner_vertices)
        {
            f32 rad = extra_rad - (d_rad * i); // Modulus to wrap around

            vert_pos[count++] = v2_add(pivot_points[corner], v2f(seperation * cosf(rad),
                                                                 seperation * sinf(rad)));
        }
    }
    Vertex vert = { 0 };
    vert.color = color;
    vert.tex_index = tex_index;

    u32 num_corner_vertices_2x = num_corner_vertices * 2;
    u32 pivot_indicies[4] = { 0, num_corner_vertices_2x + 1, num_corner_vertices_2x + 2,
                              (num_corner_vertices_2x * 2) + 3 };

    u32* p_i = &pivot_indicies[0];
    V2* p_pos = &pivot_points[0];

    count = 0;
    for_range(half, 2)
    {
        vert.pos = v3_v2f(*p_pos, pos.z);

        synt_push(*data, vert);
        for_range(quarters, 2)
        {
            for_range(j, num_corner_vertices)
            {
                vert.pos = v3_v2f(vert_pos[count++], pos.z);
                synt_push(*data, vert);
            }
        }
        p_pos++;
        vert.pos = v3_v2f(*p_pos, pos.z);
        synt_push(*data, vert);
        p_pos++;

        for_range(i, 4)
        {
            insert_indices(idx_data, *p_i, 1 + i, 2 + i);
        }
        p_i++;

        i32 i = 0;
        for (; i < 3; i++)
        {
            insert_indices(idx_data, *p_i, -(1 + i), -(2 + i));
        }
        synt_push(*idx_data, *p_i);
        synt_push(*idx_data, (*p_i) - (1 + i));
        synt_push(*idx_data, *(p_i - 1));
        p_i++;
    }
    u32 inner_square_indices[4] = { 1, num_corner_vertices_2x, num_corner_vertices_2x + 3,
                                    (num_corner_vertices_2x * 2) + 2 };
    for_range(i, 4)
    {
        synt_push(*idx_data, inner_square_indices[i]);
    }
    stack_end_scope();
}

void generate_indices(u32** data, uint32_t offset, u32 num_indices)
{
    for (u32 i = offset; i < num_indices; i++)
    {
        synt_push((*data), 0 + (4 * i));
        synt_push((*data), 1 + (4 * i));
        synt_push((*data), 2 + (4 * i));
        synt_push((*data), 2 + (4 * i));
        synt_push((*data), 3 + (4 * i));
        synt_push((*data), 0 + (4 * i));
    }
}

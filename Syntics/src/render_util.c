#ifndef SY_INCLUDES // only for clangd
#include "syntics.h"
#endif

static V3 QUAD_VERTEX[4] = { { -1.0f, -1.0f, 0.0f },
                             { -1.0f, 1.0f, 0.0f },
                             { 1.0f, 1.0f, 0.0f },
                             { 1.0f, -1.0f, 0.0f } };

AABB_2D _set_up_verticies(Vertex_Array* vert_array, u32* rect_count, V3 pos, V2 size,
                          V4 color, f32 tex_index, Tex_Coords tex_coords)
{
    Vertex vertex = {
        .pos = pos,
        .tex_coords = tex_coords.coords[0],
        .color = color,
        .tex_index = tex_index,
    };
    vertex_array_push(vert_array, vertex);
    vertex.pos.y += size.y;
    vertex.tex_coords = tex_coords.coords[1];
    vertex_array_push(vert_array, vertex);
    vertex.pos.x += size.x;
    vertex.tex_coords = tex_coords.coords[2];
    vertex_array_push(vert_array, vertex);
    vertex.pos.y -= size.y;
    vertex.tex_coords = tex_coords.coords[3];
    vertex_array_push(vert_array, vertex);

    if (rect_count)
    {
        *rect_count += 1;
    }
    AABB_2D out;
    out.min = v2_v3(pos);
    out.size = size;
    return out;
}

AABB_2D quad(Vertex_Array* vert_array, u32* rect_count, V3 pos, V2 size, V4 color,
             f32 tex_index)
{
    Tex_Coords tex_coords = { v2d(), v2f(0.0f, 1.0f), v2f(1.0f, 1.0f),
                              v2f(1.0f, 0.0f) };
    return _set_up_verticies(vert_array, rect_count, pos, size, color, tex_index,
                             tex_coords);
}

AABB_2D quad_f(Vertex_Array* vert_array, u32* rect_count, V3 pos, V2 size, V4 color,
               f32 tex_index)
{
    Tex_Coords tex_coords = { v2f(0.0f, 1.0f), v2d(), v2f(1.0f, 0.0f),
                              v2f(1.0f, 1.0f) };
    return _set_up_verticies(vert_array, rect_count, pos, size, color, tex_index,
                             tex_coords);
}

AABB_2D quad_gradiant_l_r(Vertex_Array* vert_array, u32* rect_count, V3 pos, V2 size,
                          V4 left_color, V4 right_color, f32 tex_index)
{
    Vertex verts[4] = { { v3f(pos.x, pos.y, pos.z), v3d(), v2f(0.0f, 0.0f),
                          left_color, tex_index },
                        { v3f(pos.x, pos.y + size.y, pos.z), v3d(), v2f(0.0f, 1.0f),
                          left_color, tex_index },
                        { v3f(pos.x + size.x, pos.y + size.y, pos.z), v3d(),
                          v2f(1.0f, 1.0f), right_color, tex_index },
                        { v3f(pos.x + size.x, pos.y, pos.z), v3d(), v2f(1.0f, 0.0f),
                          right_color, tex_index } };

    for (u32 i = 0; i < 4; i++)
    {
        vertex_array_push(vert_array, verts[i]);
    }
    if (rect_count)
    {
        *rect_count += 1;
    }
    AABB_2D out;
    out.min = v2_v3(pos);
    out.size = size;
    return out;
}
AABB_2D quad_gradiant_t_b(Vertex_Array* vert_array, u32* rect_count, V3 pos, V2 size,
                          V4 top_color, V4 bottom_color, f32 tex_index)
{
    Vertex verts[4] = { { v3f(pos.x, pos.y, pos.z), v3d(), v2f(0.0f, 0.0f),
                          top_color, tex_index },
                        { v3f(pos.x, pos.y + size.y, pos.z), v3d(), v2f(0.0f, 1.0f),
                          bottom_color, tex_index },
                        { v3f(pos.x + size.x, pos.y + size.y, pos.z), v3d(),
                          v2f(1.0f, 1.0f), bottom_color, tex_index },
                        { v3f(pos.x + size.x, pos.y, pos.z),
                          v3d(),
                          { 1.0f, 0.0f },
                          top_color,
                          tex_index } };

    for (u32 i = 0; i < 4; i++)
    {
        vertex_array_push(vert_array, verts[i]);
    }
    if (rect_count)
    {
        *rect_count += 1;
    }
    AABB_2D out;
    out.min = v2_v3(pos);
    out.size = size;
    return out;
}

AABB_2D quad_s_gradiant_l_r(Vertex_Array* vert_array, u32* rect_count, V3 pos,
                            V2 size, V4 left_color, V4 right_color, f32 tex_index,
                            f32 shadow_offset)
{
    const V4 s_color = v4f(0.0f, 0.0f, 0.0f, left_color.w - 0.1f);

    f32 s_pos_z = pos.z - 0.001f;
    V3 s_pos = v3f(pos.x + shadow_offset, pos.y + shadow_offset, s_pos_z);

    quad(vert_array, rect_count, s_pos, size, s_color, tex_index);

    return quad_gradiant_l_r(vert_array, rect_count, pos, size, left_color,
                             right_color, tex_index);
}

AABB_2D quad_s_gradiant_t_b(Vertex_Array* vert_array, u32* rect_count, V3 pos,
                            V2 size, V4 top_color, V4 bottom_color, f32 tex_index,
                            f32 shadow_offset)
{
    const V4 s_color = v4f(0.0f, 0.0f, 0.0f, top_color.w - 0.1f);

    V3 s_pos = v3f(pos.x + shadow_offset, pos.y + shadow_offset, pos.z);

    quad(vert_array, rect_count, s_pos, size, s_color, tex_index);
    return quad_gradiant_t_b(vert_array, rect_count, pos, size, top_color,
                             bottom_color, tex_index);
}

AABB_2D quad_s_gradiant(Vertex_Array* vert_array, u32* rect_count, V3 pos, V2 size,
                        V4 color, f32 multiplier, f32 tex_index, f32 shadow_offset)
{
    V4 bottom_color = v4_s_multi(color, multiplier);
    bottom_color.w = color.w;
    return quad_s_gradiant_t_b(vert_array, rect_count, pos, size, color,
                               bottom_color, tex_index, shadow_offset);
}

AABB_2D quad_s(Vertex_Array* vert_array, u32* rect_count, V3 pos, V2 size, V4 color,
               f32 tex_index, f32 shadow_offset)
{
    const V4 s_color = v4f(0.0f, 0.0f, 0.0f, color.w - 0.1f);
    V4 f_color = v4f(color.x, color.y, color.z, color.w);

    V3 s_pos = v3f(pos.x + shadow_offset, pos.y + shadow_offset, pos.z);

    quad(vert_array, rect_count, s_pos, size, s_color, tex_index);

    return quad(vert_array, rect_count, pos, size, f_color, tex_index);
}

AABB_2D quad_sl(Vertex_Array* vert_array, u32* rect_count, V3 pos, V2 size, V4 color,
                f32 tex_index, f32 shadow_offset)
{
    const V4 s_color = v4f(0.0f, 0.0f, 0.0f, color.w - 0.1f);
    V4 f_color = v4f(color.x, color.y, color.z, color.w);

    V4 l_color = v4_s_multi(color, 2.0f);
    l_color.w = color.w;

    f32 shadow_offset_2x = shadow_offset * 2.0f;
    pos.x += shadow_offset;
    V3 s_pos_h = v3f(pos.x - shadow_offset, pos.y + size.y, pos.z);
    V3 s_pos_v = v3f(pos.x + size.x, pos.y, pos.z);
    V3 l_pos_h = v3f(pos.x - shadow_offset, pos.y - shadow_offset, pos.z);
    V3 l_pos_v = v3f(pos.x - shadow_offset, pos.y, pos.z);
    V2 sl_size_h = v2f(size.x + shadow_offset_2x, shadow_offset);
    V2 sl_size_v = v2f(shadow_offset, size.y);

    quad(vert_array, rect_count, s_pos_h, sl_size_h, s_color, tex_index);
    quad(vert_array, rect_count, s_pos_v, sl_size_v, s_color, tex_index);
    sl_size_h.x -= shadow_offset;
    quad(vert_array, rect_count, l_pos_h, sl_size_h, l_color, tex_index);
    quad(vert_array, rect_count, l_pos_v, sl_size_v, l_color, tex_index);

    return quad(vert_array, rect_count, pos, size, f_color, tex_index);
}

AABB_2D quad_sl_gradiant(Vertex_Array* vert_array, u32* rect_count, V3 pos, V2 size,
                         V4 color, f32 tex_index, f32 shadow_offset)
{
    const V4 s_color = v4f(0.0f, 0.0f, 0.0f, color.w - 0.1f);

    V4 l_color = v4_s_multi(color, 2.0f);
    l_color.w = color.w;

    f32 shadow_offset_2x = shadow_offset * 2.0f;
    pos.x += shadow_offset;
    V3 s_pos_h = v3f(pos.x - shadow_offset, pos.y + size.y, pos.z);
    V3 s_pos_v = v3f(pos.x + size.x, pos.y, pos.z);
    V3 l_pos_h = v3f(pos.x - shadow_offset, pos.y - shadow_offset, pos.z);
    V3 l_pos_v = v3f(pos.x - shadow_offset, pos.y, pos.z);
    V2 sl_size_h = v2f(size.x + shadow_offset_2x, shadow_offset);
    V2 sl_size_v = v2f(shadow_offset, size.y);

    quad(vert_array, rect_count, s_pos_h, sl_size_h, s_color, tex_index);
    quad(vert_array, rect_count, s_pos_v, sl_size_v, s_color, tex_index);
    sl_size_h.x -= shadow_offset;
    quad(vert_array, rect_count, l_pos_h, sl_size_h, l_color, tex_index);
    quad(vert_array, rect_count, l_pos_v, sl_size_v, l_color, tex_index);

    V4 gr_color = v4_s_multi(color, 0.6f);
    gr_color.w = color.w;

    Vertex verts[4] = {
        { { pos.x, pos.y, pos.z }, v3d(), { 0.0f, 0.0f }, color, tex_index },
        { { pos.x, pos.y + size.y, pos.z },
          v3d(),
          { 0.0f, 1.0f },
          gr_color,
          tex_index },
        { { pos.x + size.x, pos.y + size.y, pos.z },
          v3d(),
          { 1.0f, 1.0f },
          gr_color,
          tex_index },
        { { pos.x + size.x, pos.y, pos.z }, v3d(), { 1.0f, 0.0f }, color, tex_index }
    };

    for (u32 i = 0; i < 4; i++)
    {
        vertex_array_push(vert_array, verts[i]);
    }
    if (rect_count)
    {
        *rect_count += 1;
    }
    AABB_2D out;
    out.min = v2_v3(pos);
    out.size = size;
    return out;
}

#if 0  
    Converting to Normalized device coordinates:
    ndc_x = (2.0 * pixel_x) / window_width - 1.0
    ndc_y = 1.0 - (2.0 * pixel_y) / window_height 
    {
        Do the transforms ...
    }
    Convert back:
    pixel_x = ((ndc_x + 1.0) * 0.5f) * window_width
    pixel_y = (1.0 - ndc_y) * window_height * 0.5f

    f32 normalized_x = ((2.0f * p_e->pos.x) / dimensions.x) - 1.0f;
    f32 normalized_y = 1.0f - ((2.0f * p_e->pos.y) / dimensions.y);

#endif

AABB_2D quad_r(Vertex_Array* vert_array, u32* rect_count, V3 pos, V2 size, V4 color,
               f32 tex_index, f32 rotation, V2 dimensions)
{
    V3 positions[4];
    positions[0] = v3f(-1.0f, -1.0f, 0.0f);
    positions[1] = v3f(-1.0f, 1.0f, 0.0f);
    positions[2] = v3f(1.0f, 1.0f, 0.0f);
    positions[3] = v3f(1.0f, -1.0f, 0.0f);

    M4 scale = m4_scale(v3f(size.x, size.y, 1.0f));
    M4 rotate = m4_rotate(rotation, Z);
    M4 translate = m4_translate(pos);
    for (u32 i = 0; i < 4; i++)
    {
        positions[i] = m4_v3_multi(scale, positions[i]);
        positions[i] = m4_v3_multi(rotate, positions[i]);
        positions[i] = m4_v3_multi(translate, positions[i]);
    }
    Vertex verts[4] = { { positions[0], v3d(), { 0.0f, 1.0f }, color, tex_index },
                        { positions[1], v3d(), { 0.0f, 0.0f }, color, tex_index },
                        { positions[2], v3d(), { 1.0f, 0.0f }, color, tex_index },
                        { positions[3], v3d(), { 1.0f, 1.0f }, color, tex_index } };

    for (u32 i = 0; i < 4; i++)
    {
        vertex_array_push(vert_array, verts[i]);
    }
    if (rect_count)
    {
        *rect_count += 1;
    }
    AABB_2D out;
    out.min = v2_v3(positions[0]);
    out.size = size;
    return out;
}

AABB_2D border_add_s(Vertex_Array* vert_array, u32* num_indices, V4 border_color,
                     V3 top_left, V2 size, f32 thickness, f32 tex_index)
{
    V2 h_size = v2f(size.x, thickness);
    V2 v_size = v2f(thickness, size.y);

    quad_s(vert_array, num_indices, top_left, h_size, border_color, tex_index, 1.0f);

    quad_s(vert_array, num_indices,
           v3f(top_left.x, top_left.y + v_size.y - thickness, top_left.z), h_size,
           border_color, tex_index, 1.0f);

    quad_s(vert_array, num_indices, top_left, v_size, border_color, tex_index, 1.0f);

    quad_s(vert_array, num_indices,
           v3f(top_left.x + h_size.x - thickness, top_left.y, top_left.z), v_size,
           border_color, tex_index, 1.0f);

    AABB_2D out = { 0 };
    out.min = v2_v3(top_left);
    out.size = size;
    return out;
}

AABB_2D border_add(Vertex_Array* vert_array, u32* num_indices, V4 border_color,
                   V3 top_left, V2 size, f32 thickness, f32 tex_index)
{
    V2 h_size = v2f(size.x, thickness);
    V2 v_size = v2f(thickness, size.y);

    quad(vert_array, num_indices, top_left, h_size, border_color, tex_index);

    quad(vert_array, num_indices,
         v3f(top_left.x, top_left.y + v_size.y - thickness, top_left.z), h_size,
         border_color, tex_index);

    quad(vert_array, num_indices, top_left, v_size, border_color, tex_index);

    quad(vert_array, num_indices,
         v3f(top_left.x + h_size.x - thickness, top_left.y, top_left.z), v_size,
         border_color, tex_index);

    AABB_2D out = { 0 };
    out.min = v2_v3(top_left);
    out.size = size;
    return out;
}

void quad_middle(Vertex_Array* vert_array, V3 pos, V2 size, V4 color, f32 tex_index)
{
    V3 first_pos = v3_sub(pos, v3_v2(v2_s_multi(size, 0.5f)));
    quad(vert_array, NULL, first_pos, size, color, tex_index);
}

void polygon2D_draw_quads(Vertex_Array* vert_array, Polygon2D poly, f32 z, V4 color,
                          f32 line_width, f32 tex_index)
{
    f32 scalars[] = { 2.0f, -2.0f };
    u32 s_i = 0;
    for (u32 i = 0; i < poly.n_sides; i++)
    {
        Vertex vert = { 0 };
        V3 normal = v3_v2(poly.normals[i]);

        f32* first = scalars + s_i;
        f32* second = scalars + (s_i + 1) % 2;

        vert.color = color;
        vert.pos = v3f(poly.points[i].x, poly.points[i].y, z);
        vert.pos = v3_add(vert.pos, v3_s_multi(normal, *first));
        vert.tex_index = tex_index;
        vertex_array_push(vert_array, vert);

        vert.pos = v3f(poly.points[i].x, poly.points[i].y, z);
        vert.pos = v3_add(vert.pos, v3_s_multi(normal, *second));
        vertex_array_push(vert_array, vert);
        s_i++;
        s_i %= 2;
    }
}

void polygon2D_draw_lines(Vertex_Array* vert_array, U32_Array* idx_array,
                          Polygon2D poly, f32 z, V4 color, f32 tex_index)
{
    Vertex vert = { 0 };
    u32 size = array_size(vert_array);
    for (u32 i = 0; i < poly.n_sides; i++)
    {
        vert.color = color;
        vert.pos = v3f(poly.points[i].x, poly.points[i].y, z);
        vert.tex_index = tex_index;
        vertex_array_push(vert_array, vert);
    }
    for (u32 i = 0; i < poly.n_sides; i++)
    {
        u32 j = (i + 1) % poly.n_sides;
        u32_array_push(idx_array, size + i);
        u32_array_push(idx_array, size + j);
    }
}

const u32 INDEX_TABLE[6] = { 0, 1, 2, 2, 3, 0 };

internal void indices_insert(U32_Array* idx_array, u32 p_i, u32 added_val0,
                             u32 added_val1)
{
    u32_array_push(idx_array, p_i);
    u32_array_push(idx_array, p_i + added_val0);
    u32_array_push(idx_array, p_i + added_val1);
}

void square_rounded_corners(Vertex_Array* vert_array, U32_Array* idx_array, V3 pos,
                            V2 size, V4 color, f32 seperation,
                            u32 corner_vertices_count, f32 tex_index)
{
    stack_begin_scope(corner_stack);

    u32 vertex_offset = vert_array->size;

    u32 indices_start = idx_array->size;

    V2 pos_plus_size = v2_add(v2_v3(pos), size);

    V2 pivot_points[4];
    pivot_points[0] = v2f(pos.x + seperation, pos.y + seperation);
    pivot_points[1] = v2f(pos.x + seperation, pos_plus_size.y - seperation);
    pivot_points[2] =
        v2f(pos_plus_size.x - seperation, pos_plus_size.y - seperation);
    pivot_points[3] = v2f(pos_plus_size.x - seperation, pos.y + seperation);

    V2* vert_pos = stack_malloc(corner_vertices_count * 4, V2);

    f32 _90_d = PI / 2.0f;
    f32 _360_d = 2.0f * PI;
    f32 d_rad = _90_d / (corner_vertices_count - 1);

    u32 count = 0;
    for (u32 corner = 0; corner < 4; corner++)
    {
        f32 extra_rad = _360_d - (_90_d * ((corner + 1) % 4));
        for (u32 i = 0; i < corner_vertices_count; i++)
        {
            f32 rad = extra_rad - (d_rad * i); // Modulus to wrap around

            vert_pos[count++] =
                v2_add(pivot_points[corner], v2f(seperation * (f32)cos((f64)rad),
                                                 seperation * (f32)sin((f64)rad)));
        }
    }
    Vertex vert = { 0 };
    vert.color = color;
    vert.tex_index = tex_index;

    u32 num_corner_vertices_2x = corner_vertices_count * 2;
    u32 pivot_indicies[4] = { 0, num_corner_vertices_2x + 1,
                              num_corner_vertices_2x + 2,
                              (num_corner_vertices_2x * 2) + 3 };

    u32* p_i = &pivot_indicies[0];
    V2* p_pos = &pivot_points[0];

    count = 0;
    for (u32 half = 0; half < 2; half++)
    {
        vert.pos = v3_v2f(*p_pos, pos.z);

        vertex_array_push(vert_array, vert);
        for (u32 quarters = 0; quarters < 2; quarters++)
        {
            for (u32 j = 0; j < corner_vertices_count; j++)
            {
                vert.pos = v3_v2f(vert_pos[count++], pos.z);
                vertex_array_push(vert_array, vert);
            }
        }
        p_pos++;
        vert.pos = v3_v2f(*p_pos, pos.z);
        vertex_array_push(vert_array, vert);
        p_pos++;

        i32 j = 1;
        for (; j <= (i32)corner_vertices_count; j++)
        {
            indices_insert(idx_array, *p_i, j, 1 + j);
        }
        p_i++;

        i32 i = 0;
        i32 low_iterations = corner_vertices_count - 1;
        for (; i < low_iterations; i++)
        {
            u32_array_push(idx_array, *p_i);
            u32_array_push(idx_array, (*(p_i - 1)) + j++);
            u32_array_push(idx_array, (*(p_i - 1)) + j);
        }
        u32_array_push(idx_array, *(p_i - 1));
        u32_array_push(idx_array, (*p_i) - (1 + i));
        u32_array_push(idx_array, *p_i);
        p_i++;
    }
    u32 inner_square_indices[4] = { 1, num_corner_vertices_2x,
                                    num_corner_vertices_2x + 3,
                                    (num_corner_vertices_2x * 2) + 2 };

    for (u32 i = 0; i < 6; i++)
    {
        u32_array_push(idx_array, inner_square_indices[INDEX_TABLE[i]]);
    }

    u32 indices_end = idx_array->size;

    for (u32 i = indices_start; i < indices_end; i++)
    {
        u32_array_val(idx_array, i) += vertex_offset;
    }

    stack_end_scope(corner_stack);
}

void square_rounded_corners_3d(Vertex_Array* vert_array, U32_Array* idx_array,
                               V3 pos, V3 size, V4 color, f32 seperation,
                               u32 corner_vertices_count, f32 tex_index)
{
    const u32 offset = vert_array->size;

    pos.y -= size.y * 0.5f;
    pos.x -= size.x * 0.5f;

    square_rounded_corners(vert_array, idx_array, pos, v2_v3(size), color,
                           seperation, corner_vertices_count, tex_index);

    const u32 offset_per_half = vert_array->size;
    for (u32 i = offset; i < offset_per_half; i++)
    {
        vertex_array_val(vert_array, i).normal = v3f(0.0f, 0.0f, -1.0f);
    }
    const u32 size_per_side = vert_array->size - offset;
    pos.z += size.z;
    square_rounded_corners(vert_array, idx_array, pos, v2_v3(size), color,
                           seperation, corner_vertices_count, tex_index);

    for (u32 i = offset_per_half; i < vert_array->size; i++)
    {
        vertex_array_val(vert_array, i).normal = v3f(0.0f, 0.0f, 1.0f);
    }
    const u32 index_table_3d[2][6] = {
        { 0, size_per_side, 1, 1, size_per_side + 1, size_per_side },
        { 0, size_per_side, 2, 2, size_per_side + 2, size_per_side }
    };

    u32 vertices_count = ((size_per_side + offset)) / 2;
    u32 i = offset + 1;
    for (u32 half = 0; half < 2; half++)
    {
        u32 table_index = 0;
        for (; i < vertices_count - 1; i++)
        {
            for (u32 j = 0; j < 6; j++)
            {
                u32 index = i + index_table_3d[table_index][j];
                u32_array_push(idx_array, index);
            }
        }
        table_index++;
        if (!half)
        {
            for (u32 j = 0; j < 6; j++)
            {
                u32 index = i + index_table_3d[table_index][j];
                u32_array_push(idx_array, index);
            }
        }
        else
        {
            u32_array_push(idx_array, i);
            u32_array_push(idx_array, i + size_per_side);
            u32 index = offset + 1;
            u32_array_push(idx_array, index);
            u32_array_push(idx_array, index);
            index += size_per_side;
            u32_array_push(idx_array, index);
            u32_array_push(idx_array, i + size_per_side);
        }

        i += 2;
        vertices_count *= 2;
    }
}

void indices_generate(U32_Array* array, u32 offset, u32 indices_count)
{
    for (u32 i = 0; i < indices_count; i++)
    {
        for (u32 j = 0; j < 6; j++)
        {
            u32_array_push(array, ((INDEX_TABLE[j] + (4 * i)) + offset));
        }
    }
}

#if 0
const V3 normalTableFaces[] = {
    { -1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f },
    { 1.0f, 0.0f, 0.0f },  { 0.0f, 1.0f, 0.0f },  { 0.0f, 0.0f, -1.0f },
};
#endif

const V3 normalTableVertex[] = {
    { -1.0f / 3.0f, -1.0f / 3.0f, 1.0f / 3.0f },
    { -1.0f / 3.0f, -1.0f / 3.0f, -1.0f / 3.0f },
    { -1.0f / 3.0f, 1.0f / 3.0f, -1.0f / 3.0f },
    { -1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f },
    { 1.0f / 3.0f, -1.0f / 3.0f, 1.0f / 3.0f },
    { 1.0f / 3.0f, -1.0f / 3.0f, -1.0f / 3.0f },
    { 1.0f / 3.0f, 1.0f / 3.0f, -1.0f / 3.0f },
    { 1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f },
};

u32 cube(Vertex_Array* vert_array, u32 offset, V3 pos, V3 size, V4 color,
         f32 tex_index)
{
    V3 left_side = v3_sub(pos, v3_s_multi(size, 0.5f));
    V3 right_side = left_side;
    right_side.x += size.x;

    Vertex verts[] = {
        { v3f(left_side.x, left_side.y, left_side.z + size.z), normalTableVertex[0],
          v2f(0.0f, 0.0f), color, tex_index },
        { left_side, normalTableVertex[1], v2f(0.0f, 1.0f), color, tex_index },
        { v3f(left_side.x, left_side.y + size.y, left_side.z), normalTableVertex[2],
          v2f(1.0f, 1.0f), color, tex_index },
        { v3f(left_side.x, left_side.y + size.y, left_side.z + size.z),
          normalTableVertex[3], v2f(1.0f, 0.0f), color, tex_index },
        { v3f(right_side.x, right_side.y, right_side.z + size.z),
          normalTableVertex[4], v2f(0.0f, 0.0f), color, tex_index },
        { right_side, normalTableVertex[5], v2f(0.0f, 1.0f), color, tex_index },
        { v3f(right_side.x, right_side.y + size.y, right_side.z),
          normalTableVertex[6], v2f(1.0f, 1.0f), color, tex_index },
        { v3f(right_side.x, right_side.y + size.y, right_side.z + size.z),
          normalTableVertex[7], v2f(1.0f, 1.0f), color, tex_index },
    };

    u32 num_verts = sy_SIZE(verts);
    for (u32 i = 0; i < num_verts; i++)
    {
        vertex_array_val(vert_array, offset++) = verts[i];
    }
    return offset;
}

void cube1(Vertex_Array* vert_array, V3 pos, V3 size, V4 color, f32 tex_index)
{
    const u32 offset = vert_array->size;
    const u32 size_increase =
        cube(vert_array, offset, pos, size, color, tex_index) - offset;
    vert_array->size += size_increase;
}

void cube_not_center1(Vertex_Array* vert_array, V3 pos, V3 size, V4 color,
                      f32 tex_index)
{
    pos = v3_add(pos, v3_s_multi(size, 0.5f));
    cube1(vert_array, pos, size, color, tex_index);
}

u32 cube_not_center(Vertex_Array* vert_array, u32 offset, V3 pos, V3 size, V4 color,
                    f32 tex_index)
{
    pos = v3_add(pos, v3_s_multi(size, 0.5f));
    return cube(vert_array, offset, pos, size, color, tex_index);
}

#if 1
const u32 CUBE_INDEX_TABLE[] = { 0, 1, 2, 2, 3, 0, 3, 2, 6, 6, 7, 3,
                                 7, 6, 5, 5, 4, 7, 4, 5, 1, 1, 0, 4,
                                 4, 0, 3, 3, 7, 4, 1, 5, 6, 6, 2, 1 };
#else
const u32 CUBE_INDEX_TABLE[] = { 0,  3,  6,  6,  9,  0,  1,  12, 15,
                                 15, 4,  1,  2,  13, 21, 21, 10, 2,

                                 14, 16, 18, 18, 22, 14, 11, 23, 19,
                                 19, 7,  11, 8,  20, 17, 17, 5,  8 };

#endif

void cube_indices_offset(U32_Array* indices, u32 offset, u32 how_many)
{
    const u32 table_size = sy_SIZE(CUBE_INDEX_TABLE);
    u32 temp_table[sy_SIZE(CUBE_INDEX_TABLE)] = { 0 };
    for (u32 i = 0; i < table_size; i++)
    {
        temp_table[i] = CUBE_INDEX_TABLE[i] + offset;
    }
    u32 step = 0;
    for (u32 i = offset; i < how_many + offset; i++)
    {
        for (u32 j = 0; j < table_size; j++)
        {
            u32_array_push(indices, temp_table[j] + (8 * step));
        }
        step++;
    }
}

void cube_indices(U32_Array* indices, u32 offset, u32 how_many)
{
    u32 table_size = sy_SIZE(CUBE_INDEX_TABLE);
    for (u32 i = offset; i < how_many + offset; i++)
    {
        for (u32 j = 0; j < table_size; j++)
        {
            u32_array_push(indices, CUBE_INDEX_TABLE[j] + (8 * i));
        }
    }
}

u32 gridd_using_line_list(Vertex_Array* vert_array, u32 vertex_offset,
                          U32_Array* indices, u32 index_offset, V3 middle_pos,
                          V2 spacing, u32 lines_width_count, u32 lines_height_count,
                          V4 color, f32 tex_index)
{
    u32 vert_offset = vertex_offset;
    assert(lines_height_count > 0);
    assert(lines_width_count > 0);

    V2 total_size = { 0 };
    total_size.width = lines_width_count * spacing.width;
    total_size.height = lines_height_count * spacing.height;

    V3 current_pos = v3_sub(middle_pos, v3_v2(v2_s_multi(total_size, 0.5f)));
    V3 saved_pos = current_pos;
    current_pos.x += spacing.x * 0.5f;

    Vertex vert = { 0 };
    vert.color = color;
    vert.tex_index = tex_index;
    for (u32 i = 0; i < lines_width_count; i++)
    {
        vert.pos = current_pos;
        vertex_array_val(vert_array, vert_offset++) = vert;
        vert.pos.y += total_size.height;
        vertex_array_val(vert_array, vert_offset++) = vert;
        current_pos.x += spacing.x;
    }
    current_pos = saved_pos;
    current_pos.y += spacing.y * 0.5f;

    for (u32 i = 0; i < lines_height_count; i++)
    {
        vert.pos = current_pos;
        vertex_array_val(vert_array, vert_offset++) = vert;
        vert.pos.x += total_size.width;
        vertex_array_val(vert_array, vert_offset++) = vert;
        current_pos.y += spacing.y;
    }
    for (u32 i = vertex_offset; i < vert_offset; i++)
    {
        u32_array_val(indices, index_offset++) = i;
    }
    u32 size = vert_offset - vertex_offset;
    return size;
}

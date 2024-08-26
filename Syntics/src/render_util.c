#ifndef SY_UNIT_BUILD
#include "render_util.h"
#include "region_alloc.h"
#include "math/syntics_math.h"
#include "collision.h"
#include "logging.h"
#endif

AABB_2D _set_up_verticies(Vertex_2D_Array* vertex_array, V2 position, V2 size, V4 color,
                          f32 texture_index, Texture_Coordinates texture_coordinates)
{
    Vertex_2D vertex = {
        .position = position,
        .texture_coordinates = texture_coordinates.coordinates[0],
        .color = color,
        .texture_index = texture_index,
    };
    array_push(vertex_array, vertex);
    vertex.position.y += size.y;
    vertex.texture_coordinates = texture_coordinates.coordinates[1];
    array_push(vertex_array, vertex);
    vertex.position.x += size.x;
    vertex.texture_coordinates = texture_coordinates.coordinates[2];
    array_push(vertex_array, vertex);
    vertex.position.y -= size.y;
    vertex.texture_coordinates = texture_coordinates.coordinates[3];
    array_push(vertex_array, vertex);

    AABB_2D out;
    out.min = position;
    out.size = size;
    return out;
}

AABB_2D quad_co(Vertex_2D_Array* vertex_array, V2 position, V2 size, V4 color,
                V4 texture_coordinates, f32 texture_index)
{
    Texture_Coordinates _tex_coords = { v2_v4(texture_coordinates),
                                        v2f(texture_coordinates.x, texture_coordinates.w),
                                        v2f(texture_coordinates.z, texture_coordinates.w),
                                        v2f(texture_coordinates.z, texture_coordinates.y) };
    return _set_up_verticies(vertex_array, position, size, color, texture_index, _tex_coords);
}

AABB_2D quad(Vertex_2D_Array* vertex_array, V2 position, V2 size, V4 color, f32 texture_index)
{
    Texture_Coordinates texture_coordinates = { v2d(), v2f(0.0f, 1.0f), v2f(1.0f, 1.0f),
                                                v2f(1.0f, 0.0f) };
    return _set_up_verticies(vertex_array, position, size, color, texture_index,
                             texture_coordinates);
}

AABB_2D quad_f(Vertex_2D_Array* vertex_array, V2 position, V2 size, V4 color, f32 texture_index)
{
    Texture_Coordinates texture_coordinates = { v2f(0.0f, 1.0f), v2d(), v2f(1.0f, 0.0f),
                                                v2f(1.0f, 1.0f) };
    return _set_up_verticies(vertex_array, position, size, color, texture_index,
                             texture_coordinates);
}

AABB_2D quad_gradiant_l_r(Vertex_2D_Array* vertex_array, V2 position, V2 size, V4 left_color,
                          V4 right_color, f32 texture_index)
{
    Vertex_2D verts[4] = {
        { left_color, v2f(position.x, position.y), v2f(0.0f, 0.0f), texture_index },
        { left_color, v2f(position.x, position.y + size.y), v2f(0.0f, 1.0f), texture_index },
        { right_color, v2f(position.x + size.x, position.y + size.y), v2f(1.0f, 1.0f),
          texture_index },
        { right_color, v2f(position.x + size.x, position.y), v2f(1.0f, 0.0f), texture_index }
    };

    for (u32 i = 0; i < 4; i++)
    {
        array_push(vertex_array, verts[i]);
    }
    AABB_2D out;
    out.min = position;
    out.size = size;
    return out;
}
AABB_2D quad_gradiant_t_b(Vertex_2D_Array* vertex_array, V2 position, V2 size, V4 top_color,
                          V4 bottom_color, f32 texture_index)
{
    Vertex_2D verts[4] = {
        { top_color, v2f(position.x, position.y), v2f(0.0f, 0.0f), texture_index },
        { bottom_color, v2f(position.x, position.y + size.y), v2f(0.0f, 1.0f), texture_index },
        { bottom_color, v2f(position.x + size.x, position.y + size.y), v2f(1.0f, 1.0f),
          texture_index },
        { top_color, v2f(position.x + size.x, position.y), v2f(1.0f, 0.0f), texture_index }
    };

    for (u32 i = 0; i < 4; i++)
    {
        array_push(vertex_array, verts[i]);
    }
    AABB_2D out;
    out.min = position;
    out.size = size;
    return out;
}

AABB_2D border_add(Vertex_2D_Array* vertex_array, u32* num_indices, V2 top_left, V2 size,
                   V4 border_color, f32 thickness, f32 texture_index)
{
    V2 h_size = v2f(size.x, thickness);
    V2 v_size = v2f(thickness, size.y);

    quad(vertex_array, top_left, h_size, border_color, texture_index);

    quad(vertex_array, v2f(top_left.x, top_left.y + v_size.y - thickness), h_size, border_color,
         texture_index);

    quad(vertex_array, top_left, v_size, border_color, texture_index);

    quad(vertex_array, v2f(top_left.x + h_size.x - thickness, top_left.y), v_size, border_color,
         texture_index);

    if (num_indices)
    {
        *num_indices += 4 * 6;
    }
    AABB_2D out = { 0 };
    out.min = top_left;
    out.size = size;
    return out;
}

void quad_middle(Vertex_2D_Array* vertex_array, V2 position, V2 size, V4 color, f32 texture_index)
{
    V2 first_pos = v2_sub(position, v2_s_multi(size, 0.5f));
    quad(vertex_array, first_pos, size, color, texture_index);
}

#if 0
void polygon2D_draw_quads(Vertex_2D_Array* vertex_array, Polygon2D poly, f32 z, V4 color,
                          f32 line_width, f32 texture_index)
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
        vert.position = v3f(poly.points[i].x, poly.points[i].y, z);
        vert.position = v3_add(vert.position, v3_s_multi(normal, *first));
        vert.texture_index = texture_index;
        array_push(vertex_array, vert);

        vert.position = v3f(poly.points[i].x, poly.points[i].y, z);
        vert.position = v3_add(vert.position, v3_s_multi(normal, *second));
        array_push(vertex_array, vert);
        s_i++;
        s_i %= 2;
    }
}

void polygon2D_draw_lines(Vertex_2D_Array* vertex_array, U32_Array* idx_array, Polygon2D poly,
                          f32 z, V4 color, f32 texture_index)
{
    Vertex vert = { 0 };
    u32 size = region_array_size(vertex_array);
    for (u32 i = 0; i < poly.n_sides; i++)
    {
        vert.color = color;
        vert.position = v3f(poly.points[i].x, poly.points[i].y, z);
        vert.texture_index = texture_index;
        array_push(vertex_array, vert);
    }
    for (u32 i = 0; i < poly.n_sides; i++)
    {
        u32 j = (i + 1) % poly.n_sides;
        array_push(idx_array, size + i);
        array_push(idx_array, size + j);
    }
}
#endif

const u32 INDEX_TABLE[2][6] = { { 0, 1, 2, 2, 3, 0 }, { 0, 3, 2, 2, 1, 0 } };

void generate_indicies(U32_Array* array, u32 offset, u32 indices_count)
{
    for (u32 i = 0; i < indices_count; i++)
    {
        for (u32 j = 0; j < 6; j++)
        {
            array_push(array, ((INDEX_TABLE[0][j] + (4 * i)) + offset));
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
    { -1.0f / 3.0f, -1.0f / 3.0f, 1.0f / 3.0f }, { -1.0f / 3.0f, -1.0f / 3.0f, -1.0f / 3.0f },
    { -1.0f / 3.0f, 1.0f / 3.0f, -1.0f / 3.0f }, { -1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f },
    { 1.0f / 3.0f, -1.0f / 3.0f, 1.0f / 3.0f },  { 1.0f / 3.0f, -1.0f / 3.0f, -1.0f / 3.0f },
    { 1.0f / 3.0f, 1.0f / 3.0f, -1.0f / 3.0f },  { 1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f },
};

u32 cube(Vertex_Array* vertex_array, u32 offset, V3 position, V3 size, V4 color, f32 texture_index)
{
    V3 left_side = v3_sub(position, v3_s_multi(size, 0.5f));
    V3 right_side = left_side;
    right_side.x += size.x;

    Vertex verts[] = {
        { v3f(left_side.x, left_side.y, left_side.z + size.z), normalTableVertex[0],
          v2f(0.0f, 0.0f), color, texture_index },
        { left_side, normalTableVertex[1], v2f(0.0f, 1.0f), color, texture_index },
        { v3f(left_side.x, left_side.y + size.y, left_side.z), normalTableVertex[2],
          v2f(1.0f, 1.0f), color, texture_index },
        { v3f(left_side.x, left_side.y + size.y, left_side.z + size.z), normalTableVertex[3],
          v2f(1.0f, 0.0f), color, texture_index },
        { v3f(right_side.x, right_side.y, right_side.z + size.z), normalTableVertex[4],
          v2f(0.0f, 0.0f), color, texture_index },
        { right_side, normalTableVertex[5], v2f(0.0f, 1.0f), color, texture_index },
        { v3f(right_side.x, right_side.y + size.y, right_side.z), normalTableVertex[6],
          v2f(1.0f, 1.0f), color, texture_index },
        { v3f(right_side.x, right_side.y + size.y, right_side.z + size.z), normalTableVertex[7],
          v2f(1.0f, 1.0f), color, texture_index },
    };

    u32 num_verts = sy_SIZE(verts);
    for (u32 i = 0; i < num_verts; i++)
    {
        array_value(vertex_array, offset++) = verts[i];
    }
    return offset;
}

void cube1(Vertex_Array* vertex_array, V3 position, V3 size, V4 color, f32 texture_index)
{
    const u32 offset = vertex_array->size;
    const u32 size_increase =
        cube(vertex_array, offset, position, size, color, texture_index) - offset;
    vertex_array->size += size_increase;
}

void cube_not_center1(Vertex_Array* vertex_array, V3 position, V3 size, V4 color, f32 texture_index)
{
    position = v3_add(position, v3_s_multi(size, 0.5f));
    cube1(vertex_array, position, size, color, texture_index);
}

u32 cube_not_center(Vertex_Array* vertex_array, u32 offset, V3 position, V3 size, V4 color,
                    f32 texture_index)
{
    position = v3_add(position, v3_s_multi(size, 0.5f));
    return cube(vertex_array, offset, position, size, color, texture_index);
}

#if 1
const u32 CUBE_INDEX_TABLE[] = { 0, 3, 2, 2, 1, 0, 3, 7, 6, 6, 2, 3, 4, 5, 6, 6, 7, 4,
                                 4, 0, 1, 1, 5, 4, 4, 7, 3, 3, 0, 4, 1, 2, 6, 6, 5, 1 };
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
            array_push(indices, temp_table[j] + (8 * step));
        }
        step++;
    }
}

void cube_indices(U32_Array* indices, u32 offset, u32 how_many)
{
    const u32 table_size = sy_SIZE(CUBE_INDEX_TABLE);
    for (u32 i = 0; i < how_many; i++)
    {
        const u32 add_index = offset + (8 * i);
        for (u32 j = 0; j < table_size; j++)
        {
            array_push(indices, CUBE_INDEX_TABLE[j] + add_index);
        }
    }
}

u32 gridd_using_line_list(Vertex_Array* vertex_array, u32 vertex_offset, U32_Array* indices,
                          u32 index_offset, V3 middle_pos, V2 spacing, u32 lines_width_count,
                          u32 lines_height_count, V4 color, f32 texture_index)
{
    u32 vert_offset = vertex_offset;
    assert(lines_height_count > 0);
    assert(lines_width_count > 0);

    V2 total_size = { 0 };
    total_size.x = lines_width_count * spacing.x;
    total_size.y = lines_height_count * spacing.y;

    V3 current_pos = v3_sub(middle_pos, v3_v2(v2_s_multi(total_size, 0.5f)));
    V3 saved_pos = current_pos;
    current_pos.x += spacing.x * 0.5f;

    Vertex vert = { 0 };
    vert.color = color;
    vert.texture_index = texture_index;
    for (u32 i = 0; i < lines_width_count; i++)
    {
        vert.position = current_pos;
        array_value(vertex_array, vert_offset++) = vert;
        vert.position.y += total_size.y;
        array_value(vertex_array, vert_offset++) = vert;
        current_pos.x += spacing.x;
    }
    current_pos = saved_pos;
    current_pos.y += spacing.y * 0.5f;

    for (u32 i = 0; i < lines_height_count; i++)
    {
        vert.position = current_pos;
        array_value(vertex_array, vert_offset++) = vert;
        vert.position.x += total_size.x;
        array_value(vertex_array, vert_offset++) = vert;
        current_pos.y += spacing.y;
    }
    for (u32 i = vertex_offset; i < vert_offset; i++)
    {
        array_value(indices, index_offset++) = i;
    }
    u32 size = vert_offset - vertex_offset;
    return size;
}

AABB_2D border_add_rounded(Vertex_2D_Array* vertex_array, u32* num_indices, V2 top_left, V2 size,
                           V4 color, f32 thickness, f32 roundness, u32 samples_per_side,
                           f32 texture_index)
{

    V2 h_size = v2f(size.x, thickness);
    V2 v_size = v2f(thickness, size.y);
    f32 half_v_size = v_size.y * 0.5f;
    const f32 pivot_offset = sy_lerp(0.0f, half_v_size, roundness);

    top_left.x += pivot_offset;
    h_size.width -= pivot_offset * 2;

    V2 pivot_point_up = top_left;
    pivot_point_up.y += pivot_offset;
    V2 pivot_point_down = v2f(top_left.x, top_left.y + v_size.y);
    pivot_point_down.y -= pivot_offset;

    quad(vertex_array, top_left, h_size, color, texture_index);

    quad(vertex_array, v2f(top_left.x, top_left.y + v_size.y - thickness), h_size, color,
         texture_index);

    quad(vertex_array, v2f(pivot_point_up.x - pivot_offset, pivot_point_up.y),
         v2f(thickness, pivot_point_down.y - pivot_point_up.y), color, texture_index);

    Texture_Coordinates texture_coordinates = { v2d(), v2f(0.0f, 1.0f), v2f(1.0f, 1.0f),
                                                v2f(1.0f, 0.0f) };

    V2* start_pivot = &pivot_point_up;
    V2* end_pivot = &pivot_point_down;
    f32 degree = PI * 0.5f;
    f32 degree_increase = degree / samples_per_side;
    for (u32 i = 0; i < 2; ++i)
    {
        V2 pivot_point = *start_pivot;
        for (u32 j = 0; j < 2; ++j)
        {
            for (u32 k = 0; k < samples_per_side; ++k)
            {

                Vertex_2D vertex = {
                    .position = v2d(),
                    .texture_coordinates = texture_coordinates.coordinates[0],
                    .color = color,
                    .texture_index = texture_index,
                };
                vertex.position = pivot_point;
                vertex.position.x += cosf(degree) * pivot_offset;
                vertex.position.y -= sinf(degree) * pivot_offset;
                array_push(vertex_array, vertex);

                vertex.position = pivot_point;
                vertex.position.x += cosf(degree) * (pivot_offset - thickness);
                vertex.position.y -= sinf(degree) * (pivot_offset - thickness);
                vertex.texture_coordinates = texture_coordinates.coordinates[1];
                array_push(vertex_array, vertex);

                degree += degree_increase;

                vertex.position = pivot_point;
                vertex.position.x += cosf(degree) * pivot_offset;
                vertex.position.y -= sinf(degree) * pivot_offset;
                vertex.texture_coordinates = texture_coordinates.coordinates[2];
                array_push(vertex_array, vertex);

                vertex.position = pivot_point;
                vertex.position.x += cosf(degree) * (pivot_offset - thickness);
                vertex.position.y -= sinf(degree) * (pivot_offset - thickness);
                vertex.texture_coordinates = texture_coordinates.coordinates[3];
                array_push(vertex_array, vertex);
            }
            pivot_point = *end_pivot;
        }
        pivot_point_up.x += h_size.x;
        pivot_point_down.x += h_size.x;

        start_pivot = &pivot_point_down;
        end_pivot = &pivot_point_up;

        degree = 1.5f * PI;
    }
    pivot_point_up.x -= h_size.x;

    quad(vertex_array, v2f(pivot_point_up.x + pivot_offset - thickness, pivot_point_up.y),
         v2f(thickness, pivot_point_down.y - pivot_point_up.y), color, texture_index);

    if (num_indices)
    {
        *num_indices += (4 + (4 * samples_per_side)) * 6;
    }
    AABB_2D out = { 0 };
    out.min = top_left;
    out.size = size;
    return out;
}

f32 ease_out_elastic(const f32 x)
{
    if (x == 0 || x == 1)
    {
        return x;
    }
    const f32 c4 = (2.0f * PI) / 3.0f;

    return powf(2.0f, -10.0f * x) * sinf((x * 10.0f - 0.75f) * c4) + 1.0f;
}

f32 ease_out_sine(const f32 x)
{
    return sinf((x * PI) / 2);
}

f32 ease_out_cubic(const f32 x)
{
    return 1 - powf(1 - x, 3);
}

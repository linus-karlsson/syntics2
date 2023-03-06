#include "render_util.h"
#include "region_alloc.h"
#include "math/transforms.h"

static V3 QUAD_VERTEX[4] = { { -0.5f, -0.5f, 0.0f },
                             { -0.5f, 0.5f, 0.0f },
                             { 0.5f, 0.5f, 0.0f },
                             { 0.5f, -0.5f, 0.0f } };

Rect2D quad(Vertex** vertices, u32* rect_count, V3 pos, V2 size, V4 color,
            f32 tex_index)
{
    Vertex verts[4] = {
        { { pos.x, pos.y, pos.z }, color, { 0.0f, 0.0f }, tex_index },
        { { pos.x, pos.y + size.y, pos.z }, color, { 0.0f, 1.0f }, tex_index },
        { { pos.x + size.x, pos.y + size.y, pos.z },
          color,
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

Rect2D quad_rect(Vertex** vertices, u32* rect_count, const Rect3D* rect)
{
    return quad(vertices, rect_count, rect->pos, rect->size, rect->color, rect->id);
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
        { { pos.x, pos.y + size.y, pos.z },
          bottom_color,
          { 0.0f, 1.0f },
          tex_index },
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
    f32 shadow_offset_2x = shadow_offset * 2.0f;
    V3 s_pos = v3f(pos.x + shadow_offset, pos.y + shadow_offset, s_pos_z);

    quad(vertices, rect_count, s_pos, size, S_COLOR, tex_index);

    return quad_gradiant_l_r(vertices, rect_count, pos, size, left_color,
                             right_color, tex_index);
}

Rect2D quad_s_gradiant_t_b(Vertex** vertices, u32* rect_count, V3 pos, V2 size,
                           V4 top_color, V4 bottom_color, f32 tex_index,
                           f32 shadow_offset)
{
    const V4 S_COLOR = v4f(0.0f, 0.0f, 0.0f, top_color.w - 0.1f);

    f32 s_pos_z = pos.z - 0.001f;
    f32 shadow_offset_2x = shadow_offset * 2.0f;
    V3 s_pos = v3f(pos.x + shadow_offset, pos.y + shadow_offset, s_pos_z);

    quad(vertices, rect_count, s_pos, size, S_COLOR, tex_index);
    return quad_gradiant_t_b(vertices, rect_count, pos, size, top_color,
                             bottom_color, tex_index);
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
    f32 shadow_offset_2x = shadow_offset * 2.0f;
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

Rect2D quad_sl_gradiant(Vertex** vertices, u32* rect_count, V3 pos, V2 size,
                        V4 color, f32 tex_index, f32 shadow_offset)
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

Rect2D add_border(Vertex** data, u32* num_indices, V4 border_color, V3 top_left,
                  V2 size, f32 thickness, f32 tex_index)
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

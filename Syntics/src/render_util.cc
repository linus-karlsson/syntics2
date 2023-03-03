#include "render_util.h"
#include "region_alloc.h"
#include "math/transforms.h"

static Vec4 QUAD_VERTEX[4] = { { -0.5f, -0.5f, 0.0f, 1.0f },
                               { -0.5f, 0.5f, 0.0f, 1.0f },
                               { 0.5f, 0.5f, 0.0f, 1.0f },
                               { 0.5f, -0.5f, 0.0f, 1.0f } };

Rect2D quad(Vertex** vertices, u32* rect_count, const V3& pos, const Vec2& size,
            const Vec4& color, f32 tex_index)
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

Rect2D quad(Vertex** vertices, u32* rect_count, const Rect3D& rect)
{
    return quad(vertices, rect_count, rect.pos, rect.size, rect.color, rect.id);
}

Rect2D quad_gradiant_l_r(Vertex** vertices, u32* rect_count, const V3& pos,
                         const Vec2& size, const Vec4& left_color,
                         const Vec4& right_color, f32 tex_index)
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
Rect2D quad_gradiant_t_b(Vertex** vertices, u32* rect_count, const V3& pos,
                         const Vec2& size, const Vec4& top_color,
                         const Vec4& bottom_color, f32 tex_index)
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

Rect2D quad_s_gradiant_l_r(Vertex** vertices, u32* rect_count, const V3& pos,
                           const Vec2& size, const Vec4& left_color,
                           const Vec4& right_color, f32 tex_index, f32 shadow_offset)
{
    const Vec4 S_COLOR = Vec4(0.0f, 0.0f, 0.0f, left_color.w - 0.1f);

    f32 s_pos_z = pos.z - 0.001f;
    f32 shadow_offset_2x = shadow_offset * 2.0f;
    V3 s_pos = V3(pos.x + shadow_offset, pos.y + shadow_offset, s_pos_z);

    quad(vertices, rect_count, s_pos, size, S_COLOR, tex_index);

    return quad_gradiant_l_r(vertices, rect_count, pos, size, left_color,
                             right_color, tex_index);
}

Rect2D quad_s_gradiant_t_b(Vertex** vertices, u32* rect_count, const V3& pos,
                           const Vec2& size, const Vec4& top_color,
                           const Vec4& bottom_color, f32 tex_index,
                           f32 shadow_offset)
{
    const Vec4 S_COLOR = Vec4(0.0f, 0.0f, 0.0f, top_color.w - 0.1f);

    f32 s_pos_z = pos.z - 0.001f;
    f32 shadow_offset_2x = shadow_offset * 2.0f;
    V3 s_pos = V3(pos.x + shadow_offset, pos.y + shadow_offset, s_pos_z);

    quad(vertices, rect_count, s_pos, size, S_COLOR, tex_index);
    return quad_gradiant_t_b(vertices, rect_count, pos, size, top_color,
                             bottom_color, tex_index);
}

Rect2D quad_s_gradiant(Vertex** vertices, u32* rect_count, V3 pos, const Vec2& size,
                       const Vec4& color, f32 multiplier, f32 tex_index,
                       f32 shadow_offset)
{
    V4 bottom_color = color * multiplier;
    bottom_color.w = color.w;
    return quad_s_gradiant_t_b(vertices, rect_count, pos, size, color, bottom_color,
                               tex_index, shadow_offset);
}

Rect2D quad_s(Vertex** vertices, u32* rect_count, const V3& pos, const Vec2& size,
              const Vec4& color, f32 tex_index, f32 shadow_offset)
{
    const Vec4 S_COLOR = Vec4(0.0f, 0.0f, 0.0f, color.w - 0.1f);
    Vec4 f_color = Vec4(color.x, color.y, color.z, color.w + 0.05f);

    f32 s_pos_z = pos.z - 0.001f;
    f32 shadow_offset_2x = shadow_offset * 2.0f;
    V3 s_pos = V3(pos.x + shadow_offset, pos.y + shadow_offset, s_pos_z);

    quad(vertices, rect_count, s_pos, size, S_COLOR, tex_index);

    return quad(vertices, rect_count, pos, size, f_color, tex_index);
}

Rect2D quad_sl(Vertex** vertices, u32* rect_count, V3 pos, const Vec2& size,
               const Vec4& color, f32 tex_index, f32 shadow_offset)
{
    const Vec4 S_COLOR = Vec4(0.0f, 0.0f, 0.0f, color.w - 0.1f);
    Vec4 f_color = Vec4(color.x, color.y, color.z, color.w + 0.05f);

    Vec4 l_color = color * 2.0f;
    l_color.w = color.w;

    f32 s_pos_z = pos.z - 0.001f;
    f32 shadow_offset_2x = shadow_offset * 2.0f;
    pos.x += shadow_offset;
    V3 s_pos_h = V3(pos.x - shadow_offset, pos.y + size.y, s_pos_z);
    V3 s_pos_v = V3(pos.x + size.x, pos.y, s_pos_z);
    V3 l_pos_h = V3(pos.x - shadow_offset, pos.y - shadow_offset, s_pos_z);
    V3 l_pos_v = V3(pos.x - shadow_offset, pos.y, s_pos_z);
    Vec2 sl_size_h = Vec2(size.x + shadow_offset_2x, shadow_offset);
    Vec2 sl_size_v = Vec2(shadow_offset, size.y);

    quad(vertices, rect_count, s_pos_h, sl_size_h, S_COLOR, tex_index);
    quad(vertices, rect_count, s_pos_v, sl_size_v, S_COLOR, tex_index);
    sl_size_h.x -= shadow_offset;
    quad(vertices, rect_count, l_pos_h, sl_size_h, l_color, tex_index);
    quad(vertices, rect_count, l_pos_v, sl_size_v, l_color, tex_index);

    return quad(vertices, rect_count, pos, size, f_color, tex_index);
}

Rect2D quad_sl_gradiant(Vertex** vertices, u32* rect_count, V3 pos, const Vec2& size,
                        const Vec4& color, f32 tex_index, f32 shadow_offset)
{
    const Vec4 S_COLOR = Vec4(0.0f, 0.0f, 0.0f, color.w - 0.1f);
    Vec4 f_color = Vec4(color.x, color.y, color.z, color.w + 0.05f);

    Vec4 l_color = color * 2.0f;
    l_color.w = color.w;

    f32 s_pos_z = pos.z - 0.001f;
    f32 shadow_offset_2x = shadow_offset * 2.0f;
    pos.x += shadow_offset;
    V3 s_pos_h = V3(pos.x - shadow_offset, pos.y + size.y, s_pos_z);
    V3 s_pos_v = V3(pos.x + size.x, pos.y, s_pos_z);
    V3 l_pos_h = V3(pos.x - shadow_offset, pos.y - shadow_offset, s_pos_z);
    V3 l_pos_v = V3(pos.x - shadow_offset, pos.y, s_pos_z);
    Vec2 sl_size_h = Vec2(size.x + shadow_offset_2x, shadow_offset);
    Vec2 sl_size_v = Vec2(shadow_offset, size.y);

    quad(vertices, rect_count, s_pos_h, sl_size_h, S_COLOR, tex_index);
    quad(vertices, rect_count, s_pos_v, sl_size_v, S_COLOR, tex_index);
    sl_size_h.x -= shadow_offset;
    quad(vertices, rect_count, l_pos_h, sl_size_h, l_color, tex_index);
    quad(vertices, rect_count, l_pos_v, sl_size_v, l_color, tex_index);

    V4 gr_color = color * 0.6f;
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

Rect2D quad(Vertex** vertices, u32* rect_count, const V3& pos, const Vec2& size,
            const Vec4& color, f32 tex_index, f32 rotation)
{
    // TODO: think translate is broken...
    Mat4f transform = translate(mat4i(10000.0f), pos) *
                      rotate(mat4i(1.0f), rotation, Z) *
                      scale(mat4i(1.0f), V3(size.x, size.y, 1.0f));

    Vec4 positions[4] = { { transform * QUAD_VERTEX[0] },
                          { transform * QUAD_VERTEX[1] },
                          { transform * QUAD_VERTEX[2] },
                          { transform * QUAD_VERTEX[3] } };

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
        { Vec4(positions[0].x, positions[0].y, positions[0].z, positions[0].w),
          color,
          { 0.0f, 0.0f },
          tex_index },
        { Vec4(positions[1].x, positions[1].y, positions[1].z, positions[1].w),
          color,
          { 0.0f, 1.0f },
          tex_index },
        { Vec4(positions[2].x, positions[2].y, positions[2].z, positions[2].w),
          color,
          { 1.0f, 1.0f },
          tex_index },
        { Vec4(positions[3].x, positions[3].y, positions[3].z, positions[3].w),
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

Rect2D quad(Vertex** vertices, const V3& pos, const V3& size, const Vec4& color,
            f32 tex_index)
{
    Vertex verts[4] = { { { pos.x, pos.y, pos.z + size.z },
                          { color.x, color.y, color.z, color.w },
                          { 0.0f, 0.0f },
                          tex_index },
                        { { pos.x, pos.y + size.y, pos.z },
                          { color.x, color.y, color.z, color.w },
                          { 0.0f, 1.0f },
                          tex_index },
                        { { pos.x + size.x, pos.y + size.y, pos.z },
                          { color.x, color.y, color.z, color.w },
                          { 1.0f, 1.0f },
                          tex_index },
                        { { pos.x + size.x, pos.y, pos.z + size.z },
                          { color.x, color.y, color.z, color.w },
                          { 1.0f, 0.0f },
                          tex_index } };

    for (u32 i = 0; i < 4; i++)
    {
        synt_push((*vertices), verts[i]);
    }

    Rect2D out;
    out.pos.x = pos.x;
    out.pos.y = pos.y;
    out.size.x = size.x;
    out.size.y = size.y;
    out.color = color;
    return out;
}

Rect2D add_border_s(Vertex** data, u32* num_indices, const V4& border_color,
                    const V3& top_left, const V2& size, f32 thickness, f32 tex_index)
{
    V2 h_size = V2(size.x, thickness);
    V2 v_size = V2(thickness, size.y);

    quad_s(data, num_indices, top_left, h_size, border_color, tex_index, 1.0f);

    quad_s(data, num_indices,
           V3(top_left.x, top_left.y + v_size.y - thickness, top_left.z), h_size,
           border_color, tex_index, 1.0f);

    quad_s(data, num_indices, top_left, v_size, border_color, tex_index, 1.0f);

    quad_s(data, num_indices,
           V3(top_left.x + h_size.x - thickness, top_left.y, top_left.z), v_size,
           border_color, tex_index, 1.0f);

    Rect2D out = {};
    out.pos.x = top_left.x;
    out.pos.y = top_left.y;
    out.size = size;
    return out;
}

Rect2D add_border(Vertex** data, u32* num_indices, const V4& border_color,
                  const V3& top_left, const V2& size, f32 thickness, f32 tex_index)
{
    V2 h_size = V2(size.x, thickness);
    V2 v_size = V2(thickness, size.y);

    quad(data, num_indices, top_left, h_size, border_color, tex_index);

    quad(data, num_indices,
         V3(top_left.x, top_left.y + v_size.y - thickness, top_left.z), h_size,
         border_color, tex_index);

    quad(data, num_indices, top_left, v_size, border_color, tex_index);

    quad(data, num_indices,
         V3(top_left.x + h_size.x - thickness, top_left.y, top_left.z), v_size,
         border_color, tex_index);

    Rect2D out = {};
    out.pos.x = top_left.x;
    out.pos.y = top_left.y;
    out.size = size;
    return out;
}

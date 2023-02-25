#pragma once

#include "math/vectors.h"

typedef struct Rect2D Rect2D;
typedef struct Vertex Vertex;

Rect2D quad(Vertex** vertices, u32* rect_count, const Vec3& pos, const Vec2& size,
            const V4& color = V4(1.0f), f32 tex_index = 0.0f);

Rect2D quad_s(Vertex** vertices, u32* rect_count, const Vec3& pos, const Vec2& size,
              const V4& color = V4(1.0f), f32 tex_index = 0.0f,
              f32 shadow_offset = 3.0f);

Rect2D quad_sl(Vertex** vertices, u32* rect_count, Vec3 pos, const Vec2& size,
               const V4& color = V4(1.0f), f32 tex_index = 0.0f,
               f32 shadow_offset = 2.0f);

Rect2D quad_sl_gradiant(Vertex** vertices, u32* rect_count, Vec3 pos,
                        const Vec2& size, const V4& color = V4(1.0f),
                        f32 tex_index = 0.0f, f32 shadow_offset = 2.0f);

Rect2D quad(Vertex** vertices, u32* rect_count, const Vec3& pos, const Vec2& size,
            const V4& color, f32 tex_index, f32 rotation);

Rect2D quad(Vertex** vertices, const Vec3& pos, const Vec3& size, const V4& color,
            f32 tex_index = 0.0f);

Rect2D quad(Vertex** vertices, u32* rect_count, const Rect3D& rect);

Rect2D quad_s_gradiant(Vertex** vertices, u32* rect_count, Vec3 pos,
                       const Vec2& size, const V4& color = V4(1.0f),
                       f32 multiplier = 0.4f, f32 tex_index = 0.0f,
                       f32 shadow_offset = 2.0f);

Rect2D quad_gradiant_l_r(Vertex** vertices, u32* rect_count, const V3& pos,
                         const Vec2& size, const V4& left_color = V4(1.0f),
                         const V4& right_color = V4(1.0f), f32 tex_index = 0.0f);

Rect2D quad_gradiant_t_b(Vertex** vertices, u32* rect_count, const V3& pos,
                         const Vec2& size, const V4& top_color = V4(1.0f),
                         const V4& bottom_color = V4(1.0f), f32 tex_index = 0.0f);

Rect2D quad_s_gradiant_l_r(Vertex** vertices, u32* rect_count, const V3& pos,
                           const Vec2& size, const V4& left_color = V4(1.0f),
                           const V4& right_color = V4(1.0f), f32 tex_index = 0.0f,
                           f32 shadow_offset = 2.0f);

Rect2D quad_s_gradiant_t_b(Vertex** vertices, u32* rect_count, const V3& pos,
                           const Vec2& size, const V4& top_color = V4(1.0f),
                           const V4& bottom_color = V4(1.0f), f32 tex_index = 0.0f,
                           f32 shadow_offset = 2.0f);

Rect2D add_border_s(Vertex** data, u32* num_indices, const V4& border_color,
                    const V3& top_left, const V2& size, f32 thickness = 1.0f,
                    f32 tex_index = 0.0f);

Rect2D add_border(Vertex** data, u32* num_indices, const V4& border_color,
                  const V3& top_left, const V2& size, f32 thickness = 1.0f,
                  f32 tex_index = 0.0f);

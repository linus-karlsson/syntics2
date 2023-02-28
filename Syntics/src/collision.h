#pragma once

#include "defines.h"

typedef struct Vec2 V2;
typedef struct Rect2D Rect2D;

b8 point_in_rect(const V2& point_pos, const Rect2D& target);

b8 rect_in_rect(const Rect2D& test_obj, const Rect2D& target_obj);

b8 dynamic_ray_rect_unsafe(const Rect2D& test_obj, const Rect2D& target_obj,
                           V2& contact_point, V2& contact_normal, f32& contact_time,
                           f32 dt, f32 low, f32 high);

b8 dynamic_ray_rect_unsafe(const Rect2D& test_obj, const Rect2D& target_obj,
                           V2& contact_normal, f32 dt, f32 low = -1.0f,
                           f32 high = 1.0f);

b8 dynamic_ray_rect(const Rect2D& test_obj, const Rect2D& target_obj,
                    V2& contact_point, V2& contact_normal, f32& contact_time,
                    f32 dt);

b8 ray_rect_rects(Rect2D& testObj, const Rect2D* targetVec, u32 num_rects, f32 dt);


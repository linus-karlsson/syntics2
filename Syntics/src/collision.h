#pragma once

#include "defines.h"

typedef struct Vec2 Vec2;
typedef struct Rect Rect;

b8 point_in_rect(const Vec2& point_pos, const Rect& target);

b8 rect_in_rect(const Rect& test_obj, const Rect& target_obj);

b8 dynamic_ray_rect(const Rect& test_obj, const Rect& target_obj,
                    Vec2& contact_point, Vec2& contact_normal, f32& contact_time,
                    f32 deltaTime);

b8 ray_rect_rects(Rect& testObj, const Rect* targetVec, u32 num_rects, f32 dt);


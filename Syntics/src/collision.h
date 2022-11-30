#pragma once

#include "defines.h"

namespace synt {

typedef struct Vec2 Vec2;
typedef struct Rect Rect;

bool point_in_rect(const Vec2& point_pos, const Rect& target);

bool rect_in_rect(const Rect& test_obj, const Rect& target_obj);

bool dynamic_ray_rect(const Rect& test_obj, const Rect& target_obj,
                      Vec2& contact_point, Vec2& contact_normal, float& contact_time,
                      float deltaTime);

bool ray_rect_rects(Rect& testObj, const Rect* targetVec, uint32 num_rects,
                    float dt);

} // namespace synt


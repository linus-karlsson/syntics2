#pragma once

namespace synt {

typedef struct Vec2 Vec2;
typedef struct Rect Rect;

bool point_in_rect(const Vec2& point_pos, const Rect& target);

bool rect_in_rect(const Rect& test_obj, const Rect& target_obj);

} // namespace synt


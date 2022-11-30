#include "collision.h"
#include "math/vectors.h"
#include "logging.h"

namespace synt {

bool point_in_rect(const Vec2& point_pos, const Rect& target)
{
    return (point_pos.x >= target.pos.x && point_pos.y >= target.pos.y &&
            point_pos.x < target.pos.x + target.size.x &&
            point_pos.y < target.pos.y + target.size.y);
}

bool rect_in_rect(const Rect& test_obj, const Rect& target_obj)
{
    return (test_obj.pos.x <= target_obj.pos.x + target_obj.size.x &&
            test_obj.pos.x + test_obj.size.x >= target_obj.pos.x &&
            test_obj.pos.y <= target_obj.pos.y + target_obj.size.y &&
            test_obj.pos.y + test_obj.size.y >= target_obj.pos.y);
}

} // namespace synt

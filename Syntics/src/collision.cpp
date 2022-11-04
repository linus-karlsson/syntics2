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

} // namespace synt

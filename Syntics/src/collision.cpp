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

static bool RayVRect(const Vec2& ray_origin, const Vec2& ray_direction,
                     const Rect& target, Vec2& contact_point, Vec2& contact_normal,
                     float& target_hit_near)
{
    contact_normal = { 0.0f, 0.0f };
    contact_point  = { 0.0f, 0.0f };

    Vec2 invdir(1.0f / ray_direction.x, 1.0f / ray_direction.y);

    Vec2 targetNear{ (target.pos - ray_origin) * invdir };
    Vec2 targetFar{ (target.pos + target.size - ray_origin) * invdir };

    if (std::isnan(targetFar.y) || std::isnan(targetFar.x)) return false;
    if (std::isnan(targetNear.y) || std::isnan(targetNear.x)) return false;

    if (targetNear.x > targetFar.x)
    {
        std::swap(targetNear.x, targetFar.x);
    }
    if (targetNear.y > targetFar.y)
    {
        std::swap(targetNear.y, targetFar.y);
    }

    if (targetNear.x > targetFar.y || targetNear.y > targetFar.x)
    {
        return false;
    }

    targetHitNear = std::max(targetNear.x, targetNear.y);
    float targetHitFar{ std::min(targetFar.x, targetFar.y) };

    if (targetHitFar < 0)
    {
        return false;
    }

    contact_point = ray_origin + targetHitNear * ray_direction;

    if (targetNear.x > targetNear.y)
        if (rayDirection.x < 0)
            contact_normal = { 1, 0 };
        else
            contact_normal = { -1, 0 };
    else if (targetNear.x < targetNear.y)
        if (rayDirection.y < 0)
            contact_normal = { 0, 1 };
        else
            contact_normal = { 0, -1 };

    return true;
}

static bool DynamicRayVRect(const Rect& testObj, const Rect& targetObj,
                            Vec2& ContactPoint, Vec2& ContactNormal,
                            float& ContactTime, float deltaTime)
{
    if (testObj.velocity.x == 0 && testObj.velocity.y == 0) return false;

    Rect expandTarget;
    expandTarget.pos  = Vec2((targetObj.pos.x - (testObj.size.x / 2)),
                             (targetObj.pos.y - (testObj.size.y / 2)));
    expandTarget.size = Vec2((targetObj.size.x + testObj.size.x),
                             (targetObj.size.y + testObj.size.y));

    if (RayVRect(Vec2((testObj.pos.x + (testObj.size.x / 2)),
                      (testObj.pos.y + (testObj.size.y / 2))),
                 (testObj.velocity * deltaTime), expandTarget, ContactPoint,
                 ContactNormal, ContactTime))
        return (ContactTime >= 0.0f && ContactTime < 1.0f);
    else
        return false;
}

bool ray_rect_rects(Rect& testObj, const Rect* targetVec, uint32 num_rects, float dt)
{
    Vec2 ContactPoint(0.0f, 0.0f);
    Vec2 ContactNormal(0.0f, 0.0f);
    float ContactTime(0.0f);
    bool hit = false;
    for_range(i, num_rects)
    {
        if (DynamicRayVRect(testObj, targets, ContactNormal, ContactNormal,
                            ContactTime, deltaTime))
        {
            testObj.velocity += ContactNormal * Vec2(std::abs(testObj.velocity.x),
                                                     std::abs(testObj.velocity.y) *
                                                         (1 - ContactTime));
        }
        else
        {
            hit = true;
        }
    }
    return hit;
}

} // namespace synt

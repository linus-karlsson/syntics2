#include "collision.h"
#include "math/transforms.h"
#include "logging.h"
#include "math.h"

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

static void swap_f32(float& first, float& second)
{
    float temp = first;
    first      = second;
    second     = temp;
}

static bool ray_rect(const Vec2& ray_origin, const Vec2& ray_direction,
                     const Rect& target, Vec2& contact_point, Vec2& contact_normal,
                     float& target_hit_near)
{
    contact_normal = { 0.0f, 0.0f };
    contact_point  = { 0.0f, 0.0f };

    Vec2 invdir(1.0f / ray_direction.x, 1.0f / ray_direction.y);

    Vec2 target_near((target.pos - ray_origin) * invdir);
    Vec2 target_far((target.pos + target.size - ray_origin) * invdir);

    if (std::isnan(target_far.y) || std::isnan(target_far.x)) return false;
    if (std::isnan(target_near.y) || std::isnan(target_near.x)) return false;

    if (target_near.x > target_far.x)
    {
        swap_f32(target_near.x, target_far.x);
    }
    if (target_near.y > target_far.y)
    {
        swap_f32(target_near.y, target_far.y);
    }

    if (target_near.x > target_far.y || target_near.y > target_far.x)
    {
        return false;
    }

    target_hit_near      = maxf32(target_near.x, target_near.y);
    float target_hit_far = minf32(target_far.x, target_far.y);

    if (target_hit_far < 0)
    {
        return false;
    }

    contact_point = ray_origin + target_hit_near * ray_direction;

    if (target_near.x > target_near.y)
        if (ray_direction.x < 0)
            contact_normal = { 1, 0 };
        else
            contact_normal = { -1, 0 };
    else if (target_near.x < target_near.y)
        if (ray_direction.y < 0)
            contact_normal = { 0, 1 };
        else
            contact_normal = { 0, -1 };

    return true;
}

bool dynamic_ray_rect(const Rect& test_obj, const Rect& target_obj,
                      Vec2& contact_point, Vec2& contact_normal, float& contact_time,
                      float deltaTime)
{
    if (test_obj.vel.x == 0 && test_obj.vel.y == 0)
    {
        return false;
    }

    Rect expandTarget;
    expandTarget.pos  = Vec2((target_obj.pos.x - (test_obj.size.x / 2)),
                             (target_obj.pos.y - (test_obj.size.y / 2)));
    expandTarget.size = Vec2((target_obj.size.x + test_obj.size.x),
                             (target_obj.size.y + test_obj.size.y));

    if (ray_rect(Vec2((test_obj.pos.x + (test_obj.size.x / 2)),
                      (test_obj.pos.y + (test_obj.size.y / 2))),
                 (test_obj.vel * deltaTime), expandTarget, contact_point,
                 contact_normal, contact_time))
    {
        return (contact_time >= 0.0f && contact_time < 1.0f);
    }
    else
    {
        return false;
    }
}

static float abs_f32(float val) { return val < 0.0f ? val * -1.0f : val; }

bool ray_rect_rects(Rect& test_obj, const Rect* targets, uint32 num_rects, float dt)
{
    Vec2 contact_point(0.0f, 0.0f);
    Vec2 contact_normal(0.0f, 0.0f);
    float contact_time(0.0f);
    bool hit = false;
    for_range(i, num_rects)
    {
        if (dynamic_ray_rect(test_obj, targets[i], contact_normal, contact_normal,
                             contact_time, dt))
        {
            test_obj.vel +=
                contact_normal * Vec2(abs_f32(test_obj.vel.x),
                                      abs_f32(test_obj.vel.y) * (1 - contact_time));
        }
        else
        {
            hit = true;
        }
    }
    return hit;
}

} // namespace synt

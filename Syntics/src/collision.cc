#include "collision.h"
#include "math/transforms.h"
#include "logging.h"
#include "math.h"

b8 point_in_rect(const V2& point_pos, const Rect2D& target)
{
    return (point_pos.x >= target.pos.x && point_pos.y >= target.pos.y &&
            point_pos.x < target.pos.x + target.size.x &&
            point_pos.y < target.pos.y + target.size.y);
}

b8 rect_in_rect(const Rect2D& test_obj, const Rect2D& target_obj)
{
    return (test_obj.pos.x <= target_obj.pos.x + target_obj.size.x &&
            test_obj.pos.x + test_obj.size.x >= target_obj.pos.x &&
            test_obj.pos.y <= target_obj.pos.y + target_obj.size.y &&
            test_obj.pos.y + test_obj.size.y >= target_obj.pos.y);
}

static void swap_f32(f32& first, f32& second)
{
    f32 temp = first;
    first = second;
    second = temp;
}

static b8 ray_rect(const V2& ray_origin, const V2& ray_direction,
                   const Rect2D& target, V2& contact_point, V2& contact_normal,
                   f32& target_hit_near)
{
    contact_normal = { 0.0f, 0.0f };
    contact_point = { 0.0f, 0.0f };

    V2 invdir(1.0f / ray_direction.x, 1.0f / ray_direction.y);

    V2 target_near((target.pos - ray_origin) * invdir);
    V2 target_far((target.pos + target.size - ray_origin) * invdir);

    if (isnan(target_far.y) || isnan(target_far.x)) return false;
    if (isnan(target_near.y) || isnan(target_near.x)) return false;

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

    target_hit_near = maxf32(target_near.x, target_near.y);
    f32 target_hit_far = minf32(target_far.x, target_far.y);

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

b8 dynamic_ray_rect(const Rect2D& test_obj, const Rect2D& target_obj,
                    V2& contact_point, V2& contact_normal, f32& contact_time,
                    f32 deltaTime)
{
    if (test_obj.vel.x == 0 && test_obj.vel.y == 0)
    {
        return false;
    }

    Rect2D expandTarget;
    expandTarget.pos = V2((target_obj.pos.x - (test_obj.size.x / 2)),
                          (target_obj.pos.y - (test_obj.size.y / 2)));
    expandTarget.size = V2((target_obj.size.x + test_obj.size.x),
                           (target_obj.size.y + test_obj.size.y));

    if (ray_rect(V2((test_obj.pos.x + (test_obj.size.x / 2)),
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

static f32 abs_f32(f32 val)
{
    return val < 0.0f ? val * -1.0f : val;
}

b8 ray_rect_rects(Rect2D& test_obj, const Rect2D* targets, u32 num_rects, f32 dt)
{
    V2 contact_point(0.0f, 0.0f);
    V2 contact_normal(0.0f, 0.0f);
    f32 contact_time(0.0f);
    b8 hit = false;
    for_range(i, num_rects)
    {
        if (dynamic_ray_rect(test_obj, targets[i], contact_normal, contact_normal,
                             contact_time, dt))
        {
            test_obj.vel +=
                contact_normal * V2(abs_f32(test_obj.vel.x),
                                    abs_f32(test_obj.vel.y) * (1 - contact_time));
        }
        else
        {
            hit = true;
        }
    }
    return hit;
}


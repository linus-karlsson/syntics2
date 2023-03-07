#include "collision.h"
#include "math/transforms.h"
#include "logging.h"
#include "math.h"

b8 point_in_rect(V2 point_pos, const Rect2D* target)
{
    return (point_pos.x >= target->pos.x && point_pos.y >= target->pos.y &&
            point_pos.x < target->pos.x + target->size.x &&
            point_pos.y < target->pos.y + target->size.y);
}

b8 rect_in_rect(const Rect2D* test_obj, const Rect2D* target_obj)
{
    return (test_obj->pos.x <= target_obj->pos.x + target_obj->size.x &&
            test_obj->pos.x + test_obj->size.x >= target_obj->pos.x &&
            test_obj->pos.y <= target_obj->pos.y + target_obj->size.y &&
            test_obj->pos.y + test_obj->size.y >= target_obj->pos.y);
}

static void swap_f32(f32* first, f32* second)
{
    f32 temp = *first;
    *first = *second;
    *second = temp;
}

static b8 ray_rect(V2 ray_origin, V2 ray_direction, const Rect2D* target,
                   V2* contact_point, V2* contact_normal, f32* target_hit_near)
{
    *contact_normal = v2d();
    *contact_point = v2d();

    V2 invdir = v2f(1.0f / ray_direction.x, 1.0f / ray_direction.y);

    V2 target_near = v2_multi(v2_sub(target->pos, ray_origin), invdir);
    V2 target_far =
        v2_multi(v2_sub(v2_add(target->pos, target->size), ray_origin), invdir);

    if (isnan(target_far.y) || isnan(target_far.x)) return false;
    if (isnan(target_near.y) || isnan(target_near.x)) return false;

    if (target_near.x > target_far.x)
    {
        swap_f32(&target_near.x, &target_far.x);
    }
    if (target_near.y > target_far.y)
    {
        swap_f32(&target_near.y, &target_far.y);
    }

    if (target_near.x > target_far.y || target_near.y > target_far.x)
    {
        return false;
    }

    *target_hit_near = maxf32(target_near.x, target_near.y);
    f32 target_hit_far = minf32(target_far.x, target_far.y);

    if (target_hit_far < 0)
    {
        return false;
    }

    *contact_point = v2_add(ray_origin, v2_s_multi(ray_direction, *target_hit_near));

    if (target_near.x > target_near.y)
        if (ray_direction.x < 0)
            *contact_normal = v2f(1, 0);
        else
            *contact_normal = v2f(-1, 0);
    else if (target_near.x < target_near.y)
        if (ray_direction.y < 0)
            *contact_normal = v2f(0, 1);
        else
            *contact_normal = v2f(0, -1);

    return true;
}

b8 dynamic_ray_rect_unsafe_d(const Rect2D* test_obj, const Rect2D* target_obj,
                             V2* contact_normal, f32 dt, f32 low, f32 high)
{
    V2 contact_point = v2d();
    f32 contact_time = 0.0f;
    return dynamic_ray_rect_unsafe(test_obj, target_obj, &contact_point,
                                   contact_normal, &contact_time, dt, low, high);
}

b8 dynamic_ray_rect_unsafe(const Rect2D* test_obj, const Rect2D* target_obj,
                           V2* contact_point, V2* contact_normal, f32* contact_time,
                           f32 dt, f32 low, f32 high)
{
    if (test_obj->vel.x == 0 && test_obj->vel.y == 0)
    {
        return false;
    }

    Rect2D expandTarget;
    expandTarget.pos = v2f((target_obj->pos.x - (test_obj->size.x / 2)),
                           (target_obj->pos.y - (test_obj->size.y / 2)));
    expandTarget.size = v2f((target_obj->size.x + test_obj->size.x),
                            (target_obj->size.y + test_obj->size.y));

    if (ray_rect(v2f((test_obj->pos.x + (test_obj->size.x / 2)),
                     (test_obj->pos.y + (test_obj->size.y / 2))),
                 v2_s_multi(test_obj->vel, dt), &expandTarget, contact_point,
                 contact_normal, contact_time))
    {
        return (*contact_time >= low && *contact_time < high);
    }
    else
    {
        return false;
    }
}

b8 dynamic_ray_rect(const Rect2D* test_obj, const Rect2D* target_obj,
                    V2* contact_point, V2* contact_normal, f32* contact_time, f32 dt)
{
    if (test_obj->vel.x == 0 && test_obj->vel.y == 0)
    {
        return false;
    }

    Rect2D expandTarget;
    expandTarget.pos = v2f((target_obj->pos.x - (test_obj->size.x / 2)),
                           (target_obj->pos.y - (test_obj->size.y / 2)));
    expandTarget.size = v2f((target_obj->size.x + test_obj->size.x),
                            (target_obj->size.y + test_obj->size.y));

    if (ray_rect(v2f((test_obj->pos.x + (test_obj->size.x / 2)),
                     (test_obj->pos.y + (test_obj->size.y / 2))),
                 v2_s_multi(test_obj->vel, dt), &expandTarget, contact_point,
                 contact_normal, contact_time))
    {
        return (*contact_time >= 0.0f && *contact_time < 1.0f);
    }
    else
    {
        return false;
    }
}

b8 ray_rect_rects(Rect2D* test_obj, const Rect2D* targets, u32 num_rects, f32 dt)
{
    V2 contact_point = v2d();
    V2 contact_normal = v2d();
    f32 contact_time = 0.0f;
    b8 hit = false;
    for_range(i, num_rects)
    {
        if (dynamic_ray_rect(test_obj, &targets[i], &contact_point, &contact_normal,
                             &contact_time, dt))
        {
            v2_add_equal(&test_obj->vel,
                         v2_multi(contact_normal, v2f(abs_f32(test_obj->vel.x),
                                                      abs_f32(test_obj->vel.y) *
                                                          (1 - contact_time))));
        }
        else
        {
            hit = true;
        }
    }
    return hit;
}


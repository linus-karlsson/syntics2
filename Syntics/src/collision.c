#include "collision.h"
#include "defines.h"
#include "math/transforms.h"
#include "logging.h"
#include "math.h"
#include "entity.h"

b8 point_in_point(V2 point_pos, P2 target, P2 target_size)
{
    target.x -= target_size.x * 0.5f;
    target.y -= target_size.y * 0.5f;
    return (point_pos.x >= target.x && point_pos.y >= target.y &&
            point_pos.x < target.x + target_size.x &&
            point_pos.y < target.y + target_size.y);
}

b8 point_in_rect(V2 point_pos, const Rect2D* target)
{
    return (point_pos.x >= target->pos.x && point_pos.y >= target->pos.y &&
            point_pos.x < target->pos.x + target->size.x &&
            point_pos.y < target->pos.y + target->size.y);
}

b8 point_in_entity_2d(V2 point_pos, const Dynamic_Entity_2D* target)
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
    {
        if (ray_direction.x < 0)
        {
            *contact_normal = v2f(1, 0);
        }
        else
        {

            *contact_normal = v2f(-1, 0);
        }
    }
    else if (target_near.x < target_near.y)
    {
        if (ray_direction.y < 0)
        {
            *contact_normal = v2f(0, 1);
        }
        else
        {
            *contact_normal = v2f(0, -1);
        }
    }

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

b8 quad_SAT(Quad2D* test, Quad2D* target)
{
    Quad2D* _test = test;
    Quad2D* _target = target;

    V3 z_unit = v3f(0.0f, 0.0f, 1.0f);
    for_range(i, 2)
    {
        for_range(j, 4)
        {
            u32 k = (j + 1) % 4;
            // TODO: might be more efficient to do pass by const pointer instead of
            // by value. The operation below is 13 copies alone. 13 * 2.5 * 4 ish 130
            // bytes of data copied... why i'm saving the normals, probably should be
            // calculated elsewhere
            _test->normals[j] = v2_normalize(v2_v3(v3_cross(
                v3_v2(p2_sub(_test->points[k], _test->points[j])), z_unit)));

            f32 min_val0 = INFINITY;
            f32 max_val0 = -INFINITY;
            for_range(h, 4)
            {
                f32 proj_val = v2_dot(*(V2*)(_test->points + h), _test->normals[j]);
                min_val0 = minf32(min_val0, proj_val);
                max_val0 = maxf32(max_val0, proj_val);
            }

            f32 min_val1 = INFINITY;
            f32 max_val1 = -INFINITY;
            for_range(h, 4)
            {
                f32 proj_val =
                    v2_dot(*(V2*)(_target->points + h), _test->normals[j]);
                min_val1 = minf32(min_val1, proj_val);
                max_val1 = maxf32(max_val1, proj_val);
            }

            if (!(min_val0 <= max_val1 && min_val1 <= max_val0))
            {
                // no overlap
                return false;
            }
        }
        _test = target;
        _target = test;
    }
    return true;
}

b8 quad_lines(Quad2D* test, Quad2D* target)
{
    Quad2D* _test = test;
    Quad2D* _target = target;

    // V3 z_unit = v3f(0.0f, 0.0f, 1.0f);
    for_range(i, 2)
    {
        for_range(j, 4)
        {
            // lines from middle to edge
            V2 y_1_3 = _test->pos;
            P2 y_2_4 = _test->points[j];
            for_range(k, 4)
            {
                u32 h = (k + 1) % 4;
                // edge to edge lines
                P2 x_1_3 = _target->points[h];
                P2 x_2_4 = _target->points[k];

                // Source:
                // https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection

                float div_val = (x_1_3.x - x_2_4.x) * (y_1_3.y - y_2_4.y) -
                                (y_1_3.x - y_2_4.x) * (x_1_3.y - x_2_4.y);

                float t = ((x_2_4.y - x_1_3.y) * (y_1_3.x - x_2_4.x) +
                           (x_1_3.x - x_2_4.x) * (y_1_3.y - x_2_4.y)) /
                          div_val;

                float u = ((y_1_3.y - y_2_4.y) * (y_1_3.x - x_2_4.x) +
                           (y_2_4.x - y_1_3.x) * (y_1_3.y - x_2_4.y)) /
                          div_val;

                /*
                 * There will be an intersection if 0 ≤ t ≤ 1 and 0 ≤ u ≤ 1. The
                 * intersection point falls within the first line segment if 0 ≤ t ≤
                 * 1, and it falls within the second line segment if 0 ≤ u ≤ 1. These
                 * inequalities can be tested without the need for division, allowing
                 * rapid determination of the existence of any line segment
                 * intersection before calculating its exact point.
                 * */
                if (t >= 0.0f && t < 1.0f && u >= 0.0f && u < 1.0f)
                {
                    return true;
                }
            }
        }
        _test = target;
        _target = test;
    }
    return false;
}

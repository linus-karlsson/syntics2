#ifndef SY_UNIT_BUILD
#include "collision.h"
#include "defines.h"
#include "math/syntics_math.h"
#include "logging.h"
#include "entity.h"
#include <math.h>
#endif

b8 aabb_2d_equal(const AABB_2D* first, const AABB_2D* second)
{
    return v2_equal(first->min, second->min) &&
           v2_equal(first->size, second->size);
}

b8 collision_point_in_point(V2 point_pos, V2 target, V2 target_size)
{
    target.x -= target_size.x * 0.5f;
    target.y -= target_size.y * 0.5f;
    return (point_pos.x >= target.x && point_pos.y >= target.y &&
            point_pos.x < target.x + target_size.x &&
            point_pos.y < target.y + target_size.y);
}

b8 collision_point_in_aabb_2d(V2 point_pos, const AABB_2D* target)
{
    b8 res = point_pos.x >= target->min.x && point_pos.y >= target->min.y &&
             point_pos.x < target->min.x + target->size.x &&
             point_pos.y < target->min.y + target->size.y;
    return res;
}

b8 collision_point_in_aabb_3d(V3 point_pos, const AABB_3D* target)
{
    b8 res = point_pos.x >= target->min.x &&
             point_pos.x < target->min.x + target->size.x &&
             point_pos.y >= target->min.y &&
             point_pos.y < target->min.y + target->size.y &&
             point_pos.z >= target->min.z &&
             point_pos.z < target->min.z + target->size.z;

    return res;
}

const V3 NORMALS_3D_TABLE[6] = {
    { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f },  { 0.0f, -1.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f },  { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, 1.0f },
};

b8 collision_aabb_in_aabb_3d_normal(const AABB_3D* test_obj,
                                    const AABB_3D* target, V3* normal)
{
    const f32 overlap_x = minf32(test_obj->min.x + test_obj->size.x,
                                 target->min.x + target->size.x) -
                          maxf32(test_obj->min.x, target->min.x);

    const f32 overlap_y = minf32(test_obj->min.y + test_obj->size.y,
                                 target->min.y + target->size.y) -
                          maxf32(test_obj->min.y, target->min.y);

    const f32 overlap_z = minf32(test_obj->min.z + test_obj->size.z,
                                 target->min.z + target->size.z) -
                          maxf32(test_obj->min.z, target->min.z);

    if (overlap_x < overlap_y)
    {
        if (test_obj->min.x < target->min.x)
        {
            *normal = NORMALS_3D_TABLE[0];
        }
        else
        {
            *normal = NORMALS_3D_TABLE[1];
        }
    }
    else
    {
        if (test_obj->min.y < target->min.y)
        {
            *normal = NORMALS_3D_TABLE[2];
        }
        else
        {
            *normal = NORMALS_3D_TABLE[3];
        }
    }
    return overlap_x > 0.0f && overlap_y > 0.0f && overlap_z > 0.0f;
}

b8 collision_point_in_entity_2d(V2 point_pos, const Dynamic_Entity_2D* target)
{
    return (point_pos.x >= target->movement->pos.x &&
            point_pos.y >= target->movement->pos.y &&
            point_pos.x < target->movement->pos.x + target->misc->size.x &&
            point_pos.y < target->movement->pos.y + target->misc->size.y);
}

const V2 NORMALS_2D_TABLE[4] = {
    { -1.0f, 0.0f }, { 1.0f, 0.0f }, { 0.0f, -1.0f }, { 0.0f, 1.0f }
};

b8 collision_rect_in_rect_normal(const Rect2D* test_obj,
                                 const Rect2D* target_obj, V2* normal)
{
    const f32 overlap_x = minf32(test_obj->pos.x + test_obj->size.x,
                                 target_obj->pos.x + target_obj->size.x) -
                          maxf32(test_obj->pos.x, target_obj->pos.x);

    const f32 overlap_y = minf32(test_obj->pos.y + test_obj->size.y,
                                 target_obj->pos.y + target_obj->size.y) -
                          maxf32(test_obj->pos.y, target_obj->pos.y);

    if (overlap_x < overlap_y)
    {
        if (test_obj->pos.x < target_obj->pos.x)
        {
            *normal = NORMALS_2D_TABLE[0];
        }
        else
        {
            *normal = NORMALS_2D_TABLE[1];
        }
    }
    else
    {
        if (test_obj->pos.y < target_obj->pos.y)
        {
            *normal = NORMALS_2D_TABLE[2];
        }
        else
        {
            *normal = NORMALS_2D_TABLE[3];
        }
    }
    return (overlap_x > 0.0f && overlap_y > 0.0f);
}

b8 collision_rect_in_rect_2d(const Rect2D* test_obj, const Rect2D* target_obj)
{
    return (test_obj->pos.x <= target_obj->pos.x + target_obj->size.x &&
            test_obj->pos.x + test_obj->size.x >= target_obj->pos.x &&
            test_obj->pos.y <= target_obj->pos.y + target_obj->size.y &&
            test_obj->pos.y + test_obj->size.y >= target_obj->pos.y);
}

b8 collision_rect_in_rect_3d(const Rect3D* test_obj, const Rect3D* target_obj)
{
    return (test_obj->pos.x <= target_obj->pos.x + target_obj->size.x &&
            test_obj->pos.x + test_obj->size.x >= target_obj->pos.x &&
            test_obj->pos.y <= target_obj->pos.y + target_obj->size.y &&
            test_obj->pos.y + test_obj->size.y >= target_obj->pos.y &&
            test_obj->pos.z <= target_obj->pos.z + target_obj->size.z &&
            test_obj->pos.z + test_obj->size.z >= target_obj->pos.z);
}

internal void swap_f32(f32* first, f32* second)
{
    f32 temp = *first;
    *first = *second;
    *second = temp;
}

internal b8 ray_rect(V2 ray_origin, V2 ray_direction, const Rect2D* target,
                     V2* contact_point, V2* contact_normal,
                     f32* target_hit_near)
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

    *contact_point =
        v2_add(ray_origin, v2_s_multi(ray_direction, *target_hit_near));

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

b8 collision_dynamic_ray_rect_unsafe(const Rect2D* test_obj,
                                     const Rect2D* target_obj,
                                     V2* contact_point, V2* contact_normal,
                                     f32* contact_time, f32 dt, f32 low,
                                     f32 high)
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

b8 collision_dynamic_ray_rect_unsafe_d(const Rect2D* test_obj,
                                       const Rect2D* target_obj,
                                       V2* contact_normal, f32 dt, f32 low,
                                       f32 high)
{
    V2 contact_point = v2d();
    f32 contact_time = 0.0f;
    return collision_dynamic_ray_rect_unsafe(test_obj, target_obj,
                                             &contact_point, contact_normal,
                                             &contact_time, dt, low, high);
}

b8 collision_dynamic_ray_rect(const Rect2D* test_obj, const Rect2D* target_obj,
                              V2* contact_point, V2* contact_normal,
                              f32* contact_time, f32 dt)
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

b8 collision_ray_rect_rects(Rect2D* test_obj, const Rect2D* targets,
                            u32 num_rects, f32 dt)
{
    V2 contact_point = v2d();
    V2 contact_normal = v2d();
    f32 contact_time = 0.0f;
    b8 hit = false;
    for (u32 i = 0; i < num_rects; i++)
    {
        if (collision_dynamic_ray_rect(test_obj, &targets[i], &contact_point,
                                       &contact_normal, &contact_time, dt))
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

b8 collision_point_SAT(V2 test, Polygon2D* target)
{
    V3 z_unit = v3f(0.0f, 0.0f, 1.0f);
    for (u32 i = 0; i < target->n_sides; i++)
    {
        u32 j = (i + 1) % target->n_sides;
        target->normals[i] = v2_normalize(v2_v3(v3_cross(
            v3_v2(v2_sub(target->points[j], target->points[i])), z_unit)));

        f32 min_val = INFINITY;
        f32 max_val = -INFINITY;
        for (u32 k = 0; k < target->n_sides; k++)
        {
            f32 proj_val = v2_dot(target->points[k], target->normals[i]);
            min_val = minf32(min_val, proj_val);
            max_val = maxf32(max_val, proj_val);
        }
        f32 point_val = v2_dot(test, target->normals[i]);

        if (!(closed_interval(min_val, point_val, max_val)))
        {
            // no overlap
            return false;
        }
    }

    // Check bounding box

    P2 min_val = p2i(INFINITY);
    P2 max_val = p2i(-INFINITY);
    for (u32 i = 0; i < target->n_sides; i++)
    {
        min_val.x = minf32(min_val.x, target->points[i].x);
        min_val.y = minf32(min_val.y, target->points[i].y);
        max_val.x = maxf32(max_val.x, target->points[i].x);
        max_val.y = maxf32(max_val.y, target->points[i].y);
    }
    AABB_2D r;
    r.min = v2f(min_val.x, min_val.y);
    r.size = p2_sub(max_val, min_val);

    if (!collision_point_in_aabb_2d(test, &r))
    {
        return false;
    }
    return true;
}

b8 collision_polygon2D_SAT(Polygon2D* test, Polygon2D* target)
{
    Polygon2D* _test = test;
    Polygon2D* _target = target;

    V3 z_unit = v3f(0.0f, 0.0f, 1.0f);
    for (u32 i = 0; i < 2; i++)
    {
        for (u32 j = 0; j < _test->n_sides; j++)
        {
            u32 k = (j + 1) % _test->n_sides;
            _test->normals[j] = v2_normalize(v2_v3(v3_cross(
                v3_v2(v2_sub(_test->points[k], _test->points[j])), z_unit)));

            f32 min_val0 = INFINITY;
            f32 max_val0 = -INFINITY;
            for (u32 h = 0; h < _test->n_sides; h++)
            {
                f32 proj_val = v2_dot(_test->points[h], _test->normals[j]);
                min_val0 = minf32(min_val0, proj_val);
                max_val0 = maxf32(max_val0, proj_val);
            }

            f32 min_val1 = INFINITY;
            f32 max_val1 = -INFINITY;
            for (u32 h = 0; h < _target->n_sides; h++)
            {
                f32 proj_val = v2_dot(_target->points[h], _test->normals[j]);
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

b8 collision_polygon2D_SAT_static(Polygon2D* test, Polygon2D* target,
                                  V2* displacement_pos, V2* normal)
{
    Polygon2D* _test = test;
    Polygon2D* _target = target;

    f32 overlap = INFINITY;

    f32 min_proj = INFINITY;

    V2 center_diff = v2_sub(target->pos, test->pos);

    V3 z_unit = v3f(0.0f, 0.0f, 1.0f);
    for (u32 i = 0; i < 2; i++)
    {
        for (u32 j = 0; j < _test->n_sides; j++)
        {
            u32 k = (j + 1) % _test->n_sides;
            _test->normals[j] = v2_normalize(v2_v3(v3_cross(
                v3_v2(v2_sub(_test->points[k], _test->points[j])), z_unit)));

            if (i == 0)
            {
                f32 proj = v2_dot(center_diff, _test->normals[i]);
                if (proj < min_proj)
                {
                    min_proj = proj;
                    *normal = _test->normals[i];
                }
            }

            f32 min_val0 = INFINITY;
            f32 max_val0 = -INFINITY;
            for (u32 h = 0; h < _test->n_sides; h++)
            {
                f32 proj_val = v2_dot(_test->points[h], _test->normals[j]);
                min_val0 = minf32(min_val0, proj_val);
                max_val0 = maxf32(max_val0, proj_val);
            }

            f32 min_val1 = INFINITY;
            f32 max_val1 = -INFINITY;
            for (u32 h = 0; h < _target->n_sides; h++)
            {
                f32 proj_val = v2_dot(_target->points[h], _test->normals[j]);
                min_val1 = minf32(min_val1, proj_val);
                max_val1 = maxf32(max_val1, proj_val);
            }

            overlap =
                minf32(minf32(max_val0, max_val1) - maxf32(min_val0, min_val1),
                       overlap);

            if (!(min_val0 <= max_val1 && min_val1 <= max_val0))
            {
                // no overlap
                return false;
            }
        }
        _test = target;
        _target = test;
    }

    V2 d = v2_sub(target->pos, test->pos);
    f32 s = v2_len(d);
    displacement_pos->x -= overlap * d.x / s;
    displacement_pos->y -= overlap * d.y / s;
    return true;
}

b8 collision_polygon2D_lines(Polygon2D* test, Polygon2D* target)
{
    Polygon2D* _test = test;
    Polygon2D* _target = target;

    for (u32 i = 0; i < 2; i++)
    {
        for (u32 j = 0; j < _test->n_sides; j++)
        {
            // lines from middle to edge
            V2 _1 = _test->pos;
            V2 _2 = _test->points[j];
            for (u32 k = 0; k < _target->n_sides; k++)
            {
                u32 h = (k + 1) % _target->n_sides;
                // edge to edge lines
                V2 _3 = _target->points[k];
                V2 _4 = _target->points[h];

                // Source:
                // https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection

                float div_val = (_1.x - _2.x) * (_3.y - _4.y) -
                                (_1.y - _2.y) * (_3.x - _4.x);

                float t = ((_1.x - _3.x) * (_3.y - _4.y) -
                           (_1.y - _3.y) * (_3.x - _4.x)) /
                          div_val;

                float u = ((_1.x - _3.x) * (_1.y - _2.y) -
                           (_1.y - _3.y) * (_1.x - _2.x)) /
                          div_val;

                /*
                 * There will be an intersection if 0 ≤ t ≤ 1 and 0 ≤ u ≤ 1. The
                 * intersection point falls within the first line segment if 0 ≤
                 * t ≤ 1, and it falls within the second line segment if 0 ≤ u
                 * ≤ 1. These inequalities can be tested without the need for
                 * division, allowing rapid determination of the existence of
                 * any line segment intersection before calculating its exact
                 * point.
                 * */
                if (closed_interval(0.0f, t, 1.0f) &&
                    closed_interval(0.0f, u, 1.0f))
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

b8 collision_polygon2D_lines_static(Polygon2D* test, Polygon2D* target,
                                    V2* displacement_pos)
{
    Polygon2D* _test = test;
    Polygon2D* _target = target;

    b8 res = false;

    for (u32 i = 0; i < 2; i++)
    {
        for (u32 j = 0; j < _test->n_sides; j++)
        {
            V2 displacement = v2d();
            // lines from middle to edge
            V2 _1 = _test->pos;
            V2 _2 = _test->points[j];
            for (u32 k = 0; k < _target->n_sides; k++)
            {
                u32 h = (k + 1) % _target->n_sides;
                // edge to edge lines
                V2 _3 = _target->points[k];
                V2 _4 = _target->points[h];

                // Source:
                // https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection

                float div_val = (_1.x - _2.x) * (_3.y - _4.y) -
                                (_1.y - _2.y) * (_3.x - _4.x);

                float t = ((_1.x - _3.x) * (_3.y - _4.y) -
                           (_1.y - _3.y) * (_3.x - _4.x)) /
                          div_val;

                float u = ((_1.x - _3.x) * (_1.y - _2.y) -
                           (_1.y - _3.y) * (_1.x - _2.x)) /
                          div_val;

                /*
                 * There will be an intersection if 0 ≤ t ≤ 1 and 0 ≤ u ≤ 1. The
                 * intersection point falls within the first line segment if 0 ≤
                 * t ≤ 1, and it falls within the second line segment if 0 ≤ u
                 * ≤ 1. These inequalities can be tested without the need for
                 * division, allowing rapid determination of the existence of
                 * any line segment intersection before calculating its exact
                 * point.
                 * */
                if (closed_interval(0.0f, t, 1.0f) &&
                    closed_interval(0.0f, u, 1.0f))
                {
                    res = true;
                    displacement = v2_add(
                        displacement, v2_s_multi(v2_sub(_2, _1), (1.0f - t)));
                }
            }

            *displacement_pos =
                v2_add(*displacement_pos,
                       v2_s_multi(displacement, (i == 0 ? -1.0f : 1.0f)));
        }
        _test = target;
        _target = test;
    }
    return res;
}

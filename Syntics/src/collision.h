#pragma once

#include "math/vectors.h"

typedef struct AABB
{
    V3 min;
    V3 size;
} AABB;

AABB operator+(AABB target, V3 offset);

#define point_in_point_d(point_pos, target)                                         \
    point_in_point(point_pos, target, v2i(10.0f))
b8 point_in_point(V2 point_pos, V2 target, V2 target_size);

b8 point_in_rect(V2 point_pos, const Rect2D* target);

b8 point_in_rect_aabb(V3 point_pos, AABB target);

b8 point_in_entity_2d(V2 point_pos, const Dynamic_Entity_2D* target);

b8 rect_in_rect(const Rect2D* test_obj, const Rect2D* target_obj);

b8 rect_in_rect(const Rect3D* test_obj, const Rect3D* target_obj);

b8 rect_in_rect_normal(const Rect2D* test_obj, const Rect2D* target_obj, V2* normal);

b8 dynamic_ray_rect_unsafe_d(const Rect2D* test_obj, const Rect2D* target_obj,
                             V2* contact_normal, f32 dt, f32 low, f32 high);

b8 dynamic_ray_rect_unsafe(const Rect2D* test_obj, const Rect2D* target_obj,
                           V2* contact_point, V2* contact_normal, f32* contact_time,
                           f32 dt, f32 low, f32 high);

b8 dynamic_ray_rect(const Rect2D* test_obj, const Rect2D* target_obj,
                    V2* contact_point, V2* contact_normal, f32* contact_time,
                    f32 dt);

b8 ray_rect_rects(Rect2D* testObj, const Rect2D* targetVec, u32 num_rects, f32 dt);

b8 entity_in_entity(const Rect2D* test_obj, const Rect2D* target_obj);

b8 dynamic_ray_entity_unsafe_d(const Rect2D* test_obj, const Rect2D* target_obj,
                               V2* contact_normal, f32 dt, f32 low, f32 high);

b8 dynamic_ray_entity_unsafe(const Rect2D* test_obj, const Rect2D* target_obj,
                             V2* contact_point, V2* contact_normal,
                             f32* contact_time, f32 dt, f32 low, f32 high);

b8 dynamic_ray_entity(const Rect2D* test_obj, const Rect2D* target_obj,
                      V2* contact_point, V2* contact_normal, f32* contact_time,
                      f32 dt);

b8 point_SAT(V2 test, Polygon2D* target);

b8 polygon2D_SAT(Polygon2D* test, Polygon2D* target);

b8 polygon2D_SAT_static(Polygon2D* test, Polygon2D* target, V2* displacement_pos,
                        V2* normal);

b8 polygon2D_lines(Polygon2D* test, Polygon2D* target);

b8 polygon2D_lines_static(Polygon2D* test, Polygon2D* target, V2* displacement_pos);

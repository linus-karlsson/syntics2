#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#include "math/syntics_math.h"
#endif

typedef struct AABB_3D
{
    V3 min;
    V3 size;
    u32 id;
} AABB_3D;

typedef struct AABB_2D
{
    V2 min;
    V2 size;
    u32 id;
} AABB_2D;

typedef struct AABB_2D_Array
{
    u32 size;
    u32 capacity;
    AABB_2D* data;
}AABB_2D_Array;

b8 aabb_2d_equal(const AABB_2D* first, const AABB_2D* second);

b8 collision_point_in_point(V2 point_pos, V2 target, V2 target_size);
b8 collision_point_in_aabb_2d(V2 point_pos, const AABB_2D* target);
b8 collision_point_in_aabb_3d(V3 point_pos, const AABB_3D* target);
b8 collision_aabb_in_aabb_3d_normal(const AABB_3D* test_obj, const AABB_3D* target, V3* normal);
b8 collision_point_in_entity_2d(V2 point_pos, const Dynamic_Entity_2D* target);

b8 collision_rect_in_rect_normal(const Rect2D* test_obj, const Rect2D* target_obj, V2* normal);
b8 collision_rect_in_rect_2d(const Rect2D* test_obj, const Rect2D* target_obj);
b8 collision_rect_in_rect_3d(const Rect3D* test_obj, const Rect3D* target_obj);

b8 collision_dynamic_ray_rect_unsafe(const Rect2D* test_obj, const Rect2D* target_obj, V2* contact_point, V2* contact_normal, f32* contact_time, f32 dt, f32 low, f32 high);
b8 collision_dynamic_ray_rect_unsafe_d(const Rect2D* test_obj, const Rect2D* target_obj, V2* contact_normal, f32 dt, f32 low, f32 high);
b8 collision_dynamic_ray_rect(const Rect2D* test_obj, const Rect2D* target_obj, V2* contact_point, V2* contact_normal, f32* contact_time, f32 dt);

b8 collision_ray_rect_rects(Rect2D* test_obj, const Rect2D* targets, u32 num_rects, f32 dt);

b8 collision_point_SAT(V2 test, Polygon2D* target);

b8 collision_polygon2D_SAT(Polygon2D* test, Polygon2D* target);
b8 collision_polygon2D_SAT_static(Polygon2D* test, Polygon2D* target, V2* displacement_pos, V2* normal);
b8 collision_polygon2D_lines(Polygon2D* test, Polygon2D* target);
b8 collision_polygon2D_lines_static(Polygon2D* test, Polygon2D* target, V2* displacement_pos);

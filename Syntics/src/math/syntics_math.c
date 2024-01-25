#ifndef SY_UNIT_BUILD
#include "math/syntics_math.h"
#include "region_alloc.h"
#include "logging.h"
#include "random.h"
#include <stdlib.h>
#endif
// TODO: wrong file
//

u64 hash_float(float value)
{
    return *(u32*)&value;
}

u64 hash_float2(const V2* vec)
{
    return hash_float(vec->x) ^ hash_float(vec->y);
}

u64 hash_float3(const V3* vec)
{
    return hash_float(vec->x) ^ hash_float(vec->y) ^ hash_float(vec->z);
}

u64 hash_float4(const V4* vec)
{
    return hash_float(vec->x) ^ hash_float(vec->y) ^ hash_float(vec->z) ^
           hash_float(vec->w);
}

u64 hash_vertex(const void* key, u32 len, u64 seed)
{
    const Vertex* vertex = (const Vertex*)key;

    // Choose suitable hash functions for each component
    u64 hash_pos = hash_float3(&vertex->pos);
    u64 hash_normal = hash_float3(&vertex->normal);
    u64 hash_tex_coords = hash_float2(&vertex->tex_coords);
    u64 hash_color = hash_float4(&vertex->color);
    u64 hash_tex_index = hash_float(vertex->tex_index);

    // Combine hash values
    u64 combined_hash =
        hash_pos ^ hash_normal ^ hash_tex_coords ^ hash_color ^ hash_tex_index;

    // Mix bits for additional entropy
    combined_hash =
        (combined_hash ^ (combined_hash >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    combined_hash =
        (combined_hash ^ (combined_hash >> 27)) * UINT64_C(0x94d049bb133111eb);
    combined_hash = combined_hash ^ (combined_hash >> 31);

    return combined_hash;
}

int array_index_out_of_bounds_check(u32 index, u32 capacity)
{
    assert(index < capacity);
    return index;
}

Vertex_Array vertex_array_ref_at_size_offset(Vertex_Array* array,
                                             u32 ref_capacity)
{
    assert(array->size + ref_capacity < array->capacity &&
           "vertex_array_ref_at_size_offset");
    Vertex_Array result = { 0 };
    result.capacity = ref_capacity;
    result.data = array->data + array->size;
    return result;
}

U32_Array u32_array_ref_at_size_offset(U32_Array* array, u32 ref_capacity)
{
    assert(array->size + ref_capacity < array->capacity &&
           "u32_array_ref_at_size_offset");
    U32_Array result = { 0 };
    result.capacity = ref_capacity;
    result.data = array->data + array->size;
    return result;
}

u32* u32_array_back(U32_Array* array)
{
    assert(array->size != 0 && "u32_array_back");
    return array->data + (array->size - 1);
}

V2 v2d(void)
{
    V2 res = { 0 };
    return res;
}

V2 v2i(f32 i)
{
    V2 res;
    res.x = i;
    res.y = i;
    return res;
}

V2 v2f(f32 x, f32 y)
{
    V2 res;
    res.x = x;
    res.y = y;
    return res;
}

V2 v2_v3(V3 v3)
{
    return v2f(v3.x, v3.y);
}

V2 v2_v4(V4 v4)
{
    return v2f(v4.x, v4.y);
}

V3 v3d(void)
{
    V3 res = { 0 };
    return res;
}

V3 v3i(f32 i)
{
    V3 res;
    res.x = i;
    res.y = i;
    res.z = i;
    return res;
}

V3 v3f(f32 x, f32 y, f32 z)
{
    V3 res;
    res.x = x;
    res.y = y;
    res.z = z;
    return res;
}

V2 v2_random(u32 seed, f32 min, f32 max)
{
    u32 seed1 = seed + seed * 31;
    return v2f(random_f32s(seed, min, max), random_f32s(seed1, min, max));
}

V3 v3_random(u32 seed, f32 min, f32 max)
{
    u32 seed1 = seed + seed * 31;
    u32 seed2 = seed1 + seed1 * 227;
    return v3f(random_f32s(seed, min, max), random_f32s(seed1, min, max),
               random_f32s(seed2, min, max));
}

V3 v3_v2(V2 v2)
{
    return v3f(v2.x, v2.y, 0.0f);
}

V3 v3_v2f(V2 v2, f32 z)
{
    return v3f(v2.x, v2.y, z);
}

V3 v3_v4(V4 v4)
{
    return v3f(v4.x, v4.y, v4.z);
}

V4 v4d(void)
{
    V4 res = { 0 };
    return res;
}

V4 v4i(f32 i)
{
    V4 res;
    res.x = i;
    res.y = i;
    res.z = i;
    res.w = i;
    return res;
}

V4 v4ic(f32 i)
{
    V4 res;
    res.x = i;
    res.y = i;
    res.z = i;
    res.w = 1.0f;
    return res;
}

V4 v4f(f32 x, f32 y, f32 z, f32 w)
{
    V4 res;
    res.x = x;
    res.y = y;
    res.z = z;
    res.w = w;
    return res;
}

V4 v4_v2(V2 v2)
{
    return v4f(v2.x, v2.y, 0.0f, 0.0f);
}

V4 v4_v2f(V2 v2, f32 z, f32 w)
{
    return v4f(v2.x, v2.y, z, w);
}

V4 v4_v3(V3 v3)
{
    return v4f(v3.x, v3.y, v3.z, 0.0f);
}

V4 v4_v3f(V3 v3, f32 w)
{
    return v4f(v3.x, v3.y, v3.z, w);
}

f32 v2_sum(V2 v)
{
    return (v.x + v.y);
}

f32 v3_sum(V3 v)
{
    return (v.x + v.y + v.z);
}

f32 v4_sum(V4 v)
{
    return (v.x + v.y + v.z + v.w);
}

V2 v2_add(V2 v1, V2 v2)
{
    return v2f(v1.x + v2.x, v1.y + v2.y);
}

V3 v3_add(V3 v1, V3 v2)
{
    return v3f(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

V4 v4_add(V4 v1, V4 v2)
{
    return v4f(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

V2 v2_sub(V2 v1, V2 v2)
{
    return v2f(v1.x - v2.x, v1.y - v2.y);
}

V3 v3_sub(V3 v1, V3 v2)
{
    return v3f(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

V4 v4_sub(V4 v1, V4 v2)
{
    return v4f(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
}

V2 v2_s_add(V2 v1, f32 s)
{
    return v2f(v1.x + s, v1.y + s);
}

V3 v3_s_add(V3 v1, f32 s)
{
    return v3f(v1.x + s, v1.y + s, v1.z + s);
}

V4 v4_s_add(V4 v1, f32 s)
{
    return v4f(v1.x + s, v1.y + s, v1.z + s, v1.w + s);
}

V2 v2_s_sub(V2 v1, f32 s)
{
    return v2f(v1.x - s, v1.y - s);
}

V3 v3_s_sub(V3 v1, f32 s)
{
    return v3f(v1.x - s, v1.y - s, v1.z - s);
}

V4 v4_s_sub(V4 v1, f32 s)
{
    return v4f(v1.x - s, v1.y - s, v1.z - s, v1.w - s);
}

V2 v2_s_multi(V2 v1, f32 s)
{
    return v2f(v1.x * s, v1.y * s);
}

V3 v3_s_multi(V3 v1, f32 s)
{
    return v3f(v1.x * s, v1.y * s, v1.z * s);
}

V4 v4_s_multi(V4 v1, f32 s)
{
    return v4f(v1.x * s, v1.y * s, v1.z * s, v1.w * s);
}

V2 v2_neg(V2 v)
{
    return v2_s_multi(v, -1.0f);
}

V3 v3_neg(V3 v)
{
    return v3_s_multi(v, -1.0f);
}

V4 v4_neg(V4 v)
{
    return v4_s_multi(v, -1.0f);
}

V2 v2_multi(V2 v1, V2 v2)
{
    return v2f(v1.x * v2.x, v1.y * v2.y);
}

V3 v3_multi(V3 v1, V3 v2)
{
    return v3f(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}

V4 v4_multi(V4 v1, V4 v2)
{
    return v4f(v1.x * v2.x, v1.y * v2.y, v1.x * v2.y, v1.z);
}

V2 v2_s_div(V2 v1, f32 s)
{
    return v2f(v1.x / s, v1.y / s);
}

V3 v3_s_div(V3 v1, f32 s)
{
    return v3f(v1.x / s, v1.y / s, v1.z / s);
}

V4 v4_s_div(V4 v1, f32 s)
{
    return v4f(v1.x / s, v1.y / s, v1.z / s, v1.w / s);
}

void v2_add_equal(V2* v1, V2 v2)
{
    *v1 = v2_add(*v1, v2);
}

void v3_add_equal(V3* v1, V3 v2)
{
    *v1 = v3_add(*v1, v2);
}

void v4_add_equal(V4* v1, V4 v2)
{
    *v1 = v4_add(*v1, v2);
}

void v2_sub_equal(V2* v1, V2 v2)
{
    *v1 = v2_sub(*v1, v2);
}

void v3_sub_equal(V3* v1, V3 v2)
{
    *v1 = v3_sub(*v1, v2);
}

void v4_sub_equal(V4* v1, V4 v2)
{
    *v1 = v4_sub(*v1, v2);
}

void v2_s_add_equal(V2* v1, f32 s)
{
    *v1 = v2_s_add(*v1, s);
}

void v3_s_add_equal(V3* v1, f32 s)
{
    *v1 = v3_s_add(*v1, s);
}

void v4_s_add_equal(V4* v1, f32 s)
{
    *v1 = v4_s_add(*v1, s);
}

void v2_s_sub_equal(V2* v1, f32 s)
{
    *v1 = v2_s_sub(*v1, s);
}

void v3_s_sub_equal(V3* v1, f32 s)
{
    *v1 = v3_s_sub(*v1, s);
}

void v4_s_sub_equal(V4* v1, f32 s)
{
    *v1 = v4_s_sub(*v1, s);
}

void v2_s_multi_equal(V2* v1, f32 s)
{
    *v1 = v2_s_multi(*v1, s);
}

void v3_s_multi_equal(V3* v1, f32 s)
{
    *v1 = v3_s_multi(*v1, s);
}

void v4_s_multi_equal(V4* v1, f32 s)
{
    *v1 = v4_s_multi(*v1, s);
}

void v2_s_div_equal(V2* v1, f32 s)
{
    *v1 = v2_s_div(*v1, s);
}

void v3_s_div_equal(V3* v1, f32 s)
{
    *v1 = v3_s_div(*v1, s);
}

void v4_s_div_equal(V4* v1, f32 s)
{
    *v1 = v4_s_div(*v1, s);
}

b8 v2_equal(V2 v1, V2 v2)
{
    return v1.x == v2.x && v1.y == v2.y;
}

b8 v3_equal(V3 v1, V3 v2)
{
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

b8 v4_equal(V4 v1, V4 v2)
{
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w;
}

b8 v2_less(V2 v1, V2 v2)
{
    return (v2_sum(v1) < v2_sum(v2));
}

b8 v3_less(V3 v1, V3 v2)
{
    return (v3_sum(v1) < v3_sum(v2));
}

b8 v4_less(V4 v1, V4 v2)
{
    return (v4_sum(v1) < v4_sum(v2));
}

b8 v2_more(V2 v1, V2 v2)
{
    return (v2_sum(v1) > v2_sum(v2));
}

b8 v3_more(V3 v1, V3 v2)
{
    return (v3_sum(v1) > v3_sum(v2));
}

b8 v4_more(V4 v1, V4 v2)
{
    return (v4_sum(v1) > v4_sum(v2));
}

/*
V2 operator+(const V2& v1, const V2& v2)
{
    return v2f(v1.x + v2.x, v1.y + v2.y);
}

V3 operator+(const V3& v1, const V3& v2)
{
    return v3f(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

V4 operator+(const V4& v1, const V4& v2)
{
    return v4f(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

V2 operator-(const V2& v1, const V2& v2)
{
    return v2f(v1.x - v2.x, v1.y - v2.y);
}

V3 operator-(const V3& v1, const V3& v2)
{
    return v3f(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

V4 operator-(const V4& v1, const V4& v2)
{
    return v4f(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
}

V2 operator+(const V2& v1, f32 s)
{
    return v2f(v1.x + s, v1.y + s);
}

V3 operator+(const V3& v1, f32 s)
{
    return v3f(v1.x + s, v1.y + s, v1.z + s);
}

V4 operator+(const V4& v1, f32 s)
{
    return v4f(v1.x + s, v1.y + s, v1.z + s, v1.w + s);
}

V2 operator-(const V2& v1, f32 s)
{
    return v2f(v1.x - s, v1.y - s);
}

V3 operator-(const V3& v1, f32 s)
{
    return v3f(v1.x - s, v1.y - s, v1.z - s);
}

V4 operator-(const V4& v1, f32 s)
{
    return v4f(v1.x - s, v1.y - s, v1.z - s, v1.w - s);
}

V2 operator-(const V2& v1)
{
    return v2f(-v1.x, -v1.y);
}

V3 operator-(const V3& v1)
{
    return v3f(-v1.x, -v1.y, -v1.z);
}

V4 operator-(const V4& v1)
{
    return v4f(-v1.x, -v1.y, -v1.z, -v1.w);
}

V2 operator*(const V2& v1, const V2& v2)
{
    return v2f(v1.x * v2.x, v1.y * v2.y);
}

V3 operator*(const V3& v1, const V3& v2)
{
    return v3f(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}

V4 operator*(const V4& v1, const V4& v2)
{
    return v4f(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w);
}

V2& operator+=(V2& v1, const V2& v2)
{
    v1 = v1 + v2;
    return v1;
}

V3& operator+=(V3& v1, const V3& v2)
{
    v1 = v1 + v2;
    return v1;
}

V4& operator+=(V4& v1, const V4& v2)
{
    v1 = v1 + v2;
    return v1;
}

V2& operator+=(V2& v1, f32 s)
{
    v1 = v1 + s;
    return v1;
}

V3& operator+=(V3& v1, f32 s)
{
    v1 = v1 + s;
    return v1;
}

V4& operator+=(V4& v1, f32 s)
{
    v1 = v1 + s;
    return v1;
}

V2& operator-=(V2& v1, const V2& v2)
{
    v1 = v1 - v2;
    return v1;
}

V3& operator-=(V3& v1, const V3& v2)
{
    v1 = v1 - v2;
    return v1;
}

V4& operator-=(V4& v1, const V4& v2)
{
    v1 = v1 - v2;
    return v1;
}

V2& operator-=(V2& v1, f32 s)
{
    v1 = v1 - s;
    return v1;
}

V3& operator-=(V3& v1, f32 s)
{
    v1 = v1 - s;
    return v1;
}

V4& operator-=(V4& v1, f32 s)
{
    v1 = v1 - s;
    return v1;
}

V2& operator*=(V2& v1, const V2& v2)
{
    v1 = v1 * v2;
    return v1;
}
V3& operator*=(V3& v1, const V3& v2)
{
    v1 = v1 * v2;
    return v1;
}

V4& operator*=(V4& v1, const V4& v2)
{
    v1 = v1 * v2;
    return v1;
}

V2 operator*(const V2& v1, f32 s)
{
    return { v1.x * s, v1.y * s };
}

V3 operator*(const V3& v1, f32 s)
{
    return { v1.x * s, v1.y * s, v1.z * s };
}

V4 operator*(const V4& v1, f32 s)
{
    return { v1.x * s, v1.y * s, v1.z * s, v1.w * s };
}

V2 operator*(f32 s, const V2& v1)
{
    return { v1.x * s, v1.y * s };
}

V3 operator*(f32 s, const V3& v1)
{
    return { v1.x * s, v1.y * s, v1.z * s };
}

V4 operator*(f32 s, const V4& v1)
{
    return { v1.x * s, v1.y * s, v1.z * s, v1.w * s };
}

V2& operator*=(V2& v1, f32 s)
{
    v1 = v1 * s;
    return v1;
}

V3& operator*=(V3& v1, f32 s)
{
    v1 = v1 * s;
    return v1;
}

V4& operator*=(V4& v1, f32 s)
{
    v1 = v1 * s;
    return v1;
}

V2 operator/(const V2& v1, f32 s)
{
    return { v1.x / s, v1.y / s };
}

V3 operator/(const V3& v1, f32 s)
{
    return { v1.x / s, v1.y / s, v1.z / s };
}
V4 operator/(const V4& v1, f32 s)
{
    return { v1.x / s, v1.y / s, v1.z / s, v1.w / s };
}

b8 operator==(const V2& v1, const V2& v2)
{
    return v1.x == v2.x && v1.y == v2.y;
}

b8 operator==(const V3& v1, const V3& v2)
{
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

b8 operator==(const V4& v1, const V4& v2)
{
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w;
}

b8 operator<(const V2& v1, const V2& v2)
{
    return (v2_sum(v1) < v2_sum(v2));
}

b8 operator<(const V3& v1, const V3& v2)
{
    return (v3_sum(v1) < v3_sum(v2));
}

b8 operator<(const V4& v1, const V4& v2)
{
    return (v4_sum(v1) < v4_sum(v2));
}

b8 operator>(const V2& v1, const V2& v2)
{
    return (v2_sum(v1) > v2_sum(v2));
}

b8 operator>(const V3& v1, const V3& v2)
{
    return (v3_sum(v1) > v3_sum(v2));
}

b8 operator>(const V4& v1, const V4& v2)
{
    return (v4_sum(v1) > v4_sum(v2));
}
*/

P2 p2d(void)
{
    P2 res = { 0 };
    return res;
}

P2 p2i(f32 i)
{
    P2 res = { i, i };
    return res;
}

P2 p2f(f32 x, f32 y)
{
    P2 res = { x, y };
    return res;
}

P2 p2_p3(P3 p3)
{
    return p2f(p3.x, p3.y);
}

P2 p2_p4(P4 p4)
{
    return p2f(p4.x, p4.y);
}

P3 p3d(void)
{
    P3 res = { 0 };
    return res;
}

P3 p3i(f32 i)
{
    P3 res = { i, i, i };
    return res;
}

P3 p3f(f32 x, f32 y, f32 z)
{
    P3 res = { x, y, z };
    return res;
}

P3 p3_p2(P2 p2)
{
    return p3f(p2.x, p2.y, 0.0f);
}

P3 v3_p2f(P2 p2, f32 z)
{
    return p3f(p2.x, p2.y, z);
}

P3 p3_p4(P4 p4)
{
    return p3f(p4.x, p4.y, p4.z);
}

P4 p4d(void)
{
    P4 res = { 0 };
    return res;
}

P4 p4i(f32 i)
{
    P4 res = { i, i, i, i };
    return res;
}

P4 p4f(f32 x, f32 y, f32 z, f32 w)
{
    P4 res = { x, y, z, w };
    return res;
}

P4 p4_p2(P2 p2)
{
    return p4f(p2.x, p2.y, 0.0f, 0.0f);
}

P4 p4_p2f(P2 p2, f32 z, f32 w)
{
    return p4f(p2.x, p2.y, z, w);
}

P4 p4_p3(P3 p3)
{
    return p4f(p3.x, p3.y, p3.z, 0.0f);
}

f32 p2_sum(P2 p)

{
    return (p.x + p.y);
}

f32 p3_sum(P3 p)
{
    return (p.x + p.y + p.z);
}

f32 p4_sum(P4 p)
{
    return (p.x + p.y + p.z + p.w);
}

P2 p2_add(P2 p1, P2 p2)
{
    return p2f(p1.x + p2.x, p1.y + p2.y);
}

P3 p3_add(P3 p1, P3 p2)
{
    return p3f(p1.x + p2.x, p1.y + p2.y, p1.z + p2.z);
}

P4 p4_add(P4 p1, P4 p2)
{
    return p4f(p1.x + p2.x, p1.y + p2.y, p1.z + p2.z, p1.w + p2.w);
}

V2 p2_sub(P2 p1, P2 p2)
{
    return v2f(p1.x - p2.x, p1.y - p2.y);
}

V3 p3_sub(P3 p1, P3 p2)
{
    return v3f(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z);
}

V4 p4_sub(P4 p1, P4 p2)
{
    return v4f(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z, p1.w - p2.w);
}

P2 p2_s_multi(P2 p1, f32 s)
{
    return p2f(p1.x * s, p1.y * s);
}

P3 p3_s_multi(P3 p1, f32 s)
{
    return p3f(p1.x * s, p1.y * s, p1.z * s);
}

P4 p4_s_multi(P4 p1, f32 s)
{
    return p4f(p1.x * s, p1.y * s, p1.z * s, p1.w * s);
}

M2 m2i(f32 i)
{
    M2 res = { 0 };
    res.data[0][0] = i;
    res.data[1][1] = i;
    return res;
}

M2 m2d(void)
{
    return m2i(1.0f);
}

M3 m3i(f32 i)
{
    M3 res = { 0 };
    res.data[0][0] = i;
    res.data[1][1] = i;
    res.data[2][2] = i;
    return res;
}

M3 m3d(void)
{
    return m3i(1.0f);
}

M3 m3f(f32 f0, f32 f1, f32 f2, f32 f3, f32 f4, f32 f5, f32 f6, f32 f7, f32 f8)
{
    M3 res;

    res.data[0][0] = f0;
    res.data[1][0] = f1;
    res.data[2][0] = f2;

    res.data[0][1] = f3;
    res.data[1][1] = f4;
    res.data[2][1] = f5;

    res.data[0][2] = f6;
    res.data[1][2] = f7;
    res.data[2][2] = f8;

    return res;
}

M3 m3_m4(M4 matrix)
{
    M3 res = { 0 };
    res.data[0][0] = matrix.data[0][0];
    res.data[0][1] = matrix.data[0][1];
    res.data[0][2] = matrix.data[0][2];

    res.data[1][0] = matrix.data[1][0];
    res.data[1][1] = matrix.data[1][1];
    res.data[1][2] = matrix.data[1][2];

    res.data[2][0] = matrix.data[2][0];
    res.data[2][1] = matrix.data[2][1];
    res.data[2][2] = matrix.data[2][2];

    return res;
}

M4 m4i(f32 i)
{
    M4 res = { 0 };
    res.data[0][0] = i;
    res.data[1][1] = i;
    res.data[2][2] = i;
    res.data[3][3] = i;
    return res;
}

M4 m4d(void)
{
    return m4i(1.0f);
}

M4 m4f(f32 f0, f32 f1, f32 f2, f32 f3, f32 f4, f32 f5, f32 f6, f32 f7, f32 f8,
       f32 f9, f32 f10, f32 f11, f32 f12, f32 f13, f32 f14, f32 f15)
{

    M4 res;
    res.data[0][0] = f0;
    res.data[0][1] = f4;
    res.data[0][2] = f8;
    res.data[0][3] = f12;

    res.data[1][0] = f1;
    res.data[1][1] = f5;
    res.data[1][2] = f9;
    res.data[1][3] = f13;

    res.data[2][0] = f2;
    res.data[2][1] = f6;
    res.data[2][2] = f10;
    res.data[2][3] = f14;

    res.data[3][0] = f3;
    res.data[3][1] = f7;
    res.data[3][2] = f11;
    res.data[3][3] = f15;
    return res;
}

M4 m4_v4(V4 c0, V4 c1, V4 c2, V4 c3)
{
    M4 res;
    res.data[0][0] = c0.x;
    res.data[0][1] = c0.y;
    res.data[0][2] = c0.z;
    res.data[0][3] = c0.w;

    res.data[1][0] = c1.x;
    res.data[1][1] = c1.y;
    res.data[1][2] = c1.z;
    res.data[1][3] = c1.w;

    res.data[2][0] = c2.x;
    res.data[2][1] = c2.y;
    res.data[2][2] = c2.z;
    res.data[2][3] = c2.w;

    res.data[3][0] = c3.x;
    res.data[3][1] = c3.y;
    res.data[3][2] = c3.z;
    res.data[3][3] = c3.w;
    return res;
}

f32 m2_sum(M2 m)
{
    f32 sum = 0.0f;

    for (i32 c = 0; c < 2; c++)
        for (i32 r = 0; r < 2; r++)
            sum += m.data[c][r];

    return sum;
}

f32 m3_sum(M3 m)
{
    f32 sum = 0.0f;

    for (i32 c = 0; c < 3; c++)
        for (i32 r = 0; r < 3; r++)
            sum += m.data[c][r];

    return sum;
}

f32 m4_sum(M4 m)
{
    f32 sum = 0.0f;

    for (i32 c = 0; c < 4; c++)
        for (i32 r = 0; r < 4; r++)
            sum += m.data[c][r];

    return sum;
}

M2 m2_add(M2 m1, M2 m2)
{
    m1.data[0][0] += m2.data[0][0];
    m1.data[0][1] += m2.data[0][1];

    m1.data[1][0] += m2.data[1][0];
    m1.data[1][1] += m2.data[1][1];

    return m1;
}

M3 m3_add(M3 m1, M3 m2)
{
    m1.data[0][0] += m2.data[0][0];
    m1.data[0][1] += m2.data[0][1];
    m1.data[0][2] += m2.data[0][2];

    m1.data[1][0] += m2.data[1][0];
    m1.data[1][1] += m2.data[1][1];
    m1.data[1][2] += m2.data[1][2];

    m1.data[2][0] += m2.data[2][0];
    m1.data[2][1] += m2.data[2][1];
    m1.data[2][2] += m2.data[2][2];

    return m1;
}

M4 m4_add(M4 m1, M4 m2)
{
    m1.data[0][0] += m2.data[0][0];
    m1.data[0][1] += m2.data[0][1];
    m1.data[0][2] += m2.data[0][2];
    m1.data[0][3] += m2.data[0][3];

    m1.data[1][0] += m2.data[1][0];
    m1.data[1][1] += m2.data[1][1];
    m1.data[1][2] += m2.data[1][2];
    m1.data[1][3] += m2.data[1][3];

    m1.data[2][0] += m2.data[2][0];
    m1.data[2][1] += m2.data[2][1];
    m1.data[2][2] += m2.data[2][2];
    m1.data[2][3] += m2.data[2][3];

    m1.data[3][0] += m2.data[3][0];
    m1.data[3][1] += m2.data[3][1];
    m1.data[3][2] += m2.data[3][2];
    m1.data[3][3] += m2.data[3][3];

    return m1;
}

M2 m2_sub(M2 m1, M2 m2)
{
    m1.data[0][0] -= m2.data[0][0];
    m1.data[0][1] -= m2.data[0][1];

    m1.data[1][0] -= m2.data[1][0];
    m1.data[1][1] -= m2.data[1][1];

    return m1;
}

M3 m3_sub(M3 m1, M3 m2)
{
    m1.data[0][0] -= m2.data[0][0];
    m1.data[0][1] -= m2.data[0][1];
    m1.data[0][2] -= m2.data[0][2];

    m1.data[1][0] -= m2.data[1][0];
    m1.data[1][1] -= m2.data[1][1];
    m1.data[1][2] -= m2.data[1][2];

    m1.data[2][0] -= m2.data[2][0];
    m1.data[2][1] -= m2.data[2][1];
    m1.data[2][2] -= m2.data[2][2];

    return m1;
}

M4 m4_sub(M4 m1, M4 m2)
{
    m1.data[0][0] -= m2.data[0][0];
    m1.data[0][1] -= m2.data[0][1];
    m1.data[0][2] -= m2.data[0][2];
    m1.data[0][3] -= m2.data[0][3];

    m1.data[1][0] -= m2.data[1][0];
    m1.data[1][1] -= m2.data[1][1];
    m1.data[1][2] -= m2.data[1][2];
    m1.data[1][3] -= m2.data[1][3];

    m1.data[2][0] -= m2.data[2][0];
    m1.data[2][1] -= m2.data[2][1];
    m1.data[2][2] -= m2.data[2][2];
    m1.data[2][3] -= m2.data[2][3];

    m1.data[3][0] -= m2.data[3][0];
    m1.data[3][1] -= m2.data[3][1];
    m1.data[3][2] -= m2.data[3][2];
    m1.data[3][3] -= m2.data[3][3];

    return m1;
}

M2 m2_s_multi(M2 m, f32 s)
{
    m.data[0][0] *= s;
    m.data[0][1] *= s;

    m.data[1][0] *= s;
    m.data[1][1] *= s;

    return m;
}

M3 m3_s_multi(M3 m, f32 s)
{
    m.data[0][0] *= s;
    m.data[0][1] *= s;
    m.data[0][2] *= s;

    m.data[1][0] *= s;
    m.data[1][1] *= s;
    m.data[1][2] *= s;

    m.data[2][0] *= s;
    m.data[2][1] *= s;
    m.data[2][2] *= s;

    return m;
}

M4 m4_s_multi(M4 m, f32 s)
{
    m.data[0][0] *= s;
    m.data[0][1] *= s;
    m.data[0][2] *= s;
    m.data[0][3] *= s;

    m.data[1][0] *= s;
    m.data[1][1] *= s;
    m.data[1][2] *= s;
    m.data[1][3] *= s;

    m.data[2][0] *= s;
    m.data[2][1] *= s;
    m.data[2][2] *= s;
    m.data[2][3] *= s;

    m.data[3][0] *= s;
    m.data[3][1] *= s;
    m.data[3][2] *= s;
    m.data[3][3] *= s;

    return m;
}

V2 m2_v2_multi(M2 m, V2 v)
{
    V2 out;
    out.x = (m.data[0][0] * v.x) + (m.data[1][0] * v.y);
    out.y = (m.data[0][1] * v.x) + (m.data[1][1] * v.y);
    return out;
}

V3 m3_v3_multi(M3 m, V3 v)
{
    V3 out;
    out.x = (m.data[0][0] * v.x) + (m.data[1][0] * v.y) + (m.data[2][0] * v.z);
    out.y = (m.data[0][1] * v.x) + (m.data[1][1] * v.y) + (m.data[2][1] * v.z);
    out.z = (m.data[0][2] * v.x) + (m.data[1][2] * v.y) + (m.data[2][2] * v.z);
    return out;
}

V3 m4_v3_multi(M4 m, V3 v)
{
    V3 out;
    out.x = (m.data[0][0] * v.x) + (m.data[1][0] * v.y) + (m.data[2][0] * v.z) +
            (m.data[3][0] * 1.0f);

    out.y = (m.data[0][1] * v.x) + (m.data[1][1] * v.y) + (m.data[2][1] * v.z) +
            (m.data[3][1] * 1.0f);

    out.z = (m.data[0][2] * v.x) + (m.data[1][2] * v.y) + (m.data[2][2] * v.z) +
            (m.data[3][2] * 1.0f);

    return out;
}

V4 m4_v4_multi(M4 m, V4 v)
{
    V4 out;
    out.x = (m.data[0][0] * v.x) + (m.data[1][0] * v.y) + (m.data[2][0] * v.z) +
            (m.data[3][0] * v.w);

    out.y = (m.data[0][1] * v.x) + (m.data[1][1] * v.y) + (m.data[2][1] * v.z) +
            (m.data[3][1] * v.w);

    out.z = (m.data[0][2] * v.x) + (m.data[1][2] * v.y) + (m.data[2][2] * v.z) +
            (m.data[3][2] * v.w);

    out.w = (m.data[0][3] * v.x) + (m.data[1][3] * v.y) + (m.data[2][3] * v.z) +
            (m.data[3][3] * v.w);
    return out;
}

M2 m2_multi(M2 m1, M2 m2)
{
    M2 out = { 0 };

    for (u32 col = 0; col < 2; col++)
        for (u32 row = 0; row < 2; row++)
            for (u32 i = 0; i < 2; i++)
                out.data[col][row] += m1.data[i][row] * m2.data[col][i];

    return out;
}

M3 m3_multi(M3 m1, M3 m2)
{
    M3 out = m3f(m1.data[0][0] * m2.data[0][0] + m1.data[1][0] * m2.data[0][1] +
                     m1.data[2][0] * m2.data[0][2],
                 m1.data[0][0] * m2.data[1][0] + m1.data[1][0] * m2.data[1][1] +
                     m1.data[2][0] * m2.data[1][2],
                 m1.data[0][0] * m2.data[2][0] + m1.data[1][0] * m2.data[2][1] +
                     m1.data[2][0] * m2.data[2][2],

                 m1.data[0][1] * m2.data[0][0] + m1.data[1][1] * m2.data[0][1] +
                     m1.data[2][1] * m2.data[0][2],
                 m1.data[0][1] * m2.data[1][0] + m1.data[1][1] * m2.data[1][1] +
                     m1.data[2][1] * m2.data[1][2],
                 m1.data[0][1] * m2.data[2][0] + m1.data[1][1] * m2.data[2][1] +
                     m1.data[2][1] * m2.data[2][2],

                 m1.data[0][2] * m2.data[0][0] + m1.data[1][2] * m2.data[0][1] +
                     m1.data[2][2] * m2.data[0][2],
                 m1.data[0][2] * m2.data[1][0] + m1.data[1][2] * m2.data[1][1] +
                     m1.data[2][2] * m2.data[1][2],
                 m1.data[0][2] * m2.data[2][0] + m1.data[1][2] * m2.data[2][1] +
                     m1.data[2][2] * m2.data[2][2]);

    return out;
}

M4 m4_multi(M4 m1, M4 m2)
{
    M4 out =
        m4f(m1.data[0][0] * m2.data[0][0] + m1.data[1][0] * m2.data[0][1] +
                m1.data[2][0] * m2.data[0][2] + m1.data[3][0] * m2.data[0][3],
            m1.data[0][0] * m2.data[1][0] + m1.data[1][0] * m2.data[1][1] +
                m1.data[2][0] * m2.data[1][2] + m1.data[3][0] * m2.data[1][3],
            m1.data[0][0] * m2.data[2][0] + m1.data[1][0] * m2.data[2][1] +
                m1.data[2][0] * m2.data[2][2] + m1.data[3][0] * m2.data[2][3],
            m1.data[0][0] * m2.data[3][0] + m1.data[1][0] * m2.data[3][1] +
                m1.data[2][0] * m2.data[3][2] + m1.data[3][0] * m2.data[3][3],

            m1.data[0][1] * m2.data[0][0] + m1.data[1][1] * m2.data[0][1] +
                m1.data[2][1] * m2.data[0][2] + m1.data[3][1] * m2.data[0][3],
            m1.data[0][1] * m2.data[1][0] + m1.data[1][1] * m2.data[1][1] +
                m1.data[2][1] * m2.data[1][2] + m1.data[3][1] * m2.data[1][3],
            m1.data[0][1] * m2.data[2][0] + m1.data[1][1] * m2.data[2][1] +
                m1.data[2][1] * m2.data[2][2] + m1.data[3][1] * m2.data[2][3],
            m1.data[0][1] * m2.data[3][0] + m1.data[1][1] * m2.data[3][1] +
                m1.data[2][1] * m2.data[3][2] + m1.data[3][1] * m2.data[3][3],

            m1.data[0][2] * m2.data[0][0] + m1.data[1][2] * m2.data[0][1] +
                m1.data[2][2] * m2.data[0][2] + m1.data[3][2] * m2.data[0][3],
            m1.data[0][2] * m2.data[1][0] + m1.data[1][2] * m2.data[1][1] +
                m1.data[2][2] * m2.data[1][2] + m1.data[3][2] * m2.data[1][3],
            m1.data[0][2] * m2.data[2][0] + m1.data[1][2] * m2.data[2][1] +
                m1.data[2][2] * m2.data[2][2] + m1.data[3][2] * m2.data[2][3],
            m1.data[0][2] * m2.data[3][0] + m1.data[1][2] * m2.data[3][1] +
                m1.data[2][2] * m2.data[3][2] + m1.data[3][2] * m2.data[3][3],

            m1.data[0][3] * m2.data[0][0] + m1.data[1][3] * m2.data[0][1] +
                m1.data[2][3] * m2.data[0][2] + m1.data[3][3] * m2.data[0][3],
            m1.data[0][3] * m2.data[1][0] + m1.data[1][3] * m2.data[1][1] +
                m1.data[2][3] * m2.data[1][2] + m1.data[3][3] * m2.data[1][3],
            m1.data[0][3] * m2.data[2][0] + m1.data[1][3] * m2.data[2][1] +
                m1.data[2][3] * m2.data[2][2] + m1.data[3][3] * m2.data[2][3],
            m1.data[0][3] * m2.data[3][0] + m1.data[1][3] * m2.data[3][1] +
                m1.data[2][3] * m2.data[3][2] + m1.data[3][3] * m2.data[3][3]);

    return out;
}

#if 0
// TODO: look at this function.
M4 m4_multi_intrin(M4 m1, M4 m2)
{
    __m128 _rows[4], _columns[4];
    for (u32 i = 0; i < 4; i++)
    {
        _rows[i] =
            _mm_set_ps(m1.data[0][i], m1.data[1][i], m1.data[2][i], m1.data[3][i]);
        _columns[i] =
            _mm_set_ps(m2.data[i][0], m2.data[i][1], m2.data[i][2], m2.data[i][3]);
    }
    M4 out;
    for (u32 i = 0; i < 4; i++)
    {
        for (u32 j = 0; j < 4; j++)
        {
            __m128 _res = _mm_mul_ps(_rows[i], _columns[j]);
            _res = _mm_hadd_ps(_res, _res);
            _res = _mm_hadd_ps(_res, _res);
            out.data[j][i] = ((f32*)&_res)[0];
        }
    }
    __m128 _column_value[16];
    for (u32 i = 0; i < 4; i++)
    {
        __m128 _column =
            _mm_set_ps(m1.data[i][3], m1.data[i][2], m1.data[i][1], m1.data[i][0]);
        const u32 offset = i * 4;
        for (u32 j = 0; j < 4; j++)
        {
            _column_value[offset + j] =
                _mm_mul_ps(_column, _mm_set1_ps(m2.data[j][i]));
        }
    }
    M4 out;
    for (u32 i = 0; i < 4; i++)
    {
        __m128 _res0 = _mm_add_ps(_column_value[i], _column_value[i + 4]);
        __m128 _res1 = _mm_add_ps(_column_value[i + 8], _column_value[i + 12]);
        __m128 _res2 = _mm_add_ps(_res0, _res1);
        for (u32 j = 0; j < 4; j++)
        {
            out.data[i][j] = ((f32*)&_res2)[j];
        }
    }
    return out;
}
#endif

// NOTE: I do not know why this happens but when I did it like this:
// M4 m4_s_div(M4 m, f32 s) The result was wrong
//{
//    m.data[0][0] /= s;
//    m.data[0][1] /= s;
//    m.data[0][2] /= s;
//    m.data[0][3] /= s;
//
//    m.data[1][0] /= s;
//    m.data[1][1] /= s;
//    m.data[1][2] /= s;
//    m.data[1][3] /= s;
//
//    m.data[2][0] /= s;
//    m.data[2][1] /= s;
//    m.data[2][2] /= s;
//    m.data[2][3] /= s;
//
//    m.data[3][0] /= s;
//    m.data[3][1] /= s;
//    m.data[3][2] /= s;
//    m.data[3][3] /= s;
//    return m;
//}
M4 m4_s_div(M4 m, f32 s)
{
    M4 out;
    out.data[0][0] = m.data[0][0] / s;
    out.data[0][1] = m.data[0][1] / s;
    out.data[0][2] = m.data[0][2] / s;
    out.data[0][3] = m.data[0][3] / s;

    out.data[1][0] = m.data[1][0] / s;
    out.data[1][1] = m.data[1][1] / s;
    out.data[1][2] = m.data[1][2] / s;
    out.data[1][3] = m.data[1][3] / s;

    out.data[2][0] = m.data[2][0] / s;
    out.data[2][1] = m.data[2][1] / s;
    out.data[2][2] = m.data[2][2] / s;
    out.data[2][3] = m.data[2][3] / s;

    out.data[3][0] = m.data[3][0] / s;
    out.data[3][1] = m.data[3][1] / s;
    out.data[3][2] = m.data[3][2] / s;
    out.data[3][3] = m.data[3][3] / s;
    return out;
}
b8 m2_equal(M2 m1, M2 m2)
{
    for (i32 c = 0; c < 2; c++)
        for (i32 r = 0; r < 2; r++)
            if (m1.data[c][r] != m2.data[c][r]) return false;

    return true;
}

b8 m3_equal(M3 m1, M3 m2)
{
    for (i32 c = 0; c < 3; c++)
        for (i32 r = 0; r < 3; r++)
            if (m1.data[c][r] != m2.data[c][r]) return false;

    return true;
}

b8 m4_equal(M4 m1, M4 m2)
{
    for (i32 c = 0; c < 4; c++)
        for (i32 r = 0; r < 4; r++)
            if (m1.data[c][r] != m2.data[c][r]) return false;

    return true;
}

b8 m2_less(M2 m1, M2 m2)
{
    return (m2_sum(m1) < m2_sum(m2));
}

b8 m3_less(M3 m1, M3 m2)
{
    return (m3_sum(m1) < m3_sum(m2));
}

b8 m4_less(M4 m1, M4 m2)
{
    return (m4_sum(m1) < m4_sum(m2));
}

b8 m2_more(M2 m1, M2 m2)
{
    return (m2_sum(m1) > m2_sum(m2));
}

b8 m3_more(M3 m1, M3 m2)
{
    return (m3_sum(m1) > m3_sum(m2));
}

b8 m4_more(M4 m1, M4 m2)
{
    return (m4_sum(m1) > m4_sum(m2));
}

b8 vertex_equal(const Vertex* f, const Vertex* s)
{
    return v3_equal(f->pos, s->pos) && v3_equal(f->normal, s->normal) &&
           v2_equal(f->tex_coords, s->tex_coords) &&
           v4_equal(f->color, s->color) && f->tex_index == f->tex_index;
}

b8 vp_equal(const VP* f, const VP* s)
{
    return m4_equal(f->view, s->view) && m4_equal(f->proj, s->proj);
}

Vertex vertex_create(V3 pos, V3 normal, V2 tex_coords, V4 color, f32 tex_index)
{
    Vertex result;
    result.pos = pos;
    result.normal = normal;
    result.tex_coords = tex_coords;
    result.color = color;
    result.tex_index = tex_index;
    return result;
}

/*
M2 operator+(const M2& m1, const M2& m2)
{
    M2 out = m2_add(m1, m2);
    return out;
}

M3 operator+(const M3& m1, const M3& m2)
{
    M3 out = m3_add(m1, m2);
    return out;
}

M4 operator+(const M4& m1, const M4& m2)
{
    M4 out = m4_add(m1, m2);
    return out;
}

M2 operator-(const M2& m1, const M2& m2)
{
    M2 out = m2_sub(m1, m2);
    return out;
}

M3 operator-(const M3& m1, const M3& m2)
{
    M3 out = m3_sub(m1, m2);
    return out;
}

M4 operator-(const M4& m1, const M4& m2)
{
    M4 out = m4_sub(m1, m2);
    return out;
}

V2 operator*(const M2& m, const V2& v)
{
    V2 out;
    out.x = (m.data[0][0] * v.x) + (m.data[1][0] * v.y);
    out.y = (m.data[0][1] * v.x) + (m.data[1][1] * v.y);
    return out;
}

V3 operator*(const M3& m, const V3& v)
{
    V3 out;
    out.x = (m.data[0][0] * v.x) + (m.data[1][0] * v.y) + (m.data[2][0] * v.z);
    out.y = (m.data[0][1] * v.x) + (m.data[1][1] * v.y) + (m.data[2][1] * v.z);
    out.z = (m.data[0][2] * v.x) + (m.data[1][2] * v.y) + (m.data[2][2] * v.z);
    return out;
}

V3 operator*(const M4& m, const V3& v)
{
    V3 out;
    out.x = (m.data[0][0] * v.x) + (m.data[1][0] * v.y) + (m.data[2][0] * v.z) +
            (m.data[3][0] * 1.0f);

    out.y = (m.data[0][1] * v.x) + (m.data[1][1] * v.y) + (m.data[2][1] * v.z) +
            (m.data[3][1] * 1.0f);

    out.z = (m.data[0][2] * v.x) + (m.data[1][2] * v.y) + (m.data[2][2] * v.z) +
            (m.data[3][2] * 1.0f);

    return out;
}

V4 operator*(const M4& m, const V4& v)
{
    V4 out;
    out.x = (m.data[0][0] * v.x) + (m.data[1][0] * v.y) + (m.data[2][0] * v.z) +
            (m.data[3][0] * v.w);

    out.y = (m.data[0][1] * v.x) + (m.data[1][1] * v.y) + (m.data[2][1] * v.z) +
            (m.data[3][1] * v.w);

    out.z = (m.data[0][2] * v.x) + (m.data[1][2] * v.y) + (m.data[2][2] * v.z) +
            (m.data[3][2] * v.w);

    out.w = (m.data[0][3] * v.x) + (m.data[1][3] * v.y) + (m.data[2][3] * v.z) +
            (m.data[3][3] * v.w);
    return out;
}

M2 operator*(const M2& m, f32 s)
{
    M2 out = m2_s_multi(m, s);
    return out;
}
M3 operator*(const M3& m, f32 s)
{
    M3 out = m3_s_multi(m, s);
    return out;
}
M4 operator*(const M4& m, f32 s)
{
    M4 out = m4_s_multi(m, s);
    return out;
}

M4 operator/(const M4& m, f32 s)
{
    M4 out = m;
    out.data[0][0] /= s;
    out.data[0][1] /= s;
    out.data[0][2] /= s;
    out.data[0][3] /= s;

    out.data[1][0] /= s;
    out.data[1][1] /= s;
    out.data[1][2] /= s;
    out.data[1][3] /= s;

    out.data[2][0] /= s;
    out.data[2][1] /= s;
    out.data[2][2] /= s;
    out.data[2][3] /= s;

    out.data[3][0] /= s;
    out.data[3][1] /= s;
    out.data[3][2] /= s;
    out.data[3][3] /= s;
    return out;
}

M2 operator*(const M2& m1, const M2& m2)
{
    M2 out = m2_multi(m1, m2);
    return out;
}

M3 operator*(const M3& m1, const M3& m2)
{
    M3 out = m3_multi(m1, m2);
    return out;
}

M4 operator*(const M4& m1, const M4& m2)
{
    M4 out = m4_multi(m1, m2);
    return out;
}

b8 operator==(const M2& m1, const M2& m2)
{
    for (i32 c = 0; c < 2; c++)
        for (i32 r = 0; r < 2; r++)
            if (m1.data[c][r] != m2.data[c][r]) return 0;

    return 1;
}

b8 operator==(const M3& m1, const M3& m2)
{
    for (i32 c = 0; c < 3; c++)
        for (i32 r = 0; r < 3; r++)
            if (m1.data[c][r] != m2.data[c][r]) return 0;

    return 1;
}

b8 operator==(const M4& m1, const M4& m2)
{
    for (i32 c = 0; c < 4; c++)
        for (i32 r = 0; r < 4; r++)
            if (m1.data[c][r] != m2.data[c][r]) return 0;

    return 1;
}

b8 operator<(const M2& m1, const M2& m2)
{
    return (m2_sum(m1) < m2_sum(m2));
}

b8 operator<(const M3& m1, const M3& m2)
{
    return (m3_sum(m1) < m3_sum(m2));
};

b8 operator<(const M4& m1, const M4& m2)
{
    return (m4_sum(m1) < m4_sum(m2));
};

b8 operator>(const M2& m1, const M2& m2)
{
    return (m2_sum(m1) > m2_sum(m2));
}

b8 operator>(const M3& m1, const M3& m2)
{
    return (m3_sum(m1) > m3_sum(m2));
}

b8 operator>(const M4& m1, const M4& m2)
{
    return (m4_sum(m1) > m4_sum(m2));
}
*/

Polygon2D poly2D(V2 pos, V2* p_arr, V2* n_arr, u32 n_sides)
{
    Polygon2D res = { 0 };
    res.pos = pos;
    res.points = p_arr;
    res.normals = n_arr;
    res.n_sides = n_sides;
    return res;
}

f32 abs_f32(f32 in)
{
    return in < 0.0f ? in * -1.0f : in;
}

f32 clampf32(f32 value, f32 min, f32 max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

f32 clampf32_low(f32 value, f32 min)
{
    if (value < min) return min;
    return value;
}

V4 clamp(V4 v1, V4 min, V4 max)
{
    return v4f(clampf32(v1.x, min.x, max.x), clampf32(v1.y, min.y, max.y),
               clampf32(v1.z, min.z, max.z), clampf32(v1.w, min.w, max.w));
}

f32 minf32(f32 f1, f32 f2)
{
    return (f1 < f2) ? f1 : f2;
}

f32 maxf32(f32 f1, f32 f2)
{
    return (f1 > f2) ? f1 : f2;
}

f32 v2_len(V2 v2)
{
    return sqrtf((v2.x * v2.x) + (v2.y * v2.y));
}

f32 v2_len_squared(V2 v2)
{
    return (v2.x * v2.x) + (v2.y * v2.y);
}

f32 v3_len_squared(V3 v3)
{
    return (v3.x * v3.x) + (v3.y * v3.y) + (v3.z * v3.z);
}

f32 v3_len(V3 v3)
{
    return sqrtf((v3.x * v3.x) + (v3.y * v3.y) + (v3.z * v3.z));
}

V3 v3_lerp(V3 v1, V3 v2, f32 t)
{
    return v3_add(v1, v3_s_multi(v3_sub(v2, v1), t));
}

f32 v2_dot(V2 v1, V2 v2)
{
    return (v1.x * v2.x) + (v1.y * v2.y);
}

f32 v3_dot(V3 v1, V3 v2)
{
    return ((v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z));
}

f32 v3_angle(V3 v1, V3 v2)
{
    f32 denominator =
        (1.0f / sqrtf(v3_dot(v1, v1))) * (1.0f / sqrtf(v3_dot(v2, v2)));
    return acosf(v3_dot(v1, v2) * denominator);
}

V2 v2_normalize(V2 v2)
{
    f32 inverse = 1 / sqrtf(v2_dot(v2, v2));
    V2 out = v2_s_multi(v2, inverse);
    return out;
}

V3 v3_normalize(V3 v3)
{
    f32 inverse = 1 / sqrtf(v3_dot(v3, v3));
    V3 out = v3_s_multi(v3, inverse);
    return out;
}

f32 v2_cross(V2 v1, V2 v2)
{
    f32 out = (v1.x * v2.y) - (v1.y * v2.x);
    return out;
}

V3 v3_cross(V3 v1, V3 v2)
{
    V3 out;

    out.x = (v1.y * v2.z) - (v1.z * v2.y);
    out.y = (v1.z * v2.x) - (v1.x * v2.z);
    out.z = (v1.x * v2.y) - (v1.y * v2.x);

    return out;
}

V3 v3_project(V3 v1, V3 v2)
{
    return v3_s_multi(v2, (v3_dot(v1, v2) / v3_dot(v2, v2)));
}

V3 v3_reject(V3 v1, V3 v2)
{
    return v3_sub(v1, v3_project(v1, v2));
}

f32 v2_distance(V2 v1, V2 v2)
{
    return v2_len(v2_sub(v1, v2));
}

f32 v3_distance_squared(V3 v1, V3 v2)
{
    return v3_len_squared(v3_sub(v1, v2));
}

f32 v3_distance(V3 v1, V3 v2)
{
    return v3_len(v3_sub(v1, v2));
}

f32 p3_distance(P3 p1, P3 p2)
{
    return v3_len(p3_sub(p1, p2));
}

f32 p3_distance_sqrt(P3 p1, P3 p2)
{
    return sqrtf(v3_len(p3_sub(p1, p2)));
}

P3 p3_lerp(P3 p1, P3 p2, f32 t)
{
    // TODO: should implement these functions
    V3 v3 = v3_s_multi(p3_sub(p2, p1), t);
    return p3_add(p1, *(P3*)&v3);
}

P3 p3_min(P3 p1, P3 p2)
{
    return p3f(minf32(p1.x, p2.x), minf32(p1.y, p2.y), minf32(p1.z, p2.z));
}
P3 p3_max(P3 p1, P3 p2)
{
    return p3f(maxf32(p1.x, p2.x), maxf32(p1.y, p2.y), maxf32(p1.z, p2.z));
}
P3 p3_abs(P3 p)
{
    return p3f(abs_f32(p.x), abs_f32(p.y), abs_f32(p.z));
}

f32 radians(f32 deg)
{
    return (f32)((deg * PI) / 180.0f);
}

f32 m3_determinant(M3 m3)
{
    f32 out;

    out = (m3.data[0][0] * m3.data[1][1] * m3.data[2][2]) +
          (m3.data[0][1] * m3.data[1][2] * m3.data[0][2]) +
          (m3.data[0][2] * m3.data[1][0] * m3.data[2][1]) -
          (m3.data[0][2] * m3.data[1][1] * m3.data[0][2]) -
          (m3.data[0][1] * m3.data[1][0] * m3.data[2][2]) -
          (m3.data[0][0] * m3.data[1][2] * m3.data[2][1]);

    return out;
}

f32 m4_determinant(M4 m4)
{
    f32 out = 0;

    return out;
}

M3 m3_transpose(M3 m3)
{
    M3 out;

    out.data[0][0] = m3.data[0][0];
    out.data[0][1] = m3.data[1][0];
    out.data[0][2] = m3.data[2][0];

    out.data[1][0] = m3.data[0][1];
    out.data[1][1] = m3.data[1][1];
    out.data[1][2] = m3.data[2][1];

    out.data[2][0] = m3.data[0][2];
    out.data[2][1] = m3.data[1][2];
    out.data[2][2] = m3.data[2][2];

    return out;
}

M4 m4_transpose(M4 m4)
{
    M4 out;

    out.data[0][0] = m4.data[0][0];
    out.data[0][1] = m4.data[1][0];
    out.data[0][2] = m4.data[2][0];
    out.data[0][3] = m4.data[3][0];

    out.data[1][0] = m4.data[0][1];
    out.data[1][1] = m4.data[1][1];
    out.data[1][2] = m4.data[2][1];
    out.data[1][3] = m4.data[3][1];

    out.data[2][0] = m4.data[0][2];
    out.data[2][1] = m4.data[1][2];
    out.data[2][2] = m4.data[2][2];
    out.data[2][3] = m4.data[3][2];

    out.data[3][0] = m4.data[0][3];
    out.data[3][1] = m4.data[1][3];
    out.data[3][2] = m4.data[2][3];
    out.data[3][3] = m4.data[3][3];

    return out;
}

M3 m3_rotate(M3 m3, f32 rad)
{
    M3 res;
    res.data[0][0] = cosf(rad);
    res.data[1][0] = -sinf(rad);
    res.data[2][0] = m3.data[2][0];
    res.data[0][1] = sinf(rad);
    res.data[1][1] = cosf(rad);
    res.data[2][1] = m3.data[2][1];
    res.data[0][2] = m3.data[0][2];
    res.data[1][2] = m3.data[1][2];
    res.data[2][2] = m3.data[2][2];
    return res;
}

M4 rotate_x(f32 rad)
{
    M4 res = m4i(1.0f);
    res.data[1][1] = cosf(rad);
    res.data[1][2] = sinf(rad);
    res.data[2][1] = -sinf(rad);
    res.data[2][2] = cosf(rad);
    return res;
}

M4 rotate_y(f32 rad)
{
    M4 res = m4i(1.0f);
    res.data[0][0] = cosf(rad);
    res.data[0][2] = -sinf(rad);
    res.data[2][0] = sinf(rad);
    res.data[2][2] = cosf(rad);
    return res;
}

M4 rotate_z(f32 rad)
{
    M4 res = m4i(1.0f);
    res.data[0][0] = cosf(rad);
    res.data[0][1] = sinf(rad);
    res.data[1][0] = -sinf(rad);
    res.data[1][1] = cosf(rad);
    return res;
}

V3 v3_rotate(V3 v3, f32 rad, V3 normal)
{
    f32 cos_ = cosf(rad);
    f32 sin_ = sinf(rad);

    return v3_add(
        v3_add(
            v3_s_multi(v3, cos_),
            v3_multi(v3_s_multi(v3_multi(v3, normal), (1.0f - cos_)), normal)),
        v3_s_multi(v3_cross(v3, normal), sin_));
}

M3 translate(V2 v)
{
    M3 out = m3i(1.0f);
    out.data[2][0] = v.x;
    out.data[2][1] = v.y;
    return out;
}

M4 m4_translate(V3 v3)
{
    M4 out = m4i(1.0f);
    out.data[3][0] = v3.x;
    out.data[3][1] = v3.y;
    out.data[3][2] = v3.z;
    return out;
}

M3 scale(V2 v)
{
    M3 out = m3i(1.0f);

    out.data[0][0] *= v.x;
    out.data[1][1] *= v.y;

    return out;
}

M4 m4_scale(V3 v)
{
    M4 out = m4i(1.0f);

    out.data[0][0] *= v.x;
    out.data[1][1] *= v.y;
    out.data[2][2] *= v.z;

    return out;
}

M4 m4_shear(V3 v, V2 hx, V2 hy, V2 hz)
{
    M4 out = m4i(1.0f);

    out.data[1][0] = hx.x;
    out.data[2][0] = hx.y;

    out.data[0][1] = hy.x;
    out.data[2][1] = hy.y;

    out.data[0][2] = hz.x;
    out.data[1][2] = hz.y;

    return out;
}

M4 ortho(f32 left, f32 right, f32 bottom, f32 top, f32 sy_near, f32 sy_far)
{
    M4 out = m4i(1.0f);

    out.data[0][0] = 2.0f / (right - left);
    out.data[1][1] = 2.0f / (top - bottom);
    out.data[2][2] = 2.0f / (sy_near - sy_far);

    out.data[3][0] = (left + right) / (left - right);
    out.data[3][1] = (bottom + top) / (bottom - top);
    out.data[3][2] = (sy_near + sy_far) / (sy_near - sy_far);

    return out;
}

M4 view(V3 eye, V3 center, V3 up)
{
    M4 out = m4i(1.0f);

    const V3 temp1 = v3_normalize(v3_sub(center, eye));
    const V3 temp2 = v3_normalize(v3_cross(temp1, up));
    const V3 temp3 = v3_cross(temp2, temp1);

    out.data[0][0] = temp2.x;
    out.data[0][1] = temp3.x;
    out.data[0][2] = -temp1.x;

    out.data[1][0] = temp2.y;
    out.data[1][1] = temp3.y;
    out.data[1][2] = -temp1.y;

    out.data[2][1] = temp3.z;
    out.data[2][0] = temp2.z;
    out.data[2][2] = -temp1.z;

    out.data[3][0] = -v3_dot(temp2, eye);
    out.data[3][1] = -v3_dot(temp3, eye);
    out.data[3][2] = v3_dot(temp1, eye);

    return out;
}

M4 perspective(f32 fov, f32 aspect, f32 sy_near, f32 sy_far)
{
    const f32 f = 1.0f / tanf((fov * 0.5f));
    const f32 X = f / aspect;
    const f32 Y = -f;
    const f32 Z1 = (sy_far + sy_near) / (sy_near - sy_far);
    const f32 Z2 = (2.0f * sy_far * sy_near) / (sy_near - sy_far);

    M4 out = m4f(X, 0, 0, 0, 0, Y, 0, 0, 0, 0, Z1, Z2, 0, 0, -1.0f, 0.0f);

    return out;
}

M4 inverse(M4 m)
{
    f32 sf00 = m.data[2][2] * m.data[3][3] - m.data[3][2] * m.data[2][3];
    f32 sf01 = m.data[2][1] * m.data[3][3] - m.data[3][1] * m.data[2][3];
    f32 sf02 = m.data[2][1] * m.data[3][2] - m.data[3][1] * m.data[2][2];
    f32 sf03 = m.data[2][0] * m.data[3][3] - m.data[3][0] * m.data[2][3];
    f32 sf04 = m.data[2][0] * m.data[3][2] - m.data[3][0] * m.data[2][2];
    f32 sf05 = m.data[2][0] * m.data[3][1] - m.data[3][0] * m.data[2][1];
    f32 sf06 = m.data[1][2] * m.data[3][3] - m.data[3][2] * m.data[1][3];
    f32 sf07 = m.data[1][1] * m.data[3][3] - m.data[3][1] * m.data[1][3];
    f32 sf08 = m.data[1][1] * m.data[3][2] - m.data[3][1] * m.data[1][2];
    f32 sf09 = m.data[1][0] * m.data[3][3] - m.data[3][0] * m.data[1][3];
    f32 sf10 = m.data[1][0] * m.data[3][2] - m.data[3][0] * m.data[1][2];
    f32 sf11 = m.data[1][0] * m.data[3][1] - m.data[3][0] * m.data[1][1];
    f32 sf12 = m.data[1][2] * m.data[2][3] - m.data[2][2] * m.data[1][3];
    f32 sf13 = m.data[1][1] * m.data[2][3] - m.data[2][1] * m.data[1][3];
    f32 sf14 = m.data[1][1] * m.data[2][2] - m.data[2][1] * m.data[1][2];
    f32 sf15 = m.data[1][0] * m.data[2][3] - m.data[2][0] * m.data[1][3];
    f32 sf16 = m.data[1][0] * m.data[2][2] - m.data[2][0] * m.data[1][2];
    f32 sf17 = m.data[1][0] * m.data[2][1] - m.data[2][0] * m.data[1][1];

    M4 res = { 0 };
    res.data[0][0] =
        +(m.data[1][1] * sf00 - m.data[1][2] * sf01 + m.data[1][3] * sf02);
    res.data[0][1] =
        -(m.data[1][0] * sf00 - m.data[1][2] * sf03 + m.data[1][3] * sf04);
    res.data[0][2] =
        +(m.data[1][0] * sf01 - m.data[1][1] * sf03 + m.data[1][3] * sf05);
    res.data[0][3] =
        -(m.data[1][0] * sf02 - m.data[1][1] * sf04 + m.data[1][2] * sf05);

    res.data[1][0] =
        -(m.data[0][1] * sf00 - m.data[0][2] * sf01 + m.data[0][3] * sf02);
    res.data[1][1] =
        +(m.data[0][0] * sf00 - m.data[0][2] * sf03 + m.data[0][3] * sf04);
    res.data[1][2] =
        -(m.data[0][0] * sf01 - m.data[0][1] * sf03 + m.data[0][3] * sf05);
    res.data[1][3] =
        +(m.data[0][0] * sf02 - m.data[0][1] * sf04 + m.data[0][2] * sf05);

    res.data[2][0] =
        +(m.data[0][1] * sf06 - m.data[0][2] * sf07 + m.data[0][3] * sf08);
    res.data[2][1] =
        -(m.data[0][0] * sf06 - m.data[0][2] * sf09 + m.data[0][3] * sf10);
    res.data[2][2] =
        +(m.data[0][0] * sf07 - m.data[0][1] * sf09 + m.data[0][3] * sf11);
    res.data[2][3] =
        -(m.data[0][0] * sf08 - m.data[0][1] * sf10 + m.data[0][2] * sf11);

    res.data[3][0] =
        -(m.data[0][1] * sf12 - m.data[0][2] * sf13 + m.data[0][3] * sf14);
    res.data[3][1] =
        +(m.data[0][0] * sf12 - m.data[0][2] * sf15 + m.data[0][3] * sf16);
    res.data[3][2] =
        -(m.data[0][0] * sf13 - m.data[0][1] * sf15 + m.data[0][3] * sf17);
    res.data[3][3] =
        +(m.data[0][0] * sf14 - m.data[0][1] * sf16 + m.data[0][2] * sf17);

    f32 d = +m.data[0][0] * res.data[0][0] + m.data[0][1] * res.data[0][1] +
            m.data[0][2] * res.data[0][2] + m.data[0][3] * res.data[0][3];

    res = m4_s_div(res, d);

    // Column major
    return m4_transpose(res);
}

#if 0
b8 is_quad2d_convex(Quad2D q)
{
    P2* A = &q.points[0];
    P2* B = &q.points[1];
    P2* C = &q.points[2];
    P2* D = &q.points[3];

    // Source Christer Ericson Real Time Collision Detection
    V3 v0 = v3_cross(v3_v2(p2_sub(*D, *B)), v3_v2(p2_sub(*A, *B)));
    V3 v1 = v3_cross(v3_v2(p2_sub(*D, *B)), v3_v2(p2_sub(*C, *B)));

    if (v3_dot(v0, v1) >= 0.0f) return false;

    V3 v2 = v3_cross(v3_v2(p2_sub(*C, *A)), v3_v2(p2_sub(*D, *A)));
    V3 v3 = v3_cross(v3_v2(p2_sub(*C, *A)), v3_v2(p2_sub(*B, *A)));

    b8 res = v3_dot(v2, v3) < 0.0f;

    return res;
#if 0
    float cp0 = v2_cross(p2_sub(*B, *A), p2_sub(*C, *B));
    float cp1 = v2_cross(p2_sub(*C, *B), p2_sub(*D, *C));
    float cp2 = v2_cross(p2_sub(*D, *C), p2_sub(*A, *D));
    float cp3 = v2_cross(p2_sub(*A, *D), p2_sub(*B, *A));

    b8 res = cp0 * cp1 > 0 && cp1 * cp2 > 0 && cp2 * cp3 > 0;

    return res;
#endif
}
#endif

b8 is_poly2d_convex(Polygon2D p)
{
    if (p.n_sides < 3) return false;

    b32 positive_or_neg = true;
    for (u32 i = 0; i < p.n_sides; i++)
    {
        u32 j = (i + 1) % p.n_sides;
        u32 k = (j + 1) % p.n_sides;
        V2* f0 = &p.points[i];
        V2* f1 = &p.points[j];
        V2* f2 = &p.points[k];
        float cp0 = v2_cross(v2_sub(*f1, *f0), v2_sub(*f2, *f1));
        if (i == 0 && cp0 < 0)
        {
            positive_or_neg = false;
        }
        else if ((cp0 < 0 && positive_or_neg) || (cp0 >= 0 && !positive_or_neg))
        {
            return false;
        }
    }
    return true;
}

Plane plane(P3 a, P3 b, P3 c)
{
    Plane res = { 0 };
    res.n = v3_normalize(v3_cross(p3_sub(b, a), p3_sub(c, a)));
    res.d = v3_dot(res.n, v3f(a.x, a.y, a.z));
    return res;
}

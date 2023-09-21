#ifndef SY_INCLUDES // only for clangd
#include "syntics.h"
#endif

//#define GAME_GRASS
//
// #define GUI_MULTI_THREADED

#define LINES
// #define MOVE_ALL
#define MAX_PARTICLES 4800

#define GRASS_WIDTH 500
#define GRASS_DEPTH 500
#define MAX_GRASS GRASS_WIDTH* GRASS_DEPTH
#define GRASS_RADIUS 0.2f

#define MAX_GRASS_THREADS 4

#define CHUNK_SIZE_X 148
#define CHUNK_SIZE_Y 1
#define CHUNK_SIZE_Z 148

#define CHUNK_SIZE CHUNK_SIZE_X* CHUNK_SIZE_Y* CHUNK_SIZE_Z

#define MAX_TERRAIN_THREADS 1
#define multithreaded

global const f32 QUAD_WIDTH = 0.5f;
global const f32 QUAD_DEPTH = 0.5f;
global const f32 OFFSET_INCREASE = 0.1f;

#define pack(d, v0, v1, v2)                                                    \
    do                                                                         \
    {                                                                          \
        assert(v0 < 2 && v1 < 0x1FFFFFFF && v2 < 4 && "pack to big values");   \
        (d) = ((u32)(v0) << 31) | ((u32)(v1) << 2) | ((u32)(v2)&0x3);          \
    } while (0)

#define unpack_side(d) ((d) >> 31)
#define unpack_curve(d) (((d) >> 2) & 0x1FFFFFFF)
#define unpack_point(d) ((d)&0x3)

#if 1
Entity_Animation_3D dude_animation()
{
    Entity_Animation_3D res = { .leg_rotation_speed = 200.0f,
                                .dude_rotation_speed = 20.0f,
                                .stop_animation_sec = 20.0f,
                                .reset = true };
    return res;
}
#endif

AABB_3D aabb_create()
{
    AABB_3D res;
    res.min = v3i(INFINITY);
    res.size = v3d();
    return res;
}

AABB_Representation aabb_rep_create(AABB_3D aabb)
{
    AABB_Representation res = { 0 };
    res.aabb = aabb;
    return res;
}

u32 hash_function(V3 key, u32 capacity)
{
    f32 result = 31.0f;
    result += result * (key.x * 227.0f);
    result += result * (key.y * 227.0f);
    result += result * (key.z * 227.0f);
    u32 index = ((u32)result) % capacity;
    return index;
}

Hash_Table_U32 hash_table_u32_create(Region_Alloc* region, u32 capacity,
                                     u32 collision_buffer_capacity)
{
    Hash_Table_U32 out = { 0 };
    out.capacity = capacity;
    out.collision_buffer_capacity = collision_buffer_capacity;
    if (region)
    {
        out.values = region_calloc(region, capacity, Node_U32);
        out.collision_buffer =
            region_calloc(region, collision_buffer_capacity, Node_U32);
    }
    else
    {
        out.values = (Node_U32*)calloc(capacity, sizeof(Node_U32));
        out.collision_buffer =
            (Node_U32*)calloc(collision_buffer_capacity, sizeof(Node_U32));
    }
    return out;
}

Node_U32* next_node_u32(Hash_Table_U32* table)
{
    assert(table->collision_buffer_size < table->collision_buffer_capacity);
    return table->collision_buffer + table->collision_buffer_size++;
}

void insert_value_u32(Hash_Table_U32* table, V3 key, u32 value)
{
    Node_U32* node = table->values + hash_function(key, table->capacity);
    if (node->active)
    {
        sy_print("Collision!\n");
        if (v3_equal(node->key, key))
        {
            if (node->value == value)
            {
                return;
            }
            goto add_node;
        }
        else
        {
            while (node->next)
            {
                node = node->next;
                if (node->active && v3_equal(node->key, key))
                {
                    if (node->value == value)
                    {
                        return;
                    }
                    goto add_node;
                }
            }
            node->next = next_node_u32(table);
            node = node->next;
        }
    }
    node->key = key;
    node->active = true;
add_node:
    node->value = value;
}

u32* get_value_u32(Hash_Table_U32* table, V3 key)
{
    Node_U32* node = table->values + hash_function(key, table->capacity);

    if (node->active)
    {
        if (v3_equal(node->key, key))
        {
            return &node->value;
        }
        while (node->next)
        {
            node = node->next;
            if (node->active && v3_equal(node->key, key))
            {
                return &node->value;
            }
        }
    }
    return NULL;
}

/////////
///

typedef struct AABB_3D_Static_Header
{
    u32 size;
    u32 capacity;
    AABB_3D aabb;
} AABB_3D_Static_Header;
typedef struct AABB_3D_Static
{
    AABB_3D*** region;
} AABB_3D_Static;

// EXPLANATION:
//      region array: points to the aabb that represent the region
//      area array: has the region aabb as a header and a bunch of smaller local
//                  aabbs pointers
//      local array: have the area aabb in index 0 and all the smaller aabbs in
//      the area after that.
//                  Memory does not get allocated in the init function consider
//                  all areas have different number of aabbs.
//
void aabb_area_init(Region_Alloc* region, u32 region_count, u32 area_count)
{
    AABB_3D_Static result = { 0 };
    result.region = region_array(region, region_count, AABB_3D**);
    for (u32 i = 0; i < region_count; i++)
    {

        const u32 head_size = sizeof(AABB_3D_Static_Header);
        assert(region->current_pos + head_size < region->capacity);

        region->current_pos += head_size;

        const u32 alignment_offset = alignment_offset_get(
            region->buffer + region->current_pos, _Alignof(AABB_3D*));

        const u32 size = (sizeof(AABB_3D*) * area_count);
        assert((size + alignment_offset) <
               (region->capacity - region->current_pos));

        region->current_pos += alignment_offset;

        AABB_3D_Static_Header* head_pos =
            (AABB_3D_Static_Header*)(region->buffer +
                                     (region->current_pos - head_size));

        *head_pos = (AABB_3D_Static_Header){ .capacity = area_count };
        head_pos++;

        memset(head_pos, 0, size);

        region->current_pos += size;
        result.region[i] = (AABB_3D**)region->buffer + region->current_pos;
    }
}

// NOTE: Not very efficient but usually is only done on small number of indices
// and only computed once
void bubble_sort_on_y(Vertex_Array* vertices, U32_Array* indices)
{
    const u32 indices_size = indices->size;
    const u32 vertices_size = vertices->size;
    for (u32 i = 0; i < vertices_size - 1; i++)
    {
        for (u32 j = 0; j < vertices_size - i - 1; j++)
        {
            Vertex* first = vertex_array_val_ptr(vertices, j);
            Vertex* second = first + 1;
            if (first->pos.y > second->pos.y)
            {
                for (u32 k = 0; k < indices_size; k++)
                {
                    u32* val = u32_array_val_ptr(indices, k);
                    if (*val == j)
                    {
                        *val = j + 1;
                    }
                    else if (*val == j + 1)
                    {
                        *val = j;
                    }
                }
                Vertex temp_vertex = *first;
                *first = *second;
                *second = temp_vertex;
            }
        }
    }
}

#define DEFAULT_TEXTURE_GAME 0
#define OBJ_TEXTURE_GAME 1

void aabb_vertices_update(Vertex_Array* vertices, u32 offset, AABB_3D aabb)
{
    const u32 vertices_per_aabb = 8;
    const u32 vertex_offset = vertices_per_aabb * offset;
    cube_not_center(vertices, vertex_offset, aabb.min, aabb.size, v4i(1.0f),
                    DEFAULT_TEXTURE_GAME);
}

void aabb_min_max_update(AABB_3D* aabb, M4 transform)
{
    V3 corners[8] = { aabb->min,
                      v3_add(aabb->min, v3f(aabb->size.x, 0.0f, 0.0f)),
                      v3_add(aabb->min, v3f(0.0f, aabb->size.y, 0.0f)),
                      v3_add(aabb->min, v3f(0.0f, 0.0f, aabb->size.z)),
                      v3_add(aabb->min, v3f(aabb->size.x, aabb->size.y, 0.0f)),
                      v3_add(aabb->min, v3f(aabb->size.x, 0.0f, aabb->size.z)),
                      v3_add(aabb->min, v3f(0.0f, aabb->size.y, aabb->size.z)),
                      v3_add(aabb->min, aabb->size) };
    const u32 corner_count = sy_SIZE(corners);

    V3 min = v3i(INFINITY);
    V3 max = v3i(-INFINITY);
    for (u32 i = 0; i < corner_count; i++)
    {
        corners[i] = m4_v3_multi(transform, corners[i]);
        min.x = minf32(min.x, corners[i].x);
        min.y = minf32(min.y, corners[i].y);
        min.z = minf32(min.z, corners[i].z);

        max.x = maxf32(max.x, corners[i].x);
        max.y = maxf32(max.y, corners[i].y);
        max.z = maxf32(max.z, corners[i].z);
    }
    aabb->min = min;
    aabb->size = v3_sub(max, min);
}

AABB_3D aabb_update(AABB_3D aabb, M4 transform, Vertex_Array* vertices,
                    u32 offset)
{
    aabb_min_max_update(&aabb, transform);
    aabb_vertices_update(vertices, offset, aabb);
    return aabb;
}

void aabb_check_min_max(AABB_3D* aabb, V3 pos, V3* current_max)
{
    if (pos.x < aabb->min.x)
    {
        aabb->min.x = pos.x;
    }
    if (pos.x > current_max->x)
    {
        current_max->x = pos.x;
    }
    if (pos.y < aabb->min.y)
    {
        aabb->min.y = pos.y;
    }
    if (pos.y > current_max->y)
    {
        current_max->y = pos.y;
    }
    if (pos.z < aabb->min.z)
    {
        aabb->min.z = pos.z;
    }
    if (pos.z > current_max->z)
    {
        current_max->z = pos.z;
    }
}

AABB_3D vertices_extract(const Obj_Load_Attrib* loader, f32 tex_index,
                         V3 pos_offset, Vertex_Array* vert_array,
                         U32_Array* index_array, b8 use_hash)
{
    stack_begin_scope(stack);
    AABB_3D res = aabb_create();
    V3 max = v3i(-INFINITY);

    const u32 size = array_size(loader->indices);

    // NOTE: Temp
    Hash_Table_U32 table;
    if (use_hash)
    {
        table = hash_table_u32_create(stack_get(), size, (u32)(size * 0.3f));
    }

    const u32 vert_size = array_size(loader->verts);
    const u32 tex_size = array_size(loader->tex_coords);

    u32 index_offset = vert_array->size;
    for (u32 i = 0; i < size; i++)
    {
        Vertex vertex = { 0 };

        const u32 current_vert_index = loader->indices[i].vertex_index;
        assert(current_vert_index < vert_size);
        vertex.pos = v3_add(loader->verts[current_vert_index], pos_offset);

        vertex.normal = loader->normals[loader->indices[i].normal_index];

        vertex.color = v4f(1.0f, 1.0f, 1.0f, 1.0f);

        if (tex_size)
        {
            const u32 current_tex_index = loader->indices[i].texture_index;
            assert(current_tex_index < tex_size);
            vertex.tex_coords.x = loader->tex_coords[current_tex_index].x;
            vertex.tex_coords.y =
                1.0f - loader->tex_coords[current_tex_index].y;
        }
        vertex.tex_index = tex_index;

        aabb_check_min_max(&res, vertex.pos, &max);

        u32 index = 0;
        if (use_hash)
        {
            // TODO: add other key values for textures and so on
            u32* index_ptr = get_value_u32(&table, vertex.pos);
            if (!index_ptr)
            {
                index = index_offset;
                insert_value_u32(&table, vertex.pos, index_offset++);
                vertex_array_push(vert_array, vertex);
            }
            else
            {
                index = *index_ptr;
            }
        }
        else
        {
            index = index_offset++;
            vertex_array_push(vert_array, vertex);
        }
        u32_array_push(index_array, index);
    }
    res.size = v3_sub(max, res.min);
    stack_end_scope(stack);
    return res;
}

#if 1

global f32 g_freq = 0.41f;
global f32 g_grain = 0.36f;
global f32 g_oct = 3.0f;
global f32 g_max_height = 8.0f;

global u32 game_index_offset = 0;

#if 1
V3 convert_to_noise_coords(V2 x_z)
{
    V3 out = v3f((x_z.x * OFFSET_INCREASE) / QUAD_WIDTH, 0.0f,
                 (x_z.y * OFFSET_INCREASE) / QUAD_DEPTH);

    out.y = (sy_value_noise2d(out.x, out.z, g_freq, g_grain, (i32)g_oct) *
             g_max_height);

    return out;
}
#endif

f32 noise_min_max(f32 x_offset, f32 z_offset, f32 freq, f32 grain, i32 oct,
                  f32 min, f32 max)
{
    f32 noise =
        (sy_value_noise2d(x_offset, z_offset, freq, grain, oct) * (max - min)) +
        min;
    return noise;
}

f32 round_down_to_half(f32 value)
{
    return value - fmodf(value, 0.5);
}

global Thread_Attrib_Terrain terrain_threads[MAX_TERRAIN_THREADS] = { 0 };

global Thread_Attrib_Grass grass_threads[MAX_GRASS_THREADS] = { 0 };

void terrain_generation(f32 x_off, f32 z_off, u32 z_chunk_offset, u32 z_chunks,
                        Vertex* verts)
{
    u32 z_index = 0;
    u32 iterations = (z_chunk_offset + z_chunks) < CHUNK_SIZE_Z
                         ? (z_chunk_offset + z_chunks)
                         : CHUNK_SIZE_Z;
    for (u32 z = z_chunk_offset; z < iterations; z++)
    {
        f32 ix_off = x_off;
        for (u32 x = 0; x < CHUNK_SIZE_X; x++)
        {
#if 0
            f32 y_noise = (sy_value_noise2d(ix_off, z_off, g_freq, g_grain, (i32)g_oct) *
                           g_max_height);
            y_noise = round_down_to_half(y_noise);
            for_range(y, CHUNK_SIZE_Y)
            {
                cube(&vert->data,
                     v3f(0.0f + (0.5f * x), y_noise + (0.5f * y), 0.0f + (0.5f * z)),
                     v3i(0.5f), v4i(1.0f), DEFAULT_TEXTURE_GAME);
            }
#else
            f32 y_noise =
                (sy_value_noise2d(ix_off, z_off, g_freq, g_grain, (i32)g_oct) *
                 g_max_height);

            V3 pos = v3f(x * QUAD_WIDTH, y_noise, z * QUAD_DEPTH);
            // f32 colorf = y_noise / g_max_height;
            V4 color = v4f(0.0f, sy_RGB(100.0f), 0.0f, 1.0f);
            f32 tex_index = DEFAULT_TEXTURE_GAME;

            Vertex vertex = vertex_create(pos, v3f(0.0f, 1.0f, 0.0f),
                                          v2f(0.0f, 0.0f), color, tex_index);

            verts[(z_index * CHUNK_SIZE_X) + x] = vertex;

#endif
            ix_off += OFFSET_INCREASE;
        }
        z_off += OFFSET_INCREASE;
        z_index++;
    }
}
#endif

#define chunks CHUNK_SIZE_Z / MAX_TERRAIN_THREADS

volatile u32 check_thread_count = 0;

void generate_terrain_threaded(void* data)
{
    Thread_Attrib_Terrain* attrib = (Thread_Attrib_Terrain*)data;
    u32 z_chunk_offset = attrib->index * chunks;
    f32 z_off = (f32)z_chunk_offset * 0.1f;
    terrain_generation(0.0f, z_off, z_chunk_offset, chunks, attrib->verts);
}

global V2 g_wind = { 0.0f, 35.0f };

global V2 g_wind_direction = { 0.8f, 0.0f };

global f32 grass_freq = 1.5f;
global f32 grass_grain = 1.0f;
global f32 grass_oct = 2.0f;

global f32 grass_wind_speed = 1.5f;

void grass_generation(u32 seed, const u32 offset, const u32 iterations,
                      const u32 vertices_count, const u32 indices_count,
                      const V2* positions, const Vertex* model_vertices,
                      const u32* model_indices, Vertex* vertices, u32* indices)
{
    const f32 min_scale = 2.0f;
    const f32 max_scale = 20.0f;
    const f32 max_y = model_vertices[vertices_count - 1].pos.y * max_scale;
    u32 count = offset;
    for (u32 i = 0; i < iterations; i++)
    {
        u32 idx_offset = count;
        for (u32 k = 0; k < indices_count; k++)
        {
            *indices = model_indices[k] + idx_offset;
            indices++;
        }

        V3 vertex_pos_offset = v3f(positions->x, 0.0f, positions->y);
        positions++;

        vertex_pos_offset.y = convert_to_noise_coords(
                                  v2f(vertex_pos_offset.x, vertex_pos_offset.z))
                                  .y;

#if 0
        __m128 _pos_xyz[3], _pos_offset_xyz[3], _fx, _fy, _fz, _res;
        for (u32 j = 0; j < 3; j++)
        {
            _pos_offset_xyz[j] = _mm_set1_ps(vertex_pos_offset.data[j]);
        }
#endif
        f32 freq = 1.5f;
        f32 grain = 1.0f;
        i32 oct = 4;
        f32 noise_value =
            noise_min_max(vertex_pos_offset.x, vertex_pos_offset.z, freq, grain,
                          oct, min_scale, max_scale);

        V3 gen_scale = v3f(noise_value * 1.5f, noise_value, noise_value * 1.5f);
        f32 random = random_f32s(seed++, 2.0f, 4.0f);
        M4 matrix = m4_scale(gen_scale);

#if 0
        for (u32 k = 0; k < vertices_count; k += 4)
        {
            Vertex vertex[4];
            for (u32 j = 0; j < 4; j++)
            {
                vertex[j] = model_vertices[k + j];
            }
            for (u32 j = 0; j < 3; j++)
            {
                _pos_xyz[j] =
                    _mm_set_ps(vertex[3].pos.data[j], vertex[2].pos.data[j],
                               vertex[1].pos.data[j], vertex[0].pos.data[j]);
            }
            _Alignas(16) f32 res_xyz[3][4];
            _Alignas(16) f32 res_xyz_offset[3][4];
            for (u32 j = 0; j < 3; j++)
            {
                // scale matrix * position;
                _fx = _mm_mul_ps(_mm_set1_ps(matrix.data[0][j]), _pos_xyz[0]);
                _fy = _mm_mul_ps(_mm_set1_ps(matrix.data[1][j]), _pos_xyz[1]);
                _fz = _mm_mul_ps(_mm_set1_ps(matrix.data[2][j]), _pos_xyz[2]);

                _res = _mm_add_ps(_fx, _fy);
                _res = _mm_add_ps(_res, _fz);

                _mm_store_ps(res_xyz[j], _res);

                // Get the offset vector by adding pos with the random offset and
                // then subtracting the result from the original position.
                // V3 offset_pos =
                //    v3_sub(v3_add(vertex[j].pos, vertex_pos_offset),
                //    vertex[j].pos);
                _res = _mm_sub_ps(_mm_add_ps(_pos_offset_xyz[j], _res), _res);
                _mm_store_ps(res_xyz_offset[j], _res);
            }
            for (u32 j = 0; j < 4; j++)
            {
                vertex[j].pos.x = res_xyz[0][j];
                vertex[j].pos.y = res_xyz[1][j];
                vertex[j].pos.z = res_xyz[2][j];
                f32 procent = vertex[j].pos.y / max_y;
                V3 lerped_color = v3_lerp(
                    v3f(0.0f, sy_RGB(100.0f), 0.0f),
                    v3f(sy_RGB(120.0f), sy_RGB(255.0f), sy_RGB(0.0f)), procent);

                vertex[j].color = v4_v3f(lerped_color, 1.0f);

                // NOTE TEMP: Sending offset to shader using
                // texture_coordinates and color alpha chanel.

                vertex[j].tex_coords.x = res_xyz_offset[0][j];
                vertex[j].tex_coords.y = res_xyz_offset[1][j];
                vertex[j].color.a = res_xyz_offset[2][j];

                vertex[j].tex_index = random;
                *vertices = vertex[j];
                vertices++;
                count++;
            }
        }
#else
        for (u32 k = 0; k < vertices_count; k++)
        {
            Vertex vertex = model_vertices[k];

            vertex.pos = m4_v3_multi(matrix, vertex.pos);

            f32 procent = vertex.pos.y / max_y;
            V3 lerped_color = v3_lerp(
                v3f(0.0f, sy_RGB(100.0f), 0.0f),
                v3f(sy_RGB(120.0f), sy_RGB(255.0f), sy_RGB(0.0f)), procent);

            vertex.color = v4_v3f(lerped_color, 1.0f);

            V3 offset_pos =
                v3_sub(v3_add(vertex.pos, vertex_pos_offset), vertex.pos);
            vertex.tex_coords.x = offset_pos.x;
            vertex.tex_coords.y = offset_pos.y;
            vertex.color.a = offset_pos.z;

            vertex.tex_index = random;
            *vertices = vertex;
            vertices++;
            count++;
        }
#endif
    }
}

#if 1
void grass_generation_threaded(void* data)
{
    Thread_Attrib_Grass* attrib = (Thread_Attrib_Grass*)data;

    grass_generation(attrib->seed, attrib->vertex_offset, attrib->grass_count,
                     attrib->model_vertices->size, attrib->model_indices->size,
                     attrib->positions, attrib->model_vertices->data,
                     attrib->model_indices->data, attrib->vertex_array,
                     attrib->indices_array);
}
#endif

void normal_generate(Vertex_Array* vert)
{
    u32 size = CHUNK_SIZE;
    for (u32 i = 0; i < size - CHUNK_SIZE_X - 1; i += 1)
    {
        V3 pos = vert->data[i].pos;
        V3 next_pos0 = vert->data[i + CHUNK_SIZE_X].pos;
        V3 next_pos1 = vert->data[i + 1].pos;
        V3 side0 = v3_sub(next_pos0, pos);
        V3 side1 = v3_sub(next_pos1, pos);
        V3 normal = v3_normalize(v3_cross(side0, side1));
        vert->data[i].normal = normal;
    }
}

global V3 g_light_pos = { { { 0.0, 1.0, 0.0 } } };

global u32 current_curve_count = 0;

void game_save_binary0(const Bezier_Spline_3D* spline, V3 camera_pos)
{
    stack_begin_scope(stack);

    u32 bezier_curves_size0 = spline->n_curves * sizeof(V3) * 4 * 2;
    u32 bezier_curves_size1 = spline->n_curves * sizeof(u32) * 4 * 2;
    u32 size = (sizeof(u32) * 2) + bezier_curves_size0 + bezier_curves_size1 +
               sizeof(V3) + sizeof(u32);

    u8* buffer = stack_calloc(size, u8);
    u8* current_pos = buffer;

    memcpy(current_pos, &spline->n_curves, sizeof(u32));
    current_pos += sizeof(u32);

    memcpy(current_pos, &spline->splitt, sizeof(u32));
    current_pos += sizeof(u32);

    ASSERT((current_pos - buffer) +
                   (bezier_curves_size0 + bezier_curves_size1) <=
               size,
           "");

    for (u32 i = 0; i < spline->n_curves; i++)
    {
        for (u32 j = 0; j < 4; j++)
        {
            memcpy(current_pos, &spline->bc[0][i].p[j], sizeof(V3));
            current_pos += sizeof(V3);
            memcpy(current_pos, &spline->bc[0][i].points_indices[j],
                   sizeof(u32));
            current_pos += sizeof(u32);
        }
    }
    for (u32 i = 0; i < spline->n_curves; i++)
    {
        for (u32 j = 0; j < 4; j++)
        {
            memcpy(current_pos, &spline->bc[1][i].p[j], sizeof(V3));
            current_pos += sizeof(V3);
            memcpy(current_pos, &spline->bc[1][i].points_indices[j],
                   sizeof(u32));
            current_pos += sizeof(u32);
        }
    }

    ASSERT((current_pos - buffer) + sizeof(V3) <= size, "");

    memcpy(current_pos, &camera_pos, sizeof(V3));
    current_pos += sizeof(V3);

    memcpy(current_pos, &current_curve_count, sizeof(u32));

    file_write_entire("saved_spline3_game.synt", (char*)buffer, size);

    stack_end_scope(stack);
}

void game_save_binary1(const Vertex_Array* vert_array,
                       const U32_Array* index_array,
                       const Bezier_Spline_3D* spline, V3 camera_pos)
{
    stack_begin_scope(stack);

    u32 vert_size = vert_array->size;
    u32 vert_size_bytes = vert_size * (u32)sizeof(Vertex);
    u32 index_size = index_array->size;
    u32 index_size_bytes = index_size * (u32)sizeof(u32);
    u32 brezier_curves_size = spline->n_curves * sizeof(Cubic_Bezier_Curve);
    u32 size = vert_size_bytes + index_size_bytes + (2 * sizeof(u32)) +
               sizeof(*spline) + (brezier_curves_size * 2) + sizeof(V3) +
               sizeof(u32);

    u8* buffer = stack_calloc(size, u8);
    u8* current_pos = buffer;

    *((u32*)current_pos) = vert_size;
    current_pos += sizeof(u32);

    ASSERT((current_pos - buffer) + vert_size_bytes < size, "");

    memcpy(current_pos, vert_array->data, vert_size_bytes);
    current_pos += vert_size_bytes;

    *((u32*)(current_pos)) = index_size;
    current_pos += sizeof(u32);

    ASSERT((current_pos - buffer) + index_size_bytes < size, "");

    memcpy(current_pos, index_array->data, index_size_bytes);
    current_pos += index_size_bytes;

    ASSERT((current_pos - buffer) + sizeof(*spline) < size, "");

    memcpy(current_pos, spline, sizeof(*spline));
    current_pos += sizeof(*spline);

    ASSERT((current_pos - buffer) + (brezier_curves_size * 2) <= size, "");

    memcpy(current_pos, spline->bc[0], brezier_curves_size);
    current_pos += brezier_curves_size;

    memcpy(current_pos, spline->bc[1], brezier_curves_size);
    current_pos += brezier_curves_size;

    ASSERT((current_pos - buffer) + sizeof(V3) <= size, "");

    memcpy(current_pos, &camera_pos, sizeof(V3));
    current_pos += sizeof(V3);

    memcpy(current_pos, &current_curve_count, sizeof(u32));

    file_write_entire("saved_spline_game.synt", (char*)buffer, size);

    stack_end_scope(stack);
}

void game_copy_buffer(void* data, VkCommandBuffer command_buffer,
                      u32 semaphore_idx)
{
    Frame_Data* frame = (Frame_Data*)data;
    assert(frame);

    data_buffer_copy(&frame->game_uniform_buffers[semaphore_idx],
                     &frame->game_cam_vp, sizeof(frame->game_cam_vp));

    VkBufferCopy buff_copy = { 0 };
    buff_copy.size = frame->game_particles_staging_buffer.size_bytes;
    buff_copy.dstOffset = frame->game_particles_staging_buffer.dst_offset;
    assert(buff_copy.dstOffset + buff_copy.size <=
           frame->game_vert_idx_buffer.vert.buffer.size_bytes);
    vkCmdCopyBuffer(command_buffer, frame->game_particles_staging_buffer.buffer,
                    frame->game_vert_idx_buffer.vert.buffer.buffer, 1,
                    &buff_copy);
}

global u32 circle_offset = 0;
global u32 circle_curr_size = 0;
global Push_Constant push;
void game_render(void* data, VkCommandBuffer command_buffer, u32 semaphore_idx)
{
    Frame_Data* frame = (Frame_Data*)data;
    // NOTE: REMEMBER TO COPY UNIFORM BUFFERS

    // NOTE: same for every draw call at the moment
    VkViewport view_port = { 0 };
    view_port.x = 0.0f;
    view_port.y = 0.0f;
    view_port.width = frame->dimensions.width;
    view_port.height = frame->dimensions.height;
    view_port.maxDepth = 1.0f;

    VkRect2D scissor_internal = { { (i32)view_port.x, (i32)view_port.y },
                                  { (u32)view_port.width,
                                    (u32)view_port.height } };
    vkCmdSetViewport(command_buffer, 0, 1, &view_port);
    vkCmdSetScissor(command_buffer, 0, 1, &scissor_internal);

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            frame->game_pipeline_layout, 0, 1,
                            &frame->game_descriptors->desc_sets[semaphore_idx],
                            0, NULL);

    vertex_index_buffer1_bind(command_buffer, &frame->game_vert_idx_buffer);

    /////// TRIANGLE STRIP ////////////////

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      frame->game_triangle_strip_pipeline);

    Push_Constant global_constant;
    global_constant.model = m4i(1.0f);
    global_constant.normal = m4i(1.0f);
    push_constant(command_buffer, frame->game_pipeline_layout, &global_constant,
                  sizeof(global_constant));
    // Terrain draw
    draw(command_buffer, frame->game_terrain_offsets.idx,
         frame->game_terrain_offsets.idx_size);

#if 0
    // Road draw
    push_constant(command_buffer, game->triangle_strip_pipeline.layout,
                  &game->road_model, sizeof(M4));
    vertex_index_buffer1_bind(command_buffer, &game->road_vert_idx);
    draw(command_buffer, 0, game->road_vert_idx.idx.curr_size);
#endif

    //////// TRIANGLE LIST ////////////////

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      frame->game_triangle_list_pipeline);

    push_constant(command_buffer, frame->game_pipeline_layout, &global_constant,
                  sizeof(global_constant));
#if 1
    // Tree draw
    draw(command_buffer, frame->game_tree_offsets.idx,
         frame->game_tree_offsets.idx_size);
#endif

    push_constant(command_buffer, frame->game_pipeline_layout,
                  &frame->game_arc_model, sizeof(M4));

    draw(command_buffer, frame->game_particles_offsets.idx,
         frame->game_particle_count);

    // Dude draw
#if 1

    const u32 cube_count = 3;
    const u32 cube_size_vertex = 8;
    const u32 cube_size_index = 36;
    const u32 model_count = array_size(frame->game_dude_models);
    for (u32 i = 0; i < model_count; i++)
    {
        for (u32 j = 0; j < cube_count; j++)
        {
            push_constant(command_buffer, frame->game_pipeline_layout,
                          &array_val2(frame->game_dude_models, i, j),
                          sizeof(M4));
            draw(command_buffer,
                 frame->game_dude_offsets.idx + (j * cube_size_index),
                 cube_size_index);
        }
    }

    const u32 sign_in_sight = array_size(frame->game_sign_constants);
    for (u32 j = 0; j < sign_in_sight; j++)
    {
        Push_Constant* current_constant =
            array_val_ptr(frame->game_sign_constants, j);
        push_constant(command_buffer, frame->game_pipeline_layout,
                      current_constant, sizeof(Push_Constant));

        draw(command_buffer, frame->game_sign_offsets.idx,
             frame->game_sign_offsets.idx_size);
    }

#endif

    // Grass draw
#ifdef GAME_GRASS
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      frame->game_grass_pipeline);

    push.model = m4i(1.0f);
    push.normal.data[0][0] = frame->game_offset_p_grass;
    push_constant(command_buffer, frame->game_pipeline_layout, &push,
                  sizeof(Push_Constant));
    draw(command_buffer, frame->game_grass_offsets.idx,
         frame->game_grass_offsets.idx_size);
#endif

    /////// LINE LIST ////////////////////////

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      frame->game_line_list_pipeline);

    M4 dd = m4i(1.0f);
    push_constant(command_buffer, frame->game_pipeline_layout, &dd, sizeof(M4));
    vertex_index_buffer1_bind(command_buffer, &frame->game_aabb_rep);
    draw(command_buffer, 0,
         frame->game_aabb_count * frame->game_aabb_indices_count);

#if 0

    // Spline draw
    push_constant(command_buffer, game->line_list_pipeline.layout, &game->road_model,
                  sizeof(M4));
    vertex_index_buffer1_bind(command_buffer, &game->road_line_vert_idx);
    draw(command_buffer, 0, circle_curr_size);
    draw(command_buffer, circle_offset,
         game->road_line_vert_idx.idx.curr_size - circle_offset);
#endif
}

void game_recreate(void* data, const Application_State* app_state)
{
    Game_State* game = (Game_State*)data;
    {
        Graphic_Pipeline_Attrib g_p_info =
            gp_default1(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
        graphic_pipline_recreate(
            app_state->device, game->pipeline_layout, &g_p_info,
            "Syntics/res/shaders/spv/game.vert.spv",
            "Syntics/res/shaders/spv/game.frag.spv", &app_state->swap_chain,
            &game->triangle_strip_pipeline);
    }
    {
        Graphic_Pipeline_Attrib g_p_info = gp_default2(
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT);
        graphic_pipline_recreate(
            app_state->device, game->pipeline_layout, &g_p_info,
            "Syntics/res/shaders/spv/game.vert.spv",
            "Syntics/res/shaders/spv/game.frag.spv", &app_state->swap_chain,
            &game->triangle_list_pipeline);
    }
    {
#if 1
        Graphic_Pipeline_Attrib g_p_info = gp_default2(
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT);
        graphic_pipline_recreate(app_state->device, game->pipeline_layout,
                                 &g_p_info,
                                 "Syntics/res/shaders/spv/game_grass.vert.spv",
                                 "Syntics/res/shaders/spv/game_grass.frag.spv",
                                 &app_state->swap_chain, &game->grass_pipeline);
#endif
    }
}

// Brezier_Spline spline = {};
Bezier_Spline_3D spline2 = { 0 };

void game_destroy(void* data, VkDevice device, u32 num_semaphores)
{
    Game_State* game = (Game_State*)data;
#if 0
    save_game_binary(game->line_g_pipeline.vert_buffer.data,
                     game->line_g_pipeline.idx_buffer.data, spline2);
#endif
#ifdef LINES

    buffer_destroy(device, game->road_line_vert_idx.vert.buffer);
    buffer_destroy(device, game->road_line_vert_idx.idx.buffer);

    buffer_destroy(device, game->aabb_rep.vert.buffer);
    buffer_destroy(device, game->aabb_rep.idx.buffer);
#endif
    buffer_destroy(device, game->road_vert_idx.vert.buffer);
    buffer_destroy(device, game->road_vert_idx.idx.buffer);

    for (u32 i = 0; i < array_size(game->textures); i++)
    {
        texture_destroy(device, game->textures[i]);
    }
}

Bezier_Spline spline_create(Region_Alloc* region, u32 n_curves)
{
    Bezier_Spline out;
    out.bc = region_array(region, n_curves, Cubic_Bezier_Curve);
    out.n_curves = n_curves;
    return out;
}

#define PROCENT_INCREASE 0.01f

u32 circle_create(Vertex_Array* vert_array, u32 offset, V3 pos, f32 radius)
{
    for (f32 i = 0; i < 360.0f; i += 36.0f)
    {
        V3 p = v3_add(pos,
                      (v3_s_multi(v3f(cosf(radians(i)), sinf(radians(i)), 0.0f),
                                  radius)));

        Vertex vertex =
            vertex_create(p, v3d(), v2d(), v4i(1.0f), DEFAULT_TEXTURE_GAME);

        vertex_array_val(vert_array, offset++) = vertex;
    }
    return offset;
}

u32 spline_circles_curve_create(Rect3D* rects, Vertex_Array* vert_array,
                                u32 offset, Bezier_Spline_3D* spline, u32 curve,
                                f32 radius)
{
    for (u32 k = 0; k < 2; k++)
    {
        for (u32 j = 0; j < 4; j++)
        {
            Rect3D rect = { 0 };
            rect.pos = spline->bc[k][curve].p[j];
            rect.size = v3i(radius);
            pack(rect.id, k, curve, j);
            u32 size = array_size(rects);
            b8 found = false;
            for (u32 i = 0; i < size; i++)
            {
                Rect3D* rect_ref = rects + i;
                if (rect_ref->id == rect.id)
                {
                    *rect_ref = rect;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                array_push(rects, rect);
            }

            spline->bc[k][curve].points_indices[j] = offset;
            offset = circle_create(vert_array, offset,
                                   spline->bc[k][curve].p[j], radius);
        }
    }
    return offset;
}

u32 spline_2d_circles_create(Rect3D* rects, Vertex_Array* vert_array,
                             u32 offset, Bezier_Spline* spline, f32 radius)
{
    for (u32 i = 0; i < spline->n_curves; i++)
    {
        for (u32 j = 0; j < 4; j++)
        {
            Rect3D rect = { 0 };
            rect.pos = spline->bc[i].p[j];
            rect.size = v3i(radius);
            // pack(rect.id, i, j);
            array_push(rects, rect);

            spline->bc[i].points_indices[j] = offset;
            offset =
                circle_create(vert_array, offset, spline->bc[i].p[j], radius);
        }
    }
    return offset;
}

u32 spline_3d_circles_create(Rect3D* rects, Vertex_Array* vert_array,
                             u32 offset, Bezier_Spline_3D* spline, f32 radius)
{
    for (u32 i = 0; i < spline->n_curves; i++)
    {
        offset = spline_circles_curve_create(rects, vert_array, offset, spline,
                                             i, radius);
    }
    return offset;
}

V3 brezier_curve_pos(const Cubic_Bezier_Curve* brezier_curve, f32 t)
{
    V3 p0 = v3_lerp(brezier_curve->p[0], brezier_curve->p[1], t);
    V3 p1 = v3_lerp(brezier_curve->p[1], brezier_curve->p[2], t);
    V3 p2 = v3_lerp(brezier_curve->p[2], brezier_curve->p[3], t);
    V3 p3 = v3_lerp(p0, p1, t);
    V3 p4 = v3_lerp(p1, p2, t);
    return v3_lerp(p3, p4, t);
}

u32 curve_generate(Cubic_Bezier_Curve brezier_curve, Vertex_Array* vert_array,
                   u32 offset)
{
    u32 count = offset;
    for (f32 i = 0.0; i <= 1.0f; i += PROCENT_INCREASE)
    {
        Vertex vertex =
            vertex_create(brezier_curve_pos(&brezier_curve, i), v3d(), v2d(),
                          v4i(1.0f), DEFAULT_TEXTURE_GAME);
        vertex_array_val(vert_array, count++) = vertex;
    }
    return count;
}

V3 generate_positions_curve(Bezier_Spline_3D* spline, V3 direction, V3 pos,
                            u32 side, u32 curve)
{
    direction = v3_normalize(direction);
    spline->bc[side][curve].p[0] = pos;
    pos.y += curve % 2 == 0 ? direction.y : direction.y * -1.0f;
    for (u32 j = 1; j < 3; j++)
    {
        pos.x += direction.x;
        pos.z += direction.z;
        spline->bc[side][curve].p[j] = pos;
    }
    pos.x += direction.x;
    pos.y += curve % 2 == 0 ? direction.y * -1.0f : direction.y;
    pos.z += direction.z;
    spline->bc[side][curve].p[3] = pos;

    return pos;
}

void generate_positions(Bezier_Spline_3D* spline, V3 pos)
{
    for (u32 i = 0; i < spline->n_curves; i++)
    {
        V3 last_pos = pos;
        for (u32 k = 0; k < 2; k++)
        {
            pos = generate_positions_curve(spline, v3f(0.5f, 1.0f, 0.0f),
                                           last_pos, k, i);
            last_pos.z += 4.0f;
        }
        pos.z -= 4.0f;
    }
}

u32 spline_generate(Bezier_Spline_3D* spline, Vertex_Array* vert_array,
                    u32 offset)
{
    b8 first = true;
    for (u32 i = 0; i < spline->n_curves; i++)
    {
        for (u32 j = 0; j < 2; j++)
        {
            spline->bc[j][i].vertex_offset = offset;
            if (first)
            {
                offset = curve_generate(spline->bc[j][i], vert_array, offset);
                spline->splitt = offset - (spline->n_curves * 8 * 10);
                spline->splitt *= 2;
                first = false;
            }
            else
            {
                offset = curve_generate(spline->bc[j][i], vert_array, offset);
            }
        }
    }
    return offset;
}

u32 normals_curve_generate(Vertex_Array* vert_array, u32 offset,
                           u32 spline_splitt, i32 half_splitt, u32 first_index,
                           u32 second_index)
{
    u32 count = offset;
    V3 next_pos[2] = { 0 };
    u32 iterations = spline_splitt / 2;
    for (u32 i = 0; i < iterations - 1; i++)
    {
        V3 pos = vertex_array_val(vert_array, count).pos;
        next_pos[first_index] =
            vertex_array_val(vert_array, count + half_splitt).pos;
        next_pos[second_index] = vertex_array_val(vert_array, count + 1).pos;
        V3 side0 = v3_sub(next_pos[0], pos);
        V3 side1 = v3_sub(next_pos[1], pos);
        V3 normal = v3_normalize(v3_cross(side0, side1));
        vertex_array_val(vert_array, count++).normal = normal;
    }
    V3 pos = vertex_array_val(vert_array, count).pos;
    next_pos[second_index] =
        vertex_array_val(vert_array, count + half_splitt).pos;
    next_pos[first_index] = vertex_array_val(vert_array, count - 1).pos;
    V3 side0 = v3_sub(next_pos[0], pos);
    V3 side1 = v3_sub(next_pos[1], pos);
    V3 normal = v3_normalize(v3_cross(side0, side1));
    vertex_array_val(vert_array, count++).normal = normal;
    return count;
}

void generate_spline_normals_2d(Vertex_Array* vert_array, u32 offset,
                                const Bezier_Spline* spline)
{
    i32 half_splitt = spline->splitt / 2;
    for (u32 i = 0; i < spline->n_curves; i++)
    {
        offset = normals_curve_generate(vert_array, offset, spline->splitt,
                                        half_splitt, 0, 1);
    }
}

void spline_3d_normals_generate(Vertex_Array* vert_array, u32 offset,
                                const Bezier_Spline_3D* spline)
{
    i32 half_splitt = spline->splitt / 2;
    for (u32 i = 0; i < spline->n_curves; i++)
    {
        offset = normals_curve_generate(vert_array, offset, spline->splitt,
                                        half_splitt, 0, 1);
        offset = normals_curve_generate(vert_array, offset, spline->splitt,
                                        -half_splitt, 1, 0);
    }
}

void generate_spline_curve(Bezier_Spline_3D* spline, u32 side, u32 curve,
                           Vertex_Array* vert_array_line,
                           Vertex_Array* vert_array_road)
{
    i32 half_splitt = spline->splitt / 2;
    // (spline->n_curves * 8 * 10) for the circle representation
    // u32 offset = (((u32)half_splitt) * side) + (curve * spline->splitt) +
    //            (spline->n_curves * 8 * 10);
    u32 offset = spline->bc[side][curve].vertex_offset;

    u32 n = curve_generate(spline->bc[side][curve], vert_array_line, offset);

    if (side == 0)
    {
        normals_curve_generate(vert_array_line, offset, spline->splitt,
                               half_splitt, 0, 1);
    }
    else
    {
        normals_curve_generate(vert_array_line, offset, spline->splitt,
                               -half_splitt, 1, 0);
    }

    u32 offset2 = ((spline->splitt / 2) * side) + (curve * spline->splitt);

    for (u32 i = offset; i < n; i++)
    {
        vertex_array_val(vert_array_road, offset2++) =
            vertex_array_val(vert_array_line, i);
    }
}

void spline_generate_at_curve(Bezier_Spline_3D* spline, u32 side, u32 curve,
                              u32 point, V3 pos, Vertex_Array* vert_array_line,
                              Vertex_Array* vert_array_road)
{
    spline->bc[side][curve].p[point] = pos;
    generate_spline_curve(spline, side, curve, vert_array_line,
                          vert_array_road);
}

void spline_generate_at_curve1(Bezier_Spline_3D* spline, u32 curve,
                               Vertex_Array* vert_array_line,
                               Vertex_Array* vert_array_road)
{
    for (u32 i = 0; i < 2; i++)
    {
        generate_spline_curve(spline, i, curve, vert_array_line,
                              vert_array_road);
    }
}

void generate_positions1(Bezier_Spline* spline, V3 pos)
{
    for (u32 i = 0; i < spline->n_curves; i++)
    {
        spline->bc[i].p[0] = pos;
        pos.y += i % 2 == 0 ? 1.0f : -1.0f;
        for (u32 j = 1; j < 3; j++)
        {
            pos.x += 0.5f;
            spline->bc[i].p[j] = pos;
        }
        pos.y += i % 2 == 0 ? -1.0f : 1.0f;
        pos.x += 0.5f;
        spline->bc[i].p[3] = pos;
    }
}

u32 generate_spline1(Bezier_Spline* spline, Vertex_Array* vert_array,
                     u32 offset)
{
    for (u32 i = 0; i < spline->n_curves; i++)
    {
        if (!i)
        {
            offset = curve_generate(spline->bc[i], vert_array, offset);
            spline->splitt = offset - (spline->n_curves * 4 * 10);
        }
        else
        {
            offset = curve_generate(spline->bc[i], vert_array, offset);
        }
    }
    return offset;
}

void generate_spline_at_curve2(Bezier_Spline* spline, u32 curve, u32 point,
                               V3 pos, Vertex_Array* vert_array_line)
{
    spline->bc[curve].p[point] = pos;

    // (spline->n_curves * 4 * 10) for the circle representation
    u32 offset = (curve * spline->splitt) + (spline->n_curves * 4 * 10);
    curve_generate(spline->bc[curve], vert_array_line, offset);
}

f32 get_procent(Bezier_Spline sp, f32 t)
{
    return t * sp.n_curves;
}

void generate_indices_terrain(U32_Array* index_array, u32 offset)
{
    int32 I = 0;
    int32 step_value = 1;
    for (u32 i = 0; i < CHUNK_SIZE_Z - 1; i++)
    {
        for (u32 j = 0; j < CHUNK_SIZE_X; j++)
        {
            u32_array_push(index_array, offset + (CHUNK_SIZE_X * i) + I);
            u32_array_push(index_array, offset + (CHUNK_SIZE_X * (i + 1)) + I);

            I += step_value;
        }
        step_value *= -1;
        I += step_value;
    }
}

global f32 translucentcy_GAME = 0.0f;
global b32 wire_frame_GAME = false;

global V3 scaling_value_GAME = { 1.0f, 1.0f, 1.0f };

global b8 reset_index_GAME = false;

global b8 show_particles_GAME = false;
global b8 emit_particle_GAME = false;

global b8 g_edit_mode_GAME = false;

global b8 grass_mode = true;

global u32 index_to_test = 0;
global u32 num_points = 0;
global u32 points_size = ((u32)(1.0f / PROCENT_INCREASE) + 1) * 2;

global f32 smoothness_GAME = 0.07f;
global f32 cam_y_GAME = 0.55f;
global f32 speed_multiplier_GAME = 6.0f;

global f32 distance_sign = 20.0f;

global b8 spline_collision = false;
global b8 pause_game = false;

void game_update_gui(Game_State* game, Gui_Context* gui_ctx, u32 fps, f32 dt,
                     V2 dimensions)
{
    gui_ctx->translucentcy = translucentcy_GAME;
    Ui_Window* win = window_begin(gui_ctx, array_val(game->win_handles, 0),
                                  "First thing", v2f(10.0f, 10.0f));
    {
        window_gridd_begin(win, 1, 1);
        {
            presist char buffer[300] = {0};
            window_text_input_add(win, buffer, NULL);
        }
        window_gridd_begin(win, 2, 1);
        {
            window_text_add(win, "Translucentcy_GAME: ");
            window_input_float_add_d(win, &translucentcy_GAME, 0.0f, 1.0f);
        }
        window_gridd_end(win);
        window_gridd_begin(win, 4, 1);
        {
            if (window_button_add(win, "OFF"))
            {
                translucentcy_GAME = 0.0f;
            }
            if (window_button_add(win, "Low"))
            {
                translucentcy_GAME = 0.2f;
            }
            if (window_button_add(win, "High"))
            {
                translucentcy_GAME = 0.8f;
            }
            if (window_button_add(win, "Fill"))
            {
                translucentcy_GAME = 1.0f;
            }
        }
        window_gridd_end(win);
        window_gridd_begin(win, 2, 4);
        {
            if (window_button_add(win, "Save spline"))
            {
                game_save_binary0(&spline2, game->road_pos);
                game_save_binary1(&game->road_line_vert_idx.vert.array,
                                  &game->road_line_vert_idx.idx.array, &spline2,
                                  game->road_pos);
                sy_print("Saved!\n");
            }
            if (window_button_add(win, "Reset index"))
            {
                reset_index_GAME = true;
            }

            if (window_button_add(win, "Show particles"))
            {
                show_particles_GAME = true;
            }

            if (window_button_add(win, "Emit particle"))
            {
                emit_particle_GAME = true;
            }
            if (window_button_add(win, "Edit mode"))
            {
                b_switch(g_edit_mode_GAME);
            }
            if (window_button_add(win, "Spline collision"))
            {
                b_switch(spline_collision);
            }
            if (window_button_add(win, "Pause game"))
            {
                b_switch(pause_game);
            }
        }
        window_gridd_end(win);
#if 0
        window_gridd_begin(win, 2, 1);
        {
            if (window_button_add(win, "Add curve"))
            {
                if (current_curve_count < spline2.n_curves)
                {
                    Vertex_Buffer* vert = &game->road_line_vert_idx.vert;
                    for (u32 i = 0; i < 2; i++)
                    {
                        Cubic_Bezier_Curve* curves = spline2.bc[i];
                        V3 pos = { 0 };
                        if (current_curve_count > 0)
                        {
                            pos = curves[current_curve_count - 1].p[3];
                        }
                        else
                        {
                            if (i == 1)
                            {
                                pos.z += 4.0f;
                            }
                        }
                        V3 direction = v3_sub(
                            pos, brezier_curve_pos(&curves[current_curve_count - 1],
                                                   1.0f - PROCENT_INCREASE));
                        generate_positions_curve(&spline2, direction, pos, i,
                                                 current_curve_count);

                        if (current_curve_count < spline2.n_curves - 1)
                        {
                            curves[current_curve_count + 1].p[0] =
                                curves[current_curve_count].p[3];
                        }
                    }

                    u32 offset = circle_curr_size / 2;
                    spline_circles_curve_create(&vert->array, offset, &spline2,
                                                current_curve_count, 0.08f);
                    spline_generate_at_curve1(&spline2, current_curve_count);

                    data_buffer_copy(&vert->buffer, vert->array.data,
                                     vert->buffer.size_bytes);
                    data_buffer_copy(
                        &game->road_vert_idx.vert.buffer,
                        game->road_vert_idx.vert.array.data,
                        game->road_vert_idx.vert.buffer.size_bytes);

                    game->road_line_vert_idx.idx.curr_size +=
                        (points_size * 2) - 4;
                    game->road_vert_idx.idx.curr_size += points_size;
                    circle_curr_size += 8 * (10 * 2);

                    current_curve_count++;
                }
            }
            if (window_button_add(win, "Remove curve"))
            {
                if (current_curve_count > 0)
                {
                    game->road_line_vert_idx.idx.curr_size -=
                        (points_size * 2) - 4;
                    game->road_vert_idx.idx.curr_size -= points_size;
                    circle_curr_size -= 8 * (10 * 2);
                    current_curve_count--;
                }
            }
        }
        window_gridd_end(win);
#endif

#if 0
        window_gridd_begin(win, 3, 1);
        {
            window_input_float_add(win, &scaling_value_GAME.x, -100.0f, 100.0f,
                                   3.0f);
            window_input_float_add(win, &scaling_value_GAME.y, -100.0f, 100.0f,
                                   3.0f);
            window_input_float_add(win, &scaling_value_GAME.z, -100.0f, 100.0f,
                                   3.0f);
        }
        window_gridd_end(win);
#endif

#if 0
        window_gridd_begin(win, 1, 1);
        {
            if (window_button_add(win, "Circle toggle"))
            {
                game_index_offset = game_index_offset == 0 ? index_to_test : 0;
            }
        }
        window_gridd_end(win);
#endif

        window_gridd_begin(win, 1, 1);
        {
            presist char temp[60] = { 0 };
            presist f32 count = 1.0f;
            if (count >= 0.1f)
            {
                f32 milli = dt * 1000.0f;
                sysprintf(temp, sizeof(temp), "Milli: %f | FPS: %u", milli,
                          fps);
                count = 0.0f;
            }
            count += dt;
            window_text_add(win, temp);
        }
        window_gridd_end(win);
        window_gridd_begin(win, 1, 2);
        {
            presist char temp[80] = "Cam pos: ";
            presist char temp1[80] = "Cam ori: ";
            presist f32 count = 1.0f;
            if (count >= 0.1f)
            {
                const u32 len = 9;
                sysprintf(temp + len, sizeof(temp) - len,
                          V3_FMT(game->cam.pos));

                sysprintf(temp1 + len, sizeof(temp1) - len,
                          V3_FMT(game->cam.ori));

                count = 0.0f;
            }
            count += dt;
            window_text_add(win, temp);
            window_text_add(win, temp1);
        }
        window_gridd_end(win);
        window_gridd_begin(win, 2, 1);
        {
            window_text_add(win, "Distance sign: ");
            window_input_float_add_d(win, &distance_sign, 0.0f, 70.0f);
        }
        window_gridd_end(win);

        window_gridd_begin(win, 2, 1);
        {
            window_text_add(win, "Wind speed: ");
            window_input_float_add(win, &grass_wind_speed, 0.0f, 10.0f, 1.0f);
        }
        window_gridd_end(win);

        window_gridd_begin(win, 2, 4);
        {
            window_text_add(win, "Camera Smoothness: ");
            window_input_float_add_d(win, &smoothness_GAME, 0.0f, 0.5f);
            window_text_add(win, "Camera height: ");
            window_input_float_add_d(win, &cam_y_GAME, 0.0f, 90.0f);
            window_text_add(win, "Speed Multiplier: ");
            window_input_float_add_d(win, &speed_multiplier_GAME, 0.0f, 20.0f);
        }
        window_gridd_end(win);

        const u32 float_gui_count = array_size(game->float_guis);
        const u32 width = 4;
        const u32 height = (u32)ceilf((f32)float_gui_count / 2.0f);
        if (height)
        {
            window_gridd_begin(win, width, height);
            {
                for (u32 i = 0; i < float_gui_count; i++)
                {
                    Float_Gui* current = game->float_guis + i;
                    window_text_add(win, current->name);
                    window_input_float_add_d(win, current->value, current->min,
                                             current->max);
                }
            }
            window_gridd_end(win);
            array_head(game->float_guis)->size = 0;
        }
    }
    window_end(&win);

    win = window_begin(gui_ctx, array_val(game->win_handles, 1), "Terminal",
                       v2f(500.0f, 100.0f));
    {
        terminal_add(gui_ctx, terminal_ptr_get(), win, 250.0f, 200.0f);
    }
    window_end(&win);
}

u32 cell_index_get(V2 pos, f32 cell_size, u32 columns)
{
    u32 cell_index =
        ((u32)(pos.y / cell_size) * columns) + (u32)(pos.x / cell_size);
    return cell_index;
}

// Inspiration from:
// Fast Poisson Disk Sampling in Arbitrary Dimensions
//              Robert Bridson
//      University of British Columbia
//
void blue_noise_2d(Region_Alloc* region, u32 seed, const u32 k, const u32 rows,
                   const u32 columns, const f32 minimum_distance,
                   V2_Array* positions)
{

    f64 start = platform_get_time();
    const f32 extent_of_sample_domain = 2.0f;
    const f32 cell_size = 1 / sqrtf(extent_of_sample_domain) * minimum_distance;
    const f32 max_z = cell_size * (f32)rows;
    const f32 max_x = cell_size * (f32)columns;
    const u32 max_count = rows * columns;

    if (!positions->data)
    {
        *positions = v2_array_create(region, max_count + 1);
    }
    U32_Array gridd_cells = u32_array_create(NULL, max_count);
    U32_Array active_indices = u32_array_create(NULL, max_count);

    V2 pos = v2_random(seed++, 0.0f, cell_size * 0.9f);

    u32 cell_index = cell_index_get(pos, cell_size, columns);
    assert(cell_index == 0);

    // first position is used as a empty spot
    positions->size++;

    u32 index = v2_array_push(positions, pos);
    u32_array_val(&gridd_cells, cell_index++) = index;
    u32_array_push(&active_indices, index);

    const i32 circle_index_table[] = {
        1,  1 + (i32)columns,  (i32)columns,  (i32)columns - 1,
        -1, -1 - (i32)columns, -(i32)columns, 1 - (i32)columns
    };
    const f32 minimum_distance_squared = minimum_distance * minimum_distance;

    u32 active_index = 1;
    while (active_indices.size)
    {
        active_index = *u32_array_back(&active_indices);
        assert(active_index < positions->size);
        pos = v2_array_val(positions, active_index);
        b32 found = false;
        for (u32 i = 0; i < k; i++)
        {
            const f32 random = random_f32s(seed++, 0.0f, 360.0f);
            const f32 x = (cosf(radians(random)) * minimum_distance) + pos.x;
            const f32 z = (sinf(radians(random)) * minimum_distance) + pos.y;
            if (x < 0.0f || x >= max_x || z < 0.0f || z >= max_z)
            {
                continue;
            }
            const V2 pos_around = v2f(x, z);
            const u32 cell_index_around =
                cell_index_get(pos_around, cell_size, columns);
            if (u32_array_val(&gridd_cells, cell_index_around))
            {
                continue;
            }
            b32 ok = true;
            for (u32 j = 0; j < sy_SIZE(circle_index_table); j++)
            {
                const u32 neighbor_index =
                    cell_index_around + circle_index_table[j];
                if (neighbor_index >= 0 && neighbor_index < max_count)
                {
                    u32 check_index =
                        u32_array_val(&gridd_cells, neighbor_index);
                    if (check_index)
                    {
                        V2 pos_dd = v2_array_val(positions, check_index);
                        V2 check_position = v2_sub(pos_dd, pos_around);
                        f32 len_squared = v2_len_squared(check_position);
                        if (len_squared < minimum_distance_squared)
                        {
                            ok = false;
                            break;
                        }
                    }
                }
            }
            if (ok)
            {
                index = positions->size++;
                v2_array_val(positions, index) = pos_around;
                u32_array_val(&gridd_cells, cell_index_around) = index;
                active_index = index;
                found = true;
                break;
            }
        }
        if (found)
        {
            u32_array_push(&active_indices, active_index);
        }
        else
        {
            u32_array_pop(&active_indices);
        }
    }
    f64 duration = platform_get_time() - start;

    free(gridd_cells.data);
    free(active_indices.data);

    v2_array_val(positions, 0) = v2_array_pop(positions);
    sy_print("Duration: %Lf\nBlue noise:\n     Max: %u\n     Found: %u\n",
             duration, max_count, positions->size);
}

V3 mouse_to_device_coords(V3 mouse, V2 dimensions)
{
    V2 center = v2_s_multi(dimensions, 0.5f);
    V3 result = v3d();
    result.x = (mouse.x - center.x) / center.x;
    result.y = -((center.y - mouse.y) / center.y);
    result.z = 1.0f;
    return result;
}

void game_init(Region_Alloc* region, VkDevice device,
               VkPhysicalDevice physical_device, VkCommandPool command_pool,
               VkQueue graphic_queue, const Swap_Chain_Attrib* swap_chain,
               const Platform* platform, Render_State* render_state,
               u32 num_semaphores, Game_State* game)
{
    stack_begin_scope(game_init_stack);

    game->rects = region_array_calloc(region, 1000, Rect3D);

    game->float_guis = region_array_calloc(region, 1000, Float_Gui);

    const char* paths[] = {
        [DEFAULT_TEXTURE_GAME] = "Syntics/res/default.png",
        [OBJ_TEXTURE_GAME] = "Syntics/res/Purisa.png",
    };
    u32 num_text = sy_SIZE(paths);
    game->textures = region_array(region, num_text, Texture);

    textures_path_create(device, physical_device, command_pool, graphic_queue,
                         false, num_text, paths, game->textures);

    array_head(game->textures)->size = num_text;

    descriptor_set_layout_create(device, num_text,
                                 &game->descriptor_set_layout);
    pipeline_layout_create(device, game->descriptor_set_layout,
                           &game->pipeline_layout);

    uniforms_descriptors_init(region, device, physical_device,
                              &game->uniform_buffers, &game->descriptors,
                              game->descriptor_set_layout, num_semaphores,
                              game->textures, num_text);

    { // Triangle strip
        Graphic_Pipeline_Attrib g_p_info =
            gp_default1(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
        graphics_pipeline_create_deluxe(
            device, game->pipeline_layout, &g_p_info,
            "Syntics/res/shaders/spv/game.vert.spv",
            "Syntics/res/shaders/spv/game.frag.spv", swap_chain,
            &game->triangle_strip_pipeline);
    }

    { // Triangle list
        Graphic_Pipeline_Attrib g_p_info = gp_default2(
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT);
        graphics_pipeline_create_deluxe(
            device, game->pipeline_layout, &g_p_info,
            "Syntics/res/shaders/spv/game.vert.spv",
            "Syntics/res/shaders/spv/game.frag.spv", swap_chain,
            &game->triangle_list_pipeline);
    }

    { // Line list
        Graphic_Pipeline_Attrib g_p_info =
            gp_default2(VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_CULL_MODE_BACK_BIT);
        g_p_info.line_width = 5.0f;
        graphics_pipeline_create_deluxe(device, game->pipeline_layout,
                                        &g_p_info,
                                        "Syntics/res/shaders/spv/gui.vert.spv",
                                        "Syntics/res/shaders/spv/gui.frag.spv",
                                        swap_chain, &game->line_list_pipeline);
    }

#if 1
    { // Grass
        Graphic_Pipeline_Attrib g_p_info = gp_default2(
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT);
        graphics_pipeline_create_deluxe(
            device, game->pipeline_layout, &g_p_info,
            "Syntics/res/shaders/spv/game_grass.vert.spv",
            "Syntics/res/shaders/spv/game_grass.frag.spv", swap_chain,
            &game->grass_pipeline);
    }
#endif

    Vertex_Array global_vert_array = vertex_array_create(NULL, 7000000);
    U32_Array global_idx_array = u32_array_create(NULL, 40000000);

    Semaphore_Counter counter = { 0 };
    { // Terrain generation

        Vertex_Array vert_array =
            vertex_array_ref_at_size_offset(&global_vert_array, CHUNK_SIZE);
        U32_Array idx_array =
            u32_array_ref_at_size_offset(&global_idx_array, CHUNK_SIZE * 2);

        vert_array.size = CHUNK_SIZE;

        Thread_Task tasks[MAX_TERRAIN_THREADS];
        u32 task_count = 0;
        for (u32 i = 1; i < MAX_TERRAIN_THREADS; i++)
        {
            u32 vert_size = (CHUNK_SIZE_Z / MAX_TERRAIN_THREADS) * CHUNK_SIZE_X;
            u32 offset = i * vert_size;
            Thread_Attrib_Terrain* th = terrain_threads + i;
            th->index = i;
            th->verts = vert_array.data + offset;
            tasks[task_count++] = thread_task(generate_terrain_threaded, th);
        }
        thread_tasks_push(tasks, task_count, &counter);

        terrain_generation(0.0f, 0.0f, 0, chunks, vert_array.data);

        semaphore_counter_wait(&counter);

        normal_generate(&vert_array);

        generate_indices_terrain(&idx_array, global_vert_array.size);

        game->terrain_offsets.idx = global_idx_array.size;
        game->terrain_offsets.idx_size = idx_array.size;

        global_vert_array.size += vert_array.size;
        global_idx_array.size += idx_array.size;
    }

    {
        u32 seed = (u32)time(NULL);

        const u32 vertices_count = 12 * 2 * 12 * 5 * 2 * 6;
        const u32 indices_count = ((12 * 2 + (12 * 5 * 2) - (1 + 5)) * 12 * 6);

        const u32 rows = 15;
        const u32 columns = 15;
        V2_Array positions = { 0 };
        blue_noise_2d(stack_get(), seed++, 20, rows, columns, 6.0f, &positions);

        const u32 pos_size = positions.size;

        Vertex_Array vert_array = vertex_array_ref_at_size_offset(
            &global_vert_array, vertices_count * pos_size);
        U32_Array idx_array = u32_array_ref_at_size_offset(
            &global_idx_array, indices_count * pos_size);

        for (u32 trees = 0; trees < pos_size; trees++)
        {
            stack_begin_scope(tree_gen_stack);

            const u32 min_segments = 8;
            const u32 max_segments = 12;
            const u32 segments =
                random_u32ss(seed++, min_segments, max_segments);

            const f32 height_procent = (f32)(segments - min_segments) /
                                       (f32)(max_segments - min_segments);
            const u32 branch_count = (u32)sy_lerp(3.1f, 5.5f, height_procent);

            const u32 branch0_segments = 12;
            const u32 vertices_per_segment = 6;

            const f32 jump = 0.4f;
            const f32 base_radius = random_f32s(seed++, 0.15f, 0.2f);
            const f32 increase_degrees = 360.0f / vertices_per_segment;

            u32 size = (branch_count * 2) + 2;
            u32* offsets = stack_array(size, u32);

            Vertex vertex = { 0 };
            vertex.color = v4i(1.0f);

            Cubic_Bezier_Curve base_positions = { 0 };
            V2 temp_pos = v2_array_val(&positions, trees);
            base_positions.p[0] = v3f(temp_pos.x, 0.0f, temp_pos.y);

            base_positions.p[0].y =
                convert_to_noise_coords(
                    v2f(base_positions.p[0].x, base_positions.p[0].z))
                    .y;

            V3* pos_for_branches = stack_array(branch_count * 2, V3);
            u32* random_segments = stack_array(branch_count * 2, u32);

            for (u32 split = 0; split < 2; split++)
            {
                array_push(offsets, vert_array.size);

                const f32 random_extra_x = random_f32s(seed++, -2.0f, 2.0f);
                const f32 random_extra_z = random_f32s(seed++, -2.0f, 2.0f);

                base_positions.p[1] =
                    v3f(base_positions.p[0].x,
                        base_positions.p[0].y + (segments * jump * 0.50f),
                        base_positions.p[0].z + 0.0f);

                base_positions.p[2] =
                    v3f(base_positions.p[0].x + random_extra_x,
                        base_positions.p[0].y + (segments * jump * 0.75f),
                        base_positions.p[0].z + random_extra_z);

                base_positions.p[3] =
                    v3f(base_positions.p[0].x + random_extra_x,
                        base_positions.p[0].y + (segments * jump),
                        base_positions.p[0].z + random_extra_z);

                const u32 min_segment_index = (u32)(segments * 0.3f);
                for (u32 i = 0; i < branch_count; i++)
                {
                    array_push(
                        random_segments,
                        random_u32ss(seed++, min_segment_index, segments - 2));
                }
                f32 trunk_radius = base_radius;
                for (u32 i = 0; i < segments; i++)
                {
                    const f32 procent = (f32)i / ((f32)segments - 1.0f);
                    V3 pos = brezier_curve_pos(&base_positions, procent);
                    for (u32 j = 0; j < vertices_per_segment; j++)
                    {
                        const f32 current_radian =
                            radians(increase_degrees * j);
                        f32 x = pos.x + (cosf(current_radian) * base_radius);
                        f32 z = pos.z + (sinf(current_radian) * base_radius);
                        vertex.pos = v3f(x, pos.y, z);
                        vertex.normal = v3_sub(vertex.pos, pos);
                        vertex.color.r = (f32)j / vertices_per_segment;
                        vertex_array_push(&vert_array, vertex);
                    }
                    for (u32 j = 0; j < branch_count; j++)
                    {
                        if (i == array_val(random_segments,
                                           (split * branch_count) + j))
                        {
                            array_push(pos_for_branches, pos);
                        }
                    }
                    trunk_radius *= 0.96f;
                }
            }
            const u32 branch_pos_size = array_size(pos_for_branches);
            assert(branch_pos_size == branch_count * 2);
            for (u32 i = 0; i < branch_pos_size; i++)
            {
                array_push(offsets, vert_array.size);

                base_positions.p[0] = array_val(pos_for_branches, i);
                V3 base_pos = base_positions.p[0];

                f32 random_angle = radians(random_f32s(seed++, 0.0f, 360.0f));

                V3 dir = v3_normalize(
                    v3_sub(v3f(base_pos.x + cosf(random_angle), base_pos.y,
                               base_pos.z + sinf(random_angle)),
                           base_pos));

                f32 random_multiplier = random_f32s(seed++, 1.0f, 1.4f);
                base_positions.p[1] =
                    v3_add(base_pos, v3_s_multi(dir, random_multiplier));
                base_positions.p[1].y = base_pos.y + 0.4f;

                random_multiplier += 0.5f;
                base_positions.p[2] =
                    v3_add(base_pos, v3_s_multi(dir, random_multiplier));
                base_positions.p[2].y = base_pos.y + 0.7f;
                base_positions.p[3] =
                    v3_add(base_pos, v3_s_multi(dir, random_multiplier));
                base_positions.p[3].y = base_pos.y + 1.4f;

                V3* branch_segment_positions =
                    stack_array(branch0_segments + 1, V3);
                for (u32 j = 0; j <= branch0_segments; j++)
                {
                    const f32 procent = (f32)j / ((f32)branch0_segments);
                    array_push(branch_segment_positions,
                               brezier_curve_pos(&base_positions, procent));
                }
                f32 branch_radius = base_radius * 0.7f;
                for (u32 j = 0; j < branch0_segments; j++)
                {
                    V3 pos = array_val(branch_segment_positions, j);

                    V3 branch_segment_direction = v3_normalize(v3_sub(
                        array_val(branch_segment_positions, j + 1), pos));

                    const V3 normal =
                        v3_rotate(branch_segment_direction, radians(90.0f),
                                  v3_cross(dir, v3f(0.0f, 1.0f, 0.0f)));

                    branch_radius = branch_radius * 0.90f;
                    for (u32 k = 0; k < vertices_per_segment; k++)
                    {
                        const f32 current_radian =
                            radians(increase_degrees * k);
                        V3 add = v3_s_multi(v3_rotate(normal, current_radian,
                                                      branch_segment_direction),
                                            branch_radius);
                        vertex.pos = v3_add(pos, add);
                        vertex.normal = v3_sub(vertex.pos, pos);
                        vertex.color.r = (f32)k / vertices_per_segment;
                        vertex_array_push(&vert_array, vertex);
                    }
                }
            }
            const u32 index_table[] = {
                0, vertices_per_segment, 1,
                1, vertices_per_segment, vertices_per_segment + 1
            };

            const i32 index_last_table[] = { 0,
                                             vertices_per_segment,
                                             1 - vertices_per_segment,
                                             1 - vertices_per_segment,
                                             vertices_per_segment,
                                             1 };

            u32* iterations = stack_array(size, u32);
            array_push(iterations, segments - 1);
            array_push(iterations, segments - 1);
            for (u32 i = 0; i < branch_count * 2; i++)
            {
                array_push(iterations, branch0_segments - 1);
            }
            size = array_size(offsets);
            assert(size == array_size(iterations));
            u32 offset = 0;
            for (u32 i = 0; i < size; i++)
            {
                offset = offsets[i] + global_vert_array.size;
                for (u32 j = 0; j < iterations[i]; j++)
                {
                    for (u32 k = 0; k < vertices_per_segment - 1; k++)
                    {
                        for (u32 h = 0; h < sy_SIZE(index_table); h++)
                        {
                            u32_array_push(&idx_array, index_table[h] + offset);
                        }
                        offset++;
                    }
                    for (u32 h = 0; h < sy_SIZE(index_table); h++)
                    {
                        u32_array_push(&idx_array,
                                       index_last_table[h] + offset);
                    }
                    offset++;
                }
            }
            stack_end_scope(tree_gen_stack);
        }

        game->tree_offsets.idx = global_idx_array.size;
        game->tree_offsets.idx_size = idx_array.size;

        global_vert_array.size += vert_array.size;
        global_idx_array.size += idx_array.size;
    }

    {
        const u32 dude_count = 2;
        const u32 cube_count = 3;
        const u32 cube_size_vertex = 8 * cube_count;
        const u32 cube_size_index = 36 * cube_count;

        Vertex_Array vert_array = vertex_array_ref_at_size_offset(
            &global_vert_array, cube_size_vertex);
        U32_Array idx_array =
            u32_array_ref_at_size_offset(&global_idx_array, cube_size_index);

        const V3 dude_size = v3i(0.5f);
        vert_array.size = cube(&vert_array, vert_array.size, v3d(), dude_size,
                               v4i(1.0f), DEFAULT_TEXTURE_GAME);

        const V3 leg_size = v3f(0.125f, dude_size.y, 0.125f);
        const f32 down = leg_size.y * -0.5f;

        vert_array.size =
            cube(&vert_array, vert_array.size, v3f(0.0f, down, 0.0f), leg_size,
                 v4i(1.0f), DEFAULT_TEXTURE_GAME);

        vert_array.size =
            cube(&vert_array, vert_array.size, v3f(0.0f, down, 0.0f), leg_size,
                 v4i(1.0f), DEFAULT_TEXTURE_GAME);

        cube_indices(&idx_array, global_vert_array.size, cube_count);

        entity_3d_init(region, 0, 100, &game->entity_state);

        game->dude = entity_dynamic_3d_add(&game->entity_state, NULL);
        Dynamic_Entity_3D dude =
            entity_dynamic_3d_access(&game->entity_state, game->dude);
        dude.movement->pos = v3f(10.0f, 0.0f, 7.0f);
        dude.movement->vel = v3f(0.0f, -1.0f, 0.0f);
        dude.misc->speed = 40.0f;
        dude.misc->size = dude_size;

        game->dude2 = entity_dynamic_3d_add(&game->entity_state, &dude);

        const u32 dynamic_entity_count =
            array_size(game->entity_state.movements);
        u32 i = 0;
        for (Entity_Animation_3D* animation =
                 entity_animation_3d_iterate(&game->entity_state, i);
             animation;
             animation = entity_animation_3d_iterate(&game->entity_state, ++i))
        {
            *animation = dude_animation();
        }

        game->dude_offsets.idx = global_idx_array.size;
        game->dude_offsets.idx_size = idx_array.size;

        global_vert_array.size += vert_array.size;
        global_idx_array.size += idx_array.size;
    }

    game->sign_constants = region_array(region, 10, Push_Constant);

    {
        Vertex_Array vert_array =
            vertex_array_ref_at_size_offset(&global_vert_array, 1000);
        U32_Array idx_array =
            u32_array_ref_at_size_offset(&global_idx_array, 1000);

        square_rounded_corners_3d(
            &vert_array, &idx_array, global_vert_array.size, v3d(),
            v3f(2.0f, 1.5f, 0.3f), v4i(1.0f), 0.2f, 6, DEFAULT_TEXTURE_GAME);

        game->sign_aabb.min = v3i(INFINITY);
        V3 max = v3i(-INFINITY);
        for (u32 i = 0; i < vert_array.size; i++)
        {
            aabb_check_min_max(&game->sign_aabb,
                               vertex_array_val(&vert_array, i).pos, &max);
        }
        game->sign_aabb.size = v3_sub(max, game->sign_aabb.min);

        game->font = font_file_load(region, "Syntics/res/Purisa.fnt");
        game->font.tex_index = 1;

        V2 dimensions = v2f((f32)swap_chain->extent_2D.width,
                            (f32)swap_chain->extent_2D.height);
        M4 ortho_ = ortho(0.0f, dimensions.x, dimensions.y, 0.0f, -1.0f, 1.0f);

        const char* sign_texts[] = {
            "Snoppish",
            "Yes",
            "No",
        };
        const V3 sign_pos[] = {
            { 0.2f, -0.2f, 0.5f },
            { 0.2f, -1.1f, 0.5f },
            { 1.2f, -1.1f, 0.5f },
        };
        assert_static(sy_SIZE(sign_texts) == sy_SIZE(sign_pos), "");
        AABB_3D* aabbs[] = { &game->sign_aabb_text, &game->sign_aabb_yes,
                             &game->sign_aabb_no };

        const u32 offset = vert_array.size;
        u32 text_quad_count = 0;
        u32 text_offset = 0;
        for (u32 i = 0; i < sy_SIZE(sign_texts); i++)
        {
            text_offset = vert_array.size;
            text_quad_count +=
                text_2D(game->font, 1.0f, sign_texts[i],
                        (u32)strlen(sign_texts[i]), v3f(0.0f, 0.0f, 0.0f),
                        v4ic(0.0f), 2.0f, NULL, NULL, &vert_array);

            aabbs[i]->min = v3i(INFINITY);
            max = v3i(-INFINITY);
            for (u32 j = text_offset; j < vert_array.size; j++)
            {
                V3* pos = &vertex_array_val(&vert_array, j).pos;
                *pos = m4_v3_multi(ortho_, *pos);
                v3_add_equal(pos, sign_pos[i]);
                aabb_check_min_max(aabbs[i], *pos, &max);
            }
            aabbs[i]->size = v3_sub(max, aabbs[i]->min);
        }
        indices_generate(&idx_array, offset + global_vert_array.size,
                         text_quad_count);

        game->sign_offsets.idx = global_idx_array.size;
        game->sign_offsets.idx_size = idx_array.size;

        global_vert_array.size += vert_array.size;
        global_idx_array.size += idx_array.size;
    }

#ifdef GAME_GRASS
    {
        Obj_Load_Attrib loader;
        model_load(&loader, "Syntics/res/grass/first_draftsmall.obj");

        const u32 size = array_size(loader.indices);

        Vertex_Array temp_vert = vertex_array_create(stack_get(), size);
        U32_Array temp_u32 = u32_array_create(stack_get(), size);

        vertices_extract(&loader, DEFAULT_TEXTURE_GAME, v3d(), &temp_vert,
                         &temp_u32, true);

        obj_load_free(&loader);

        const u32 vertices_count = temp_vert.size;
        const u32 indices_count = temp_u32.size;
        game->grass_vert_count = vertices_count;

        assert(vertices_count % 4 == 0 && "For 128 wide intrinsics");

        bubble_sort_on_y(&temp_vert, &temp_u32);

#if 1
        V2_Array positions = { 0 };
        blue_noise_2d(NULL, (u32)time(NULL), 30, GRASS_DEPTH, GRASS_WIDTH,
                      0.15f, &positions);
        u32 position_size = positions.size;

        file_write_entire("saved_grass_game.synt", (char*)(positions.data),
                          position_size * sizeof(V2));
#else
        File_Attrib file = { 0 };
        file_read(&file, NULL, "saved_grass_game.synt", "rb");
        u32 position_size = file.size / sizeof(V3);
        V3_Array positions = { .size = position_size,
                               ._capacity = position_size,
                               .data = (V3*)file.buffer };
#endif

        // Align
        position_size -= position_size % MAX_GRASS_THREADS;

        Vertex_Array vert_array = vertex_array_ref_at_size_offset(
            &global_vert_array, vertices_count * position_size);
        U32_Array idx_array = u32_array_ref_at_size_offset(
            &global_idx_array, indices_count * position_size);

        const u32 seed = (u32)time(NULL);
        const u32 thread_split = position_size / MAX_GRASS_THREADS;
        const u32 vert_size = thread_split * vertices_count;
        const u32 indices_size = thread_split * indices_count;
        Thread_Task tasks[MAX_GRASS_THREADS - 1];
        u32 task_count = 0;
        for (u32 i = 1; i < MAX_GRASS_THREADS; i++)
        {
            Thread_Attrib_Grass* th = grass_threads + i;
            th->index = i;
            th->seed = random_u32s(seed + (227 * i));
            th->grass_count = thread_split;

            th->vertex_offset = i * vert_size;
            th->vertex_array = vert_array.data + th->vertex_offset;
            th->vertex_offset +=
                global_vert_array.size; // For the final vertex buffer

            const u32 indices_offset = i * indices_size;
            th->indices_array = idx_array.data + indices_offset;

            th->positions = v2_array_val_ptr(&positions, (thread_split * i));
            th->model_vertices = &temp_vert;
            th->model_indices = &temp_u32;

            tasks[task_count++] = thread_task(grass_generation_threaded, th);
        }
        thread_tasks_push(tasks, task_count, &counter);

        grass_generation(random_u32s(seed), global_vert_array.size,
                         vert_size / vertices_count, vertices_count,
                         indices_count, positions.data, temp_vert.data,
                         temp_u32.data, vert_array.data, idx_array.data);

        semaphore_counter_wait_and_free(&counter);

        sy_print("Grass idx: %llu\n", idx_array._capacity * (u32)sizeof(u32));
        sy_print("Grass vert: %llu\n",
                 vert_array._capacity * (u32)sizeof(Vertex));

        game->grass_offsets.idx = global_idx_array.size;
        game->grass_offsets.idx_size = idx_array._capacity;

        global_vert_array.size += vert_array._capacity;
        global_idx_array.size += idx_array._capacity;

        free(positions.data);
    }
#endif
    {
        const u32 cube_size_vertex = 8;
        const u32 cube_size_index = 36;
        const u32 vert_size_particles = cube_size_vertex * MAX_PARTICLES;
        const u32 index_size_particles = cube_size_index * MAX_PARTICLES;

        Vertex_Array vert_array = vertex_array_ref_at_size_offset(
            &global_vert_array, vert_size_particles);
        U32_Array idx_array = u32_array_ref_at_size_offset(
            &global_idx_array, index_size_particles);

        cube_indices(&idx_array, global_vert_array.size, MAX_PARTICLES);
        vert_array.size = vert_size_particles;

        game->boom_curve.p[0] = v3d();
        game->boom_curve.p[1] = v3_add(
            v3d(), v3_s_multi(v3_rotate(v3f(0.0f, 0.0f, 1.0f), -radians(50.0f),
                                        v3f(0.0f, 1.0f, 0.0f)),
                              20.0f));
        game->boom_curve.p[2] = v3_add(
            v3d(), v3_s_multi(v3_rotate(v3f(0.0f, 0.0f, 1.0f), radians(50.0f),
                                        v3f(0.0f, 1.0f, 0.0f)),
                              20.0f));
        game->boom_curve.p[3] = v3d();

        u32 seed = (u32)time(NULL);
        game->particle_arc_offsets = v3_array_create(region, MAX_PARTICLES);
        game->particle_arc_offsets_change =
            region_array_calloc(region, MAX_PARTICLES, f32);

        V2_Array positions = { 0 };
        for (f32 i = 0.0f; i <= 1.0f; i += 0.006f)
        {
            positions.size = 0;
            blue_noise_2d(NULL, seed++, 30, 10, 10, 0.05f, &positions);
            for (u32 j = 0; j < 20; j++)
            {
                array_push(game->particle_arc_offsets_change,
                           random_f32s(seed++, 0.5f, 1.5f));
                V3 pos = brezier_curve_pos(&game->boom_curve, i);
                V2 adding = v2_array_val(&positions, j);
                pos.x += adding.x;
                pos.z += adding.y;
                v3_array_push(&game->particle_arc_offsets, pos);
            }
        }
        free(positions.data);

        Vertex_Array* vert = &game->particles_vert_array;
        *vert = vertex_array_create(region, vert_size_particles);

        particles_3d_init(region, &game->particles, MAX_PARTICLES);

        game->particles_staging_buffer.size_bytes =
            vert_array.size * sizeof(Vertex);
        game->particles_staging_buffer.dst_offset =
            global_vert_array.size * sizeof(Vertex);

        game->particles_offsets.idx = global_idx_array.size;
        game->particles_offsets.idx_size = idx_array.size;

        global_vert_array.size += vert_array.size;
        global_idx_array.size += idx_array.size;
    }

    Vertex_Buffer* game_vert = &game->vert_idx_buffer.vert;
    Index_Buffer* game_idx = &game->vert_idx_buffer.idx;

    game_vert->array = global_vert_array;
    game_vert->array._capacity = game_vert->array.size;
    game_idx->array = global_idx_array;
    game_idx->array._capacity = game_idx->array.size;

    printf("Game Vertex size: %u\n", game_vert->array._capacity);
    printf("Game Index size: %u\n", game_idx->array._capacity);

    vertex_index_buffer_create_default1(device, physical_device, command_pool,
                                        graphic_queue, VERTEX_INDEX_LOCAL_LOCAL,
                                        &game->vert_idx_buffer);

    free(global_vert_array.data);
    free(global_idx_array.data);

    game->cam = cam_3di(4.0f, 5.0f);
    game->cam.pos = v3f(-14.2f, 8.0f, 3.0f);
    game->cam.ori = v3f(0.9f, -0.12f, 0.0f);
    game->global_model = m4i(1.0f);

#if 0
    u32 vert_offset = 0;
    { // Road Lines
        stack_begin_scope(road_line_stack);
#if 0
        File_Attrib file = {};
        read_file(&file, stack_get(), "saved_spline_game.synt", "rb");

        u32 vert_size = *((u32*)file.buffer);
        file.buffer += sizeof(u32);

        g_p->vert_buffer.data = region_array_calloc(region, vert_size, Vertex);
        memcpy(g_p->vert_buffer.data, file.buffer, vert_size * sizeof(Vertex));
        array_head(g_p->vert_buffer.data)->size = vert_size;
        file.buffer += vert_size * sizeof(Vertex);

        u32 index_size = *((u32*)file.buffer);
        file.buffer += sizeof(u32);

        g_p->idx_buffer.data = region_array_calloc(region, index_size, u32);
        memcpy(g_p->idx_buffer.data, file.buffer, index_size * sizeof(u32));
        array_head(g_p->idx_buffer.data)->size = index_size;
        file.buffer += index_size * sizeof(u32);
#else
#endif
#if 1
        File_Attrib file = { 0 };
        const char* file_path = path_extend_d1("saved_spline3_game.synt");
        file_read(&file, stack_get(), file_path, "rb");

        spline2.n_curves = *((u32*)file.buffer);
        file.buffer += sizeof(u32);
        spline2.splitt = *((u32*)file.buffer);
        file.buffer += sizeof(u32);

        spline2.bc[0] =
            region_array_calloc(region, spline2.n_curves, Cubic_Bezier_Curve);
        spline2.bc[1] =
            region_array_calloc(region, spline2.n_curves, Cubic_Bezier_Curve);

        for (u32 i = 0; i < spline2.n_curves; i++)
        {
            for (u32 j = 0; j < 4; j++)
            {
                memcpy(&spline2.bc[0][i].p[j], file.buffer, sizeof(V3));
                file.buffer += sizeof(V3);
                memcpy(&spline2.bc[0][i].points_indices[j], file.buffer,
                       sizeof(u32));
                file.buffer += sizeof(u32);
            }
        }
        for (u32 i = 0; i < spline2.n_curves; i++)
        {
            for (u32 j = 0; j < 4; j++)
            {
                memcpy(&spline2.bc[1][i].p[j], file.buffer, sizeof(V3));
                file.buffer += sizeof(V3);
                memcpy(&spline2.bc[1][i].points_indices[j], file.buffer,
                       sizeof(u32));
                file.buffer += sizeof(u32);
            }
        }
#else
        File_Attrib file = {};
        read_file(&file, stack_get(), "saved_spline2_game.synt", "rb");

        spline2 = *((Brezier_Spline_3D*)file.buffer);
        file.buffer += sizeof(Brezier_Spline_3D);

        spline2.bc[0] =
            region_array_calloc(region, spline2.n_curves, Cubic_Brezier_Curve);
        spline2.bc[1] =
            region_array_calloc(region, spline2.n_curves, Cubic_Brezier_Curve);

        memcpy(spline2.bc[0], file.buffer,
               spline2.n_curves * sizeof(Cubic_Brezier_Curve));
        file.buffer += spline2.n_curves * sizeof(Cubic_Brezier_Curve);
        memcpy(spline2.bc[1], file.buffer,
               spline2.n_curves * sizeof(Cubic_Brezier_Curve));
        file.buffer += spline2.n_curves * sizeof(Cubic_Brezier_Curve);
#endif
        vert_offset = spline2.n_curves * 8 * 10;

        game->road_pos = *((V3*)file.buffer);
        file.buffer += sizeof(V3);

        current_curve_count = *((u32*)file.buffer);

        Vertex_Buffer* vert = &game->road_line_vert_idx.vert;
        Index_Buffer* idx = &game->road_line_vert_idx.idx;

        const u32 point_all_size = points_size * spline2.n_curves;
        num_points = spline2.n_curves * 8;

        idx->array = u32_array_create(stack_get(),
                                      (point_all_size + (num_points * 10) + 1) * 2);

        u32 count = 0;
        u32 first_index = 0;
        for (u32 i = 0; i < num_points; i++)
        {
            first_index = count;
            for (u32 j = 0; j < 9; j++)
            {
                u32_array_push(&idx->array, count++);
                u32_array_push(&idx->array, count);
            }
            u32_array_push(&idx->array, count++);
            u32_array_push(&idx->array, first_index);
        }

        const u32 size = point_all_size + (num_points * 10);
        vert->array = vertex_array_create(region, size);

        vert_offset =
            spline_3d_circles_create(game->rects, &vert->array, 0, &spline2, 0.08f);
        u32 size33 = spline_generate(&spline2, &vert->array, vert_offset);
        vert->array.size = size33;
        spline_3d_normals_generate(&vert->array, vert_offset, &spline2);

        size33 -= vert_offset;
        size33 += count;

        u32 vertex_count = 0;
        u32 i;
        const u32 half_splitt = spline2.splitt / 2;
        for (i = count; i < size33 - 1; i++)
        {
            if (++vertex_count % half_splitt != 0)
            {
                u32_array_push(&idx->array, i);
                u32_array_push(&idx->array, i + 1);
            }
        }
        num_points = spline2.n_curves * 8;

        circle_offset = (num_points * 10) * 2;
        circle_curr_size = (8 * (10 * 2)) * current_curve_count;

        idx->curr_size =
            circle_offset + (current_curve_count * (points_size * 2 - 4));
        vertex_index_buffer_create_default1(
            device, physical_device, command_pool, graphic_queue,
            VERTEX_INDEX_VISIBLE_LOCAL, &game->road_line_vert_idx);

        stack_end_scope(road_line_stack);
    }

    { // Road
        stack_begin_scope(road_stack);

        Vertex_Buffer* vert = &game->road_vert_idx.vert;
        Index_Buffer* idx = &game->road_vert_idx.idx;

        const u32 size = game->road_line_vert_idx.vert.array.size - vert_offset;

        vert->array = vertex_array_create(region, size);
        for (u32 i = vert_offset; i < size + vert_offset; i++)
        {
            vertex_array_push(
                &vert->array,
                vertex_array_val(&game->road_line_vert_idx.vert.array, i));
        }

        idx->array = u32_array_create(stack_get(), size);
#if 1
        u32 vertex_count = 0;
        u32 count = 0;
        const u32 half_size = size / 2;
        const u32 half_splitt = spline2.splitt / 2;
        for (u32 i = 0; i < half_size; i++)
        {
            u32_array_push(&idx->array, count);
            u32_array_push(&idx->array, count++ + half_splitt);
            if (++vertex_count % half_splitt == 0)
            {
                count += half_splitt;
            }
        }
#endif
        assert(idx->array.size == size);

        idx->curr_size = points_size * current_curve_count;
        vertex_index_buffer_create_default1(
            device, physical_device, command_pool, graphic_queue,
            VERTEX_INDEX_VISIBLE_LOCAL, &game->road_vert_idx);

        stack_end_scope(road_stack);
    }
#endif

    {
        Vertex_Buffer* vert = &game->aabb_rep.vert;
        Index_Buffer* idx = &game->aabb_rep.idx;

        const u32 aabb_count = 30;
        const u32 coll_idx[] = { 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6,
                                 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7 };

        const u32 aabb_idx_size = sy_SIZE(coll_idx);
        game->aabb_indices_count = aabb_idx_size;
        const u32 index_count = aabb_idx_size * aabb_count;
        const u32 vert_count = 8 * aabb_count;

        vert->array = vertex_array_create(region, vert_count);
        idx->array = u32_array_create(stack_get(), index_count);

        for (u32 i = 0; i < aabb_count; i++)
        {
            const u32 offset = vert->array.size;
            cube_not_center1(&vert->array, game->sign_aabb.min,
                             game->sign_aabb.size, v4i(1.0f),
                             DEFAULT_TEXTURE_GAME);
            for (u32 j = 0; j < aabb_idx_size; j++)
            {
                u32_array_push(&idx->array, coll_idx[j] + offset);
            }
        }
        idx->curr_size = idx->array.size;
        vertex_index_buffer_create_default1(
            device, physical_device, command_pool, graphic_queue,
            VERTEX_INDEX_VISIBLE_LOCAL, &game->aabb_rep);
    }

    event_subscribe(&game->mouse_evt, EVT_MOUSE);
    event_subscribe(&game->wheel_evt, EVT_WHEEL);

    subscribe_recreate_gp_callback(render_state, game_recreate, game);

    stack_end_scope(game_init_stack);
}

b8 collide_with_spline(const Bezier_Spline_3D* spline, V3 offset_pos,
                       V3 test_pos, V3* collision_pos, V3* normal,
                       b8* side_collision);

void update_dudes_position(Entity_State_3D* entity_state, V3 road_pos, f32 dt)
{
    u32 i = 0;
    Dynamic_Entity_3D e = entity_dynamic_3d_iterate(entity_state, i);
    for (; e.movement; e = entity_dynamic_3d_iterate(entity_state, ++i))
    {
        e.movement->vel =
            v3_add(v3_s_multi(e.movement->acc, dt), e.movement->vel);
        V3 collide_pos =
            v3_add(v3_s_multi(e.movement->acc, 0.5f * dt * dt),
                   v3_add(v3_s_multi(e.movement->vel, dt), e.movement->pos));
        e.movement->pos = collide_pos;
        e.movement->vel.y -= 0.5f * e.movement->vel.y * dt;

        Entity_Animation_3D* animation = e.animation;
        {
            V3 terrain_coords0 = { 0 };
#if 0
            if (spline_collision && i == 0)
            {
                collide_with_spline(&spline2, road_pos, e.movement->pos,
                                    &terrain_coords0, NULL, NULL);
            }
            else
#endif
            {
                terrain_coords0 = convert_to_noise_coords(
                    v2f(e.movement->pos.x, e.movement->pos.z));
            }

            const f32 extra_padding = 0.2f + e.misc->size.y;
            if (collide_pos.y <= terrain_coords0.y + extra_padding)
            {
                // V3 n = v3f(0.0f, 1.0f, 0.0f);
                //  e.movement->vel =
                //     v3_sub(e.movement->vel,
                //           v3_s_multi(n, 1.0f * v3_dot(e.movement->vel, n)));
                e.movement->pos.y = terrain_coords0.y + extra_padding;
                animation->sec_off_ground = 0.0f;
                animation->off_the_ground = false;
            }
            else
            {
                // e.movement->pos = collide_pos;
                animation->sec_off_ground += dt;
            }
            if (animation->sec_off_ground >= 0.01f)
            {
                animation->off_the_ground = true;
            }
        }

        f32 idle_translation = 0;
        if (v2_len_squared(v2f(e.movement->acc.x, e.movement->acc.z)))
        {
            if (animation->reset)
            {
                animation->left_leg_rotation_angle = 0.0f;
                animation->right_leg_rotation_angle = 0.0f;
                animation->dude_rotation_angle = 0.0f;
                animation->reset = false;
            }
            animation->left_leg_rotation_angle +=
                animation->leg_rotation_speed * dt;
            animation->right_leg_rotation_angle -=
                animation->leg_rotation_speed * dt;
            if (animation->left_leg_rotation_angle >= 40.0f ||
                animation->left_leg_rotation_angle <= -40.0f)
            {
                animation->leg_rotation_speed *= -1.0f;
                animation->left_leg_rotation_angle +=
                    animation->leg_rotation_speed * dt;
                animation->right_leg_rotation_angle -=
                    animation->leg_rotation_speed * dt;
            }

            animation->dude_rotation_angle +=
                animation->dude_rotation_speed * dt;
            if (animation->dude_rotation_angle >= 3.0f ||
                animation->dude_rotation_angle <= -3.0f)
            {
                animation->dude_rotation_speed *= -1.0f;
                animation->dude_rotation_angle +=
                    animation->dude_rotation_speed * dt;
            }
            animation->stop_animation_sec = 0.0f;
        }
        else
        {
            if (!animation->reset)
            {
                animation->stop_animation_sec = 0;
                animation->left_leg_rotation_angle = 0.0f;
                animation->right_leg_rotation_angle = 0.0f;
                animation->dude_rotation_angle = 0.0f;
                animation->reset = true;
            }
            const f32 len_squared = v3_len_squared(e.movement->vel);
            if (len_squared < 0.2f)
            { // idle
                const f32 idle_animation_duration = 0.4f;
                const f32 procent =
                    animation->stop_animation_sec / idle_animation_duration;
                animation->stop_animation_sec += dt;
                if (procent > 1.0f)
                {
                    animation->stop_animation_sec = 0;
                }
                idle_translation =
                    sinf(radians(sy_lerp(0.0f, 360.0f, procent))) * 0.01f;
            }
        }
        animation->dude_models[0] =
            m4_multi(m4_translate(e.movement->pos), rotate_y(animation->angle));

        animation->dude_models[1] = m4_multi(
            m4_multi(animation->dude_models[0],
                     m4_translate(v3f(-0.125f, e.misc->size.y * -0.4f, 0.0f))),
            rotate_x(radians(animation->left_leg_rotation_angle)));

        animation->dude_models[2] = m4_multi(
            m4_multi(animation->dude_models[0],
                     m4_translate(v3f(0.125f, e.misc->size.y * -0.4f, 0.0f))),
            rotate_x(radians(animation->right_leg_rotation_angle)));

        animation->dude_models[0] =
            m4_multi(animation->dude_models[0],
                     rotate_x(radians(animation->dude_rotation_angle)));

        animation->dude_models[0] =
            m4_multi(m4_translate(v3f(0.0f, idle_translation, 0.0f)),
                     animation->dude_models[0]);
    }
}

#define rec_sample_count 1000

b8 record(M4* view_matrix, f32 dt)
{
    presist b8 p_pressed = false;
    presist b8 q_pressed = false;
    presist u32 count_rec = 0;
    presist u32 count_play = 0;
    presist M4 rec[rec_sample_count] = { 0 };
    presist f32 sec = 0.0f;
    presist const f32 sample_time = MILLISECONDS(15.0f);

    presist b8 q_clicked = false;
    if (q_clicked)
    {
        if (!q_pressed)
        {
            count_rec = 0;
            q_pressed = true;
        }
        else
        {
            sy_print("Stop Rec\n");
            q_pressed = false;
        }
    }
    presist b8 first_clicked = true;
    q_clicked = false; // is_key_clicked(&first_clicked, SYNT_KEY_Q);
    if (q_pressed && !p_pressed)
    {
        sec += dt;
        if (sec >= sample_time)
        {
            if (count_rec < rec_sample_count)
            {
                sy_print("Rec: %u / %u\n", count_rec + 1, rec_sample_count);
                rec[count_rec++] = *view_matrix;
            }
            else
            {
                q_clicked = true;
            }
            sec = 0.0f;
        }
    }
    presist b8 first_clicked1 = true;
    if (is_key_clicked(&first_clicked1, SYNT_KEY_P))
    {
        if (!p_pressed)
        {
            sy_print("Start Playing\n");
            p_pressed = true;
        }
        else
        {
            sy_print("Stop Playing\n");
            p_pressed = false;
        }
        count_play = 0;
    }
    if (p_pressed)
    {
        sec += dt;
        if (sec >= sample_time)
        {
            *view_matrix = rec[count_play++];
            count_play %= count_rec;
            sec = 0.0f;
        }
    }
    return p_pressed;
}

V3 shoot_camera_ray(VP vp, V3 mouse_device_coords)
{
    V4 ray_clip = v4_v3f(mouse_device_coords, 1.0f);
    ray_clip.z = -1.0f;

    V4 ray_eye = m4_v4_multi(inverse(vp.proj), ray_clip);
    ray_eye.z = -1.0f;
    ray_eye.w = 0.0f;

    V3 ray = v3_v4(m4_v4_multi(inverse(vp.view), ray_eye));
    ray = v3_normalize(ray);

    return ray;
}

void swap(f32* x, f32* y)
{
    f32 temp = *x;
    *x = *y;
    *y = temp;
}

b8 ray_hit_target_aabb(V3 ray_direction, V3 ray_origin, AABB_3D target)
{
    V3 min_p = target.min;
    V3 max_p = v3_add(target.min, target.size);
    V2 x;
    x.min = (min_p.x - ray_origin.x) / ray_direction.x;
    x.max = (max_p.x - ray_origin.x) / ray_direction.x;

    if (x.min > x.max) swap(&x.min, &x.max);

    V2 y;
    y.min = (min_p.y - ray_origin.y) / ray_direction.y;
    y.max = (max_p.y - ray_origin.y) / ray_direction.y;

    if (y.min > y.max) swap(&y.min, &y.max);

    if (x.min > y.max || x.max < y.min)
    {
        return false;
    }
    V2 res = x;
    res.min = y.min > x.min ? y.min : x.min;
    res.max = y.max < x.max ? y.max : x.max;

    V2 z;
    z.min = (min_p.z - ray_origin.z) / ray_direction.z;
    z.max = (max_p.z - ray_origin.z) / ray_direction.z;

    if (z.min > z.max) swap(&z.min, &z.max);

    if (res.min > z.max || res.max < z.min)
    {
        return false;
    }
    res.min = z.min > res.min ? z.min : res.min;
    res.max = z.max < res.max ? z.max : res.max;

    return true;
}

V3 ray_hit(V3 ray, V3 camera_pos, V3 target_pos)
{
    f32 d = v3_distance(camera_pos, target_pos);
    v3_s_multi_equal(&ray, d);
    v3_add_equal(&ray, camera_pos);
    return ray;
}

void bubble_sort_rects(Rect3D* rects, u32 size)
{
    for (u32 i = 0; i < size - 1; i++)
    {
        for (u32 j = 0; j < size - i - 1; j++)
        {
            Rect3D* first = rects + j;
            Rect3D* second = first + 1;
            if (first->misc > second->misc)
            {
                Rect3D temp = *first;
                *first = *second;
                *second = temp;
            }
        }
    }
}

void edit_spline(Game_State* game, V2 dimensions, b8 camera_moved, V3 ray,
                 b8 first, b8 should_update, b8* hit, b8* xyz_pressed)
{
    presist Rect3D* rect = NULL;
    if (!(*hit))
    {
        if (camera_moved || should_update)
        {
            u32 rect_size = array_size(game->rects);
            for (u32 i = 0; i < rect_size; i++)
            {
                game->rects[i].misc = v3_distance(
                    game->cam.pos, v3_add(game->rects[i].pos, game->road_pos));
            }
            bubble_sort_rects(game->rects, rect_size);
            camera_moved = false;
        }
        u32 rect_size = array_size(game->rects);
        for (u32 i = 0; i < rect_size; i++)
        {
            rect = game->rects + i;
            AABB_3D aabb = { v3_add(v3_sub(rect->pos, rect->size),
                                    game->road_pos),
                             v3_s_multi(rect->size, 2.0f) };
            *hit = ray_hit_target_aabb(ray, game->cam.pos, aabb);
            if (*hit)
            {
                sy_print("hhh\n");
                break;
            }
        }
    }
    Vertex_Buffer* vert = &game->road_line_vert_idx.vert;
    if (*hit)
    {
        if (is_key_pressed(SYNT_KEY_X))
        {
            rect->pos.x = v3_sub(ray_hit(ray, game->cam.pos,
                                         v3_add(rect->pos, game->road_pos)),
                                 game->road_pos)
                              .x;
            *xyz_pressed = true;
        }
        if (is_key_pressed(SYNT_KEY_C))
        {
            rect->pos.y = v3_sub(ray_hit(ray, game->cam.pos,
                                         v3_add(rect->pos, game->road_pos)),
                                 game->road_pos)
                              .y;
            *xyz_pressed = true;
        }
        if (is_key_pressed(SYNT_KEY_Z))
        {
            rect->pos.z = v3_sub(ray_hit(ray, game->cam.pos,
                                         v3_add(rect->pos, game->road_pos)),
                                 game->road_pos)
                              .z;
            *xyz_pressed = true;
        }

        if (!*xyz_pressed)
        {
            rect->pos = v3_sub(
                ray_hit(ray, game->cam.pos, v3_add(rect->pos, game->road_pos)),
                game->road_pos);
        }

        u32 iterations = 1;
        if (unpack_point(rect->id) == 3 &&
            unpack_curve(rect->id) < spline2.n_curves - 1)
        {
            iterations = 2;
        }
        for (u32 i = 0; i < iterations; i++)
        {
            u32 side = unpack_side(rect->id);
            u32 curve = unpack_curve(rect->id) + i;
            u32 point = (unpack_point(rect->id) + i) % 4;
            circle_create(&vert->array,
                          spline2.bc[side][curve].points_indices[point],
                          rect->pos, 0.08f);
            spline_generate_at_curve(&spline2, side, curve, point, rect->pos,
                                     &game->road_line_vert_idx.vert.array,
                                     &game->road_vert_idx.vert.array);
            if (!is_key_pressed(SYNT_KEY_SHIFT))
            {
                presist Rect3D* rect2 = NULL;
                presist V3 diff = { 0 };
                ++side;
                side %= 2;
                if (first)
                {
                    u32 id = 0;
                    pack(id, side, curve, point);
                    u32 rect_size = array_size(game->rects);
                    for (u32 j = 0; j < rect_size; j++)
                    {
                        if (game->rects[j].id == id)
                        {
                            rect2 = game->rects + j;
                            break;
                        }
                    }
                    assert(rect2 && "Rect2 is null");
                    diff = v3_sub(rect2->pos, rect->pos);
                }
                rect2->pos = v3_add(rect->pos, diff);
                circle_create(&vert->array,
                              spline2.bc[side][curve].points_indices[point],
                              rect2->pos, 0.08f);
                spline_generate_at_curve(&spline2, side, curve, point,
                                         rect2->pos,
                                         &game->road_line_vert_idx.vert.array,
                                         &game->road_vert_idx.vert.array);
            }
        }
        data_buffer_copy(&vert->buffer, vert->array.data,
                         vert->buffer.size_bytes);
        data_buffer_copy(&game->road_vert_idx.vert.buffer,
                         game->road_vert_idx.vert.array.data,
                         game->road_vert_idx.vert.buffer.size_bytes);
    }
}

f32 point_procent_along_curve_linear(Cubic_Bezier_Curve curve,
                                     V3 offset_position, V3 point_pos,
                                     f32 precision)
{
    f32 smallest = INFINITY;
    f32 result = 0.0f;
    for (; result <= 1.0f; result += precision)
    {
        V3 current_point =
            v3_add(brezier_curve_pos(&curve, result), offset_position);
        f32 curr_dist_squared = v3_distance_squared(point_pos, current_point);

        if (curr_dist_squared < smallest)
        {
            smallest = curr_dist_squared;
        }
        else
        {
            break;
        }
    }
    return result;
}

f32 point_procent_along_curve_binary(Cubic_Bezier_Curve curve,
                                     V3 offset_position, V3 point_pos,
                                     f32 precision)
{
    f32 min = 0.0f;
    f32 max = 1.0f;
    f32 result = 0.0f;

    while (max - min > precision)
    {
        result = (max + min) / 2.0f;

        V3 current_point =
            v3_add(brezier_curve_pos(&curve, result), offset_position);
        f32 curr_dist_squared = v3_distance_squared(point_pos, current_point);

        V3 next_point = v3_add(brezier_curve_pos(&curve, result + precision),
                               offset_position);
        f32 next_dist_squared = v3_distance_squared(point_pos, next_point);

        if (next_dist_squared > curr_dist_squared)
        {
            max = result;
        }
        else
        {
            min = result;
        }
    }
    return result;
}

b8 collide_with_spline(const Bezier_Spline_3D* spline, V3 offset_pos,
                       V3 test_pos, V3* collision_pos, V3* normal,
                       b8* side_collision)
{
    // TODO: can only use this function for one spline at the moment
    presist u32 left_side_curve_index = 0;
    presist u32 right_side_curve_index = 0;
    if (reset_index_GAME)
    {
        left_side_curve_index = 0;
        right_side_curve_index = 0;
        reset_index_GAME = false;
    }
    f32 precision = 0.001f;

    f32 procent0 = point_procent_along_curve_binary(
        spline->bc[0][left_side_curve_index], offset_pos, test_pos, precision);
    f32 procent1 = point_procent_along_curve_binary(
        spline->bc[1][right_side_curve_index], offset_pos, test_pos, precision);

    V3 first = v3_add(
        brezier_curve_pos(&spline->bc[0][left_side_curve_index], procent0),
        offset_pos);
    V3 second = v3_add(
        brezier_curve_pos(&spline->bc[1][right_side_curve_index], procent1),
        offset_pos);

    V3 between_vec = v3_sub(second, first);
    f32 distance_between = v3_len(between_vec);

    V3 first_to_pos = v3_sub(test_pos, first);
    V3 line = v3d();
    V3 normal_ = v3d();
    if (distance_between <
        (v3_dot(first_to_pos, between_vec) / distance_between))
    {
        if (side_collision)
        {
            *side_collision = true;
        }
        normal_ = v3_normalize(v3_neg(between_vec));
        line = second;
    }
    else
    {
#if 0
        V3 right_to_pos = v3_sub(test_pos, second);
        V3 right_to_left = v3_sub(first, second);
        if (distance_between <
            (v3_dot(right_to_pos, right_to_left) / distance_between))
#else
        if (v3_dot(first_to_pos, between_vec) <= 0)
#endif
        {
            if (side_collision)
            {
                *side_collision = true;
            }
            line = first;
            normal_ = v3_normalize(between_vec);
        }
        else
        {
            f32 distance_to_position = v3_len(first_to_pos);
            f32 p0 = distance_to_position / distance_between;
            line = v3_lerp(first, second, p0);
        }
    }
    if (collision_pos)
    {
        *collision_pos = line;
    }
    if (normal)
    {
        *normal = normal_;
    }
    if (procent0 >= 1.0f - precision)
    {
        if (left_side_curve_index < current_curve_count - 1)
        {
            left_side_curve_index++;
        }
    }
    else if (procent0 <= 0.0f + precision)
    {
        if (left_side_curve_index > 0)
        {
            left_side_curve_index--;
        }
    }
    if (procent1 >= 1.0f - precision)
    {
        if (right_side_curve_index < current_curve_count - 1)
        {
            right_side_curve_index++;
        }
    }
    else if (procent1 <= 0.0f + precision)
    {
        if (right_side_curve_index > 0)
        {
            right_side_curve_index--;
        }
    }
    return true; // if(line.y <= test_pos.y) return true;
}

void camera_move(Camera_3D* cam, V3 end_position, V3 alignment_point,
                 f32 cam_distance)
{
    const f32 distance = v3_distance(cam->pos, end_position);
    const V3 dir = v3_normalize(v3_sub(end_position, cam->pos));
    const f32 speed = minf32(distance * smoothness_GAME, 0.4f * cam_distance);
    cam->vel = v3_s_multi(dir, speed);
    cam->pos = v3_add(cam->pos, cam->vel);
    cam->ori = v3_normalize(v3_sub(alignment_point, cam->pos));
}

void game_update(Game_State* game, Gui_Context* gui_ctx,
                 Application_State* app_state, Frame_Data* frame, V2 dimensions,
                 u32 semaphore_idx, f32 dt)
{
    f32 cam_dt = dt;
    if (pause_game)
    {
        dt = 0.0f;
        presist b8 first_clicked = false;
        if (is_key_clicked(&first_clicked, SYNT_KEY_G))
        {
            dt = (f32)MILLISECONDS(16.6);
        }
    }
    game->aabb_count = 0;

    presist b8 first_update_edit = true;
    presist b8 first_update_not_edit = true;
    if (g_edit_mode_GAME)
    {
        if (first_update_edit)
        {
            game->cam.speed = 2.0f;
            first_update_edit = false;
            first_update_not_edit = true;
        }
    }
    else
    {
        if (first_update_not_edit)
        {
            game->cam.speed = 2000.0f;
            first_update_not_edit = false;
            first_update_edit = true;
        }
    }
    presist b8 camera_moved = false;
    presist b8 gravity = false;
    if (!gui_is_focus())
    {
        camera_moved |=
            camera_update(&game->cam, app_state->platform, game->mouse_evt,
                          cam_dt, false, g_edit_mode_GAME);
    }

    game->grass_model = m4i(1.0f);

    game->offset_p += grass_wind_speed * dt;

#if 0
    if (!g_edit_mode_GAME)
    {
        V3 line = v3d();
        V3 normal = v3d();
        b8 side_collision = false;
        if (collide_with_spline(&spline2, game->road_pos, game->cam.pos, &line,
                                &normal, &side_collision))
        {
            presist f32 sec_off_ground = 0.0f;
            if (game->cam.pos.y <= line.y + 0.18f)
            {
                game->cam.pos.y = line.y + 0.18f;
                sec_off_ground = 0.0f;
                // off_the_ground = false;
            }
            else
            {
                sec_off_ground += dt;
            }
            if (sec_off_ground >= 0.1f)
            {
                // off_the_ground = true;
            }
#if 0
            if (side_collision)
            {
                // TODO: speed to fast so vel gets flipped. Should not be updated if
                // it in the same frame hits the side.
                game->cam.vel = v3_sub(
                    game->cam.vel,
                    v3_s_multi(normal, 2.0f * v3_dot(game->cam.vel, normal)));
            }
#endif
            game->cam.vel.x -= 5.0f * game->cam.vel.x * dt;
            game->cam.vel.z -= 5.0f * game->cam.vel.z * dt;
        }
    }
    if (g_edit_mode_GAME)
    {

        presist b8 first = true;
        presist b8 spline_hit = false;
        presist b8 should_update = false;
        presist b8 xyz_pressed = false;
        if (!is_focus() &&
            game->mouse_evt->mouse_evt.button_evt.action == SYNT_BUTTON_PRESS &&
            game->mouse_evt->mouse_evt.button_evt.button == SYNT_LEFT_BUTTON)
        {
            i16 x, y;
            platform_mouse_get_pos(&x, &y);
            V3 mouse_pos = v3f((f32)x, (f32)y, 0.0f);

            mouse_pos = mouse_to_device_coords(mouse_pos, dimensions);
            V3 ray = shoot_camera_ray(game->cam.vp, mouse_pos);

            edit_spline(game, dimensions, camera_moved, ray, first, should_update,
                        &spline_hit, &xyz_pressed);

            first = false;
            should_update = false;
        }
        else
        {
            if (xyz_pressed)
            {
                should_update = true;
            }
            xyz_pressed = false;
            spline_hit = false;
            first = true;
        }
    }
#endif

#if 0

    if (show_particles_GAME)
    {
        presist f32 sec = 0.0f;
        sec += dt;
        if (sec >= 1.0f)
        {
            for (u32 i = 0; i < MAX_PARTICLES / 12; i++)
            {
                Particle_Attrib_3D attrib = { 0 };
                f32 x = fmodf((i * random_f32(0.0f, 0.8f)), CHUNK_SIZE_X * 0.5);
                f32 z = fmodf((i * random_f32(0.0f, 0.8f)), CHUNK_SIZE_Z * 0.5);
                attrib.position = v3f(x, 45.0f, z);
                attrib.color = v4i(1.0f);
                attrib.size = v3i(random_f32(0.05f, 0.1f));
                particle_3d_emit(&game->particles, &attrib,
                                 v3f(0.0f, -10.0f, 0.0f), v3d(),
                                 random_f32(0.5f, 1.0f), 10.0f);
            }
            sec = 0.0f;
        }
        u32 cube_index_size = 36;
        u32 particle_size = particles_3d_update(
            &game->particles, &game->particles_vert_array, 0, dt);

        frame->game_particle_count = particle_size * cube_index_size;

        assert(frame->game_particle_count <
               frame->game_particles_offsets.idx_size);

        data_buffer_copy(&frame->game_particles_staging_buffer,
                         game->particles_vert_array.data,
                         (particle_size * 8) * sizeof(Vertex));
    }
    else
    {
        frame->game_particle_count = 0;
    }
#endif

    if (!record(&game->cam.vp.view, dt))
    {
        game->cam.vp.view = view(
            game->cam.pos, v3_add(game->cam.pos, game->cam.ori), game->cam.up);
    }
    presist f32 rotation = 45.0f;

    game->cam.vp.proj = perspective(radians(rotation),
                                    dimensions.x / dimensions.y, 0.1f, 100.0f);

#if 0
    if (gravity)
    {
        V3 x_z = convert_to_noise_coords(v2f(game->cam.pos.x, game->cam.pos.z));

        presist f32 sec_off_ground = 0.0f;

        f32 extra_padding = 0.25f;
        if (game->cam.pos.y <= x_z.y + extra_padding)
        {
            game->cam.pos.y = x_z.y + extra_padding;

#if 0
            V3 first_point =
                v3f(game->cam.pos.x + 0.5f, 0.0f, game->cam.pos.z);
            first_point.y =
                convert_to_noise_coords(v2f(first_point.x, first_point.z)).y +
                extra_padding;

            V3 second_point =
                v3f(game->cam.pos.x, 0.0f, game->cam.pos.z + 0.5f);
            second_point.y =
                convert_to_noise_coords(v2f(second_point.x, second_point.z)).y +
                extra_padding;

            V3 side0 = v3_sub(first_point, game->cam.pos);
            V3 side1 = v3_sub(second_point, game->cam.pos);
            V3 normal = v3_normalize(v3_cross(side1, side0));

            f32 angle = v3_angle(game->cam.vel, normal);
#endif

            game->cam.vel.x -= game->cam.vel.x * (dt);
            game->cam.vel.z -= game->cam.vel.z * (dt);

            // PRINT_V3(game->cam.vel);

            sec_off_ground = 0.0f;
            off_the_ground = false;
        }
        else
        {
            sec_off_ground += dt;
        }
        if (sec_off_ground >= 0.01f)
        {
            off_the_ground = true;
            // game->cam.vel.x -= 5.0f * game->cam.vel.x * dt;
            // game->cam.vel.z -= 5.0f * game->cam.vel.z * dt;
        }
    }
#endif

#if 0
    Vertex_Buffer* vert = &game->terrain_g_pipeline.vert_buffer;
#ifdef multithreaded

    HANDLE end_semaphore = terrain_threads[0].end_semaphore;
    HANDLE start_semaphore = terrain_threads[1].start_semaphore;
    for (u32 i = 0; i < MAX_TERRAIN_THREADS; i++)
    {
        WaitForSingleObject(end_semaphore, INFINITE);
    }
#else
    generate_terrain(0.0f, 0.0f, 0, CHUNK_SIZE_Z, vert->data);
#endif
    data_buffer_copy(&vert->buffer, vert->data, vert->buffer.size_bytes);
#ifdef multithreaded
    for (u32 i = 0; i < MAX_TERRAIN_THREADS; i++)
    {
        ReleaseSemaphore(start_semaphore, 1, 0);
    }
#endif
#endif

#if 1

    {
        presist b8 first_clicked = true;
        if (is_key_clicked(&first_clicked, SYNT_KEY_E))
        {
            b_switch(g_edit_mode_GAME);
        }
    }

    game->road_model = m4_translate(game->road_pos);

    Dynamic_Entity_3D dude =
        entity_dynamic_3d_access(&game->entity_state, game->dude);
    if (!g_edit_mode_GAME)
    {
        presist b8 first_clicked = false;
        if (game->mouse_evt->mouse_evt.button_evt.action == SYNT_BUTTON_PRESS &&
            game->mouse_evt->mouse_evt.button_evt.button == SYNT_RIGHT_BUTTON)
        {
            platform_cursor_hide(app_state->platform);

            static i16 last_x = 0;
            static i16 last_y = 0;
            V2 rotation_ =
                mouse_rotation_get(app_state->platform, game->cam.sens,
                                   &first_clicked, &last_x, &last_y, dt);

            dude.animation->angle -= rotation_.x * dt;
            cam_y_GAME += rotation_.y * dt;
        }
        else if (game->mouse_evt->mouse_evt.button_evt.action ==
                     SYNT_BUTTON_RELEASE &&
                 !first_clicked)
        {
            platform_cursor_show_last_pos(app_state->platform);
            first_clicked = true;
        }

        f32 rotation_speed = 1.0f;
        if (is_key_pressed(SYNT_KEY_UP))
        {
            cam_y_GAME += rotation_speed * dt;
        }
        if (is_key_pressed(SYNT_KEY_DOWN))
        {
            cam_y_GAME += -rotation_speed * dt;
        }

        assert(dude.movement);

        V3 dude_ori = v3_normalize(v3_rotate(v3f(0.0f, 0.0f, 1.0f),
                                             -dude.animation->angle,
                                             v3f(0.0f, 1.0f, 0.0f)));
        dude_ori.y = 0.0f;

        f32 movement_speed = dude.misc->speed;
        dude.movement->acc = v3d();
        {
            if (is_key_pressed(SYNT_KEY_SHIFT))
            {
                movement_speed *= speed_multiplier_GAME;
            }
            if (is_key_pressed(SYNT_KEY_W))
            {
                dude.movement->acc = v3_add(
                    dude.movement->acc, v3_s_multi(dude_ori, movement_speed));
            }

            if (is_key_pressed(SYNT_KEY_S))
            {
                dude.movement->acc = v3_add(
                    dude.movement->acc, v3_s_multi(dude_ori, -movement_speed));
            }
            if (is_key_pressed(SYNT_KEY_A))
            {
                dude.animation->angle += rotation_speed * dt;
            }
            if (is_key_pressed(SYNT_KEY_D))
            {
                dude.animation->angle += -rotation_speed * dt;
            }
            if (is_key_pressed(SYNT_KEY_Q))
            {
                V3 side_vector = v3_normalize(v3_cross(dude_ori, game->cam.up));
                dude.movement->acc =
                    v3_add(dude.movement->acc,
                           v3_s_multi(side_vector, -movement_speed));
            }
            presist b8 first_clicked_ = true;
            if (is_key_clicked(&first_clicked_, SYNT_KEY_SPACE))
            {
                dude.movement->acc = v3_add(dude.movement->acc,
                                            v3_s_multi(game->cam.up, 1000.0f));
            }
#if 0
            if (is_key_pressed(SYNT_KEY_E))
            {
                v3_add_equal(
                    &dude.movement->acc,
                    v3_s_multi(v3_normalize(v3_cross(
                                   v3f(dude_ori.x, 0.0f, dude_ori.z), game->cam.up)),
                               (movement_speed * dt)));
            }
#endif
            if (dude.animation->off_the_ground)
            {
                dude.movement->acc = v3_add(dude.movement->acc,
                                            v3_s_multi(game->cam.up, -15.0f));
            }
            f32 friction_multiplier = 8.0f;
            dude.movement->vel.x -=
                dude.movement->vel.x * friction_multiplier * dt;
            dude.movement->vel.z -=
                dude.movement->vel.z * friction_multiplier * dt;
        }
        {
            Dynamic_Entity_3D dude2 =
                entity_dynamic_3d_access(&game->entity_state, game->dude2);
            presist b8 lcick = false;
            presist b8 _clicked = true;
            presist f32 pr = 0.0f;
            presist b8 _clicked2 = true;
            presist b8 arc_show = false;
            presist b8 arc_particle_show = false;

            presist f32 particles_bounce = 0.0f;

            if (is_key_clicked(&_clicked2, SYNT_KEY_V))
            {
                b_switch(arc_show);
            }
            if (arc_show)
            {
                game->arc_model = m4_multi(m4_translate(dude.movement->pos),
                                           rotate_y(dude.animation->angle));
                game->boom_curve.p[0] = dude.movement->pos;
                game->boom_curve.p[1] =
                    v3_add(dude.movement->pos,
                           v3_s_multi(v3_rotate(dude_ori, radians(50.0f),
                                                v3f(0.0f, 1.0f, 0.0f)),
                                      20.0f));
                game->boom_curve.p[2] =
                    v3_add(dude.movement->pos,
                           v3_s_multi(v3_rotate(dude_ori, -radians(50.0f),
                                                v3f(0.0f, 1.0f, 0.0f)),
                                      20.0f));
                game->boom_curve.p[3] = dude.movement->pos;

                u32 count = 0;
                for (f32 i = 0.0f; i <= 1.0f; i += 0.006f)
                {
                    for (u32 j = 0; j < 20; j++)
                    {
                        f32 change =
                            array_val(game->particle_arc_offsets_change, count);
                        Particle_Attrib_3D attrib = { 0 };
                        attrib.position =
                            v3_array_val(&game->particle_arc_offsets, count++);
                        attrib.position.y +=
                            sinf(particles_bounce * change) * 0.1f;
                        attrib.color = v4f(1.0f, 0.0f, 0.0f, 1.0f);
                        attrib.size = v3i(0.02f);
                        particle_3d_emit(&game->particles, &attrib,
                                         v3f(0.0f, 0.0f, 0.0f), v3d(),
                                         random_f32(0.5f, 1.0f), dt);
                    }
                }

                particles_bounce += dt;
                if (is_key_clicked(&_clicked, SYNT_KEY_B))
                {
                    dude2.movement->pos = dude.movement->pos;
                    dude2.movement->vel = v3d();
                    dude2.movement->acc = v3d();
#if 0
                    dude2.movement->vel =
                        v3_s_multi(v3_rotate(dude_ori, radians(55.0f),
                                             v3f(0.0f, 1.0f, 0.0f)),
                                   35.0f);
#endif

                    dude2.animation->angle = dude.animation->angle;
                    lcick = true;
                    pr = 0.0f;
                }
                else if (lcick)
                {

#if 1
                    pr +=
                        dt * sy_lerp(0.3f, 0.7f,
                                     sinf(radians(sy_lerp(0.0f, 180.0f, pr))));
                    pr = clampf32(pr, 0.0f, 1.0f);

                    dude2.movement->pos =
                        brezier_curve_pos(&game->boom_curve, pr);
#else
                    pr += dude_delta * pr_multi;
                    pr = clampf32(pr, 0.0f, 1.0f);

                    V3 attraction_force =
                        v3_sub(dude.movement->pos, dude2.movement->pos);
                    dude2.movement->acc =
                        v3_s_multi(v3_normalize(attraction_force),
                                   sy_lerp(0.0f, 30.0f, pr));

                    V3 normal =
                        v3_normalize(v3_cross(v3f(0.0f, 1.0f, 0.0f), dude_ori));

                    V3 tangent_force =
                        v3_s_multi(normal, sy_lerp(35.0f, 0.0f, pr));

                    v3_add_equal(&dude2.movement->acc, tangent_force);
                    V3 damping_force = v3_s_multi(dude2.movement->vel, -2.8f);
                    v3_add_equal(&dude2.movement->acc, damping_force);
#endif

                    dude2.animation->angle += 5.0f * dt;
                }
                else
                {
                    pr = 0.0f;
                }
            }
            u32 cube_index_size = 36;
            u32 particle_size = particles_3d_update(
                &game->particles, &game->particles_vert_array, 0, dt);

            frame->game_particle_count = (particle_size * cube_index_size);

            assert(frame->game_particle_count <
                   frame->game_particles_offsets.idx_size);

            data_buffer_copy(&frame->game_particles_staging_buffer,
                             game->particles_vert_array.data,
                             (particle_size * 8) * sizeof(Vertex));
        }

        {
            presist f32 cam_distance = 8.0f;

            if (game->wheel_evt->activated && !gui_is_focus())
            {
                f32 scroll_speed = 1.0f;
                if (game->wheel_evt->wheel_evt.z_delta > 0)
                {
                    scroll_speed *= -1.0f;
                }
                cam_distance += scroll_speed;
            }
            cam_distance = clampf32_low(cam_distance, 1.0f);
            cam_y_GAME = clampf32(cam_y_GAME, 0.0f, radians(70.0f));
            V3 cam_pos = v3f(0.0f, 0.0f, -cam_distance);
            M4 cam_rotate_model =
                m4_multi(rotate_y(dude.animation->angle), rotate_x(cam_y_GAME));

            cam_rotate_model =
                m4_multi(m4_translate(dude.movement->pos), cam_rotate_model);

            cam_pos = m4_v3_multi(cam_rotate_model, cam_pos);
            {
#if 1
                V3 terrain_coords_camera =
                    convert_to_noise_coords(v2f(cam_pos.x, cam_pos.z));

                terrain_coords_camera.y += 0.8f;
                if (cam_pos.y <= terrain_coords_camera.y)
                {
                    cam_y_GAME += 0.02f;
                }
#endif
            }
            camera_move(&game->cam, cam_pos, dude.movement->pos, cam_distance);
        }
    }
    V3 ray;
    {
        i16 x, y;
        platform_mouse_get_pos(&x, &y);
        V3 mouse_pos = v3f((f32)x, (f32)y, 0.0f);

        mouse_pos = mouse_to_device_coords(mouse_pos, dimensions);

        ray = shoot_camera_ray(game->cam.vp, mouse_pos);
    }
    array_head(game->sign_constants)->size = 0;
    u32 i = 1;
    Dynamic_Entity_3D e = entity_dynamic_3d_iterate(&game->entity_state, i);
    V3 pos_to_follow = g_edit_mode_GAME ? game->cam.pos : dude.movement->pos;
    for (; e.movement; e = entity_dynamic_3d_iterate(&game->entity_state, ++i))
    {
        if (v3_distance(pos_to_follow, e.movement->pos) < distance_sign)
        {
            V3 sign_ori = v3_normalize(v3_sub(game->cam.pos, e.movement->pos));
            f32 yaw = atan2f(sign_ori.x, sign_ori.z);
            f32 pitch = asinf(sign_ori.y);
            M4 rotate_model = m4_multi(rotate_y(yaw), rotate_x(-pitch));

            Push_Constant push_constant;
            push_constant.model =
                m4i(1.0f); // m4_multi(
                           // m4_translate(v3f(e.movement->pos.x,
                           // e.movement->pos.y + 2.0f,
                           //                 e.movement->pos.z)),
            // rotate_model);

            aabb_update(game->sign_aabb, push_constant.model,
                        &game->aabb_rep.vert.array, game->aabb_count++);

            aabb_update(game->sign_aabb_text, push_constant.model,
                        &game->aabb_rep.vert.array, game->aabb_count++);

            b32 ray_hit_yes = false;
            b32 ray_hit_no = false;

            presist b8 first_clicked = false;
            b32 left_button_pressed =
                !gui_is_focus() &&
                game->mouse_evt->mouse_evt.button_evt.action ==
                    SYNT_BUTTON_PRESS &&
                game->mouse_evt->mouse_evt.button_evt.button ==
                    SYNT_LEFT_BUTTON;
            b32 button_clicked = false;
            if (left_button_pressed && !first_clicked)
            {
                first_clicked = true;
                button_clicked = true;
            }
            else if (!left_button_pressed)
            {
                first_clicked = false;
            }

            AABB_3D yes =
                aabb_update(game->sign_aabb_yes, push_constant.model,
                            &game->aabb_rep.vert.array, game->aabb_count++);
            ray_hit_yes = ray_hit_target_aabb(ray, game->cam.pos, yes);
            if (ray_hit_yes && button_clicked)
            {
                g_edit_mode_GAME = false;
            }

            AABB_3D no =
                aabb_update(game->sign_aabb_no, push_constant.model,
                            &game->aabb_rep.vert.array, game->aabb_count++);
            ray_hit_no = ray_hit_target_aabb(ray, game->cam.pos, no);
            if (ray_hit_no && button_clicked)
            {
                g_edit_mode_GAME = true;
            }

            push_constant.normal = inverse(rotate_model);

            array_push(game->sign_constants, push_constant);
        }
    }
    update_dudes_position(&game->entity_state, game->road_pos, dt);

    if (game->aabb_count)
    {
        const u32 size_bytes = (game->aabb_count * 8) * sizeof(Vertex);
        data_buffer_copy(&game->aabb_rep.vert.buffer,
                         game->aabb_rep.vert.array.data, size_bytes);
    }

#endif

    const u32 sign_constant_count = array_size(game->sign_constants);
    frame->game_sign_constants =
        region_array(&frame->frame_region, sign_constant_count, Push_Constant);
    for (u32 j = 0; j < sign_constant_count; j++)
    {
        array_push(frame->game_sign_constants,
                   array_val(game->sign_constants, j));
    }
    const u32 dude_count = array_size(game->entity_state.animations);
    frame->game_dude_models =
        region_array(&frame->frame_region, dude_count, M4*);
    i = 0;
    Entity_Animation_3D* e_animation =
        entity_animation_3d_iterate(&game->entity_state, i);
    for (; e_animation;
         e_animation = entity_animation_3d_iterate(&game->entity_state, ++i))
    {
        array_push(frame->game_dude_models,
                   region_array(&frame->frame_region, 3, M4));
        for (u32 j = 0; j < 3; j++)
        {
            array_push(array_val(frame->game_dude_models, i),
                       e_animation->dude_models[j]);
        }
    }
    frame->game_cam_vp = game->cam.vp;
    frame->game_aabb_count = game->aabb_count;
    frame->game_aabb_indices_count = game->aabb_indices_count;
    frame->game_offset_p_grass = game->offset_p;
    frame->game_arc_model = game->arc_model;

    game->dimensions = dimensions;

    Render_Task task = { .callback = game_render, .data = frame };
    array_push(frame->render_tasks, task);
    task = (Render_Task){ .callback = game_copy_buffer, .data = frame };
    array_push(frame->copy_tasks, task);

    game_update_gui(game, gui_ctx, app_state->fps, dt, dimensions);
}

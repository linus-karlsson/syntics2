#ifndef SY_INCLUDES // only for clangd
#include "syntics.h"
#endif

#define GUI_MULTI_THREADED

#define LINES
// #define MOVE_ALL
#define MAX_PARTICLES 4800

#define GRASS_WIDTH 100
#define GRASS_DEPTH 100
#define MAX_GRASS GRASS_WIDTH* GRASS_DEPTH
#define GRASS_RADIUS 0.2f

#define pack(d, v0, v1, v2)                                                         \
    do                                                                              \
    {                                                                               \
        ASSERT(v0 < 2 && v1 < 0x1FFFFFFF && v2 < 4, "pack to big values");          \
        (d) = ((u32)(v0) << 31) | ((u32)(v1) << 2) | ((u32)(v2)&0x3);               \
    } while (0)

#define unpack_side(d) ((d) >> 31)
#define unpack_curve(d) (((d) >> 2) & 0x1FFFFFFF)
#define unpack_point(d) ((d)&0x3)

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

// NOTE: Not very efficient but usually is only done on small number of indices and
// only computed once
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

global Game_State g_state_GAME;

#define DEFAULT_TEXTURE_GAME 0
#define OBJ_TEXTURE_GAME 1

void aabb_check_min_max(AABB_3D* aabb, V3 pos, V3* current_max)
{
    assert(aabb);
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

AABB_3D vertices_extract(const Obj_Load_Attrib* loader, f32 tex_index, V3 pos_offset,
                         Vertex_Array* vert_array, U32_Array* index_array,
                         b8 use_hash)
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
            vertex.tex_coords.y = 1.0f - loader->tex_coords[current_tex_index].y;
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
#define CHUNK_SIZE_X 200
#define CHUNK_SIZE_Y 1
#define CHUNK_SIZE_Z 200

#define CHUNK_SIZE CHUNK_SIZE_X* CHUNK_SIZE_Y* CHUNK_SIZE_Z

#define MAX_TERRAIN_THREADS 1
#define multithreaded

global const f32 QUAD_WIDTH = 0.5f;
global const f32 QUAD_DEPTH = 0.5f;
global const f32 OFFSET_INCREASE = 0.1f;

global f32 g_freq = 0.41f;
global f32 g_grain = 0.36f;
global f32 g_oct = 3.0f;
global f32 g_max_height = 8.0f;

u32 game_index_offset = 0;

#if 1
V3 convert_to_noise_coords(V2 x_z)
{
    V3 out = v3f((x_z.x * OFFSET_INCREASE) / QUAD_WIDTH, 0.0f,
                 (x_z.y * OFFSET_INCREASE) / QUAD_DEPTH);

    out.y =
        (sy_value_noise2d(out.x, out.z, g_freq, g_grain, (i32)g_oct) * g_max_height);

    return out;
}
#endif

f32 noise_min_max(f32 x_offset, f32 z_offset, f32 freq, f32 grain, i32 oct, f32 min,
                  f32 max)
{
    f32 noise =
        (sy_value_noise2d(x_offset, z_offset, freq, grain, oct) * (max - min)) + min;
    return noise;
}

f32 round_down_to_half(f32 value)
{
    return value - fmodf(value, 0.5);
}

global Thread_Attrib_Terrain terrain_threads[MAX_TERRAIN_THREADS] = { 0 };

#define MAX_GRASS_THREADS 4
static_assert(MAX_GRASS % MAX_GRASS_THREADS == 0);

global Thread_Attrib_Grass grass_threads[MAX_GRASS_THREADS] = { 0 };

void generate_terrain(f32 x_off, f32 z_off, u32 z_chunk_offset, u32 z_chunks,
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
            V4 color = v4i(1.0f);
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

static_assert(CHUNK_SIZE_Z % MAX_TERRAIN_THREADS == 0);
#define chunks CHUNK_SIZE_Z / MAX_TERRAIN_THREADS

volatile u32 check_thread_count = 0;

void generate_terrain_threaded(void* data)
{
    Thread_Attrib_Terrain* attrib = (Thread_Attrib_Terrain*)data;
    u32 z_chunk_offset = attrib->index * chunks;
    f32 z_off = (f32)z_chunk_offset * 0.1f;
    generate_terrain(0.0f, z_off, z_chunk_offset, chunks, attrib->verts);
}

global V2 g_wind = { 0.0f, 35.0f };

global V2 g_wind_direction = { 0.8f, 0.0f };

global f32 grass_freq = 1.5f;
global f32 grass_grain = 1.0f;
global f32 grass_oct = 2.0f;

global f32 grass_wind_speed = 1.5f;

void grass_generation(u32 seed, const u32 offset, const u32 iterations,
                      const u32 vertices_count, const u32 indices_count,
                      const Vertex* model_vertices, const u32* model_indices,
                      Vertex* vertices, u32* indices)
{

    // u32 cache_index = offset * vertices_count * 2;
    const f32 min_scale = 0.8f;
    const f32 max_scale = 3.0f;
    const f32 max_y = model_vertices[vertices_count - 1].pos.y * max_scale;
    u32 count = offset;
    __m128 _pos_xyz[3], _pos_offset_xyz[3], _fx, _fy, _fz, _res;
    for (u32 i = 0; i < iterations; i++)
    {
        V3 vertex_pos_offset = v3_random(seed++, 0.0f, GRASS_DEPTH * 0.037f);

        vertex_pos_offset.y =
            convert_to_noise_coords(v2f(vertex_pos_offset.x, vertex_pos_offset.z)).y;

#if 1
        for (u32 j = 0; j < 3; j++)
        {
            _pos_offset_xyz[j] = _mm_set1_ps(vertex_pos_offset.data[j]);
        }
#endif
        f32 freq = 1.5f;
        f32 grain = 1.0f;
        i32 oct = 3;
        f32 noise_value = noise_min_max(vertex_pos_offset.x, vertex_pos_offset.z,
                                        freq, grain, oct, min_scale, max_scale);

        V3 gen_scale = v3f(1.0f, noise_value, 1.0f);
        f32 random = random_f32s(seed++, 2.0f, 4.0f);
        M4 matrix = m4_scale(gen_scale);
#if 1
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
            f32 res_xyz[3][4];
            f32 res_xyz_offset[3][4];
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
            V3 lerped_color =
                v3_lerp(v3f(0.0f, sy_RGB(100.0f), 0.0f),
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
        u32 idx_offset = count;
        for (u32 k = 0; k < indices_count; k++)
        {
            *indices = model_indices[k] + idx_offset;
            indices++;
        }
    }
}

#if 1
void grass_generation_threaded(void* data)
{
    Thread_Attrib_Grass* attrib = (Thread_Attrib_Grass*)data;

    const u32 grass_count =
        attrib->vertex_array._capacity / attrib->model_vertices.size;

    const u32 offset = attrib->index * attrib->vertex_array._capacity;

    grass_generation(attrib->seed, offset, grass_count, attrib->model_vertices.size,
                     attrib->model_indices.size, attrib->model_vertices.data,
                     attrib->model_indices.data, attrib->vertex_array.data,
                     attrib->indices_array.data);
}
#endif

void normal_generate()
{
    Vertex_Array* vert = &g_state_GAME.terrain_vert_idx.vert.array;
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

    ASSERT((current_pos - buffer) + (bezier_curves_size0 + bezier_curves_size1) <=
               size,
           "");

    for (u32 i = 0; i < spline->n_curves; i++)
    {
        for (u32 j = 0; j < 4; j++)
        {
            memcpy(current_pos, &spline->bc[0][i].p[j], sizeof(V3));
            current_pos += sizeof(V3);
            memcpy(current_pos, &spline->bc[0][i].points_indices[j], sizeof(u32));
            current_pos += sizeof(u32);
        }
    }
    for (u32 i = 0; i < spline->n_curves; i++)
    {
        for (u32 j = 0; j < 4; j++)
        {
            memcpy(current_pos, &spline->bc[1][i].p[j], sizeof(V3));
            current_pos += sizeof(V3);
            memcpy(current_pos, &spline->bc[1][i].points_indices[j], sizeof(u32));
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

void game_save_binary1(const Vertex_Array* vert_array, const U32_Array* index_array,
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

global u32 circle_offset = 0;
global u32 circle_curr_size = 0;

void game_render(void* data, VkCommandBuffer command_buffer, u32 semaphore_idx)
{
    // NOTE: REMEMBER TO COPY UNIFORM BUFFERS
    //
    // NOTE: same for every draw call at the moment
    V2* dimensions = (V2*)data;
    VkViewport view_port = { 0 };
    view_port.x = 0.0f;
    view_port.y = 0.0f;
    view_port.width = dimensions->width;
    view_port.height = dimensions->height;
    view_port.maxDepth = 1.0f;

    VkRect2D scissor_internal = { { (i32)view_port.x, (i32)view_port.y },
                                  { (u32)view_port.width, (u32)view_port.height } };
    vkCmdSetViewport(command_buffer, 0, 1, &view_port);
    vkCmdSetScissor(command_buffer, 0, 1, &scissor_internal);

    /////// TRIANGLE STRIP ////////////////
    graphics_pipline_bind(command_buffer, &g_state_GAME.triangle_strip_pipeline,
                          semaphore_idx);

    push_constant(command_buffer, g_state_GAME.triangle_strip_pipeline.layout,
                  &g_state_GAME.global_model, sizeof(M4));
    // Terrain draw
    vertex_index_buffer1_bind(command_buffer, &g_state_GAME.terrain_vert_idx);
    draw(command_buffer, 0, g_state_GAME.terrain_vert_idx.idx.curr_size);

    // Road draw
    push_constant(command_buffer, g_state_GAME.triangle_strip_pipeline.layout,
                  &g_state_GAME.road_model, sizeof(M4));
    vertex_index_buffer1_bind(command_buffer, &g_state_GAME.road_vert_idx);
    draw(command_buffer, 0, g_state_GAME.road_vert_idx.idx.curr_size);

    //////// TRIANGLE LIST ////////////////
    graphics_pipline_bind(command_buffer, &g_state_GAME.triangle_list_pipeline,
                          semaphore_idx);

    push_constant(command_buffer, g_state_GAME.triangle_list_pipeline.layout,
                  &g_state_GAME.global_model, sizeof(M4));

    // Particles draw
    vertex_index_buffer1_bind(command_buffer, &g_state_GAME.particles_vert_idx);
    draw(command_buffer, 0, g_state_GAME.particles_vert_idx.idx.curr_size);

    // Car draw
#if 1
    push_constant(command_buffer, g_state_GAME.triangle_list_pipeline.layout,
                  &g_state_GAME.car_model, sizeof(M4));
    vertex_index_buffer1_bind(command_buffer, &g_state_GAME.car_vert_idx);
    draw(command_buffer, 0, g_state_GAME.car_vert_idx.idx.curr_size);
#endif
    // Grass draw
#if 1
    graphics_pipline_bind(command_buffer, &g_state_GAME.grass_pipeline,
                          semaphore_idx);

    Push_Constant push;
    push.model = g_state_GAME.global_model;
    push.offset_p = g_state_GAME.offset_p;
    push_constant(command_buffer, g_state_GAME.grass_pipeline.layout, &push,
                  sizeof(Push_Constant));
#endif
    vertex_index_buffer1_bind(command_buffer, &g_state_GAME.grass_vert_idx);
    draw(command_buffer, 0, g_state_GAME.grass_vert_idx.idx.curr_size);

    /////// LINE LIST ////////////////////////
#ifdef LINES
    graphics_pipline_bind(command_buffer, &g_state_GAME.line_list_pipeline,
                          semaphore_idx);

    // Spline draw
    push_constant(command_buffer, g_state_GAME.line_list_pipeline.layout,
                  &g_state_GAME.road_model, sizeof(M4));
    vertex_index_buffer1_bind(command_buffer, &g_state_GAME.road_line_vert_idx);
    draw(command_buffer, 0, circle_curr_size);
    draw(command_buffer, circle_offset,
         g_state_GAME.road_line_vert_idx.idx.curr_size - circle_offset);

#if 0
    // car aabb
    push_constant(command_buffer, g_state_GAME.line_list_pipeline.layout,
                      g_state_GAME.car_model);
    vertex_index_buffer1_bind(command_buffer, &g_state_GAME.aabb_rep);
    draw(command_buffer, 0, g_state_GAME.aabb_rep.idx.curr_size);
#endif
#endif
}

void game_recreate(void* data, const Application_State* app_state)
{
    graphic_pipline_ap_recreate(app_state,
                                "Syntics/res/shaders/spv/game_grass.vert.spv",
                                "Syntics/res/shaders/spv/game_grass.frag.spv",
                                &g_state_GAME.triangle_list_pipeline,
                                array_size(g_state_GAME.textures), NULL);

    graphic_pipline_ap_recreate(app_state, "Syntics/res/shaders/spv/game.vert.spv",
                                "Syntics/res/shaders/spv/game.frag.spv",
                                &g_state_GAME.triangle_strip_pipeline,
                                array_size(g_state_GAME.textures), NULL);

#if 1
    graphic_pipline_ap_recreate(app_state,
                                "Syntics/res/shaders/spv/game_grass.vert.spv",
                                "Syntics/res/shaders/spv/game_grass.frag.spv",
                                &g_state_GAME.grass_pipeline, 1, NULL);
#endif
}

// Brezier_Spline spline = {};
Bezier_Spline_3D spline2 = { 0 };

void game_destroy(void* data, VkDevice device, u32 num_semaphores)
{
#if 0
    save_game_binary(test.line_g_pipeline.vert_buffer.data,
                     test.line_g_pipeline.idx_buffer.data, spline2);
#endif
    graphic_pipeline_destroy(device, num_semaphores,
                             &g_state_GAME.triangle_strip_pipeline);
    graphic_pipeline_destroy(device, num_semaphores,
                             &g_state_GAME.triangle_list_pipeline);
#ifdef LINES
    graphic_pipeline_destroy(device, num_semaphores,
                             &g_state_GAME.line_list_pipeline);

    buffer_destroy(device, g_state_GAME.road_line_vert_idx.vert.buffer);
    buffer_destroy(device, g_state_GAME.road_line_vert_idx.idx.buffer);

    buffer_destroy(device, g_state_GAME.aabb_rep.vert.buffer);
    buffer_destroy(device, g_state_GAME.aabb_rep.idx.buffer);
#endif
    buffer_destroy(device, g_state_GAME.terrain_vert_idx.vert.buffer);
    buffer_destroy(device, g_state_GAME.terrain_vert_idx.idx.buffer);

    buffer_destroy(device, g_state_GAME.road_vert_idx.vert.buffer);
    buffer_destroy(device, g_state_GAME.road_vert_idx.idx.buffer);

    /*
    buffer_destroy(device, g_state_GAME.car_vert_idx.vert.buffer);
    buffer_destroy(device, g_state_GAME.car_vert_idx.idx.buffer);
    */

    buffer_destroy(device, g_state_GAME.particles_vert_idx.vert.buffer);
    buffer_destroy(device, g_state_GAME.particles_vert_idx.idx.buffer);

    for (u32 i = 0; i < array_size(g_state_GAME.textures); i++)
    {
        texture_destroy(device, g_state_GAME.textures[i]);
    }

    gui_destroy(&g_state_GAME.gui_ctx, device, num_semaphores);
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
        V3 p = v3_add(pos, (v3_s_multi(v3f(cosf(radians(i)), sinf(radians(i)), 0.0f),
                                       radius)));

        Vertex vertex =
            vertex_create(p, v3d(), v2d(), v4i(1.0f), DEFAULT_TEXTURE_GAME);

        vertex_array_val(vert_array, offset++) = vertex;
    }
    return offset;
}

u32 spline_circles_curve_create(Vertex_Array* vert_array, u32 offset,
                                Bezier_Spline_3D* spline, u32 curve, f32 radius)
{
    for (u32 k = 0; k < 2; k++)
    {
        for (u32 j = 0; j < 4; j++)
        {
            Rect3D rect = { 0 };
            rect.pos = spline->bc[k][curve].p[j];
            rect.size = v3i(radius);
            pack(rect.id, k, curve, j);
            u32 size = array_size(g_state_GAME.rects);
            b8 found = false;
            for (u32 i = 0; i < size; i++)
            {
                Rect3D* rect_ref = g_state_GAME.rects + i;
                if (rect_ref->id == rect.id)
                {
                    *rect_ref = rect;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                array_push(g_state_GAME.rects, rect);
            }

            spline->bc[k][curve].points_indices[j] = offset;
            offset =
                circle_create(vert_array, offset, spline->bc[k][curve].p[j], radius);
        }
    }
    return offset;
}

u32 spline_2d_circles_create(Vertex_Array* vert_array, u32 offset,
                             Bezier_Spline* spline, f32 radius)
{
    for (u32 i = 0; i < spline->n_curves; i++)
    {
        for (u32 j = 0; j < 4; j++)
        {
            Rect3D rect = { 0 };
            rect.pos = spline->bc[i].p[j];
            rect.size = v3i(radius);
            // pack(rect.id, i, j);
            array_push(g_state_GAME.rects, rect);

            spline->bc[i].points_indices[j] = offset;
            offset = circle_create(vert_array, offset, spline->bc[i].p[j], radius);
        }
    }
    return offset;
}

u32 spline_3d_circles_create(Vertex_Array* vert_array, u32 offset,
                             Bezier_Spline_3D* spline, f32 radius)
{
    for (u32 i = 0; i < spline->n_curves; i++)
    {
        offset = spline_circles_curve_create(vert_array, offset, spline, i, radius);
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
        Vertex vertex = vertex_create(brezier_curve_pos(&brezier_curve, i), v3d(),
                                      v2d(), v4i(1.0f), DEFAULT_TEXTURE_GAME);
        vertex_array_val(vert_array, count++) = vertex;
    }
    return count;
}

V3 generate_positions_curve(Bezier_Spline_3D* spline, V3 direction, V3 pos, u32 side,
                            u32 curve)
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
            pos = generate_positions_curve(spline, v3f(0.5f, 1.0f, 0.0f), last_pos,
                                           k, i);
            last_pos.z += 4.0f;
        }
        pos.z -= 4.0f;
    }
}

u32 spline_generate(Bezier_Spline_3D* spline, Vertex_Array* vert_array, u32 offset)
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

u32 normals_curve_generate(Vertex_Array* vert_array, u32 offset, u32 spline_splitt,
                           i32 half_splitt, u32 first_index, u32 second_index)
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
    next_pos[second_index] = vertex_array_val(vert_array, count + half_splitt).pos;
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

void generate_spline_curve(Bezier_Spline_3D* spline, u32 side, u32 curve)
{
    i32 half_splitt = spline->splitt / 2;
    // (spline->n_curves * 8 * 10) for the circle representation
    // u32 offset = (((u32)half_splitt) * side) + (curve * spline->splitt) +
    //            (spline->n_curves * 8 * 10);
    u32 offset = spline->bc[side][curve].vertex_offset;

    Vertex_Array* vert_array = &g_state_GAME.road_line_vert_idx.vert.array;
    u32 n = curve_generate(spline->bc[side][curve], vert_array, offset);

    if (side == 0)
    {
        normals_curve_generate(vert_array, offset, spline->splitt, half_splitt, 0,
                               1);
    }
    else
    {
        normals_curve_generate(vert_array, offset, spline->splitt, -half_splitt, 1,
                               0);
    }

    u32 offset2 = ((spline->splitt / 2) * side) + (curve * spline->splitt);
    Vertex_Array* vert_array2 = &g_state_GAME.road_vert_idx.vert.array;

    for (u32 i = offset; i < n; i++)
    {
        vertex_array_val(vert_array2, offset2++) = vertex_array_val(vert_array, i);
    }
}

void spline_generate_at_curve(Bezier_Spline_3D* spline, u32 side, u32 curve,
                              u32 point, V3 pos)
{
    spline->bc[side][curve].p[point] = pos;
    generate_spline_curve(spline, side, curve);
}

void spline_generate_at_curve1(Bezier_Spline_3D* spline, u32 curve)
{
    for (u32 i = 0; i < 2; i++)
    {
        generate_spline_curve(spline, i, curve);
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

u32 generate_spline1(Bezier_Spline* spline, Vertex_Array* vert_array, u32 offset)
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

void generate_spline_at_curve2(Bezier_Spline* spline, u32 curve, u32 point, V3 pos)
{
    spline->bc[curve].p[point] = pos;

    // (spline->n_curves * 4 * 10) for the circle representation
    u32 offset = (curve * spline->splitt) + (spline->n_curves * 4 * 10);
    Vertex_Array* vert_array = &g_state_GAME.road_line_vert_idx.vert.array;
    curve_generate(spline->bc[curve], vert_array, offset);
}

f32 get_procent(Bezier_Spline sp, f32 t)
{
    return t * sp.n_curves;
}

void generate_indices_terrain(U32_Array* index_array)
{
    int32 I = 0;
    int32 step_value = 1;
    for (u32 i = 0; i < CHUNK_SIZE_Z - 1; i++)
    {
        for (u32 j = 0; j < CHUNK_SIZE_X; j++)
        {
            u32_array_push(index_array, (CHUNK_SIZE_X * i) + I);
            u32_array_push(index_array, (CHUNK_SIZE_X * (i + 1)) + I);

            I += step_value;
        }
        step_value *= -1;
        I += step_value;
    }
}

global f32 translucentcy_GAME = 0.8f;
global b32 wire_frame_GAME = false;

global V3 scaling_value_GAME = { 1.0f, 1.0f, 1.0f };

global b8 reset_index_GAME = false;

global b8 show_particles_GAME = false;
global b8 emit_particle_GAME = false;

global b8 g_edit_mode_GAME = true;

global b8 grass_mode = true;

global u32 index_to_test = 0;
global u32 num_points = 0;
global u32 points_size = ((u32)(1.0f / PROCENT_INCREASE) + 1) * 2;

void game_update_gui(const Application_State* app_state, f32 dt, V2 dimensions)
{
    Ui_Window* win = window_begin(&g_state_GAME.gui_ctx, g_state_GAME.win_handles[0],
                                  "First thing", v2f(10.0f, 10.0f));
    {
        window_gridd_begin(win, 1, 1);
        {
            presist char buffer[100] = { 0 };
            u32 size = 0;
            if (window_text_input_add(win, buffer, &size))
            {
                buffer[size] = '\0';
                sy_print("%s\n", buffer);
            }
        }
        window_gridd_end(win);
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
        window_gridd_begin(win, 2, 3);
        {
            if (window_button_add(win, "Wire Frame"))
            {
                if (!wire_frame_GAME)
                {
                    g_state_GAME.triangle_list_pipeline.poly_mode =
                        VK_POLYGON_MODE_LINE;
                    g_state_GAME.triangle_strip_pipeline.poly_mode =
                        VK_POLYGON_MODE_LINE;
                }
                else
                {
                    g_state_GAME.triangle_list_pipeline.poly_mode =
                        VK_POLYGON_MODE_FILL;
                    g_state_GAME.triangle_strip_pipeline.poly_mode =
                        VK_POLYGON_MODE_FILL;
                }
                b_switch(wire_frame_GAME);
                game_recreate(NULL, app_state);
            }
            if (window_button_add(win, "Save spline"))
            {
                game_save_binary0(&spline2, g_state_GAME.road_pos);
                game_save_binary1(&g_state_GAME.road_line_vert_idx.vert.array,
                                  &g_state_GAME.road_line_vert_idx.idx.array,
                                  &spline2, g_state_GAME.road_pos);
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
        }
        window_gridd_end(win);
        window_gridd_begin(win, 2, 1);
        {
            if (window_button_add(win, "Add curve"))
            {
                if (current_curve_count < spline2.n_curves)
                {
                    Vertex_Buffer* vert = &g_state_GAME.road_line_vert_idx.vert;
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
                        &g_state_GAME.road_vert_idx.vert.buffer,
                        g_state_GAME.road_vert_idx.vert.array.data,
                        g_state_GAME.road_vert_idx.vert.buffer.size_bytes);

                    g_state_GAME.road_line_vert_idx.idx.curr_size +=
                        (points_size * 2) - 4;
                    g_state_GAME.road_vert_idx.idx.curr_size += points_size;
                    circle_curr_size += 8 * (10 * 2);

                    current_curve_count++;
                }
            }
            if (window_button_add(win, "Remove curve"))
            {
                if (current_curve_count > 0)
                {
                    g_state_GAME.road_line_vert_idx.idx.curr_size -=
                        (points_size * 2) - 4;
                    g_state_GAME.road_vert_idx.idx.curr_size -= points_size;
                    circle_curr_size -= 8 * (10 * 2);
                    current_curve_count--;
                }
            }
        }
        window_gridd_end(win);

        window_gridd_begin(win, 1, 1);
        {
            window_text_add(win, "Sheer");
        }
        window_gridd_end(win);

        window_gridd_begin(win, 1, 1);
        {
            window_text_add(win, "Position (x, y, z) This is a test");
        }
        window_gridd_end(win);

#if 1
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

        window_gridd_begin(win, 1, 1);
        {
            if (window_button_add(win, "Circle toggle"))
            {
                game_index_offset = game_index_offset == 0 ? index_to_test : 0;
            }
        }
        window_gridd_end(win);

        window_gridd_begin(win, 1, 1);
        {
            presist char temp[60] = { 0 };
            presist f32 count = 1.0f;
            if (count >= 0.1f)
            {
                f32 milli = dt * 1000.0f;
                sprintf_s(temp, sizeof(temp), "Milli: %f | FPS: %u", milli,
                          app_state->fps);
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
                sprintf_s(temp + len, sizeof(temp) - len,
                          V3_FMT(g_state_GAME.cam.pos));

                sprintf_s(temp1 + len, sizeof(temp1) - len,
                          V3_FMT(g_state_GAME.cam.ori));

                count = 0.0f;
            }
            count += dt;
            window_text_add(win, temp);
            window_text_add(win, temp1);
        }
        window_gridd_end(win);

        window_gridd_begin(win, 1, 1);
        {
            window_text_add(win, "grass_Freq --- grass_Grain --- grass_Oct");
        }
        window_gridd_end(win);

        window_gridd_begin(win, 3, 1);
        {
            window_input_float_add(win, &grass_freq, 0.0f, 10.0f, 1.0f);
            window_input_float_add(win, &grass_grain, 0.0f, 10.0f, 1.0f);
            window_input_float_add(win, &grass_oct, 0.0f, 10.0f, 1.0f);
        }
        window_gridd_end(win);

        window_gridd_begin(win, 2, 1);
        {
            window_text_add(win, "Wind speed: ");
            window_input_float_add(win, &grass_wind_speed, 0.0f, 10.0f, 1.0f);
        }
        window_gridd_end(win);

        window_gridd_begin(win, 2, 2);
        {
            window_text_add(win, "Wind min: ");
            window_input_float_add_d(win, &g_wind.min, -180.0f, 180.0f);
            window_text_add(win, "Wind max: ");
            window_input_float_add_d(win, &g_wind.max, 0.0f, 180.0f);
        }
        window_gridd_end(win);

        window_gridd_begin(win, 2, 2);
        {
            window_text_add(win, "Wind X: ");
            if (window_input_float_add_d(win, &g_wind_direction.min, 0.0f, 1.0f))
            {
                int i = 0;
                i++;
            }
            window_text_add(win, "Wind Y: ");
            window_input_float_add_d(win, &g_wind_direction.max, 0.0f, 1.0f);
        }
        window_gridd_end(win);
    }
    window_end(&win);

    win = window_begin(&g_state_GAME.gui_ctx, g_state_GAME.win_handles[1],
                       "Terminal", v2f(500.0f, 100.0f));
    {
        terminal_add(&g_state_GAME.gui_ctx, terminal_ptr_get(), win, 250.0f, 200.0f);
    }
    window_end(&win);
}

volatile u32 check_value_GAME = 0;
unsigned long game_update_gui_threaded(void* data)
{
    Thread_Attrib_Gui* attrib = (Thread_Attrib_Gui*)data;
    for (;;)
    {
        WaitForSingleObject(attrib->start_semaphore, INFINITE);

        gui_update_begin(attrib->ctx, attrib->dimensions, attrib->semaphore_idx,
                         attrib->dt);
        game_update_gui(attrib->app_state, attrib->dt, attrib->dimensions);

        InterlockedIncrement((LONG volatile*)&check_value_GAME);

        ReleaseSemaphore(attrib->end_semaphore, 1, 0);
    }
}

u32 cell_index_get(V3 pos, f32 cell_size, u32 columns)
{
    u32 cell_index = ((u32)(pos.z / cell_size) * columns) + (u32)(pos.x / cell_size);
    return cell_index;
}

// Inspiration from:
// Fast Poisson Disk Sampling in Arbitrary Dimensions
//              Robert Bridson
//      University of British Columbia
//
// TODO: Check neighbor cells if position is ok.
//
void blue_noise(Region_Alloc* region, u32 seed, const u32 rows, const u32 columns,
                const f32 minimum_distance, V3** positions)
{
    f64 start = platform_get_time();
    const f32 extent_of_sample_domain = 2.0f;
    const f32 k = 30.0f;
    const f32 cell_size = inverse_sqrt(extent_of_sample_domain) * minimum_distance;
    const f32 max_z = cell_size * (f32)rows;
    const f32 max_x = cell_size * (f32)columns;
    const u32 max_count = rows * columns;

    u32* gridd_cells = region_array_calloc(region, max_count, u32);
    *positions = region_array_calloc(region, max_count + 1, V3);

    V3 pos = v3_random(seed++, 0.0f, cell_size * 0.9f);
    pos.y = 0.0f;

    u32 cell_index = cell_index_get(pos, cell_size, columns);
    assert(cell_index == 0);

    // first position is used as a empty spot
    array_head((*positions))->size++;

    u32 index = array_head((*positions))->size++;
    array_val((*positions), index) = pos;
    array_val(gridd_cells, cell_index++) = index;

    u32* active_indices = region_array(region, max_count, u32);
    for (u32 i = 0; i < max_count; i++)
    {
        active_indices[i] = 1;
    }
    Array_Head* active_list_head = array_head(active_indices);
    active_list_head->size = max_count;

    const i32 circle_index_table[] = {
        1,  1 + (i32)columns,  (i32)columns,  (i32)columns - 1,
        -1, -1 - (i32)columns, -(i32)columns, 1 - (i32)columns
    };
    u32 active_index = 1;
    u32 last_active = 1;
    while (active_list_head->size)
    {
        assert(active_index < array_size((*positions)));
        pos = array_val((*positions), active_index);
        b32 found = false;
        for (u32 i = 0; i < k; i++)
        {
            const f32 random = random_f32s(seed++, 0.0f, 360.0f);
            const f32 x = (cosf(radians(random)) * minimum_distance) + pos.x;
            const f32 z = (sinf(radians(random)) * minimum_distance) + pos.z;
#if 1
            if (x < 0.0f || x >= max_x || z < 0.0f || z >= max_z)
            {
                continue;
            }
#endif
            const V3 pos_around = v3f(x, 0.0f, z);
            const u32 cell_index_around =
                cell_index_get(pos_around, cell_size, columns);
            if (array_val(gridd_cells, cell_index_around))
            {
                continue;
            }
#if 0
            for (u32 j = 0; j < sy_SIZE(circle_index_table); j++)
            {
                const u32 neighbor_index = cell_index_around + circle_index_table[j];
            }
#endif
            index = array_head((*positions))->size++;
            array_val((*positions), index) = pos_around;
            array_val(gridd_cells, cell_index_around) = index;
            active_index = index;
            found = true;
            break;
        }
        if (found)
        {
            array_val(active_indices, last_active) = active_index;
        }
        else
        {
            array_val(active_indices, active_index) = array_pop(active_indices);
            if (active_list_head->size)
            {
                u32 random_index = random_u32ss(seed++, 0, active_list_head->size);
                active_index = array_val(active_indices, random_index);
            }
        }
        last_active = active_index;
    }
    f64 duration = platform_get_time() - start;

    sy_print("Duration: %Lf\nBlue noise, Max: %u, Found: %u\n", duration, max_count,
             array_size((*positions)) - 1);
}

void game_init(Region_Alloc* region, VkDevice device,
               VkPhysicalDevice physical_device, VkCommandPool command_pool,
               VkQueue graphic_queue, const Swap_Chain_Attrib* swap_chain,
               const Platform* platform, Render_State* render_state,
               u32 num_semaphores)
{
    stack_begin_scope(game_init_stack);

    g_state_GAME.win_handles = region_array_calloc(region, 10, Window_Handle);
    g_state_GAME.rects = region_array_calloc(region, 1000, Rect3D);

    entity_3d_init(region, 0, 100, &g_state_GAME.entity_state);

    const char* paths[] = {
        [DEFAULT_TEXTURE_GAME] = "Syntics/res/default.png",
        [OBJ_TEXTURE_GAME] = "Syntics/res/kiha32/1591184735691.png",
    };
    u32 num_text = sy_SIZE(paths);
    g_state_GAME.textures = region_array(region, num_text, Texture);

    textures_path_create(device, physical_device, command_pool, graphic_queue, true,
                         num_text, paths, g_state_GAME.textures);

    array_head(g_state_GAME.textures)->size = num_text;

    { // Triangle strip
        Graphic_Pipeline* g_p = &g_state_GAME.triangle_strip_pipeline;
        *g_p = gp_default1(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
        graphics_pipeline_create_deluxe(
            region, device, physical_device, num_semaphores,
            "Syntics/res/shaders/spv/game.vert.spv",
            "Syntics/res/shaders/spv/game.frag.spv", swap_chain,
            g_state_GAME.textures, num_text, g_p);
    }

    { // Triangle list
        Graphic_Pipeline* g_p = &g_state_GAME.triangle_list_pipeline;
        *g_p = gp_default1(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        graphics_pipeline_create_deluxe(
            region, device, physical_device, num_semaphores,
            "Syntics/res/shaders/spv/game.vert.spv",
            "Syntics/res/shaders/spv/game.frag.spv", swap_chain,
            g_state_GAME.textures, num_text, g_p);
    }

    { // Line list
        Graphic_Pipeline* g_p = &g_state_GAME.line_list_pipeline;
        *g_p = gp_default1(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
        g_p->line_width = 5.0f;
        graphics_pipeline_create_deluxe(
            region, device, physical_device, num_semaphores,
            "Syntics/res/shaders/spv/gui.vert.spv",
            "Syntics/res/shaders/spv/gui_graph.frag.spv", swap_chain,
            g_state_GAME.textures, num_text, g_p);
    }

#if 1
    { // Grass
        Graphic_Pipeline* g_p = &g_state_GAME.grass_pipeline;
        *g_p = gp_default1(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        graphics_pipeline_create_deluxe(
            region, device, physical_device, num_semaphores,
            "Syntics/res/shaders/spv/game_grass.vert.spv",
            "Syntics/res/shaders/spv/game_grass.frag.spv", swap_chain,
            g_state_GAME.textures, 1, g_p);
    }
#endif

    { // Terrain generation
        stack_begin_scope(terrain_stack);

        Vertex_Buffer* vert = &g_state_GAME.terrain_vert_idx.vert;
        Index_Buffer* idx = &g_state_GAME.terrain_vert_idx.idx;

        vert->array = vertex_array_create(stack_get(), CHUNK_SIZE);
        vert->array.size = CHUNK_SIZE;

#ifdef multithreaded
        HANDLE terrain_semaphore;
        for (u32 i = 0; i < MAX_TERRAIN_THREADS; i++)
        {
            u32 vert_size = (CHUNK_SIZE_Z / MAX_TERRAIN_THREADS) * CHUNK_SIZE_X;
            u32 offset = i * vert_size;
            Thread_Attrib_Terrain* th = terrain_threads + i;
            th->index = i;
            th->verts = vert->array.data + offset;
            terrain_semaphore = thread_task_push(generate_terrain_threaded, th);
        }
        for (u32 i = 0; i < MAX_TERRAIN_THREADS; i++)
        {
            WaitForSingleObject(terrain_semaphore, INFINITE);
        }
#else
        generate_terrain(0.0f, 0.0f, 0, CHUNK_SIZE_Z, vert->data);
#endif
        normal_generate();
        idx->array = u32_array_create(stack_get(), 2 * CHUNK_SIZE);
        generate_indices_terrain(&idx->array);

        idx->curr_size = idx->array.size;
        vertex_index_buffer_create_default1(device, physical_device, command_pool,
                                            graphic_queue, VERTEX_INDEX_LOCAL_LOCAL,
                                            &g_state_GAME.terrain_vert_idx);
#if 0
#ifdef multithreaded
        for (u32 i = 0; i < MAX_TERRAIN_THREADS; i++)
        {
            ReleaseSemaphore(start_semaphore, 1, 0);
        }
#endif
#endif
        stack_end_scope(terrain_stack);
    }

    g_state_GAME.cam = cam_3di(2000.0f, 5.0f);
    g_state_GAME.cam.pos = v3f(-1.2f, 1.0f, 0.8f);
    g_state_GAME.cam.ori = v3f(0.9f, 0.235f, 0.354f);
    g_state_GAME.global_model = m4i(1.0f);

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

        g_state_GAME.road_pos = *((V3*)file.buffer);
        file.buffer += sizeof(V3);

        current_curve_count = *((u32*)file.buffer);

        Vertex_Buffer* vert = &g_state_GAME.road_line_vert_idx.vert;
        Index_Buffer* idx = &g_state_GAME.road_line_vert_idx.idx;

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

        vert_offset = spline_3d_circles_create(&vert->array, 0, &spline2, 0.08f);
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
            VERTEX_INDEX_VISIBLE_LOCAL, &g_state_GAME.road_line_vert_idx);

        stack_end_scope(road_line_stack);
    }

    { // Road
        stack_begin_scope(road_stack);

        Vertex_Buffer* vert = &g_state_GAME.road_vert_idx.vert;
        Index_Buffer* idx = &g_state_GAME.road_vert_idx.idx;

        const u32 size =
            g_state_GAME.road_line_vert_idx.vert.array.size - vert_offset;

        vert->array = vertex_array_create(region, size);
        for (u32 i = vert_offset; i < size + vert_offset; i++)
        {
            vertex_array_push(
                &vert->array,
                vertex_array_val(&g_state_GAME.road_line_vert_idx.vert.array, i));
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
            VERTEX_INDEX_VISIBLE_LOCAL, &g_state_GAME.road_vert_idx);

        stack_end_scope(road_stack);
    }

    {
        stack_begin_scope(particles_stack);

        Vertex_Buffer* vert = &g_state_GAME.particles_vert_idx.vert;
        Index_Buffer* idx = &g_state_GAME.particles_vert_idx.idx;

        const u32 cube_size_vertex = 8;
        const u32 cube_size_index = 36;
        const u32 vert_size_particles = cube_size_vertex * MAX_PARTICLES;
        const u32 index_size_particles = cube_size_index * MAX_PARTICLES;

        particles_3d_init(region, &g_state_GAME.particles, MAX_PARTICLES);

        vert->array = vertex_array_create(region, vert_size_particles);
        idx->array = u32_array_create(stack_get(), index_size_particles);

        cube_indices(&idx->array, 0, MAX_PARTICLES);

        idx->curr_size = 0;
        vertex_index_buffer_create_default1(
            device, physical_device, command_pool, graphic_queue,
            VERTEX_INDEX_VISIBLE_LOCAL, &g_state_GAME.particles_vert_idx);

        stack_end_scope(particles_stack);
    }

#if 1
    // AABB_3D aabb = { 0 };
    {
        stack_begin_scope(dino_stack);
        Vertex_Buffer* vert = &g_state_GAME.car_vert_idx.vert;
        Index_Buffer* idx = &g_state_GAME.car_vert_idx.idx;

        const u32 cube_size_vertex = 8;
        const u32 cube_size_index = 36;

        vert->array = vertex_array_create(region, cube_size_vertex);
        idx->array = u32_array_create(region, cube_size_index);

        cube(&vert->array, 0, v3d(), v3i(0.3f), v4i(1.0f), DEFAULT_TEXTURE_GAME);
        cube_indices(&idx->array, 0, 1);

        idx->curr_size = idx->array.size;
        vertex_index_buffer_create_default1(device, physical_device, command_pool,
                                            graphic_queue, VERTEX_INDEX_LOCAL_LOCAL,
                                            &g_state_GAME.car_vert_idx);

        stack_end_scope(dino_stack);
    }
#endif

#if 0
    {
        g_state_GAME.car_aabb = aabb_rep_create(aabb);

        Vertex_Buffer* vert = &g_state_GAME.aabb_rep.vert;
        Index_Buffer* idx = &g_state_GAME.aabb_rep.idx;

        u32 coll_idx[] = { 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6,
                           6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7 };

        u32 aabb_idx_size = sy_SIZE(coll_idx);

        vert->array = vertex_array_create(region, 8);
        idx->array = u32_array_create(stack_get(), aabb_idx_size);

        assert(idx->array._capacity == sy_SIZE(coll_idx));

        for (u32 i = 0; i < aabb_idx_size; i++)
        {
            u32_array_push(&idx->array, coll_idx[i]);
        }
        cube_not_center1(&vert->array, aabb.min, aabb.size, v4i(1.0f),
                         DEFAULT_TEXTURE_GAME);

        idx->curr_size = idx->array.size;
        vertex_index_buffer_create_default1(
            device, physical_device, command_pool, graphic_queue,
            VERTEX_INDEX_VISIBLE_LOCAL, &g_state_GAME.aabb_rep);
    }
#endif

    {
        stack_begin_scope(grass_stack);

        Vertex_Buffer* vert = &g_state_GAME.grass_vert_idx.vert;
        Index_Buffer* idx = &g_state_GAME.grass_vert_idx.idx;

        Obj_Load_Attrib loader;
        model_load(&loader, "Syntics/res/grass/second_draft.obj");

        const u32 size = array_size(loader.indices);
        Vertex_Array temp_vert = vertex_array_create(stack_get(), size);
        U32_Array temp_u32 = u32_array_create(stack_get(), size);

        vertices_extract(&loader, DEFAULT_TEXTURE_GAME, v3d(), &temp_vert, &temp_u32,
                         true);

        obj_load_free(&loader);

        const u32 vertices_count = temp_vert.size;
        const u32 indices_count = temp_u32.size;
        g_state_GAME.grass_vert_count = vertices_count;

        assert(vertices_count % 4 == 0 && "For 128 wide intrinsics");

        bubble_sort_on_y(&temp_vert, &temp_u32);

        vert->array = vertex_array_create(region, vertices_count * MAX_GRASS);
        idx->array = u32_array_create(stack_get(), indices_count * MAX_GRASS);
#if 0
        g_state_GAME.grass_pos_offset_cache =
            region_array(region, vertices_count * MAX_GRASS * 2, V3);
#endif

#if 1
        V3* positions;
        blue_noise(region, (u32)time(NULL), GRASS_DEPTH, GRASS_WIDTH, 0.05f,
                   &positions);

        u32 seed = (u32)time(NULL);
        const f32 min_scale = 0.8f;
        const f32 max_scale = 3.0f;
        const f32 max_y = temp_vert.data[vertices_count - 1].pos.y * max_scale;
        const u32 position_size = array_size(positions) - 1;
        for (u32 i = 0; i < position_size; i++)
        {
            V3 vertex_pos_offset = array_val(positions, i);

            vertex_pos_offset.y = convert_to_noise_coords(
                                      v2f(vertex_pos_offset.x, vertex_pos_offset.z))
                                      .y;

            f32 freq = 1.5f;
            f32 grain = 1.0f;
            i32 oct = 3;
            f32 noise_value = noise_min_max(vertex_pos_offset.x, vertex_pos_offset.z,
                                            freq, grain, oct, min_scale, max_scale);

            V3 gen_scale = v3f(1.0f, noise_value, 1.0f);
            f32 random = random_f32s(seed++, 2.0f, 4.0f);
            M4 matrix = m4_scale(gen_scale);
            for (u32 k = 0; k < vertices_count; k++)
            {
                Vertex vertex = temp_vert.data[k];

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
                vertex_array_push(&vert->array, vertex);
            }
            u32 idx_offset = i * vertices_count;
            for (u32 k = 0; k < indices_count; k++)
            {
                u32_array_push(&idx->array, temp_u32.data[k] + idx_offset);
            }
        }

#else
        HANDLE grass_semaphore;

        const u32 seed = (u32)time(NULL);
        const u32 vert_size = (MAX_GRASS / MAX_GRASS_THREADS) * vertices_count;
        const u32 indices_size = (MAX_GRASS / MAX_GRASS_THREADS) * indices_count;
        for (u32 i = 1; i < MAX_GRASS_THREADS; i++)
        {
            Thread_Attrib_Grass* th = grass_threads + i;
            th->index = i;
            th->seed = random_u32s(seed + (227 * i));

            const u32 vertex_offset = i * vert_size;
            th->vertex_array.size = 0;
            th->vertex_array._capacity = vert_size;
            th->vertex_array.data = vert->array.data + vertex_offset;

            const u32 indices_offset = i * indices_size;
            th->indices_array.size = 0;
            th->indices_array._capacity = indices_size;
            th->indices_array.data = idx->array.data + indices_offset;

            th->model_vertices = temp_vert;
            th->model_indices = temp_u32;

            grass_semaphore = thread_task_push(grass_generation_threaded, th);
        }
        grass_generation(random_u32s(seed), 0, vert_size / vertices_count,
                         vertices_count, indices_count, temp_vert.data,
                         temp_u32.data, vert->array.data, idx->array.data);
        for (u32 i = 1; i < MAX_GRASS_THREADS; i++)
        {
            WaitForSingleObject(grass_semaphore, INFINITE);
        }
#endif
        idx->curr_size = idx->array._capacity;
        vertex_index_buffer_create_default1(device, physical_device, command_pool,
                                            graphic_queue, VERTEX_INDEX_LOCAL_LOCAL,
                                            &g_state_GAME.grass_vert_idx);

        stack_end_scope(grass_stack);
    }

    event_subscribe(&g_state_GAME.mouse_evt, EVT_MOUSE);

    subscribe_recreate_gp_callback(render_state, game_recreate, NULL);
    subscribe_destroy_callback(render_state, game_destroy, NULL);

#if 1
    gui_init(region, device, physical_device, command_pool, graphic_queue,
             swap_chain, platform, num_semaphores, true, &g_state_GAME.gui_ctx);

#ifdef GUI_MULTI_THREADED
    Thread_Attrib_Gui* th_gui = &g_state_GAME.gui_thread;
    th_gui->start_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
    th_gui->end_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
    th_gui->ctx = &g_state_GAME.gui_ctx;

    g_state_GAME.gui_thread_handle =
        thread_create(th_gui, game_update_gui_threaded, 0, NULL);
#endif

    g_state_GAME.win_handles[0] = window_create(&g_state_GAME.gui_ctx);
    g_state_GAME.win_handles[1] = window_create(&g_state_GAME.gui_ctx);
#endif

    // test.cam.pos = test.road_pos;

    stack_end_scope(game_init_stack);
}

#define rec_sample_count 1000

b8 record(f32 dt)
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
    q_clicked = is_key_clicked(&first_clicked, SYNT_KEY_Q);
    if (q_pressed && !p_pressed)
    {
        sec += dt;
        if (sec >= sample_time)
        {
            if (count_rec < rec_sample_count)
            {
                sy_print("Rec: %u / %u\n", count_rec + 1, rec_sample_count);
                rec[count_rec++] = g_state_GAME.cam.vp.view;
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
            g_state_GAME.cam.vp.view = rec[count_play++];
            count_play %= count_rec;
            sec = 0.0f;
        }
    }
    return p_pressed;
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

V3 shoot_camera_ray(V3 mouse_device_coords)
{
    V4 ray_clip = v4_v3f(mouse_device_coords, 1.0f);
    ray_clip.z = -1.0f;

    V4 ray_eye = m4_v4_multi(inverse(g_state_GAME.cam.vp.proj), ray_clip);
    ray_eye.z = -1.0f;
    ray_eye.w = 0.0f;

    V3 ray = v3_v4(m4_v4_multi(inverse(g_state_GAME.cam.vp.view), ray_eye));
    ray = v3_normalize(ray);

    return ray;
}

void swap(f32* x, f32* y)
{
    f32 temp = *x;
    *x = *y;
    *y = temp;
}

b8 ray_hit_target_aabb(V3 ray_direction, V3 ray_origin, f32 t, AABB_3D target)
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

void edit_spline(V2 dimensions, b8 camera_moved, V3 ray, b8 first, b8 should_update,
                 b8* hit, b8* xyz_pressed)
{
    presist Rect3D* rect = NULL;
    if (!(*hit))
    {
        if (camera_moved || should_update)
        {
            u32 rect_size = array_size(g_state_GAME.rects);
            for (u32 i = 0; i < rect_size; i++)
            {
                g_state_GAME.rects[i].misc = v3_distance(
                    g_state_GAME.cam.pos,
                    v3_add(g_state_GAME.rects[i].pos, g_state_GAME.road_pos));
            }
            bubble_sort_rects(g_state_GAME.rects, rect_size);
            camera_moved = false;
        }
        u32 rect_size = array_size(g_state_GAME.rects);
        for (u32 i = 0; i < rect_size; i++)
        {
            rect = g_state_GAME.rects + i;
            AABB_3D aabb = { v3_add(v3_sub(rect->pos, rect->size),
                                    g_state_GAME.road_pos),
                             v3_s_multi(rect->size, 2.0f) };
            *hit = ray_hit_target_aabb(ray, g_state_GAME.cam.pos, rect->misc, aabb);
            if (*hit)
            {
                sy_print("hhh\n");
                break;
            }
        }
    }
    Vertex_Buffer* vert = &g_state_GAME.road_line_vert_idx.vert;
    if (*hit)
    {
        if (is_key_pressed(SYNT_KEY_X))
        {
            rect->pos.x = v3_sub(ray_hit(ray, g_state_GAME.cam.pos,
                                         v3_add(rect->pos, g_state_GAME.road_pos)),
                                 g_state_GAME.road_pos)
                              .x;
            *xyz_pressed = true;
        }
        if (is_key_pressed(SYNT_KEY_C))
        {
            rect->pos.y = v3_sub(ray_hit(ray, g_state_GAME.cam.pos,
                                         v3_add(rect->pos, g_state_GAME.road_pos)),
                                 g_state_GAME.road_pos)
                              .y;
            *xyz_pressed = true;
        }
        if (is_key_pressed(SYNT_KEY_Z))
        {
            rect->pos.z = v3_sub(ray_hit(ray, g_state_GAME.cam.pos,
                                         v3_add(rect->pos, g_state_GAME.road_pos)),
                                 g_state_GAME.road_pos)
                              .z;
            *xyz_pressed = true;
        }

        if (!*xyz_pressed)
        {
            rect->pos = v3_sub(ray_hit(ray, g_state_GAME.cam.pos,
                                       v3_add(rect->pos, g_state_GAME.road_pos)),
                               g_state_GAME.road_pos);
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
                          spline2.bc[side][curve].points_indices[point], rect->pos,
                          0.08f);
            spline_generate_at_curve(&spline2, side, curve, point, rect->pos);
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
                    u32 rect_size = array_size(g_state_GAME.rects);
                    for (u32 j = 0; j < rect_size; j++)
                    {
                        if (g_state_GAME.rects[j].id == id)
                        {
                            rect2 = g_state_GAME.rects + j;
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
                spline_generate_at_curve(&spline2, side, curve, point, rect2->pos);
            }
        }
        data_buffer_copy(&vert->buffer, vert->array.data, vert->buffer.size_bytes);
        data_buffer_copy(&g_state_GAME.road_vert_idx.vert.buffer,
                         g_state_GAME.road_vert_idx.vert.array.data,
                         g_state_GAME.road_vert_idx.vert.buffer.size_bytes);
    }
}

f32 point_procent_along_curve_linear(Cubic_Bezier_Curve curve, V3 offset_position,
                                     V3 point_pos, f32 precision)
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

f32 point_procent_along_curve_binary(Cubic_Bezier_Curve curve, V3 offset_position,
                                     V3 point_pos, f32 precision)
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

        V3 next_point =
            v3_add(brezier_curve_pos(&curve, result + precision), offset_position);
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

b8 collide_with_spline(const Bezier_Spline_3D* spline, V3 offset_pos, V3 test_pos,
                       V3* collision_pos, V3* normal, b8* side_collision)
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

    V3 first =
        v3_add(brezier_curve_pos(&spline->bc[0][left_side_curve_index], procent0),
               offset_pos);
    V3 second =
        v3_add(brezier_curve_pos(&spline->bc[1][right_side_curve_index], procent1),
               offset_pos);

    V3 between_vec = v3_sub(second, first);
    f32 distance_between = v3_len(between_vec);

    V3 first_to_pos = v3_sub(g_state_GAME.cam.pos, first);
    V3 line = v3d();
    V3 normal_ = v3d();
    if (distance_between < (v3_dot(first_to_pos, between_vec) / distance_between))
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
        V3 right_to_pos = v3_sub(g_state_GAME.cam.pos, second);
        V3 right_to_left = v3_sub(first, second);
        if (distance_between <
            (v3_dot(right_to_pos, right_to_left) / distance_between))
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

void game_update(Region_Alloc* region, const Application_State* app_state,
                 Render_State* render_state, V2 dimensions, u32 semaphore_idx,
                 f32 dt)
{
#ifdef GUI_MULTI_THREADED
    Thread_Attrib_Gui* gui_thread = &g_state_GAME.gui_thread;
    gui_thread->app_state = app_state;
    gui_thread->dt = dt;
    gui_thread->dimensions = dimensions;
    gui_thread->semaphore_idx = semaphore_idx;
    gui_thread->ctx->translucentcy = translucentcy_GAME;
    ReleaseSemaphore(gui_thread->start_semaphore, 1, 0);
#endif

    presist V2 preserved_dimensions = { 0 };
    preserved_dimensions = dimensions;

    presist b8 off_the_ground = true;
    presist b8 first_update_edit = true;
    presist b8 first_update_not_edit = true;
    if (g_edit_mode_GAME)
    {
        if (first_update_edit)
        {
            g_state_GAME.cam.speed = 2.0f;
            first_update_edit = false;
            first_update_not_edit = true;
        }
    }
    else
    {
        if (first_update_not_edit)
        {
            g_state_GAME.cam.speed = 2000.0f;
            first_update_not_edit = false;
            first_update_edit = true;
        }
    }
    presist b8 camera_moved = false;
    if (!is_focus())
    {
        camera_moved |= camera_update(&g_state_GAME.cam, app_state->platform,
                                      g_state_GAME.mouse_evt, dt, off_the_ground,
                                      g_edit_mode_GAME);
    }

    g_state_GAME.grass_model = m4i(1.0f);

#if 0
    {
        Vertex_Buffer* vert = &g_state_GAME.grass_vert_idx.vert;

        presist f32 offset_p = 0.0f;

        const u32 vertices_count = g_state_GAME.grass_vert_count;

        u32 cache_index = 0;
        u32 index = 0;
        __m128 _start_pos_xyz[3], _fx, _fy, _fz, _res;
        for (u32 i = 0; i < GRASS_DEPTH; i++)
        {
            V3 pos = vertex_array_val(&vert->array, index).pos;

            pos = v3f((pos.x * OFFSET_INCREASE), 0.0f, (pos.z * OFFSET_INCREASE));

            const f32 angle_noise = noise_min_max(
                pos.x + offset_p, pos.z + offset_p, grass_freq, grass_grain,
                (i32)grass_oct, radians(g_wind.min), radians(g_wind.max));

            M4 matrix = m4_rotate(angle_noise * g_wind_direction.x, X);

            for (u32 j = 0; j < GRASS_WIDTH; j++)
            {
                for (u32 k = 0; k < vertices_count; k += 4)
                {
                    V3 start_pos[4];
                    V3 pos_offset[4];
                    for (u32 h = 0; h < 4; h++)
                    {
                        start_pos[h] = array_val(g_state_GAME.grass_pos_offset_cache,
                                                 cache_index++);
                        pos_offset[h] = array_val(
                            g_state_GAME.grass_pos_offset_cache, cache_index++);
                    }
                    for (u32 h = 0; h < 3; h++)
                    {
                        _start_pos_xyz[h] =
                            _mm_set_ps(start_pos[3].data[h], start_pos[2].data[h],
                                       start_pos[1].data[h], start_pos[0].data[h]);
                    }
                    f32 res_xyz[3][4];
                    for (u32 h = 0; h < 3; h++)
                    {
                        // matrix.data[0][h] * start_pos.x;
                        // matrix.data[1][h] * start_pos.y;
                        // matrix.data[2][h] * start_pos.z;

                        _fx = _mm_mul_ps(_mm_set1_ps(matrix.data[0][h]),
                                         _start_pos_xyz[0]);
                        _fy = _mm_mul_ps(_mm_set1_ps(matrix.data[1][h]),
                                         _start_pos_xyz[1]);
                        _fz = _mm_mul_ps(_mm_set1_ps(matrix.data[2][h]),
                                         _start_pos_xyz[2]);

                        // V3 out;
                        // out.x = f0 + f1 + f2;

                        _res = _mm_add_ps(_fx, _fy);
                        _res = _mm_add_ps(_res, _fz);

                        // start_pos.x += pos_offset.x;
                        // and y and z
                        _res = _mm_add_ps(_res, _mm_set_ps(pos_offset[3].data[h],
                                                           pos_offset[2].data[h],
                                                           pos_offset[1].data[h],
                                                           pos_offset[0].data[h]));
                        _mm_store_ps(res_xyz[h], _res);
                    }
                    for (u32 h = 0; h < 4; h++)
                    {
                        
                        V3* current_pos =
                            &vertex_array_val(&vert->array, index++).pos;
                        current_pos->x = res_xyz[0][h];
                        current_pos->y = res_xyz[1][h];
                        current_pos->z = res_xyz[2][h];
                    }
                }
            }
        }
        data_buffer_copy(&vert->buffer, vert->array.data, vert->buffer.size_bytes);
        offset_p += grass_wind_speed * dt;
    }
#endif
    g_state_GAME.offset_p += grass_wind_speed * dt;

    if (!g_edit_mode_GAME)
    {
        V3 line = v3d();
        V3 normal = v3d();
        b8 side_collision = false;
        if (collide_with_spline(&spline2, g_state_GAME.road_pos,
                                g_state_GAME.cam.pos, &line, &normal,
                                &side_collision))
        {
            presist f32 sec_off_ground = 0.0f;
            if (g_state_GAME.cam.pos.y <= line.y + 0.18f)
            {
                g_state_GAME.cam.pos.y = line.y + 0.18f;
                sec_off_ground = 0.0f;
                off_the_ground = false;
            }
            else
            {
                sec_off_ground += dt;
            }
            if (sec_off_ground >= 0.1f)
            {
                off_the_ground = true;
            }
#if 0
            if (side_collision)
            {
                // TODO: speed to fast so vel gets flipped. Should not be updated if
                // it in the same frame hits the side.
                g_state_GAME.cam.vel = v3_sub(
                    g_state_GAME.cam.vel,
                    v3_s_multi(normal, 2.0f * v3_dot(g_state_GAME.cam.vel, normal)));
            }
#endif
            g_state_GAME.cam.vel.x -= 5.0f * g_state_GAME.cam.vel.x * dt;
            g_state_GAME.cam.vel.z -= 5.0f * g_state_GAME.cam.vel.z * dt;
        }
    }
    else
    {
        presist b8 first = true;
        presist b8 spline_hit = false;
        presist b8 should_update = false;
        presist b8 xyz_pressed = false;
        if (!is_focus() &&
            g_state_GAME.mouse_evt->mouse_evt.button_evt.action ==
                SYNT_BUTTON_PRESS &&
            g_state_GAME.mouse_evt->mouse_evt.button_evt.button == SYNT_LEFT_BUTTON)
        {
            i16 x, y;
            platform_mouse_get_pos(&x, &y);
            V3 mouse_pos = v3f((f32)x, (f32)y, 0.0f);

            mouse_pos = mouse_to_device_coords(mouse_pos, dimensions);
            V3 ray = shoot_camera_ray(mouse_pos);

            edit_spline(dimensions, camera_moved, ray, first, should_update,
                        &spline_hit, &xyz_pressed);

#if 0
            V3 middle = v3_add(g_state_GAME.car_aabb.aabb.min,
                               v3_s_multi(g_state_GAME.car_aabb.aabb.size, 0.5f));
            if (ray_hit_target_aabb(ray, g_state_GAME.cam.pos,
                                    v3_distance(g_state_GAME.cam.pos, middle),
                                    g_state_GAME.car_aabb.aabb))
            {
                sy_print("Hello\n");
            }
#endif

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
                particle_3d_emit(&g_state_GAME.particles, &attrib,
                                 v3f(0.0f, -10.0f, 0.0f), v3d(),
                                 random_f32(0.5f, 1.0f), 10.0f);
            }
            sec = 0.0f;
        }
        Vertex_Buffer* vert = &g_state_GAME.particles_vert_idx.vert;
        Index_Buffer* idx = &g_state_GAME.particles_vert_idx.idx;

        u32 cube_index_size = 36;
        u32 particle_size =
            particles_3d_update(&g_state_GAME.particles, &vert->array, 0, dt);

        idx->curr_size = particle_size * cube_index_size;

        assert(particle_size < idx->array.size);

        data_buffer_copy(&vert->buffer, vert->array.data,
                         (particle_size * 8) * sizeof(Vertex));
    }

    if (!record(dt))
    {
        g_state_GAME.cam.vp.view = view(
            g_state_GAME.cam.pos, v3_add(g_state_GAME.cam.pos, g_state_GAME.cam.ori),
            g_state_GAME.cam.up);
    }
    presist f32 rotation = 45.0f;

    g_state_GAME.cam.vp.proj =
        perspective(radians(rotation), dimensions.x / dimensions.y, 0.1f, 100.0f);

    if (is_key_pressed(SYNT_KEY_G))
    {
        rotation += 200.0f * dt;
    }
    if (is_key_pressed(SYNT_KEY_F))
    {
        rotation -= 200.0f * dt;
    }

    presist b8 gravity = false;
    presist b8 first_clicked = true;
    if (is_key_clicked(&first_clicked, SYNT_KEY_T))
    {

        b_switch(gravity);
    }

#if 0
    V3 x_z = convert_to_noise_coords(v2f(test.cam.pos.x, test.cam.pos.z));

    presist f32 sec_off_ground = 0.0f;

    f32 extra_padding = 0.5f;
    if (test.cam.pos.y <= x_z.y + extra_padding)
    {
        test.cam.pos.y = x_z.y + extra_padding;

        V3 first_point = v3f(test.cam.pos.x + 0.5f, 0.0f, test.cam.pos.z);
        first_point.y =
            convert_to_noise_coords(v2f(first_point.x, first_point.z)).y +
            extra_padding;

        V3 second_point = v3f(test.cam.pos.x, 0.0f, test.cam.pos.z + 0.5f);
        second_point.y =
            convert_to_noise_coords(v2f(second_point.x, second_point.z)).y +
            extra_padding;

        V3 side0 = v3_sub(first_point, test.cam.pos);
        V3 side1 = v3_sub(second_point, test.cam.pos);
        V3 normal = v3_normalize(v3_cross(side1, side0));


        f32 angle = v3_angle(test.cam.vel, normal);

        test.cam.vel.x -= test.cam.vel.x * (5.0f * angle * dt);
        test.cam.vel.z -= test.cam.vel.z * (5.0f * angle * dt);

        // PRINT_V3(test.cam.vel);

        sec_off_ground = 0.0f;
        off_the_ground = false;
    }
    else
    {
        sec_off_ground += dt;
    }
    if (sec_off_ground >= 0.1f)
    {
        off_the_ground = true;
        test.cam.vel.x -= 5.0f * test.cam.vel.x * dt;
        test.cam.vel.z -= 5.0f * test.cam.vel.z * dt;
    }
#endif

#if 0
    Vertex_Buffer* vert = &test.terrain_g_pipeline.vert_buffer;
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
    data_buffer_copy(
        &g_state_GAME.triangle_strip_pipeline.uniform_buffers[semaphore_idx].buffer,
        &g_state_GAME.cam.vp, sizeof(g_state_GAME.cam.vp));

    if (is_key_pressed(SYNT_KEY_LEFT))
    {
        g_state_GAME.road_pos.x -= 3.0f * dt;
    }
    if (is_key_pressed(SYNT_KEY_UP))
    {
        g_state_GAME.road_pos.z -= 3.0f * dt;
    }
    if (is_key_pressed(SYNT_KEY_RIGHT))
    {
        g_state_GAME.road_pos.x += 3.0f * dt;
    }
    if (is_key_pressed(SYNT_KEY_DOWN))
    {
        g_state_GAME.road_pos.z += 3.0f * dt;
    }
    g_state_GAME.road_model = m4_translate(g_state_GAME.road_pos);

    g_state_GAME.car_model = m4_translate(v3f(0.0f, 0.0f, 0.0f));
    g_state_GAME.car_model =
        m4_multi(g_state_GAME.car_model, m4_rotate(scaling_value_GAME.x, X));

    data_buffer_copy(
        &g_state_GAME.triangle_strip_pipeline.uniform_buffers[semaphore_idx].buffer,
        &g_state_GAME.cam.vp, sizeof(g_state_GAME.cam.vp));

    data_buffer_copy(
        &g_state_GAME.triangle_list_pipeline.uniform_buffers[semaphore_idx].buffer,
        &g_state_GAME.cam.vp, sizeof(g_state_GAME.cam.vp));
#ifdef LINES
    data_buffer_copy(
        &g_state_GAME.line_list_pipeline.uniform_buffers[semaphore_idx].buffer,
        &g_state_GAME.cam.vp, sizeof(g_state_GAME.cam.vp));
#endif
#endif

#if 1
    data_buffer_copy(
        &g_state_GAME.grass_pipeline.uniform_buffers[semaphore_idx].buffer,
        &g_state_GAME.cam.vp, sizeof(g_state_GAME.cam.vp));
#endif

    render_callback(render_state, game_render, (void*)&preserved_dimensions);

#ifdef GUI_MULTI_THREADED
    WaitForSingleObject(gui_thread->end_semaphore, INFINITE);
    assert(check_value_GAME == 1);
    check_value_GAME = 0;
#else
    g_state_GAME.gui_ctx.translucentcy = translucentcy_GAME;
    gui_update_begin(&g_state_GAME.gui_ctx, dimensions, semaphore_idx, dt);
    {
        game_update_gui(app_state, dt, dimensions);
    }
#endif
    gui_update_end(&g_state_GAME.gui_ctx, render_state);
}

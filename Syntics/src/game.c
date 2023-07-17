
#define LINES
// #define MOVE_ALL
#define MAX_PARTICLES 4800

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

typedef struct AABB_Representation
{
    AABB_3D aabb;
} AABB_Representation;

AABB_Representation aabb_rep_create(AABB_3D aabb)
{
    AABB_Representation res = { 0 };
    res.aabb = aabb;
    return res;
}

typedef struct Game_State
{
    Graphic_Pipeline triangle_strip_pipeline;
    Graphic_Pipeline triangle_list_pipeline;
    Graphic_Pipeline line_list_pipeline;

    Vertex_Index_Buffer terrain_vert_idx;
    Vertex_Index_Buffer road_vert_idx;
    Vertex_Index_Buffer road_line_vert_idx;
    Vertex_Index_Buffer car_vert_idx;
    Vertex_Index_Buffer particles_vert_idx;
    Vertex_Index_Buffer aabb_rep;

    AABB_Representation car_aabb;
    Rect3D* rects;

    Camera_3D cam;
    M4 global_model;

    V3 road_pos;
    M4 road_model;

    V3 car_pos;
    M4 car_model;

    Texture* textures;
    Font font;
    Events* mouse_evt;

    Particles_3D particles;
    u32 particle_vert_offset;

    Gui_Context gui_ctx;
    Window_Handle* win_handles;

} Game_State;

typedef struct Cube
{
    Vertex verts[8];
} Cube;

typedef struct Thread_Attrib
{
    u32 index;
    Vertex* verts;
    HANDLE start_semaphore;
    HANDLE end_semaphore;
    HANDLE mutex;
} Thread_Attrib;

typedef struct Cubic_Bezier_Curve
{
    V3 p[4];
    u32 points_indices[4];
    u32 vertex_offset;
} Cubic_Bezier_Curve;

typedef struct Bezier_Spline
{
    Cubic_Bezier_Curve* bc;
    u32 n_curves;
    u32 splitt;
} Bezier_Spline;

typedef struct Bezier_Spline_3D
{
    Cubic_Bezier_Curve* bc[2];
    u32 n_curves;
    u32 splitt;
} Bezier_Spline_3D;

global Game_State g_state_GAME;

#define DEFAULT_TEXTURE 0
#define OBJ_TEXTURE 1

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

#if 0
AABB_3D load_vertices_indices(Region_Alloc* region, Vertex_Buffer* vert,
                                    Index_Buffer* idx, const char* obj_path)
{
#if 1
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, obj_path))
        SY_ERROR((warn + err).c_str());

    uint32_t sum = 0;
    for (const auto& shape : shapes)
        sum += (uint32_t)shape.mesh.indices.size();

    vert->data = region_array(region, sum, Vertex);
    idx->data = region_array(region, sum, u32);

    AABB_3D res = aabb_create();
    V3 max = v3i(-INFINITY);

    u32 i = 0;
    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex = {};

            vertex.pos = v3f(attrib.vertices[3 * index.vertex_index + 0],
                             attrib.vertices[3 * index.vertex_index + 1],
                             attrib.vertices[3 * index.vertex_index + 2]);

            vertex.normal = v3f(attrib.normals[3 * index.normal_index + 0],
                                attrib.normals[3 * index.normal_index + 1],
                                attrib.normals[3 * index.normal_index + 2]);

            aabb_check_min_max(&res, vertex.pos, &max);

            vertex.tex_coords = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
            };

            vertex.color = { 1.0f, 1.0f, 1.0f, 1.0f };

            vertex.tex_index = 0.0f;

            array_push(vert->data, vertex);
            array_push(idx->data, i++);
        }
    }
#else
    Obj_Load_Attrib loader;

    load_model(&loader, obj_path);

    u32 size = array_size(loader.indices);

    graphic_pipline->vert_buffer.data = region_array(region, size, Vertex);
    graphic_pipline->idx_buffer.data = region_array(region, size, u32);

    u32 vert_size = array_size(loader.verts);
    u32 tex_size = array_size(loader.tex_coords);
    u32 idx = 0;
    for (u32 i = 0; i < size; i++)
    {
        Vertex vertex = { 0 };

        u32 current_vert_index = loader.indices[i].vertex_index;
        assert(current_vert_index < vert_size);
        vertex.pos = loader.verts[current_vert_index];

        vertex.color = v4f(1.0f, 1.0f, 1.0f, 1.0f);

        u32 current_tex_index = loader.indices[i].texture_index;
        assert(current_tex_index < tex_size);
        vertex.tex_coords.x = loader.tex_coords[current_tex_index].x;
        vertex.tex_coords.y = 1.0f - loader.tex_coords[current_tex_index].y;

        vertex.tex_index = 0;

        array_push(graphic_pipline->vert_buffer.data, vertex);
        array_push(graphic_pipline->idx_buffer.data, idx++);
    }
#endif
    res.size = max - res.min;
    return res;
}
#endif

#if 1
#define CHUNK_SIZE_X 200
#define CHUNK_SIZE_Y 1
#define CHUNK_SIZE_Z 200

#define CHUNK_SIZE CHUNK_SIZE_X* CHUNK_SIZE_Y* CHUNK_SIZE_Z

#define MAX_THREADS 1
#define multithreaded

global const f32 QUAD_WIDTH = 0.5f;
global const f32 QUAD_DEPTH = 0.5f;
global const f32 OFFSET_INCREASE = 0.1f;

global f32 freq = 0.41f;
global f32 grain = 0.36f;
global f32 oct = 3.0f;
global f32 max_height = 8.0f;

u32 game_index_offset = 0;

f32 round_down_to_half(f32 value)
{
    return value - (f32)fmod((f64)value, 0.5);
}

// tells the preproccesor to not reorder things
#define fence _mm_mfence()
// tell the compiler to not reorder things
#define write_barrier                                                               \
    _WriteBarrier();                                                                \
    fence
#define read_barrier _ReadBarrier()

#define InterlockedIncrement _InterlockedIncrement

// Volatile, the value may be changed in another place in the code. Somebody
// else in the system might be changing it
global Thread_Attrib threads[MAX_THREADS] = { 0 };

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
            f32 y_noise = (sy_value_noise2d(ix_off, z_off, freq, grain, (i32)oct) *
                           max_height);
            y_noise = round_down_to_half(y_noise);
            for_range(y, CHUNK_SIZE_Y)
            {
                cube(&vert->data,
                     v3f(0.0f + (0.5f * x), y_noise + (0.5f * y), 0.0f + (0.5f * z)),
                     v3i(0.5f), v4i(1.0f), DEFAULT_TEXTURE);
            }
#else
            f32 y_noise = (sy_value_noise2d(ix_off, z_off, freq, grain, (i32)oct) *
                           max_height);

            V3 pos = v3f(x * QUAD_WIDTH, y_noise, z * QUAD_DEPTH);
            // f32 colorf = y_noise / max_height;
            V4 color = v4i(1.0f);
            f32 tex_index = DEFAULT_TEXTURE;

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

#define chunks CHUNK_SIZE_Z / MAX_THREADS

volatile u32 check_thread_count = 0;

unsigned long generate_terrain_threaded(void* data)
{
    Thread_Attrib* attrib = (Thread_Attrib*)data;
    u32 z_chunk_offset = attrib->index * chunks;
    f32 z_off = (f32)z_chunk_offset * 0.1f;
    for (;;)
    {
        generate_terrain(0.0f, z_off, z_chunk_offset, chunks, attrib->verts);

        WaitForSingleObject(attrib->start_semaphore, INFINITE);

#if 0
        u32 size = array_size(attrib->verts);

        WaitForSingleObject(attrib->mutex, INFINITE);

        Vertex_Buffer* vert = &test.terrain_g_pipeline.vert_buffer;
        memcpy(vert->data + (attrib->index * size), attrib->verts,
               size * sizeof(Vertex));

        ReleaseMutex(attrib->mutex);
#endif
        InterlockedIncrement((LONG volatile*)&check_thread_count);
        ReleaseSemaphore(attrib->end_semaphore, 1, 0);
    }
}

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
    stack_begin_scope();

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

    stack_end_scope();
}

void game_save_binary1(const Vertex_Array* vert_array, const U32_Array* index_array,
                       const Bezier_Spline_3D* spline, V3 camera_pos)
{
    stack_begin_scope();

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

    stack_end_scope();
}

global u32 circle_offset = 0;
global u32 circle_curr_size = 0;

void game_render(void* data, VkCommandBuffer command_buffer, u32 semaphore_idx)
{
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

#if 0
    vkCmdPushConstants(command_buffer, test.terrain_g_pipeline.layout,
                       VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(V3), &g_light_pos);
#endif
    /////// TRIANGLE STRIP ////////////////
    graphics_pipline_bind(command_buffer, &g_state_GAME.triangle_strip_pipeline,
                          semaphore_idx);

    model_matrix_push(command_buffer, g_state_GAME.triangle_strip_pipeline.layout,
                      g_state_GAME.global_model);
    vertex_index_buffer1_bind(command_buffer, &g_state_GAME.terrain_vert_idx);
    vkCmdDrawIndexed(command_buffer, g_state_GAME.terrain_vert_idx.idx.curr_size, 1,
                     0, 0, 0);

    model_matrix_push(command_buffer, g_state_GAME.triangle_strip_pipeline.layout,
                      g_state_GAME.road_model);
    vertex_index_buffer1_bind(command_buffer, &g_state_GAME.road_vert_idx);
    vkCmdDrawIndexed(command_buffer, g_state_GAME.road_vert_idx.idx.curr_size, 1, 0,
                     0, 0);

    //////// TRIANGLE LIST ////////////////
    graphics_pipline_bind(command_buffer, &g_state_GAME.triangle_list_pipeline,
                          semaphore_idx);

    model_matrix_push(command_buffer, g_state_GAME.triangle_list_pipeline.layout,
                      g_state_GAME.global_model);
    vertex_index_buffer1_bind(command_buffer, &g_state_GAME.particles_vert_idx);
    vkCmdDrawIndexed(command_buffer, g_state_GAME.particles_vert_idx.idx.curr_size,
                     1, 0, 0, 0);

    /*
    push_model(command_buffer, g_state_GAME.triangle_list_pipeline.layout,
               g_state_GAME.car_model);
    bind_vertex_index_buffer1(command_buffer, &g_state_GAME.car_vert_idx);
    vkCmdDrawIndexed(command_buffer, g_state_GAME.car_vert_idx.idx.curr_size, 1, 0,
    0, 0);
    */

    /////// LINE LIST ////////////////////////
#ifdef LINES
    graphics_pipline_bind(command_buffer, &g_state_GAME.line_list_pipeline,
                          semaphore_idx);

    model_matrix_push(command_buffer, g_state_GAME.line_list_pipeline.layout,
                      g_state_GAME.road_model);
    vertex_index_buffer1_bind(command_buffer, &g_state_GAME.road_line_vert_idx);
    vkCmdDrawIndexed(command_buffer, circle_curr_size, 1, 0, 0, 0);
    vkCmdDrawIndexed(command_buffer,
                     g_state_GAME.road_line_vert_idx.idx.curr_size - circle_offset,
                     1, circle_offset, 0, 0);

    // car aabb
    model_matrix_push(command_buffer, g_state_GAME.line_list_pipeline.layout,
                      g_state_GAME.car_model);
    vertex_index_buffer1_bind(command_buffer, &g_state_GAME.aabb_rep);
    vkCmdDrawIndexed(command_buffer, g_state_GAME.aabb_rep.idx.curr_size, 1, 0, 0,
                     0);
#endif
}

void game_recreate(void* data, Region_Alloc* region,
                   const Application_State* app_state)
{
    graphic_pipline_ap_recreate(app_state, "Syntics/res/shaders/spv/game.vert.spv",
                                "Syntics/res/shaders/spv/game.frag.spv",
                                &g_state_GAME.triangle_list_pipeline,
                                array_size(g_state_GAME.textures), NULL);

    graphic_pipline_ap_recreate(app_state, "Syntics/res/shaders/spv/game.vert.spv",
                                "Syntics/res/shaders/spv/game.frag.spv",
                                &g_state_GAME.triangle_strip_pipeline,
                                array_size(g_state_GAME.textures), NULL);
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
        V3 p = v3_add(pos, (v3_s_multi(v3f((f32)cos((f64)radians(i)), (f32)sin((f64)radians(i)), 0.0f),
                                       radius)));

        Vertex vertex = vertex_create(p, v3d(), v2d(), v4i(1.0f), DEFAULT_TEXTURE);

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

V3 brezier_curve_pos(Cubic_Bezier_Curve brezier_curve, f32 t)
{
    V3 p0 = v3_lerp(brezier_curve.p[0], brezier_curve.p[1], t);
    V3 p1 = v3_lerp(brezier_curve.p[1], brezier_curve.p[2], t);
    V3 p2 = v3_lerp(brezier_curve.p[2], brezier_curve.p[3], t);
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
        Vertex vertex = vertex_create(brezier_curve_pos(brezier_curve, i), v3d(),
                                      v2d(), v4i(1.0f), DEFAULT_TEXTURE);
        vertex_array_val(vert_array, count++) = vertex;
    }
    return count;
}

// TODO: have a direction param
V3 generate_positions_curve(Bezier_Spline_3D* spline, V3 pos, u32 side, u32 curve)
{
    spline->bc[side][curve].p[0] = pos;
    pos.y += curve % 2 == 0 ? 1.0f : -1.0f;
    for (u32 j = 1; j < 3; j++)
    {
        pos.x += 0.5f;
        spline->bc[side][curve].p[j] = pos;
    }
    pos.y += curve % 2 == 0 ? -1.0f : 1.0f;
    pos.x += 0.5f;
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
            pos = generate_positions_curve(spline, last_pos, k, i);
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
        val(vert_array2, offset2++) = val(vert_array, i);
    }
}

void generate_spline_at_curve(Bezier_Spline_3D* spline, u32 side, u32 curve,
                              u32 point, V3 pos)
{
    spline->bc[side][curve].p[point] = pos;
    generate_spline_curve(spline, side, curve);
}

void generate_spline_at_curve1(Bezier_Spline_3D* spline, u32 curve)
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

global HANDLE thread_handle[MAX_THREADS] = { 0 };

global u32 index_to_test = 0;
global u32 num_points = 0;
global u32 points_size = ((u32)(1.0f / PROCENT_INCREASE) + 1) * 2;

void game_init(Region_Alloc* region, VkDevice device,
               VkPhysicalDevice physical_device, VkCommandPool command_pool,
               VkQueue graphic_queue, const Swap_Chain_Attrib* swap_chain,
               const Platform* platform, Render_State* render_state,
               u32 num_semaphores)
{
    stack_begin_scope();

    g_state_GAME.win_handles = region_array_calloc(region, 10, Window_Handle);
    g_state_GAME.rects = region_array_calloc(region, 1000, Rect3D);

    const char* paths[] = {
        "Syntics/res/default.png",
        "Syntics/res/kiha32/1591184735691.png",
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

    { // Terrain generation
        Vertex_Buffer* vert = &g_state_GAME.terrain_vert_idx.vert;
        Index_Buffer* idx = &g_state_GAME.terrain_vert_idx.idx;

        vert->array = vertex_array_create(stack_get(), CHUNK_SIZE);
        vert->array.size = CHUNK_SIZE;
#ifdef multithreaded
        HANDLE start_semaphore = CreateSemaphore(NULL, 0, sy_SIZE(threads), NULL);
        HANDLE end_semaphore = CreateSemaphore(NULL, 0, sy_SIZE(threads), NULL);
        HANDLE mutex = CreateMutex(NULL, false, NULL);

        for (u32 i = 0; i < MAX_THREADS; i++)
        {
            u32 vert_size = (CHUNK_SIZE_Z / MAX_THREADS) * CHUNK_SIZE_X;
            u32 offset = i * vert_size;
            Thread_Attrib* th = threads + i;
            th->mutex = mutex;
            th->start_semaphore = start_semaphore;
            th->end_semaphore = end_semaphore;
            th->index = i;
            th->verts = vert->array.data + offset;
            thread_handle[i] = thread_create(th, generate_terrain_threaded, 0, NULL);
        }
        for (u32 i = 0; i < MAX_THREADS; i++)
        {
            ReleaseSemaphore(start_semaphore, 1, 0);
        }
        for (u32 i = 0; i < MAX_THREADS; i++)
        {
            WaitForSingleObject(end_semaphore, INFINITE);
        }
        assert(check_thread_count == MAX_THREADS);
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
        for (u32 i = 0; i < MAX_THREADS; i++)
        {
            ReleaseSemaphore(start_semaphore, 1, 0);
        }
#endif
#endif
    }

    g_state_GAME.cam = cam_3di(2000.0f, 5.0f);
    g_state_GAME.global_model = m4i(1.0f);

    u32 vert_offset = 0;
    { // Road Lines
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

        u32 point_all_size = points_size * spline2.n_curves;
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

        u32 size = point_all_size + (num_points * 10);
        vert->array = vertex_array_create(region, size);

        vert_offset = spline_3d_circles_create(&vert->array, 0, &spline2, 0.08f);
        u32 size33 = spline_generate(&spline2, &vert->array, vert_offset);
        vert->array.size = size33;
        spline_3d_normals_generate(&vert->array, vert_offset, &spline2);

        size33 -= vert_offset;
        size33 += count;

        u32 vertex_count = 0;
        u32 i;
        u32 half_splitt = spline2.splitt / 2;
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
    }

    { // Road
        Vertex_Buffer* vert = &g_state_GAME.road_vert_idx.vert;
        Index_Buffer* idx = &g_state_GAME.road_vert_idx.idx;

        u32 size = g_state_GAME.road_line_vert_idx.vert.array.size - vert_offset;

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
        u32 half_size = size / 2;
        u32 half_splitt = spline2.splitt / 2;
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
    }

    {
        Vertex_Buffer* vert = &g_state_GAME.particles_vert_idx.vert;
        Index_Buffer* idx = &g_state_GAME.particles_vert_idx.idx;

        u32 cube_size_vertex = 8;
        u32 cube_size_index = 36;
        u32 vert_size_particles = cube_size_vertex * MAX_PARTICLES;
        u32 index_size_particles = cube_size_index * MAX_PARTICLES;

        particles_3d_init(region, &g_state_GAME.particles, MAX_PARTICLES);

        vert->array = vertex_array_create(region, vert_size_particles);
        idx->array = u32_array_create(stack_get(), index_size_particles);

        cube_indices(&idx->array, 0, MAX_PARTICLES);

        vert->buffer.size_bytes = vert->array._capacity * sizeof(Vertex);
        idx->buffer.size_bytes = idx->array._capacity * sizeof(u32);
        idx->curr_size = 0;
        vertex_buffer_create_visible(device, physical_device, vert);
        index_buffer_create_local(device, physical_device, command_pool,
                                  graphic_queue, idx);
    }
    AABB_3D aabb = { 0 };
    {
        /*
        Vertex_Buffer* vert = &g_state_GAME.car_vert_idx.vert;
        Index_Buffer* idx = &g_state_GAME.car_vert_idx.idx;

        aabb = load_vertices_indices(region, vert, idx, "Syntics/res/car/F1.obj");

        idx->curr_size = array_size(idx->data);
        create_vertex_index_buffer_default1(device, physical_device, command_pool,
                                           graphic_queue, VERTEX_INDEX_VISIBLE_LOCAL,
                                           &g_state_GAME.car_vert_idx);
        */
    }

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
                         DEFAULT_TEXTURE);

        idx->curr_size = idx->array.size;
        vertex_index_buffer_create_default1(
            device, physical_device, command_pool, graphic_queue,
            VERTEX_INDEX_VISIBLE_LOCAL, &g_state_GAME.aabb_rep);
    }
    event_subscribe(&g_state_GAME.mouse_evt, EVT_MOUSE);

    // subscribe_recreate_callback(recreate_game, NULL);
    subscribe_destroy_callback(render_state, game_destroy, NULL);

#if 0
    gui_init(region, device, physical_device, command_pool, graphic_queue,
             swap_chain, num_semaphores, true);

    g_state_GAME.win_handles[0] = create_window();
    g_state_GAME.win_handles[1] = create_window();
#endif

    // test.cam.pos = test.road_pos;

    stack_end_scope();
}

global f32 translucentcy_GAME = 0.8f;
global b32 wire_frame_GAME = false;

global V3 scaling_value_GAME = { 1.0f, 1.0f, 1.0f };

global b8 reset_index_GAME = false;

global b8 show_particles_GAME = false;
global b8 emit_particle_GAME = false;

global b8 g_edit_mode_GAME = true;

void game_update_gui(Region_Alloc* region, const Application_State* app_state, f32 dt,
                V2 dimensions)
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
                game_recreate(NULL, region, app_state);
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
            // TODO: BUG, should be added in the direction the curve is currently
            // heading,
            // SOLUTION: create direction vector from previous point to last point
            // and use that as direction.
            if (window_button_add(win, "Add curve"))
            {
                if (current_curve_count < spline2.n_curves)
                {
                    Vertex_Buffer* vert = &g_state_GAME.road_line_vert_idx.vert;
                    V3 pos[2] = { 0 };
                    if (current_curve_count > 0)
                    {
                        pos[0] = spline2.bc[0][current_curve_count - 1].p[3];
                        pos[1] = spline2.bc[1][current_curve_count - 1].p[3];
                    }
                    else
                    {
                        pos[1].z += 4.0f;
                    }
                    generate_positions_curve(&spline2, pos[0], 0,
                                             current_curve_count);
                    generate_positions_curve(&spline2, pos[1], 1,
                                             current_curve_count);

                    u32 offset = circle_curr_size / 2;
                    spline_circles_curve_create(&vert->array, offset, &spline2,
                                                current_curve_count, 0.08f);
                    generate_spline_at_curve1(&spline2, current_curve_count);

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
            window_text_add(win, "Freq --- Grain --- Oct --- Max Height");
        }
        window_gridd_end(win);

        window_gridd_begin(win, 4, 1);
        {
            window_input_float_add(win, &freq, 0.0f, 10.0f, 1.0f);
            window_input_float_add(win, &grain, 0.0f, 10.0f, 1.0f);
            window_input_float_add(win, &oct, 0.0f, 10.0f, 1.0f);
            window_input_float_add(win, &max_height, 0.0f, 20.0f, 2.0f);
        }
        window_gridd_end(win);

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
        window_gridd_begin(win, 1, 1);
        {
            presist char temp[60] = { 0 };
            presist f32 count = 1.0f;
            if (count >= 0.1f)
            {
                sprintf_s(temp, sizeof(temp), V3_FMT(g_state_GAME.cam.pos));
                count = 0.0f;
            }
            count += dt;
            window_text_add(win, temp);
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

#if 1
V3 convert_to_noise_coords(V2 x_z)
{
    V3 out = v3f((x_z.x * OFFSET_INCREASE) / QUAD_WIDTH, 0.0f,
                 (x_z.y * OFFSET_INCREASE) / QUAD_DEPTH);

    out.y = (sy_value_noise2d(out.x, out.z, freq, grain, (i32)oct) * max_height);

    return out;
}
#endif

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

#if 0
    Vertex dd = vertex_create(camera_pos, v3d(), v2d(), v4i(1.0f), DEFAULT_TEXTURE);
    Vertex dd2 = vertex_create(ray, v3d(), v2d(), v4i(1.0f), DEFAULT_TEXTURE);

    val(test.line_g_pipeline.vert_buffer.data, index_to_test) = dd;
    val(test.line_g_pipeline.vert_buffer.data, index_to_test + 1) = dd2;
#endif
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
            AABB_3D aabb = { v3_sub(v3_add(rect->pos, g_state_GAME.road_pos),
                                    rect->size),
                             v3_s_multi(rect->size, 2.0f) };
            *hit = ray_hit_target_aabb(ray, g_state_GAME.cam.pos, rect->misc, aabb);
            if (*hit) break;
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
            generate_spline_at_curve(&spline2, side, curve, point, rect->pos);
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
                generate_spline_at_curve(&spline2, side, curve, point, rect2->pos);
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
        V3 current_point = v3_add(brezier_curve_pos(curve, result), offset_position);
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

        V3 current_point = v3_add(brezier_curve_pos(curve, result), offset_position);
        f32 curr_dist_squared = v3_distance_squared(point_pos, current_point);

        V3 next_point =
            v3_add(brezier_curve_pos(curve, result + precision), offset_position);
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

b8 colide_with_spline(const Bezier_Spline_3D* spline, V3 offset_pos, V3 test_pos,
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
        v3_add(brezier_curve_pos(spline->bc[0][left_side_curve_index], procent0),
               offset_pos);
    V3 second =
        v3_add(brezier_curve_pos(spline->bc[1][right_side_curve_index], procent1),
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
        normal_ = v3_normalize_len(v3_neg(between_vec), distance_between);
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
            normal_ = v3_normalize_len(between_vec, distance_between);
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

    if (!g_edit_mode_GAME)
    {
        V3 line = v3d();
        V3 normal = v3d();
        b8 side_collision = false;
        if (colide_with_spline(&spline2, g_state_GAME.road_pos, g_state_GAME.cam.pos,
                               &line, &normal, &side_collision))
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
            if (side_collision)
            {
                // TODO: speed to fast so vel gets flipped. Should not be updated if
                // it in the same frame hits the side.
                g_state_GAME.cam.vel = v3_sub(
                    g_state_GAME.cam.vel,
                    v3_s_multi(normal, 2.0f * v3_dot(g_state_GAME.cam.vel, normal)));
            }
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

            V3 middle = v3_add(g_state_GAME.car_aabb.aabb.min,
                               v3_s_multi(g_state_GAME.car_aabb.aabb.size, 0.5f));
            if (ray_hit_target_aabb(ray, g_state_GAME.cam.pos,
                                    v3_distance(g_state_GAME.cam.pos, middle),
                                    g_state_GAME.car_aabb.aabb))
            {
                sy_print("Hello\n");
            }

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
                f32 x = (f32)fmod((f64)(i * rand_f32(0.0f, 0.8f)), (f64)CHUNK_SIZE_X * 0.5);
                f32 z = (f32)fmod((f64)(i * rand_f32(0.0f, 0.8f)), (f64)CHUNK_SIZE_Z * 0.5);
                attrib.position = v3f(x, 30.0f, z);
                attrib.color = v4i(1.0f);
                attrib.size = v3i(rand_f32(0.05f, 0.1f));
                particle_3d_emit(&g_state_GAME.particles, &attrib,
                                 v3f(0.0f, -10.0f, 0.0f), v3d(),
                                 rand_f32(0.5f, 1.0f), 4.0f);
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

    HANDLE end_semaphore = threads[0].end_semaphore;
    HANDLE start_semaphore = threads[1].start_semaphore;
    for (u32 i = 0; i < MAX_THREADS; i++)
    {
        WaitForSingleObject(end_semaphore, INFINITE);
    }
#else
    generate_terrain(0.0f, 0.0f, 0, CHUNK_SIZE_Z, vert->data);
#endif
    data_buffer_copy(&vert->buffer, vert->data, vert->buffer.size_bytes);
#ifdef multithreaded
    for (u32 i = 0; i < MAX_THREADS; i++)
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

    g_state_GAME.car_model = m4_rotate(scaling_value_GAME.x, X);

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
    render_callback(render_state, game_render, (void*)&preserved_dimensions);

    g_state_GAME.gui_ctx.translucentcy = translucentcy_GAME;
    gui_update_begin(&g_state_GAME.gui_ctx, dimensions, semaphore_idx, dt);
    {
        game_update_gui(region, app_state, dt, dimensions);
    }
    gui_update_end(&g_state_GAME.gui_ctx, render_state);
}


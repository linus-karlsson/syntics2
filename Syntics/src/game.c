#include "game.h"
#include "logging.h"
#include "region_alloc.h"
#include "font.h"
#include "camera.h"
#include "buffers.h"
#include "swap_chain.h"
#include "gui.h"
#include "event_system.h"
#include "file_reading.h"
#include "vulkan_types.h"
#include "obj_load.h"
#include "noise.h"
#include "render_util.h"
#include "random.h"
#include <intrin.h>
#include <math.h>
#if 1
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include "win32/sy_winthread.h"
#endif

#define LINES

global const char* OBJ_PATH = "Syntics/res/kiha32/kiha32.obj";

typedef struct String
{
    char* buffer;
    u32 length;
} String;

#define str(text) string(text)
String string(char* text)
{
    String out;
    out.buffer = text;
    out.length = text ? (u32)strlen(text) : 0;
    return out;
}

typedef struct Render_Test_State
{
    Graphic_Pipeline main_g_pipeline;
    Graphic_Pipeline figur_g_pipeline;

    Graphic_Pipeline line_g_pipeline;

    Camera_3D cam;
    Camera_3D figur_cam;

    Texture* textures;
    Font font;
    Events* mouse_evt;

} Render_Test_State;

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

global Render_Test_State test;

#define DEFAULT_TEXTURE 0
#define OBJ_TEXTURE 1

#if 0
internal void load_vertices_indices(Region_Alloc* region,
                                    Graphic_Pipeline* graphic_pipline)
{
    // TODO: fix small glitches.
    Obj_Load_Attrib loader;

    load_model(&loader, OBJ_PATH);

    u32 size = size_arr(loader.indices);

    graphic_pipline->vert_buffer.data = dyn_arrayP(region, size * 3, Vertex);
    graphic_pipline->idx_buffer.data = dyn_arrayP(region, size * 3, u32);

    u32 idx = 0;
    for (u32 i = 0; i < size; i++)
    {
        for (u32 j = 0; j < 3; j++)
        {
            Vertex vertex = { 0 };

            vertex.pos = loader.verts[loader.indices[i].vertex_index[j]];

            vertex.color = v4f(1.0f, 1.0f, 1.0f, 1.0f);

            vertex.tex_coords.x =
                loader.tex_coords[loader.indices[i].texture_index[j]].x;
            vertex.tex_coords.y =
                1.0f - loader.tex_coords[loader.indices[i].texture_index[j]].y;

            vertex.tex_index = OBJ_TEXTURE;

            synt_push(graphic_pipline->vert_buffer.data, vertex);
            synt_push(graphic_pipline->idx_buffer.data, idx++);
        }
    }
}
#endif

#if 1
#define CHUNK_SIZE_X 100
#define CHUNK_SIZE_Y 1
#define CHUNK_SIZE_Z 100

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

f32 round_down_to_half(f32 value)
{
    return value - fmodf(value, 0.5f);
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

internal void generate_terrain(f32 x_off, f32 z_off, u32 z_chunk_offset,
                               u32 z_chunks, Vertex* verts)
{
    u32 z_index = 0;
    u32 iterations = (z_chunk_offset + z_chunks) < CHUNK_SIZE_Z
                         ? (z_chunk_offset + z_chunks)
                         : CHUNK_SIZE_Z;
    for (u32 z = z_chunk_offset; z < iterations; z++)
    {
        f32 ix_off = x_off;
        for_range(x, CHUNK_SIZE_X)
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
        u32 size = size_arr(attrib->verts);

        WaitForSingleObject(attrib->mutex, INFINITE);

        Vertex_Buffer* vert = &test.main_g_pipeline.vert_buffer;
        memcpy(vert->data + (attrib->index * size), attrib->verts,
               size * sizeof(Vertex));

        ReleaseMutex(attrib->mutex);
#endif
        ReleaseSemaphore(attrib->end_semaphore, 1, 0);
    }
}

internal void generate_normal()
{
    Vertex_Buffer* vert = &test.main_g_pipeline.vert_buffer;
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

internal void render_game(void* data, VkCommandBuffer command_buffer,
                          u32 semaphore_idx)
{
#if 1
    vkCmdPushConstants(command_buffer, test.main_g_pipeline.layout,
                       VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(V3), &g_light_pos);
#endif
#if 1

    Index_Buffer* idx = &test.main_g_pipeline.idx_buffer;
    bind_and_draw_graphics_pipline(
        command_buffer, test.main_g_pipeline.descriptors.desc_sets[semaphore_idx], 0,
        idx->curr_size, &test.main_g_pipeline);
#endif

    Index_Buffer* idx2 = &test.figur_g_pipeline.idx_buffer;
    bind_and_draw_graphics_pipline(
        command_buffer, test.figur_g_pipeline.descriptors.desc_sets[semaphore_idx],
        0, idx2->curr_size, &test.figur_g_pipeline);

#ifdef LINES
    Index_Buffer* idx3 = &test.line_g_pipeline.idx_buffer;
    bind_and_draw_graphics_pipline(
        command_buffer, test.line_g_pipeline.descriptors.desc_sets[semaphore_idx], 0,
        idx3->curr_size, &test.line_g_pipeline);
#endif
}

internal void recreate_game(void* data, Region_Alloc* region,
                            const Application_State* app_state)
{
    recreate_graphic_pipline_ap(region, app_state, "Syntics/res/game.vert.spv",
                                "Syntics/res/game.frag.spv", &test.main_g_pipeline,
                                size_arr(test.textures), NULL);
    gui_recreate(region);
}

internal void destroy_game(void* data, VkDevice device, u32 num_semaphores)
{
    destroy_graphic_pipeline(device, num_semaphores, &test.main_g_pipeline);
    destroy_graphic_pipeline(device, num_semaphores, &test.figur_g_pipeline);
#ifdef LINES
    destroy_graphic_pipeline(device, num_semaphores, &test.line_g_pipeline);
#endif

    for (u32 i = 0; i < size_arr(test.textures); i++)
    {
        destroy_texture(device, test.textures[i]);
    }

    destroy_gui(device, num_semaphores);
}

typedef struct Cubic_Brezier_Curve
{
    V3 p[4];
} Cubic_Brezier_Curve;

typedef struct Brezier_Spline
{
    Cubic_Brezier_Curve* bc;
    u32 n_curves;
} Brezier_Spline;

Brezier_Spline spline_create(Region_Alloc* region, u32 n_curves)
{
    Brezier_Spline out;
    out.bc = dyn_arrayP(region, n_curves, Cubic_Brezier_Curve);
    out.n_curves = n_curves;
    return out;
}

#define PROCENT_INCREASE 0.01f

internal V3 brezier_curve_pos(Cubic_Brezier_Curve brezier_curve, f32 t)
{
    V3 p0 = v3_lerp(brezier_curve.p[0], brezier_curve.p[1], t);
    V3 p1 = v3_lerp(brezier_curve.p[1], brezier_curve.p[2], t);
    V3 p2 = v3_lerp(brezier_curve.p[2], brezier_curve.p[3], t);
    V3 p3 = v3_lerp(p0, p1, t);
    V3 p4 = v3_lerp(p1, p2, t);
    return v3_lerp(p3, p4, t);
}

internal void generate_curve(Cubic_Brezier_Curve brezier_curve)
{
    Vertex_Buffer* vert = &test.line_g_pipeline.vert_buffer;
    for (f32 i = 0.0; i <= 1.0f; i += PROCENT_INCREASE)
    {
        Vertex vertex = vertex_create(brezier_curve_pos(brezier_curve, i), v3d(),
                                      v2d(), v4i(1.0f), DEFAULT_TEXTURE);
        synt_push(vert->data, vertex);
    }
}

internal Brezier_Spline generate_spline(Region_Alloc* region, u32 n_curves, V3 pos)
{
    Brezier_Spline sp = { 0 };
    sp.bc = dyn_arrayP(region, n_curves, Cubic_Brezier_Curve);
    sp.n_curves = n_curves;
    for (u32 i = 0; i < sp.n_curves; i++)
    {
        sp.bc[i].p[0] = pos;
        pos.y += i % 2 == 0 ? 1.0f : -1.0f;
        for (u32 j = 1; j < 3; j++)
        {
            pos.x += 0.5f;
            sp.bc[i].p[j] = pos;
        }
        pos.y += i % 2 == 0 ? -1.0f : 1.0f;
        pos.x += 0.5f;
        sp.bc[i].p[3] = pos;
        generate_curve(sp.bc[i]);
    }
    return sp;
}

internal V3 spline_curve_pos(Brezier_Spline sp, f32 t)
{
    f32 t_corrected = t * sp.n_curves;
    u32 index = (u32)t_corrected;
    return brezier_curve_pos(sp.bc[index], t_corrected - (f32)index);
}

f32 get_procent(Brezier_Spline sp, f32 t)
{
    return t * sp.n_curves;
}

internal void generate_indices_terrain(u32* index_buffer)
{
    int32 I = 0;
    int32 step_value = 1;
    for_range(i, CHUNK_SIZE_Z - 1)
    {
        for_range(j, CHUNK_SIZE_X)
        {
            synt_push(index_buffer, (CHUNK_SIZE_X * i) + I);
            synt_push(index_buffer, (CHUNK_SIZE_X * (i + 1)) + I);

            I += step_value;
        }
        step_value *= -1;
        I += step_value;
    }
}

HANDLE thread_handle[MAX_THREADS] = { 0 };

Brezier_Spline spline = { 0 };

void init_game(Region_Alloc* region, VkDevice device,
               VkPhysicalDevice physical_device, VkCommandPool command_pool,
               VkQueue graphic_queue, const Swap_Chain_attrib* swap_chain,
               u32 num_semaphores)
{
    stack_begin_scope();
    const char* paths[] = {
        [DEFAULT_TEXTURE] = "Syntics/res/default.png",
        [OBJ_TEXTURE] = "Syntics/res/kiha32/1591184735691.png",
    };
    u32 num_text = sy_SIZE(paths);
    test.textures = dyn_arrayP(region, num_text, Texture);

    create_textures_path(device, physical_device, command_pool, graphic_queue, true,
                         num_text, paths, test.textures);

    get_head(test.textures)->size = num_text;

    { // Terrain generation
        Graphic_Pipeline* g_p = &test.main_g_pipeline;
        *g_p = gp_default1(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
#if 1
        g_p->vert_buffer.data = dyn_arrayP(get_stack(), CHUNK_SIZE, Vertex);
        get_head(g_p->vert_buffer.data)->size = CHUNK_SIZE;
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
            th->verts = g_p->vert_buffer.data + offset;
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
#else
        generate_terrain(0.0f, 0.0f, 0, CHUNK_SIZE_Z, g_p->vert_buffer.data);
#endif
        generate_normal();
        g_p->idx_buffer.data = dyn_arrayP(get_stack(), 2 * CHUNK_SIZE, u32);
        generate_indices_terrain(g_p->idx_buffer.data);
#else
        load_vertices_indices(region, g_p);
#endif
        g_p->idx_buffer.curr_size = size_arr(g_p->idx_buffer.data);
        g_p->textures = test.textures;
        g_p->vert_path = "Syntics/res/game.vert.spv";
        g_p->frag_path = "Syntics/res/game.frag.spv";
        create_graphics_pipeline_deluxe(region, device, physical_device,
                                        command_pool, graphic_queue, num_semaphores,
                                        swap_chain, swap_chain->extent_2D, num_text,
                                        NULL, VERTEX_INDEX_LOCAL_LOCAL, g_p);
#if 0
#ifdef multithreaded
        for (u32 i = 0; i < MAX_THREADS; i++)
        {
            ReleaseSemaphore(start_semaphore, 1, 0);
        }
#endif
#endif
    } ///////////////////////////////////////////////////////

    { // Small cube
        Graphic_Pipeline* f_g_p = &test.figur_g_pipeline;

        f_g_p->vert_buffer.data = dyn_arrayP(get_stack(), 1 * 8, Vertex);
        cube(f_g_p->vert_buffer.data, v3d(), v3i(0.5f), v4i(1.0f), DEFAULT_TEXTURE);

        f_g_p->idx_buffer.data = dyn_arrayP(get_stack(), 1 * 36, u32);
        cube_indices(f_g_p->idx_buffer.data, 1);

        f_g_p->idx_buffer.curr_size = size_arr(f_g_p->idx_buffer.data);
        f_g_p->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        f_g_p->cull_mode = VK_CULL_MODE_NONE;
        f_g_p->poly_mode = VK_POLYGON_MODE_FILL;
        f_g_p->vert_path = "Syntics/res/game.vert.spv";
        f_g_p->frag_path = "Syntics/res/game.frag.spv";
        f_g_p->textures = test.textures;
        create_graphics_pipeline_deluxe(region, device, physical_device,
                                        command_pool, graphic_queue, num_semaphores,
                                        swap_chain, swap_chain->extent_2D, num_text,
                                        NULL, VERTEX_INDEX_LOCAL_LOCAL, f_g_p);
    } ////////////////////////////////////////////////////////////////

    { // Lines
#ifdef LINES
#if 1
        Graphic_Pipeline* l_g_p = &test.line_g_pipeline;

        u32 points_size = ((u32)(1.0f / PROCENT_INCREASE) + 50) * 3;
        l_g_p->vert_buffer.data = dyn_arrayP(region, points_size, Vertex);
        l_g_p->idx_buffer.data = dyn_arrayP(get_stack(), points_size * 2, u32);
        for (u32 i = 0; i < points_size; i++)
        {
            synt_push(l_g_p->idx_buffer.data, i);
            synt_push(l_g_p->idx_buffer.data, i + 1);
        }
        spline = generate_spline(region, 3, v3d());
#else
        u32 vert_size = size_arr(g_p->vert_buffer.data);
        l_g_p->vert_buffer.data = dyn_arrayP(region, 1 * vert_size, Vertex);
        l_g_p->idx_buffer.data = dyn_arrayP(region, 1 * vert_size, u32);

        u32 count = 0;
        for (u32 i = 0; i < vert_size; i += 2)
        {
            Vertex vert = g_p->vert_buffer.data[i];
            synt_push(l_g_p->vert_buffer.data, vert);
            vert.pos = v3_add(vert.pos, vert.normal);
            synt_push(l_g_p->vert_buffer.data, vert);

            synt_push(l_g_p->idx_buffer.data, count++);
            synt_push(l_g_p->idx_buffer.data, count++);
        }
#endif

        // l_g_p->idx_buffer.curr_size = size_arr(l_g_p->idx_buffer.data);
        l_g_p->idx_buffer.curr_size = (size_arr(l_g_p->vert_buffer.data) * 2) - 2;
        l_g_p->topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        l_g_p->cull_mode = VK_CULL_MODE_NONE;
        l_g_p->poly_mode = VK_POLYGON_MODE_FILL;
        l_g_p->line_width = 5.0f;
        l_g_p->textures = test.textures;
        l_g_p->vert_path = "Syntics/res/gui.vert.spv";
        l_g_p->frag_path = "Syntics/res/gui_graph.frag.spv";
        create_graphics_pipeline_deluxe(region, device, physical_device,
                                        command_pool, graphic_queue, num_semaphores,
                                        swap_chain, swap_chain->extent_2D, 1, NULL,
                                        VERTEX_INDEX_VISIBLE_LOCAL, l_g_p);

#endif
    } ///////////////////////////////////////////////////////

    test.cam = cam_3di(4.0f, 5.0f);
    test.figur_cam = cam_3di(2000.0f, 5.0f);
    test.figur_cam.pos.x += 10.0f;

    subscribe(&test.mouse_evt, EVT_MOUSE);

    subscribe_recreate_callback(recreate_game, NULL);
    subscribe_destroy_callback(destroy_game, NULL);

    init_gui(region, device, physical_device, command_pool, graphic_queue,
             swap_chain, num_semaphores, true);

    stack_end_scope();
}

global f32 translucentcy = 0.8f;
global b32 wire_frame = false;

global V3 scaling_value = { { { 1.0f, 1.0f, 1.0f } } };

internal void update_gui(Region_Alloc* region, const Application_State* app_state,
                         f32 dt, V2 dimensions)
{
    back_bord_begin("First thing", v2f(10.0f, 10.0f));
    {
        gridd_begin(1, 1);
        {
            presist char buffer[100] = { 0 };
            u32 size = 0;
            if (add_input_text(buffer, &size))
            {
                buffer[size] = '\0';
                print("%s\n", buffer);
            }
        }
        gridd_end();
        gridd_begin(2, 1);
        {
            add_text("Translucentcy: ");
            add_input_float_d(&translucentcy, 0.0f, 1.0f);
        }
        gridd_end();
        gridd_begin(4, 1);
        {
            if (add_button("OFF"))
            {
                translucentcy = 0.0f;
            }
            if (add_button("Low"))
            {
                translucentcy = 0.2f;
            }
            if (add_button("High"))
            {
                translucentcy = 0.8f;
            }
            if (add_button("Fill"))
            {
                translucentcy = 1.0f;
            }
        }
        gridd_end();
        gridd_begin(1, 1);
        {
            if (add_button("Wire Frame"))
            {
                if (!wire_frame)
                {
                    test.main_g_pipeline.poly_mode = VK_POLYGON_MODE_LINE;
                }
                else
                {
                    test.main_g_pipeline.poly_mode = VK_POLYGON_MODE_FILL;
                }
                b_switch(wire_frame);
                recreate_game(NULL, region, app_state);
            }
        }
        gridd_end();
        gridd_begin(1, 1);
        {
            add_text("Position (x, y, z) This is a test");
        }
        gridd_end();

        gridd_begin(3, 1);
        {
            add_input_float(&scaling_value.x, -100.0f, 100.0f, 3.0f);
            add_input_float(&scaling_value.y, -100.0f, 100.0f, 3.0f);
            add_input_float(&scaling_value.z, -100.0f, 100.0f, 3.0f);
        }
        gridd_end();

        gridd_begin(1, 1);
        {
            add_text("Freq --- Grain --- Oct --- Max Height");
        }
        gridd_end();

        gridd_begin(4, 1);
        {
            add_input_float(&freq, 0.0f, 10.0f, 1.0f);
            add_input_float(&grain, 0.0f, 10.0f, 1.0f);
            add_input_float(&oct, 0.0f, 10.0f, 1.0f);
            add_input_float(&max_height, 0.0f, 20.0f, 2.0f);
        }
        gridd_end();

        gridd_begin(1, 1);
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
            add_text(temp);
        }
        gridd_end();
        gridd_begin(1, 1);
        {
            presist char temp[200] = { 0 };
            presist f32 count = 1.0f;
            if (count >= 0.1f)
            {
                sprintf_s(
                    temp, sizeof(temp),
                    "|%f,%f,%f,%f|\n|%f,%f,%f,%f|\n|%f,%f,%f,%f|\n|%f,%f,%f,%f|\n\n",
                    test.figur_cam.mvp.model.data[0][0],
                    test.figur_cam.mvp.model.data[1][0],
                    test.figur_cam.mvp.model.data[2][0],
                    test.figur_cam.mvp.model.data[3][0],
                    test.figur_cam.mvp.model.data[0][1],
                    test.figur_cam.mvp.model.data[1][1],
                    test.figur_cam.mvp.model.data[2][1],
                    test.figur_cam.mvp.model.data[3][1],
                    test.figur_cam.mvp.model.data[0][2],
                    test.figur_cam.mvp.model.data[1][2],
                    test.figur_cam.mvp.model.data[2][2],
                    test.figur_cam.mvp.model.data[3][2],
                    test.figur_cam.mvp.model.data[0][3],
                    test.figur_cam.mvp.model.data[1][3],
                    test.figur_cam.mvp.model.data[2][3],
                    test.figur_cam.mvp.model.data[3][3]);
                count = 0.0f;
            }
            count += dt;
            add_text(temp);
        }
        gridd_end();
    }
    back_bord_end();

    back_bord_begin("Terminal", v2f(500.0f, 100.0f));
    {
        add_terminal(250.0f, 200.0f);
    }
    back_bord_end();
}

#if 1
internal V3 convert_to_noise_coords(V2 x_z)
{
    V3 out = v3f((x_z.x * OFFSET_INCREASE) / QUAD_WIDTH, 0.0f,
                 (x_z.y * OFFSET_INCREASE) / QUAD_DEPTH);

    out.y = (sy_value_noise2d(out.x, out.z, freq, grain, (i32)oct) * max_height);

    return out;
}
#endif

#define sample_count 1000

internal b8 record(f32 dt)
{
    presist b8 p_pressed = false;
    presist b8 q_pressed = false;
    presist u32 count_rec = 0;
    presist u32 count_play = 0;
    presist M4 rec[sample_count] = { 0 };
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
            print("Stop Rec\n");
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
            if (count_rec < sample_count)
            {
                print("Rec: %u / %u\n", count_rec + 1, sample_count);
                rec[count_rec++] = test.cam.mvp.view;
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
            print("Start Playing\n");
            p_pressed = true;
        }
        else
        {
            print("Stop Playing\n");
            p_pressed = false;
        }
        count_play = 0;
    }
    if (p_pressed)
    {
        sec += dt;
        if (sec >= sample_time)
        {
            test.cam.mvp.view = rec[count_play++];
            count_play %= count_rec;
            sec = 0.0f;
        }
    }
    return p_pressed;
}

void update_game(Region_Alloc* region, const Application_State* app_state,
                 VkDevice device, V2 dimensions, u32 semaphore_idx, f32 dt)
{
    presist b8 off_the_ground = true;

#if 1
    presist f32 sec_brezier = 0.0f;
    sec_brezier += dt;
    if (sec_brezier >= 0.01f)
    {
        u32 size = (size_arr(test.line_g_pipeline.vert_buffer.data) * 2) - 2;
        Index_Buffer* idx = &test.line_g_pipeline.idx_buffer;
        if (idx->curr_size < size)
        {
            idx->curr_size++;
        }
        else
        {
            presist f32 next_timer = 0.0f;
            next_timer += sec_brezier;
            if (next_timer >= 2.0f)
            {
                idx->curr_size = 0;
                next_timer = 0.0f;
            }
        }
        f32 t = (f32)idx->curr_size / (f32)size;
        test.figur_cam.pos = spline_curve_pos(spline, t);

        sec_brezier = 0.0f;
    }
#endif

    if (!gui_focus())
    {
        update_camera(&test.cam, test.mouse_evt, dt, off_the_ground);
    }

    if (!record(dt))
    {
        test.cam.mvp.view =
            view(test.cam.pos, v3_add(test.cam.pos, test.cam.ori), test.cam.up);
    }

    presist f32 rotation = 45.0f;

    test.cam.mvp.proj =
        perspective(radians(rotation), dimensions.x / dimensions.y, 0.1f, 100.0f);

    test.figur_cam.mvp.proj =
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
    Vertex_Buffer* vert = &test.main_g_pipeline.vert_buffer;
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
    copy_data_buffer(&vert->buffer, vert->data, vert->buffer.size_bytes);
#ifdef multithreaded
    for (u32 i = 0; i < MAX_THREADS; i++)
    {
        ReleaseSemaphore(start_semaphore, 1, 0);
    }
#endif
#endif

#if 1
    copy_data_buffer(&test.main_g_pipeline.uniform_buffers[semaphore_idx].buffer,
                     &test.cam.mvp, sizeof(test.cam.mvp));

    if (is_key_pressed(SYNT_KEY_LEFT))
    {
        test.figur_cam.pos.x -= 3.0f * dt;
    }
    if (is_key_pressed(SYNT_KEY_UP))
    {
        test.figur_cam.pos.z -= 3.0f * dt;
    }
    if (is_key_pressed(SYNT_KEY_RIGHT))
    {
        test.figur_cam.pos.x += 3.0f * dt;
    }
    if (is_key_pressed(SYNT_KEY_DOWN))
    {
        test.figur_cam.pos.z += 3.0f * dt;
    }

#if 0
    test.figur_cam.mvp.view =
        view(test.figur_cam.pos, v3_add(test.figur_cam.pos, test.figur_cam.ori),
             test.figur_cam.up);
#endif

    MVP final_mvp = test.figur_cam.mvp;
    final_mvp.view = test.cam.mvp.view;
    final_mvp.model = m4_translate(test.figur_cam.pos);

    copy_data_buffer(&test.figur_g_pipeline.uniform_buffers[semaphore_idx].buffer,
                     &final_mvp, sizeof(final_mvp));

#ifdef LINES
    copy_data_buffer(&test.line_g_pipeline.uniform_buffers[semaphore_idx].buffer,
                     &test.cam.mvp, sizeof(test.cam.mvp));
#endif
#endif

    draw_pipeline(render_game, NULL);

    gui_update_begin(region, dimensions, semaphore_idx, dt, translucentcy);
    {
        update_gui(region, app_state, dt, dimensions);
    }
    gui_update_end();
}

#if 0
void render_render_testing(VkCommandBuffer command_buffer, u32semaphore_idx)
{
    bind_and_draw_graphics_pipline(
        command_buffer, test.g_pipeline.descriptors.desc_sets[semaphore_idx],
        test.g_pipeline);
}
#endif


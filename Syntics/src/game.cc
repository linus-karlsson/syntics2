#include "game.h"
#include "defines.h"
#include "logging.h"
#include "math/matrix.h"
#include "math/vectors.h"
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
// #define MOVE_ALL

#define pack(d, v0, v1, v2)                                                         \
    do                                                                              \
    {                                                                               \
        ASSERT(v0 < 2 && v1 < 0x1FFFFFFF && v2 < 4, "pack to big values");          \
        (d) = ((u32)(v0) << 31) | ((u32)(v1) << 2) | ((u32)(v2)&0x3);               \
    } while (0)

#define unpack_side(d) ((d) >> 31)
#define unpack_curve(d) (((d) >> 2) & 0x1FFFFFFF)
#define unpack_point(d) ((d)&0x3)

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

    Rect3D* rects;

    Camera_3D cam;
    Camera_3D figur_cam;

    Texture* textures;
    Font font;
    Events* mouse_evt;

    sygui::Window_Handle* win_handles;

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

typedef struct Cubic_Brezier_Curve
{
    V3 p[4];
    u32 points_indices[4];
    u32 vertex_offset;
} Cubic_Brezier_Curve;

typedef struct Brezier_Spline
{
    Cubic_Brezier_Curve* bc;
    u32 n_curves;
    u32 splitt;
} Brezier_Spline;

typedef struct Brezier_Spline_3D
{
    Cubic_Brezier_Curve* bc[2];
    u32 n_curves;
    u32 splitt;
} Brezier_Spline_3D;

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

u32 index_offset = 0;

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
global Thread_Attrib threads[MAX_THREADS] = {};

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
        V3 side0 = next_pos0 - pos;
        V3 side1 = next_pos1 - pos;
        V3 normal = v3_normalize(v3_cross(side0, side1));
        vert->data[i].normal = normal;
    }
}

global V3 g_light_pos = { { { 0.0, 1.0, 0.0 } } };

global u32 current_number_of_curves = 0;

internal void save_game_binary(const Brezier_Spline_3D& spline, V3 camera_pos)
{
    stack_begin_scope();

    u32 brezier_curves_size0 = spline.n_curves * sizeof(V3) * 4 * 2;
    u32 brezier_curves_size1 = spline.n_curves * sizeof(u32) * 4 * 2;
    u32 size = (sizeof(u32) * 2) + brezier_curves_size0 + brezier_curves_size1 +
               sizeof(V3) + sizeof(u32);

    u8* buffer = stack_calloc(size, u8);
    u8* current_pos = buffer;

    memcpy(current_pos, &spline.n_curves, sizeof(u32));
    current_pos += sizeof(u32);

    memcpy(current_pos, &spline.splitt, sizeof(u32));
    current_pos += sizeof(u32);

    ASSERT((current_pos - buffer) + (brezier_curves_size0 + brezier_curves_size1) <=
               size,
           "");

    for (u32 i = 0; i < spline.n_curves; i++)
    {
        for (u32 j = 0; j < 4; j++)
        {
            memcpy(current_pos, &spline.bc[0][i].p[j], sizeof(V3));
            current_pos += sizeof(V3);
            memcpy(current_pos, &spline.bc[0][i].points_indices[j], sizeof(u32));
            current_pos += sizeof(u32);
        }
    }
    for (u32 i = 0; i < spline.n_curves; i++)
    {
        for (u32 j = 0; j < 4; j++)
        {
            memcpy(current_pos, &spline.bc[1][i].p[j], sizeof(V3));
            current_pos += sizeof(V3);
            memcpy(current_pos, &spline.bc[1][i].points_indices[j], sizeof(u32));
            current_pos += sizeof(u32);
        }
    }

    ASSERT((current_pos - buffer) + sizeof(V3) <= size, "");

    memcpy(current_pos, &camera_pos, sizeof(V3));
    current_pos += sizeof(V3);

    memcpy(current_pos, &current_number_of_curves, sizeof(u32));

    write_entire_file("saved_spline3_game.synt", (char*)buffer, size);

    stack_end_scope();
}

internal void save_game_binary(Vertex* vert_data, u32* index_data,
                               const Brezier_Spline_3D& spline, V3 camera_pos)
{
    stack_begin_scope();

    u32 vert_size = size_arr(vert_data);
    u32 vert_size_bytes = vert_size * (u32)sizeof(Vertex);
    u32 index_size = size_arr(index_data);
    u32 index_size_bytes = index_size * (u32)sizeof(u32);
    u32 brezier_curves_size = spline.n_curves * sizeof(Cubic_Brezier_Curve);
    u32 size = vert_size_bytes + index_size_bytes + (2 * sizeof(u32)) +
               sizeof(spline) + (brezier_curves_size * 2) + sizeof(V3) + sizeof(u32);

    u8* buffer = stack_calloc(size, u8);
    u8* current_pos = buffer;

    *((u32*)current_pos) = vert_size;
    current_pos += sizeof(u32);

    ASSERT((current_pos - buffer) + vert_size_bytes < size, "");

    memcpy(current_pos, vert_data, vert_size_bytes);
    current_pos += vert_size_bytes;

    *((u32*)(current_pos)) = index_size;
    current_pos += sizeof(u32);

    ASSERT((current_pos - buffer) + index_size_bytes < size, "");

    memcpy(current_pos, index_data, index_size_bytes);
    current_pos += index_size_bytes;

    ASSERT((current_pos - buffer) + sizeof(spline) < size, "");

    memcpy(current_pos, &spline, sizeof(spline));
    current_pos += sizeof(spline);

    ASSERT((current_pos - buffer) + (brezier_curves_size * 2) <= size, "");

    memcpy(current_pos, spline.bc[0], brezier_curves_size);
    current_pos += brezier_curves_size;

    memcpy(current_pos, spline.bc[1], brezier_curves_size);
    current_pos += brezier_curves_size;

    ASSERT((current_pos - buffer) + sizeof(V3) <= size, "");

    memcpy(current_pos, &camera_pos, sizeof(V3));
    current_pos += sizeof(V3);

    memcpy(current_pos, &current_number_of_curves, sizeof(u32));

    write_entire_file("saved_spline_game.synt", (char*)buffer, size);

    stack_end_scope();
}

global u32 circle_offset = 0;

global u32 circle_curr_size = 0;

internal void render_game(void* data, VkCommandBuffer command_buffer,
                          u32 semaphore_idx)
{
#if 0
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
        circle_curr_size, &test.line_g_pipeline);

    bind_and_draw_graphics_pipline(
        command_buffer, test.line_g_pipeline.descriptors.desc_sets[semaphore_idx],
        circle_offset, idx3->curr_size - circle_offset, &test.line_g_pipeline);

#endif
}

internal void recreate_game(void* data, Region_Alloc* region,
                            const Application_State* app_state)
{
    recreate_graphic_pipline_ap(region, app_state, "Syntics/res/game.vert.spv",
                                "Syntics/res/game.frag.spv", &test.main_g_pipeline,
                                size_arr(test.textures), NULL);
    sygui::recreate(region);
}

// Brezier_Spline spline = {};
Brezier_Spline_3D spline2 = {};

internal void destroy_game(void* data, VkDevice device, u32 num_semaphores)
{
#if 0
    save_game_binary(test.line_g_pipeline.vert_buffer.data,
                     test.line_g_pipeline.idx_buffer.data, spline2);
#endif
    destroy_graphic_pipeline(device, num_semaphores, &test.main_g_pipeline);
    destroy_graphic_pipeline(device, num_semaphores, &test.figur_g_pipeline);
#ifdef LINES
    destroy_graphic_pipeline(device, num_semaphores, &test.line_g_pipeline);
#endif

    for (u32 i = 0; i < size_arr(test.textures); i++)
    {
        destroy_texture(device, test.textures[i]);
    }

    sygui::destroy(device, num_semaphores);
}

Brezier_Spline spline_create(Region_Alloc* region, u32 n_curves)
{
    Brezier_Spline out;
    out.bc = dyn_arrayP(region, n_curves, Cubic_Brezier_Curve);
    out.n_curves = n_curves;
    return out;
}

#define PROCENT_INCREASE 0.01f

internal u32 create_circle(Vertex* data, u32 offset, V3 pos, f32 radius)
{
    for (f32 i = 0; i < 360.0f; i += 36.0f)
    {
        V3 p = pos + (v3f(cosf(radians(i)), sinf(radians(i)), 0.0f) * radius);

        Vertex vertex = vertex_create(p, v3d(), v2d(), v4i(1.0f), DEFAULT_TEXTURE);

        val(data, offset++) = vertex;
    }
    return offset;
}

internal u32 create_circles_spline_curve(Vertex* data, u32 offset,
                                         Brezier_Spline_3D* spline, u32 curve,
                                         f32 radius)
{
    for (u32 k = 0; k < 2; k++)
    {
        for (u32 j = 0; j < 4; j++)
        {
            Rect3D rect = {};
            rect.pos = spline->bc[k][curve].p[j];
            rect.size = v3i(radius);
            pack(rect.id, k, curve, j);
            u32 size = size_arr(test.rects);
            b8 found = false;
            for (u32 i = 0; i < size; i++)
            {
                Rect3D* rect_ref = test.rects + i;
                if (rect_ref->id == rect.id)
                {
                    *rect_ref = rect;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                synt_push(test.rects, rect);
            }

            spline->bc[k][curve].points_indices[j] = offset;
            offset = create_circle(data, offset, spline->bc[k][curve].p[j], radius);
        }
    }
    return offset;
}

internal u32 create_circles_spline(Vertex* data, u32 offset, Brezier_Spline* spline,
                                   f32 radius)
{
    for (u32 i = 0; i < spline->n_curves; i++)
    {
        for (u32 j = 0; j < 4; j++)
        {
            Rect3D rect = {};
            rect.pos = spline->bc[i].p[j];
            rect.size = v3i(radius);
            // pack(rect.id, i, j);
            synt_push(test.rects, rect);

            spline->bc[i].points_indices[j] = offset;
            offset = create_circle(data, offset, spline->bc[i].p[j], radius);
        }
    }
    return offset;
}

internal u32 create_circles_spline(Vertex* data, u32 offset,
                                   Brezier_Spline_3D* spline, f32 radius)
{
    for (u32 i = 0; i < spline->n_curves; i++)
    {
        offset = create_circles_spline_curve(data, offset, spline, i, radius);
    }
    return offset;
}

internal V3 brezier_curve_pos(Cubic_Brezier_Curve brezier_curve, f32 t)
{
    V3 p0 = v3_lerp(brezier_curve.p[0], brezier_curve.p[1], t);
    V3 p1 = v3_lerp(brezier_curve.p[1], brezier_curve.p[2], t);
    V3 p2 = v3_lerp(brezier_curve.p[2], brezier_curve.p[3], t);
    V3 p3 = v3_lerp(p0, p1, t);
    V3 p4 = v3_lerp(p1, p2, t);
    return v3_lerp(p3, p4, t);
}

internal u32 generate_curve(Cubic_Brezier_Curve brezier_curve, Vertex* data,
                            u32 offset)
{
    u32 count = offset;
    for (f32 i = 0.0; i <= 1.0f; i += PROCENT_INCREASE)
    {
        Vertex vertex = vertex_create(brezier_curve_pos(brezier_curve, i), v3d(),
                                      v2d(), v4i(1.0f), DEFAULT_TEXTURE);
        val(data, count++) = vertex;
    }
    return count;
}

internal V3 generate_positions_curve(Brezier_Spline_3D* spline, V3 pos, u32 side,
                                     u32 curve)
{
    V3 last_pos = pos;
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

internal void generate_positions(Brezier_Spline_3D* spline, V3 pos)
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

internal u32 generate_spline(Brezier_Spline_3D* spline, Vertex* data, u32 offset)
{
    b8 first = true;
    for (u32 i = 0; i < spline->n_curves; i++)
    {
        for (u32 j = 0; j < 2; j++)
        {
            spline->bc[j][i].vertex_offset = offset;
            if (first)
            {
                offset = generate_curve(spline->bc[j][i], data, offset);
                spline->splitt = offset - (spline->n_curves * 8 * 10);
                spline->splitt *= 2;
                first = false;
            }
            else
            {
                offset = generate_curve(spline->bc[j][i], data, offset);
            }
        }
    }
    return offset;
}

u32 generate_curve_normals(Vertex* data, u32 offset, u32 spline_splitt,
                           i32 half_splitt, u32 first_index, u32 second_index)
{
    u32 count = offset;
    V3 next_pos[2] = {};
    u32 iterations = spline_splitt / 2;
    for (u32 i = 0; i < iterations - 1; i++)
    {
        V3 pos = val(data, count).pos;
        next_pos[first_index] = val(data, count + half_splitt).pos;
        next_pos[second_index] = val(data, count + 1).pos;
        V3 side0 = next_pos[0] - pos;
        V3 side1 = next_pos[1] - pos;
        V3 normal = v3_normalize(v3_cross(side0, side1));
        val(data, count++).normal = normal;
    }
    V3 pos = val(data, count).pos;
    next_pos[second_index] = val(data, count + half_splitt).pos;
    next_pos[first_index] = val(data, count - 1).pos;
    V3 side0 = next_pos[0] - pos;
    V3 side1 = next_pos[1] - pos;
    V3 normal = v3_normalize(v3_cross(side0, side1));
    val(data, count++).normal = normal;
    return count;
}

void generate_spline_normals(Vertex* data, u32 offset, const Brezier_Spline& spline)
{
    i32 half_splitt = spline.splitt / 2;
    for (u32 i = 0; i < spline.n_curves; i++)
    {
        offset =
            generate_curve_normals(data, offset, spline.splitt, half_splitt, 0, 1);
    }
}

void generate_spline_normals(Vertex* data, u32 offset,
                             const Brezier_Spline_3D& spline)
{
    i32 half_splitt = spline.splitt / 2;
    for (u32 i = 0; i < spline.n_curves; i++)
    {
        offset =
            generate_curve_normals(data, offset, spline.splitt, half_splitt, 0, 1);
        offset =
            generate_curve_normals(data, offset, spline.splitt, -half_splitt, 1, 0);
    }
}

internal void generate_spline_curve(Brezier_Spline_3D* spline, u32 side, u32 curve)
{
    i32 half_splitt = spline->splitt / 2;
    // (spline->n_curves * 8 * 10) for the circle representation
    // u32 offset = (((u32)half_splitt) * side) + (curve * spline->splitt) +
    //            (spline->n_curves * 8 * 10);
    u32 offset = spline->bc[side][curve].vertex_offset;

    Vertex* data = test.line_g_pipeline.vert_buffer.data;
    u32 n = generate_curve(spline->bc[side][curve], data, offset);

    if (side == 0)
    {
        generate_curve_normals(data, offset, spline->splitt, half_splitt, 0, 1);
    }
    else
    {
        generate_curve_normals(data, offset, spline->splitt, -half_splitt, 1, 0);
    }

    u32 offset2 = ((spline->splitt / 2) * side) + (curve * spline->splitt);
    Vertex* data2 = test.figur_g_pipeline.vert_buffer.data;

    for (u32 i = offset; i < n; i++)
    {
        val(data2, offset2++) = val(data, i);
    }
}

internal void generate_spline_at_curve(Brezier_Spline_3D* spline, u32 side,
                                       u32 curve, u32 point, V3 pos)
{
    spline->bc[side][curve].p[point] = pos;
    generate_spline_curve(spline, side, curve);
}

internal void generate_spline_at_curve(Brezier_Spline_3D* spline, u32 curve)
{
    for (u32 i = 0; i < 2; i++)
    {
        generate_spline_curve(spline, i, curve);
    }
}

internal void generate_positions(Brezier_Spline* spline, V3 pos)
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

internal u32 generate_spline(Brezier_Spline* spline, Vertex* data, u32 offset)
{
    for (u32 i = 0; i < spline->n_curves; i++)
    {
        if (!i)
        {
            offset = generate_curve(spline->bc[i], data, offset);
            spline->splitt = offset - (spline->n_curves * 4 * 10);
        }
        else
        {
            offset = generate_curve(spline->bc[i], data, offset);
        }
    }
    return offset;
}

internal void generate_spline_at_curve(Brezier_Spline* spline, u32 curve, u32 point,
                                       V3 pos)
{
    spline->bc[curve].p[point] = pos;

    // (spline->n_curves * 4 * 10) for the circle representation
    u32 offset = (curve * spline->splitt) + (spline->n_curves * 4 * 10);
    Vertex* data = test.line_g_pipeline.vert_buffer.data;
    generate_curve(spline->bc[curve], data, offset);
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

global HANDLE thread_handle[MAX_THREADS] = { 0 };

global u32 index_to_test = 0;
global u32 num_points = 0;
global u32 points_size = ((u32)(1.0f / PROCENT_INCREASE) + 1) * 2;

void init_game(Region_Alloc* region, VkDevice device,
               VkPhysicalDevice physical_device, VkCommandPool command_pool,
               VkQueue graphic_queue, const Swap_Chain_attrib* swap_chain,
               u32 num_semaphores)
{
    stack_begin_scope();

    test.win_handles = dyn_array_callocP(region, 10, sygui::Window_Handle);
    test.rects = dyn_array_callocP(region, 1000, Rect3D);

    const char* paths[] = {
        "Syntics/res/default.png",
        "Syntics/res/kiha32/1591184735691.png",
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

    test.cam = cam_3di(4.0f, 5.0f);
    test.figur_cam = cam_3di(2000.0f, 5.0f);
    u32 vert_offset = 0;
    { // Lines
        Graphic_Pipeline* g_p = &test.line_g_pipeline;
        *g_p = gp_default1(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

#if 0
        File_Attrib file = {};
        read_file(&file, get_stack(), "saved_spline_game.synt", "rb");

        u32 vert_size = *((u32*)file.buffer);
        file.buffer += sizeof(u32);

        g_p->vert_buffer.data = dyn_array_callocP(region, vert_size, Vertex);
        memcpy(g_p->vert_buffer.data, file.buffer, vert_size * sizeof(Vertex));
        get_head(g_p->vert_buffer.data)->size = vert_size;
        file.buffer += vert_size * sizeof(Vertex);

        u32 index_size = *((u32*)file.buffer);
        file.buffer += sizeof(u32);

        g_p->idx_buffer.data = dyn_array_callocP(region, index_size, u32);
        memcpy(g_p->idx_buffer.data, file.buffer, index_size * sizeof(u32));
        get_head(g_p->idx_buffer.data)->size = index_size;
        file.buffer += index_size * sizeof(u32);
#else
#endif
#if 1
        File_Attrib file = {};
        read_file(&file, get_stack(), "saved_spline3_game.synt", "rb");

        spline2.n_curves = *((u32*)file.buffer);
        file.buffer += sizeof(u32);
        spline2.splitt = *((u32*)file.buffer);
        file.buffer += sizeof(u32);

        spline2.bc[0] =
            dyn_array_callocP(region, spline2.n_curves, Cubic_Brezier_Curve);
        spline2.bc[1] =
            dyn_array_callocP(region, spline2.n_curves, Cubic_Brezier_Curve);

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
        read_file(&file, get_stack(), "saved_spline2_game.synt", "rb");

        spline2 = *((Brezier_Spline_3D*)file.buffer);
        file.buffer += sizeof(Brezier_Spline_3D);

        spline2.bc[0] =
            dyn_array_callocP(region, spline2.n_curves, Cubic_Brezier_Curve);
        spline2.bc[1] =
            dyn_array_callocP(region, spline2.n_curves, Cubic_Brezier_Curve);

        memcpy(spline2.bc[0], file.buffer,
               spline2.n_curves * sizeof(Cubic_Brezier_Curve));
        file.buffer += spline2.n_curves * sizeof(Cubic_Brezier_Curve);
        memcpy(spline2.bc[1], file.buffer,
               spline2.n_curves * sizeof(Cubic_Brezier_Curve));
        file.buffer += spline2.n_curves * sizeof(Cubic_Brezier_Curve);
#endif

        vert_offset = spline2.n_curves * 8 * 10;

        test.figur_cam.pos = *((V3*)file.buffer);
        file.buffer += sizeof(V3);

        current_number_of_curves = *((u32*)file.buffer);

        u32 point_all_size = points_size * spline2.n_curves;
        num_points = spline2.n_curves * 8;
        g_p->idx_buffer.data = dyn_array_callocP(
            region, (point_all_size + (num_points * 10) + 1) * 2, u32);
        u32 count = 0;
        u32 first_index = 0;
        for (u32 i = 0; i < num_points; i++)
        {
            first_index = count;
            for (u32 j = 0; j < 9; j++)
            {
                synt_push(g_p->idx_buffer.data, count++);
                synt_push(g_p->idx_buffer.data, count);
            }
            synt_push(g_p->idx_buffer.data, count++);
            synt_push(g_p->idx_buffer.data, first_index);
        }

        u32 size = point_all_size + (num_points * 10);
        g_p->vert_buffer.data = dyn_array_callocP(region, size, Vertex);

        vert_offset =
            create_circles_spline(g_p->vert_buffer.data, 0, &spline2, 0.08f);
        u32 size33 = generate_spline(&spline2, g_p->vert_buffer.data, vert_offset);
        get_head(g_p->vert_buffer.data)->size = size33;
        generate_spline_normals(g_p->vert_buffer.data, vert_offset, spline2);

        size33 -= vert_offset;
        size33 += count;

        u32 vertex_count = 0;
        u32 i;
        u32 half_splitt = spline2.splitt / 2;
        for (i = count; i < size33 - 1; i++)
        {
            if (++vertex_count % half_splitt != 0)
            {
                synt_push(g_p->idx_buffer.data, i);
                synt_push(g_p->idx_buffer.data, i + 1);
            }
        }
        num_points = spline2.n_curves * 8;

        circle_offset = (num_points * 10) * 2;
        circle_curr_size = (8 * (10 * 2)) * current_number_of_curves;

        g_p->idx_buffer.curr_size =
            circle_offset + (current_number_of_curves * (points_size * 2 - 4));
        g_p->line_width = 5.0f;
        g_p->textures = test.textures;
        g_p->vert_path = "Syntics/res/gui.vert.spv";
        g_p->frag_path = "Syntics/res/gui_graph.frag.spv";
        create_graphics_pipeline_deluxe(region, device, physical_device,
                                        command_pool, graphic_queue, num_semaphores,
                                        swap_chain, swap_chain->extent_2D, 1, NULL,
                                        VERTEX_INDEX_VISIBLE_LOCAL, g_p);

    } ///////////////////////////////////////////////////////

    { // Small cube
        Graphic_Pipeline* g_p = &test.figur_g_pipeline;
        *g_p = gp_default1(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);

        u32 size = size_arr(test.line_g_pipeline.vert_buffer.data) - vert_offset;
        g_p->vert_buffer.data = dyn_arrayP(region, size, Vertex);
        for (u32 i = vert_offset; i < size + vert_offset; i++)
        {
            synt_push(g_p->vert_buffer.data,
                      val(test.line_g_pipeline.vert_buffer.data, i));
        }

        g_p->idx_buffer.data = dyn_arrayP(get_stack(), size, u32);

#if 1
        u32 vertex_count = 0;
        u32 count = 0;
        u32 half_size = size / 2;
        u32 half_splitt = spline2.splitt / 2;
        for (u32 i = 0; i < half_size; i++)
        {
            synt_push(g_p->idx_buffer.data, count);
            synt_push(g_p->idx_buffer.data, count++ + half_splitt);
            if (++vertex_count % half_splitt == 0)
            {
                count += half_splitt;
            }
        }
#endif
        g_p->idx_buffer.curr_size = points_size * current_number_of_curves;
        g_p->vert_path = "Syntics/res/game.vert.spv";
        g_p->frag_path = "Syntics/res/game.frag.spv";
        g_p->textures = test.textures;
        create_graphics_pipeline_deluxe(region, device, physical_device,
                                        command_pool, graphic_queue, num_semaphores,
                                        swap_chain, swap_chain->extent_2D, num_text,
                                        NULL, VERTEX_INDEX_VISIBLE_LOCAL, g_p);
    } ////////////////////////////////////////////////////////////////

    subscribe(&test.mouse_evt, EVT_MOUSE);

    subscribe_recreate_callback(recreate_game, NULL);
    subscribe_destroy_callback(destroy_game, NULL);

    sygui::init(region, device, physical_device, command_pool, graphic_queue,
                swap_chain, num_semaphores, true);

    test.win_handles[0] = sygui::create_window();
    test.win_handles[1] = sygui::create_window();

    test.cam.pos = test.figur_cam.pos;

    stack_end_scope();
}

global f32 translucentcy = 0.8f;
global b32 wire_frame = false;

global V3 scaling_value = { { { 0.0f, 0.0f, 0.0f } } };

internal void update_gui(Region_Alloc* region, const Application_State* app_state,
                         f32 dt, V2 dimensions)
{
    sygui::begin_pane(test.win_handles[0], "First thing", v2f(10.0f, 10.0f));
    {
        sygui::begin_gridd(1, 1);
        {
            presist char buffer[100] = { 0 };
            u32 size = 0;
            if (sygui::add_input_text(buffer, &size))
            {
                buffer[size] = '\0';
                print("%s\n", buffer);
            }
        }
        sygui::end_gridd();
        sygui::begin_gridd(2, 1);
        {
            sygui::add_text("Translucentcy: ");
            sygui::add_input_float_d(&translucentcy, 0.0f, 1.0f);
        }
        sygui::end_gridd();
        sygui::begin_gridd(4, 1);
        {
            if (sygui::add_button("OFF"))
            {
                translucentcy = 0.0f;
            }
            if (sygui::add_button("Low"))
            {
                translucentcy = 0.2f;
            }
            if (sygui::add_button("High"))
            {
                translucentcy = 0.8f;
            }
            if (sygui::add_button("Fill"))
            {
                translucentcy = 1.0f;
            }
        }
        sygui::end_gridd();
        sygui::begin_gridd(2, 1);
        {
            if (sygui::add_button("Wire Frame"))
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
            if (sygui::add_button("Save spline"))
            {
                save_game_binary(spline2, test.figur_cam.pos);
                save_game_binary(test.line_g_pipeline.vert_buffer.data,
                                 test.line_g_pipeline.idx_buffer.data, spline2,
                                 test.figur_cam.pos);
                print("Saved!\n");
            }
        }
        sygui::end_gridd();
        sygui::begin_gridd(2, 1);
        {
            if (sygui::add_button("Add curve"))
            {
                if (current_number_of_curves < spline2.n_curves)
                {
                    Vertex_Buffer* vert = &test.line_g_pipeline.vert_buffer;
                    V3 pos[2] = {};
                    if (current_number_of_curves > 0)
                    {
                        pos[0] = spline2.bc[0][current_number_of_curves - 1].p[3];
                        pos[1] = spline2.bc[1][current_number_of_curves - 1].p[3];
                    }
                    else
                    {
                        pos[1].z += 4.0f;
                    }
                    generate_positions_curve(&spline2, pos[0], 0,
                                             current_number_of_curves);
                    generate_positions_curve(&spline2, pos[1], 1,
                                             current_number_of_curves);

                    u32 offset = circle_curr_size / 2;
                    create_circles_spline_curve(vert->data, offset, &spline2,
                                                current_number_of_curves, 0.08f);
                    generate_spline_at_curve(&spline2, current_number_of_curves);

                    copy_data_buffer(&vert->buffer, vert->data,
                                     vert->buffer.size_bytes);
                    copy_data_buffer(
                        &test.figur_g_pipeline.vert_buffer.buffer,
                        test.figur_g_pipeline.vert_buffer.data,
                        test.figur_g_pipeline.vert_buffer.buffer.size_bytes);

                    Graphic_Pipeline* l_gp = &test.line_g_pipeline;
                    l_gp->idx_buffer.curr_size += (points_size * 2) - 4;
                    Graphic_Pipeline* f_gp = &test.figur_g_pipeline;
                    f_gp->idx_buffer.curr_size += points_size;
                    circle_curr_size += 8 * (10 * 2);
                    current_number_of_curves++;
                }
            }
            if (sygui::add_button("Remove curve"))
            {
                if (current_number_of_curves > 0)
                {
                    Graphic_Pipeline* l_gp = &test.line_g_pipeline;
                    l_gp->idx_buffer.curr_size -= (points_size * 2) - 4;
                    Graphic_Pipeline* f_gp = &test.figur_g_pipeline;
                    f_gp->idx_buffer.curr_size -= points_size;
                    circle_curr_size -= 8 * (10 * 2);
                    current_number_of_curves--;
                }
            }
        }
        sygui::end_gridd();
        sygui::begin_gridd(1, 1);
        {
            sygui::add_text("Position (x, y, z) This is a test");
        }
        sygui::end_gridd();

#if 0
        sygui::begin_gridd(3, 1);
        {
            sygui::add_input_float(&scaling_value.x, -100.0f, 100.0f, 3.0f);
            sygui::add_input_float(&scaling_value.y, -100.0f, 100.0f, 3.0f);
            sygui::add_input_float(&scaling_value.z, -100.0f, 100.0f, 3.0f);
        }
        sygui::end_gridd();
#endif

        sygui::begin_gridd(1, 1);
        {
            sygui::add_text("Freq --- Grain --- Oct --- Max Height");
        }
        sygui::end_gridd();

        sygui::begin_gridd(4, 1);
        {
            sygui::add_input_float(&freq, 0.0f, 10.0f, 1.0f);
            sygui::add_input_float(&grain, 0.0f, 10.0f, 1.0f);
            sygui::add_input_float(&oct, 0.0f, 10.0f, 1.0f);
            sygui::add_input_float(&max_height, 0.0f, 20.0f, 2.0f);
        }
        sygui::end_gridd();

        sygui::begin_gridd(1, 1);
        {
            if (sygui::add_button("Circle toggle"))
            {
                index_offset = index_offset == 0 ? index_to_test : 0;
            }
        }
        sygui::end_gridd();

        sygui::begin_gridd(1, 1);
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
            sygui::add_text(temp);
        }
        sygui::end_gridd();
        sygui::begin_gridd(1, 1);
        {
            presist char temp[60] = { 0 };
            presist f32 count = 1.0f;
            if (count >= 0.1f)
            {
                sprintf_s(temp, sizeof(temp), V3_FMT(test.cam.pos));
                count = 0.0f;
            }
            count += dt;
            sygui::add_text(temp);
        }
        sygui::end_gridd();
    }
    sygui::end_pane();

    sygui::begin_pane(test.win_handles[1], "Terminal", v2f(500.0f, 100.0f));
    {
        sygui::add_terminal(250.0f, 200.0f);
    }
    sygui::end_pane();
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
    presist M4 rec[sample_count] = {};
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

V3 mouse_to_device_coords(V3 mouse, V2 dimensions)
{
    V2 center = dimensions * 0.5f;
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

    V4 ray_eye = inverse(test.cam.mvp.proj) * ray_clip;
    ray_eye.z = -1.0f;
    ray_eye.w = 0.0f;

    V3 ray = v3_v4(inverse(test.cam.mvp.view) * ray_eye);
    ray = v3_normalize(ray);

    return ray;
}

b8 ray_hit_target(V3 ray, V3 camera_pos, V3 target_pos, V3 target_size, f32 distance)
{
    // f32 d = v3_distance(camera_pos, target_pos);
    //

    ray *= distance;
    ray += camera_pos;

    b8 result = false;
    if (ray.x >= target_pos.x - target_size.x &&
        ray.x <= target_pos.x + target_size.x &&
        ray.y >= target_pos.y - target_size.y &&
        ray.y <= target_pos.y + target_size.y &&
        ray.z >= target_pos.z - target_size.z &&
        ray.z <= target_pos.z + target_size.z)
    {
        result = true;
    }

#if 0
    Vertex dd = vertex_create(camera_pos, v3d(), v2d(), v4i(1.0f), DEFAULT_TEXTURE);
    Vertex dd2 = vertex_create(ray, v3d(), v2d(), v4i(1.0f), DEFAULT_TEXTURE);

    val(test.line_g_pipeline.vert_buffer.data, index_to_test) = dd;
    val(test.line_g_pipeline.vert_buffer.data, index_to_test + 1) = dd2;
#endif
    return result;
}

V3 ray_hit(V3 ray, V3 camera_pos, V3 target_pos)
{
    f32 d = v3_distance(camera_pos, target_pos);
    ray *= d;
    ray += camera_pos;
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

internal void edit_spline(V2 dimensions, b8 camera_moved)
{
    presist b8 hit = false;
    presist b8 first = true;
    presist b8 xyz_pressed = false;
    presist b8 needs_updating = false;
    presist b8 all_move = false;
    if (!sygui::is_focus() &&
        test.mouse_evt->mouse_evt.button_evt.action == SYNT_BUTTON_PRESS &&
        test.mouse_evt->mouse_evt.button_evt.button == SYNT_LEFT_BUTTON)
    {
        i16 x, y;
        get_pos(&x, &y);
        V3 mouse_pos = v3f((f32)x, (f32)y, 0.0f);
        mouse_pos = mouse_to_device_coords(mouse_pos, dimensions);
        V3 ray = shoot_camera_ray(mouse_pos);
        presist Rect3D* rect = NULL;

        presist V3 offset_diff = {};
        if (!hit)
        {
            if (camera_moved || needs_updating)
            {
                u32 rect_size = size_arr(test.rects);
                for (u32 i = 0; i < rect_size; i++)
                {
                    test.rects[i].misc = v3_distance(
                        test.cam.pos, test.rects[i].pos + test.figur_cam.pos);
                }
                bubble_sort_rects(test.rects, rect_size);
                camera_moved = false;
                needs_updating = false;
            }
            u32 rect_size = size_arr(test.rects);
            for (u32 i = 0; i < rect_size; i++)
            {
                rect = test.rects + i;
                hit =
                    ray_hit_target(ray, test.cam.pos, rect->pos + test.figur_cam.pos,
                                   rect->size, rect->misc);
                if (hit) break;
            }
#ifdef MOVE_ALL
            if (!hit)
            {
                offset_diff = test.figur_cam.pos -
                              ray_hit(ray, test.cam.pos, test.figur_cam.pos);
                all_move = true;
                hit = true;
            }
#endif
        }
        Vertex_Buffer* vert = &test.line_g_pipeline.vert_buffer;
#ifdef MOVE_ALL
        if (all_move)
        {
            if (is_key_pressed(SYNT_KEY_X))
            {
                test.figur_cam.pos.x =
                    (ray_hit(ray, test.cam.pos, test.figur_cam.pos) + offset_diff).x;
                xyz_pressed = true;
            }
            if (is_key_pressed(SYNT_KEY_C))
            {
                test.figur_cam.pos.y =
                    (ray_hit(ray, test.cam.pos, test.figur_cam.pos) + offset_diff).y;
                xyz_pressed = true;
            }
            if (is_key_pressed(SYNT_KEY_Z))
            {
                test.figur_cam.pos.z =
                    (ray_hit(ray, test.cam.pos, test.figur_cam.pos) + offset_diff).z;
                xyz_pressed = true;
            }

            if (!xyz_pressed)
            {
                test.figur_cam.pos =
                    ray_hit(ray, test.cam.pos, test.figur_cam.pos) + offset_diff;
            }
        }
#endif
        if (hit)
        {
            if (is_key_pressed(SYNT_KEY_X))
            {
                rect->pos.x =
                    (ray_hit(ray, test.cam.pos, rect->pos + test.figur_cam.pos) -
                     test.figur_cam.pos)
                        .x;
                xyz_pressed = true;
            }
            if (is_key_pressed(SYNT_KEY_C))
            {
                rect->pos.y =
                    (ray_hit(ray, test.cam.pos, rect->pos + test.figur_cam.pos) -
                     test.figur_cam.pos)
                        .y;
                xyz_pressed = true;
            }
            if (is_key_pressed(SYNT_KEY_Z))
            {
                rect->pos.z =
                    (ray_hit(ray, test.cam.pos, rect->pos + test.figur_cam.pos) -
                     test.figur_cam.pos)
                        .z;
                xyz_pressed = true;
            }

            if (!xyz_pressed)
            {
                rect->pos =
                    ray_hit(ray, test.cam.pos, rect->pos + test.figur_cam.pos) -
                    test.figur_cam.pos;
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
                create_circle(vert->data,
                              spline2.bc[side][curve].points_indices[point],
                              rect->pos, 0.08f);
                generate_spline_at_curve(&spline2, side, curve, point, rect->pos);
                if (!is_key_pressed(SYNT_KEY_SHIFT))
                {
                    presist Rect3D* rect2 = NULL;
                    presist V3 diff = v3d();
                    ++side %= 2;
                    if (first)
                    {
                        u32 id = 0;
                        pack(id, side, curve, point);
                        u32 rect_size = size_arr(test.rects);
                        for (u32 j = 0; j < rect_size; j++)
                        {
                            if (test.rects[j].id == id)
                            {
                                rect2 = test.rects + j;
                                break;
                            }
                        }
                        ASSERT(rect2, "Rect2 is null");
                        diff = rect2->pos - rect->pos;
                        first = false;
                    }
                    rect2->pos = rect->pos + diff;
                    create_circle(vert->data,
                                  spline2.bc[side][curve].points_indices[point],
                                  rect2->pos, 0.08f);
                    generate_spline_at_curve(&spline2, side, curve, point,
                                             rect2->pos);
                }
            }
            copy_data_buffer(&vert->buffer, vert->data, vert->buffer.size_bytes);
            copy_data_buffer(&test.figur_g_pipeline.vert_buffer.buffer,
                             test.figur_g_pipeline.vert_buffer.data,
                             test.figur_g_pipeline.vert_buffer.buffer.size_bytes);
        }
        else
        {
            first = false;
        }
    }
    else
    {
        if (xyz_pressed)
        {
            needs_updating = true;
        }
        xyz_pressed = false;
        first = true;
        hit = false;
        all_move = false;
    }
}

internal f32 point_procent_along_curve(Cubic_Brezier_Curve curve, V3 offset_position,
                                       V3 point_pos, f32 precision)
{
    f32 smallest = INFINITY;
    f32 min = 0.0f;
    f32 max = 1.0f;
    f32 result = 0.5f;

    while (max - min > precision)
    {
        V3 current_point = brezier_curve_pos(curve, result) + offset_position;
        f32 dist_squared = v3_distance_squared(point_pos, current_point);

        if (dist_squared < smallest)
        {
            smallest = dist_squared;
        }
        if (dist_squared < precision)
        {
            break;
        }
        else if (current_point.x < test.cam.pos.x)
        {
            min = result;
            result = (result + max) / 2.0f;
        }
        else
        {
            max = result;
            result = (result + min) / 2.0f;
        }
    }
    if (result <= precision)
    {
        result = 0.0f;
    }
    else if (result >= 1.0f - precision)
    {
        result = 1.0f;
    }
    return result;
}

internal V3 closest_point_along_curve(Cubic_Brezier_Curve curve, V3 offset_position,
                                      V3 point_pos, f32 precision)
{
    f32 smallest = INFINITY;
    f32 min = 0.0f;
    f32 max = 1.0f;
    f32 t = 0.5f;

    V3 current_point = v3d();
    while (max - min > precision)
    {
        current_point = brezier_curve_pos(curve, t) + offset_position;
        f32 dist_squared = v3_distance_squared(point_pos, current_point);

        if (dist_squared < smallest)
        {
            smallest = dist_squared;
        }
        if (dist_squared < precision)
        {
            break;
        }
        else if (current_point.x < test.cam.pos.x)
        {
            min = t;
            t = (t + max) / 2.0f;
        }
        else
        {
            max = t;
            t = (t + min) / 2.0f;
        }
    }
    return current_point;
}

internal b8 colide_with_spline(const Brezier_Spline_3D& spline, V3 offset_pos,
                               V3 test_pos, V3* collision_pos)
{
    // TODO: can only use this function for one spline at the moment
    presist u32 left_side_curve_index = 0;
    presist u32 right_side_curve_index = 0;
    f32 precision = 0.00001f;
    f32 procent0 = point_procent_along_curve(spline.bc[0][left_side_curve_index],
                                             offset_pos, test_pos, precision);
    // TODO: causes a small jump in the transision;
    if (procent0 >= 1.0f)
    {
        if (left_side_curve_index < current_number_of_curves - 1)
        {
            left_side_curve_index++;
        }
    }
    else if (procent0 <= 0.0f)
    {
        if (left_side_curve_index > 0)
        {
            left_side_curve_index--;
        }
    }
    f32 procent1 = point_procent_along_curve(spline.bc[1][right_side_curve_index],
                                             offset_pos, test_pos, 0.00001f);
    if (procent1 >= 1.0f)
    {
        if (right_side_curve_index < current_number_of_curves - 1)
        {
            right_side_curve_index++;
        }
    }
    else if (procent1 <= 0.0f)
    {
        if (right_side_curve_index > 0)
        {
            right_side_curve_index--;
        }
    }

    V3 first = brezier_curve_pos(spline.bc[0][left_side_curve_index], procent0) +
               offset_pos;
    V3 second = brezier_curve_pos(spline.bc[1][right_side_curve_index], procent1) +
                offset_pos;

    f32 distance_between = v3_distance(first, second);
    f32 dds = v3_distance(first, test.cam.pos);

    f32 p0 = dds / distance_between;
    V3 line = v3_lerp(first, second, p0);

    if (collision_pos)
    {
        *collision_pos = line;
    }
    return true; // if(line.y <= test_pos.y) return true;
}

void update_game(Region_Alloc* region, const Application_State* app_state,
                 VkDevice device, V2 dimensions, u32 semaphore_idx, f32 dt)
{
    presist b8 off_the_ground = true;

#if 0
    presist f32 sec_brezier = 0.0f;
    sec_brezier += dt;
    if (sec_brezier >= 0.007f)
    {
        u32 size = size_arr(test.figur_g_pipeline.idx_buffer.data);
        Index_Buffer* idx = &test.figur_g_pipeline.idx_buffer;
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
        sec_brezier = 0.0f;
    }
#endif
    presist b8 camera_moved = false;
    if (!sygui::is_focus())
    {
        camera_moved |= update_camera(&test.cam, test.mouse_evt, dt, off_the_ground);
    }

    if (is_key_pressed(SYNT_KEY_CTRL))
    {
        V3 line;
        if (colide_with_spline(spline2, test.figur_cam.pos, test.cam.pos, &line))
        {
            test.cam.pos.y = line.y + 0.18f;
        }
    }

    edit_spline(dimensions, camera_moved);

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
                     &final_mvp, sizeof(final_mvp));
#endif
#endif

    draw_pipeline(render_game, NULL);

    sygui::begin_update(region, dimensions, semaphore_idx, dt, translucentcy);
    {
        update_gui(region, app_state, dt, dimensions);
    }
    sygui::end_update();
}

#if 0
void render_render_testing(VkCommandBuffer command_buffer, u32semaphore_idx)
{
    bind_and_draw_graphics_pipline(
        command_buffer, test.g_pipeline.descriptors.desc_sets[semaphore_idx],
        test.g_pipeline);
}
#endif


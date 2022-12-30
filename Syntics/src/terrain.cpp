#include "render-testing.h"
#include "logging.h"
#include "region_alloc.h"
#include "font.h"
#include "camera.h"
#include "buffers.h"
#include "swap_chain.h"
#include "gui.h"
#include "event_system.h"
#include "file_reading.h"
#include "random.h"
#include <math.h>

namespace synt {

typedef struct Terrain_State
{
    Graphic_Pipline g_pipline;

    Camera cam;

    Texture* textures;
    Font font;
    Events* mouse_evt;

} Terrain_State;

static Terrain_State terrain_state;

static const float QUAD_WIDTH = 0.5f;
static const float QUAD_HEIHT = -0.5f;

static const uint32 TERRAIN_SIZE_X = 200;
static const uint32 TERRAIN_SIZE_Z = 150;

static const uint32 TERRAIN_SIZE = TERRAIN_SIZE_X * TERRAIN_SIZE_Z;

static int32 PERMUTATION[] = {
    151, 160, 137, 91,  90,  15,  131, 13,  201, 95,  96,  53,  194, 233, 7,   225,
    140, 36,  103, 30,  69,  142, 8,   99,  37,  240, 21,  10,  23,  190, 6,   148,
    247, 120, 234, 75,  0,   26,  197, 62,  94,  252, 219, 203, 117, 35,  11,  32,
    57,  177, 33,  88,  237, 149, 56,  87,  174, 20,  125, 136, 171, 168, 68,  175,
    74,  165, 71,  134, 139, 48,  27,  166, 77,  146, 158, 231, 83,  111, 229, 122,
    60,  211, 133, 230, 220, 105, 92,  41,  55,  46,  245, 40,  244, 102, 143, 54,
    65,  25,  63,  161, 1,   216, 80,  73,  209, 76,  132, 187, 208, 89,  18,  169,
    200, 196, 135, 130, 116, 188, 159, 86,  164, 100, 109, 198, 173, 186, 3,   64,
    52,  217, 226, 250, 124, 123, 5,   202, 38,  147, 118, 126, 255, 82,  85,  212,
    207, 206, 59,  227, 47,  16,  58,  17,  182, 189, 28,  42,  223, 183, 170, 213,
    119, 248, 152, 2,   44,  154, 163, 70,  221, 153, 101, 155, 167, 43,  172, 9,
    129, 22,  39,  253, 19,  98,  108, 110, 79,  113, 224, 232, 178, 185, 112, 104,
    218, 246, 97,  228, 251, 34,  242, 193, 238, 210, 144, 12,  191, 179, 162, 241,
    81,  51,  145, 235, 249, 14,  239, 107, 49,  192, 214, 31,  181, 199, 106, 157,
    184, 84,  204, 176, 115, 121, 50,  45,  127, 4,   150, 254, 138, 236, 205, 93,
    222, 114, 67,  29,  24,  72,  243, 141, 128, 195, 78,  66,  215, 61,  156, 180
};

static float fade(float t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

static float lerp(float a, float b, float t)
{
    return a + (t * (b - a));
}

static int32 SEED = 0;

// source nowl perlin.c
int32 noise2(int32 x, int32 y)
{
    int32 tmp = PERMUTATION[(y + SEED) % 256];
    return PERMUTATION[(tmp + x) % 256];
}

float smooth_inter(float a, float b, float t)
{
    return lerp(a, b, t * t * (3 - 2 * t));
}

float noise2d(float x, float y)
{
    int32 x_int  = x;
    int32 y_int  = y;
    float x_frac = x - x_int;
    float y_frac = y - y_int;
    int32 s      = noise2(x_int, y_int);
    int32 t      = noise2(x_int + 1, y_int);
    int32 u      = noise2(x_int, y_int + 1);
    int32 v      = noise2(x_int + 1, y_int + 1);
    float low    = smooth_inter(s, t, x_frac);
    float high   = smooth_inter(u, v, x_frac);
    return smooth_inter(low, high, y_frac);
}

float perlin2d(float x, float y, float freq, float gain, int32 oct)
{
    float amp    = gain;
    float result = 0.0f;
    float max    = 0.0f;

    for_range(i, oct)
    {
        max += 256.0f * amp;
        result += noise2d(x * freq, y * freq) * amp;
        amp *= gain;
        freq *= 2.0f;
    }

    return result / max;
}

#define MAX_HEIGT 6.0f

static float freq  = 0.41f;
static float grain = 0.6f;

static void generate_terrain(float x_off, float z_off)
{
    Vertex_Buffer* vert = &terrain_state.g_pipline.vert_buffer;
    uint32 num_indices  = 0;

    int32 X          = 0;
    int32 step_value = -1;

    float last_random[TERRAIN_SIZE_X] = {};
    Vec4 last_color[TERRAIN_SIZE_X];

    float step_value_offset = -0.1f;

    for_range(z, TERRAIN_SIZE_Z)
    {
        step_value *= -1;
        step_value_offset *= -1.0f;
        for_range(x, TERRAIN_SIZE_X)
        {
            float random_f = (perlin2d(x_off, z_off, freq, grain, 3) * MAX_HEIGT);

            Vec3 pos        = Vec3(X * QUAD_WIDTH, 0.0f, z * QUAD_HEIHT);
            Vec3 size       = Vec3(QUAD_WIDTH, 0.0f, QUAD_HEIHT);
            Vec4 color      = Vec4(random_f / MAX_HEIGT);
            color.w         = 1.0f;
            float tex_index = 0.0f;

            if (z == 0)
            {
                last_random[X] = random_f;
                last_color[X]  = color;
            }

            Vertex verts[2] = {
                { { pos.x, last_random[X], pos.z, 1.0f },
                  Vec4(last_color[X].x, last_color[X].y, last_color[X].z, 1.0f),
                  { 0.0f, 0.0f },
                  tex_index },
                { { pos.x, random_f, pos.z + size.z, 1.0f },
                  color,
                  { 0.0f, 1.0f },
                  tex_index },
            };

            last_random[X] = random_f;
            last_color[X]  = color;

            for (uint32 i = 0; i < 2; i++)
            {
                synt_push(vert->data, verts[i]);
            }

            X += step_value;
            x_off += step_value_offset;
        }
        X += step_value * -1;
        x_off += step_value_offset * -1.0f;
        z_off += 0.1f;
    }
}

void init_terrain(Region_Alloc* region, VkDevice device,
                  VkPhysicalDevice physical_device, VkCommandPool command_pool,
                  VkQueue graphic_queue, const Swap_Chain_attrib& swap_chain,
                  uint32 num_semaphores)
{

    terrain_state.textures = dyn_arrayP(region, 2, Texture);

    create_texture(device, physical_device, command_pool, graphic_queue, true,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/default.png",
                   &terrain_state.textures[0]);

    get_head(terrain_state.textures)->size++;

    terrain_state.g_pipline.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    create_graphics_pipeline(
        region, device, swap_chain.color_format, swap_chain.render_pass,
        swap_chain.sample_count, "Syntics/res/terrain.vert.spv",
        "Syntics/res/terrain.frag.spv", swap_chain.extent_2D.width,
        swap_chain.extent_2D.height, VK_CULL_MODE_NONE,
        size_arr(terrain_state.textures), &terrain_state.g_pipline);

    terrain_state.g_pipline.vert_buffer.data =
        dyn_arrayP(region, (TERRAIN_SIZE)*2, Vertex);

    terrain_state.g_pipline.idx_buffer.data =
        dyn_arrayT(region, (TERRAIN_SIZE)*2, uint32);

    for_range(i, TERRAIN_SIZE * 2)
    {
        synt_push(terrain_state.g_pipline.idx_buffer.data, i);
    }

    terrain_state.g_pipline.vert_buffer.size_bytes =
        capacity_arr(terrain_state.g_pipline.vert_buffer.data) * sizeof(Vertex);
    create_vertex_buffer(device, physical_device, command_pool, graphic_queue,
                         &terrain_state.g_pipline.vert_buffer);

    terrain_state.g_pipline.idx_buffer.size_bytes =
        size_arr(terrain_state.g_pipline.idx_buffer.data) * sizeof(uint32);
    terrain_state.g_pipline.idx_buffer.curr_size =
        size_arr(terrain_state.g_pipline.idx_buffer.data);
    create_index_buffer(device, physical_device, command_pool, graphic_queue,
                        &terrain_state.g_pipline.idx_buffer);

    region_pop(region, capacity_arr(terrain_state.g_pipline.idx_buffer.data), uint32,
               TEMP_ARRAY);

    terrain_state.g_pipline.uniform_buffers =
        region_mallocP(region, num_semaphores, Uniform_Buffer);
    terrain_state.g_pipline.descriptors.desc_sets =
        region_mallocP(region, num_semaphores, VkDescriptorSet);

    for_range(i, num_semaphores)
    {
        terrain_state.g_pipline.uniform_buffers[i].size_bytes = (uint32)sizeof(MVP);

        create_uniform_buffer(device, physical_device,
                              &terrain_state.g_pipline.uniform_buffers[i]);
    }

    create_descriptors(region, device, &terrain_state.g_pipline.descriptors,
                       num_semaphores, terrain_state.g_pipline.set_layout,
                       terrain_state.textures, size_arr(terrain_state.textures),
                       terrain_state.g_pipline.uniform_buffers);

    terrain_state.cam.position    = synt::v3f(52.0f, 8.15f, -0.5f);
    terrain_state.cam.orientation = synt::v3f(0.026f, -0.365f, -0.93f);

    terrain_state.cam.speed = 10.0f;

    terrain_state.cam.mvp.model = scale(mat4i(1.0f), v3f(1.0f, 1.0f, 1.0f));
    terrain_state.cam.mvp.view =
        synt::view(terrain_state.cam.position,
                   terrain_state.cam.position + terrain_state.cam.orientation,
                   terrain_state.cam.up);

    subscribe(&terrain_state.mouse_evt, EVT_MOUSE);
}

static void update_gui(Region_Alloc* region, float dt)
{
    back_bord_begin("TTTT", Vec2(1.0f));
    {
        gridd_begin(1, 1);
        {
            add_text("Freq -- Grain");
        }
        gridd_end();
        gridd_begin(2, 1);
        {
            add_input_float(freq, 0.0f, 1.0f);
            add_input_float(grain, 0.0f, 1.0f);
        }
        gridd_end();
    }
    back_bord_end();
}

void recreate_terrain(Region_Alloc* region, const Application_State& app_state)
{
    recreate_graphic_pipline(region, app_state, "Syntics/res/vert.spv",
                             "Syntics/res/frag.spv", terrain_state.g_pipline,
                             size_arr(terrain_state.textures));
}

void update_terrain(Region_Alloc* region, VkDevice device, const Vec2& dimensions,
                    uint32 semaphore_idx, float dt)
{
#if 1
    gui_update_begin(region, device, dimensions, semaphore_idx, dt);
    {
        update_gui(region, dt);
    }
    gui_update_end(region, device);

    if (!gui_focus())
#endif
    {
        update_camera(&terrain_state.cam, terrain_state.mouse_evt, dt);
    }

    Vertex_Buffer* vert = &terrain_state.g_pipline.vert_buffer;

    get_head(vert->data)->size = 0;

    static float x_off = 0.0f;
    static float y_off = 0.0f;

    generate_terrain(x_off, y_off);

    // x_off += 2.0f * dt;
    y_off += 2.0f * dt;

    map_copy_mem(device, &(vert->buffer_memory), vert->size_bytes, vert->data);

    terrain_state.cam.mvp.view =
        synt::view(terrain_state.cam.position,
                   terrain_state.cam.position + terrain_state.cam.orientation,
                   terrain_state.cam.up);
    terrain_state.cam.mvp.proj =
        perspective(radians(53.0f), dimensions.x / dimensions.y, 0.1f, 100.0f);

    update_uniform_buffers(device,
                           terrain_state.g_pipline.uniform_buffers[semaphore_idx],
                           &terrain_state.cam.mvp, sizeof(terrain_state.cam.mvp));
}

void render_terrain(VkCommandBuffer command_buffer, uint32 semaphore_idx)
{
    bind_and_draw_graphics_pipline(
        command_buffer, terrain_state.g_pipline.descriptors.desc_sets[semaphore_idx],
        terrain_state.g_pipline);
}

void destroy_terrain(VkDevice device, uint32 num_semaphores)
{
    vkDestroyPipelineLayout(device, terrain_state.g_pipline.layout, NULL);
    vkDestroyPipeline(device, terrain_state.g_pipline.pipeline, NULL);
    vkDestroyDescriptorSetLayout(device, terrain_state.g_pipline.set_layout, NULL);
    destroy_buffer(device, terrain_state.g_pipline.vert_buffer.buffer,
                   terrain_state.g_pipline.vert_buffer.buffer_memory);
    destroy_buffer(device, terrain_state.g_pipline.idx_buffer.buffer,
                   terrain_state.g_pipline.idx_buffer.buffer_memory);

    vkDestroyDescriptorPool(device, terrain_state.g_pipline.descriptors.desc_pool,
                            NULL);

    for (uint32 i = 0; i < num_semaphores; i++)
    {
        destroy_buffer(device, terrain_state.g_pipline.uniform_buffers[i].buffer,
                       terrain_state.g_pipline.uniform_buffers[i].buffer_memory);
    }
    for (uint32 i = 0; i < size_arr(terrain_state.textures); i++)
    {
        destroy_texture(device, terrain_state.textures[i]);
    }
}

} // namespace synt

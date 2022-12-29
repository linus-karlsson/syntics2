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

static int PERMUTATION[] = {
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

// source WIKIPEDIA
static float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }

static float lerp(float a, float b, float t) { return a + (t * (b - a)); }

static Vec2 grad(int32 x, int32 y)
{
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2; // rotation width
    unsigned a = x, b = y;
    a *= 3284157443;
    b ^= a << s | a >> w - s;
    b *= 1911520717;
    a ^= b << s | b >> w - s;
    a *= 2048419325;
    float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]
    Vec2 v;
    v.x = cos(random);
    v.y = sin(random);
    return v;
}

// Computes the dot product of the distance and gradient vectors.
float dotGridGradient(int ix, int iy, float x, float y)
{
    // Get gradient from integer coordinates
    Vec2 gradient = grad(ix, iy);

    // Compute the distance vector
    float dx = x - (float)ix;
    float dy = y - (float)iy;

    // Compute the dot-product
    return (dx * gradient.x + dy * gradient.y);
}

float p_noise(float x, float y)
{
    // Determine grid cell coordinates
    int x0 = (int)floor(x);
    int x1 = x0 + 1;
    int y0 = (int)floor(y);
    int y1 = y0 + 1;

    // Determine interpolation weights
    // Could also use higher order polynomial/s-curve here
    float sx = x - (float)x0;
    float sy = y - (float)y0;

    // Interpolate between grid point gradients
    float n0, n1, ix0, ix1, value;

    n0  = dotGridGradient(x0, y0, x, y);
    n1  = dotGridGradient(x1, y0, x, y);
    ix0 = lerp(n0, n1, sx);

    n0  = dotGridGradient(x0, y1, x, y);
    n1  = dotGridGradient(x1, y1, x, y);
    ix1 = lerp(n0, n1, sx);

    value = lerp(ix0, ix1, sy);
    return value; // Will return in range -1 to 1. To make it in range 0 to 1,
                  // multiply by 0.5 and add 0.5
}

#define MAX_HEIGT 8.0f

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
            float random_f =
                (((p_noise(x_off, z_off) * MAX_HEIGT) * 0.5f) + (MAX_HEIGT * 0.5f));

            Vec3 pos        = Vec3(X * QUAD_WIDTH, 0.0f, z * QUAD_HEIHT);
            Vec3 size       = Vec3(QUAD_WIDTH, 0.0f, QUAD_HEIHT);
            Vec4 color      = Vec4(random_f / MAX_HEIGT);
            color.w         = 1.0f;
            float tex_index = 0.0f;

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

#define WIDTH_MAP 500
#define HEIGHT_MAP 500
#define BYTE_PIXELS 4
#define BYTE_SIZE WIDTH_MAP* HEIGHT_MAP* BYTE_PIXELS

static void generate_map(uint32** buffer)
{
    for (uint32 i = 0; i < WIDTH_MAP; i++)
    {
        for (uint32 j = 0; j < HEIGHT_MAP; j++)
        {
            float rand_f = rand_f32(0.0f, 1.0f);

            Vec4 color = Vec4(rand_f);
            color.w    = 1.0f;

            (*buffer)[(i * HEIGHT_MAP) + j] = float_rgba(color);
        }
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

    generate_terrain(0, 0);

    terrain_state.g_pipline.vert_buffer.size_bytes =
        size_arr(terrain_state.g_pipline.vert_buffer.data) * sizeof(Vertex);
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

    terrain_state.cam.speed = 2.0f;

    terrain_state.cam.mvp.model = scale(mat4i(1.0f), v3f(1.0f, 1.0f, 1.0f));
    terrain_state.cam.mvp.view =
        synt::view(terrain_state.cam.position,
                   terrain_state.cam.position + terrain_state.cam.orientation,
                   terrain_state.cam.up);

    subscribe(&terrain_state.mouse_evt, EVT_MOUSE);
}

static void update_gui(Region_Alloc* region, float dt) {}

void recreate_terrain(Region_Alloc* region, const Application_State& app_state)
{
    recreate_graphic_pipline(region, app_state, "Syntics/res/vert.spv",
                             "Syntics/res/frag.spv", terrain_state.g_pipline,
                             size_arr(terrain_state.textures));
}

void update_terrain(Region_Alloc* region, VkDevice device, const Vec2& dimensions,
                    uint32 semaphore_idx, float dt)
{
#if 0
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

    // x_off += 0.01f;
    y_off += 0.02f;

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

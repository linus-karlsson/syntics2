#include "logging.h"
#include "region_alloc.h"
#include "font.h"
#include "camera.h"
#include "buffers.h"
#include "swap_chain.h"
#include "gui.h"
#include "event_system.h"
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

static const uint32 TERRAIN_SIZE_X = 300;
static const uint32 TERRAIN_SIZE_Z = 200;

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
static float oct   = 3.0f;

static void generate_terrain(float x_off, float z_off)
{
    Vertex_Buffer* vert = &terrain_state.g_pipline.vert_buffer;

    for_range(z, TERRAIN_SIZE_Z)
    {
        float ix_off = x_off;
        for_range(x, TERRAIN_SIZE_X)
        {
            // float random_f =
            //     (perlin2d(ix_off, z_off, freq, grain, (int32)oct) * MAX_HEIGT);

            Vec4 pos        = Vec4(x * QUAD_WIDTH, 1.0f, z * QUAD_HEIHT, 1.0f);
            Vec4 color      = Vec4(1.0f);
            color.w         = 1.0f;
            float tex_index = 0.0f;

            Vertex vertex = { pos, color, Vec2(ix_off, z_off), tex_index };

            synt_push(vert->data, vertex);

            ix_off += 0.1f;
        }
        z_off += 0.1f;
    }
}
static void update_terrain(float x_off, float z_off)
{
    Vertex_Buffer* vert = &terrain_state.g_pipline.vert_buffer;

    int32 idx = 0;
    for_range(z, TERRAIN_SIZE_Z)
    {
        float ix_off = x_off;
        for_range(x, TERRAIN_SIZE_X)
        {
            vert->data[idx++].tex_coords = Vec2(ix_off, z_off);
            ix_off += 0.1f;
        }
        z_off += 0.1f;
    }
}

static void update_voxel_test(float x_off, float z_off)
{
    Vertex_Buffer* vert = &terrain_state.g_pipline.vert_buffer;
    Index_Buffer* idx   = &terrain_state.g_pipline.idx_buffer;

    uint32 z_max = TERRAIN_SIZE_Z / 2;
    uint32 x_max = TERRAIN_SIZE_X / 2;

    int32 size = size_arr(idx->data);

    int32 i = 0;
    for_range(z, z_max - 1)
    {
        i += (TERRAIN_SIZE_X * 2);
        float ix_off = x_off;
        for_range(x, x_max)
        {
            vert->data[idx->data[i++]].tex_coords = Vec2(ix_off, z_off);
            vert->data[idx->data[i++]].tex_coords = Vec2(ix_off, z_off);
            vert->data[idx->data[i++]].tex_coords = Vec2(ix_off, z_off);
            vert->data[idx->data[i++]].tex_coords = Vec2(ix_off, z_off);
            ix_off += 0.1f;
        }
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
        dyn_arrayP(region, (TERRAIN_SIZE)*1, Vertex);

    terrain_state.g_pipline.idx_buffer.data =
        dyn_arrayP(region, (TERRAIN_SIZE)*2, uint32);

    generate_terrain(0.0f, 0.0f);

    Index_Buffer* idx   = &terrain_state.g_pipline.idx_buffer;
    Vertex_Buffer* vert = &terrain_state.g_pipline.vert_buffer;

    int32 I          = 0;
    int32 step_value = 1;
    for_range(i, TERRAIN_SIZE_Z - 1)
    {
        for_range(j, TERRAIN_SIZE_X)
        {
            synt_push(idx->data, (TERRAIN_SIZE_X * i) + I);
            synt_push(idx->data, (TERRAIN_SIZE_X * (i + 1)) + I);

            I += step_value;
        }
        step_value *= -1;
        I += step_value;
    }

    // NOTE: Calulate normal
#if 0

    Vec3 first  = Vec3(vert->data[idx->data[0]].pos);
    Vec3 second = Vec3(vert->data[idx->data[1]].pos);
    Vec3 third  = Vec3(vert->data[idx->data[2]].pos);

    Vec3 second_c = second - first;
    Vec3 third_c  = third - first;

    PRINT_VEC3(first);
    PRINT_VEC3(second_c);
    PRINT_VEC3(third_c);
    PRINT_VEC3(normalize(cross(third_c, second_c)));

#endif

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

    terrain_state.cam.position    = v3f(52.0f, 15.15f, -10.5f);
    terrain_state.cam.orientation = v3f(0.0f, 0.0f, -1.0f);

    terrain_state.cam.speed = 10.0f;

    terrain_state.cam.mvp.model = scale(mat4i(1.0f), v3f(1.0f, 1.0f, 1.0f));
    terrain_state.cam.mvp.view =
        view(terrain_state.cam.position,
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
            add_text("Freq --- Grain --- Oct");
        }
        gridd_end();
        gridd_begin(3, 1);
        {
            add_input_float(freq, 0.0f, 1.0f);
            add_input_float(grain, 0.0f, 2.0f);
            add_input_float(oct, 0.0f, 10.0f);
        }
        gridd_end();
    }
    back_bord_end();
}

void recreate_terrain(Region_Alloc* region, const Application_State& app_state)
{
    recreate_graphic_pipline(region, app_state, "Syntics/res/terrain.vert.spv",
                             "Syntics/res/terrain.frag.spv", terrain_state.g_pipline,
                             size_arr(terrain_state.textures));
}

void update_terrain(Region_Alloc* region, VkDevice device, const Vec2& dimensions,
                    uint32 semaphore_idx, float dt)
{
    static Vec3 pos = terrain_state.cam.position;
#if 0
    gui_update_begin(region, device, dimensions, semaphore_idx, dt);
    {
        update_gui(region, dt);
    }
    gui_update_end(region, device);

    if (!gui_focus())
#endif
    {
        terrain_state.cam.position = pos;
        update_camera(&terrain_state.cam, terrain_state.mouse_evt, dt);
        pos = terrain_state.cam.position;
    }

    update_terrain(pos.x * 0.2f, pos.z * -0.2f);
    // update_voxel_test(pos.x * -0.2f, pos.z * -0.2f);

    Vertex_Buffer* vert = &terrain_state.g_pipline.vert_buffer;
    map_copy_mem(device, &(vert->buffer_memory), vert->size_bytes, vert->data);

    terrain_state.cam.position.x = 70.0f;
    terrain_state.cam.position.z = -45.0f;

    terrain_state.cam.mvp.view =
        view(terrain_state.cam.position,
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

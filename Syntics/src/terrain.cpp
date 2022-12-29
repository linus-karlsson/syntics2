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

static const uint32 TERRAIN_SIZE_X = 30;
static const uint32 TERRAIN_SIZE_Z = 30;

static const uint32 TERRAIN_SIZE = TERRAIN_SIZE_X * TERRAIN_SIZE_Z;

static void generate_terrain()
{
    Vertex_Buffer* vert = &terrain_state.g_pipline.vert_buffer;
    for_range(z, TERRAIN_SIZE_Z)
    {
        for_range(x, TERRAIN_SIZE_X)
        {
            Vec3 pos        = Vec3(x * QUAD_WIDTH, 0.0f, z * QUAD_HEIHT);
            Vec3 size       = Vec3(QUAD_WIDTH, 0.0f, QUAD_HEIHT);
            Vec4 color      = Vec4(rand_f32(0.0f, 1.0f));
            color.w         = 1.0f;
            float tex_index = 0.0f;

            Vertex verts[4] = {
                { { pos.x, rand_f32(0.0f, 1.0f), pos.z + size.z, 1.0f },
                  { color.x, color.y, color.z, color.w },
                  { 0.0f, 0.0f },
                  tex_index },
                { { pos.x, rand_f32(0.0f, 1.0f), pos.z, 1.0f },
                  { color.x, color.y, color.z, color.w },
                  { 0.0f, 1.0f },
                  tex_index },
                { { pos.x + size.x, rand_f32(0.0f, 1.0f), pos.z, 1.0f },
                  { color.x, color.y, color.z, color.w },
                  { 1.0f, 1.0f },
                  tex_index },
                { { pos.x + size.x, rand_f32(0.0f, 1.0f), pos.z + size.z, 1.0f },
                  { color.x, color.y, color.z, color.w },
                  { 1.0f, 0.0f },
                  tex_index }
            };

            for (uint32 i = 0; i < 4; i++)
            {
                synt_push(vert->data, verts[i]);
            }
        }
    }
    Index_Buffer* idx = &terrain_state.g_pipline.idx_buffer;
    generate_indices(&(idx->data), TERRAIN_SIZE);
}

void init_terrain(Region_Alloc* region, VkDevice device,
                  VkPhysicalDevice physical_device, VkCommandPool command_pool,
                  VkQueue graphic_queue, const Swap_Chain_attrib& swap_chain,
                  uint32 num_semaphores)
{

    terrain_state.textures = dyn_arrayP(region, 1, Texture);

    create_texture(device, physical_device, command_pool, graphic_queue, true,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/default.png",
                   &terrain_state.textures[0]);

    get_head(terrain_state.textures)->size++;

    terrain_state.g_pipline.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    create_graphics_pipeline(
        region, device, swap_chain.color_format, swap_chain.render_pass,
        swap_chain.sample_count, "Syntics/res/terrain.vert.spv",
        "Syntics/res/terrain.frag.spv", swap_chain.extent_2D.width,
        swap_chain.extent_2D.height, VK_CULL_MODE_NONE,
        size_arr(terrain_state.textures), &terrain_state.g_pipline);

    terrain_state.g_pipline.vert_buffer.data =
        dyn_arrayT(region, TERRAIN_SIZE * 4, Vertex);

    terrain_state.g_pipline.idx_buffer.data =
        dyn_arrayT(region, TERRAIN_SIZE * 6, uint32);

    generate_terrain();

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
    region_pop(region, capacity_arr(terrain_state.g_pipline.vert_buffer.data),
               Vertex, TEMP_ARRAY);

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

    terrain_state.cam.position    = synt::v3f(2.64f, 8.15f, 5.55f);
    terrain_state.cam.orientation = synt::v3f(0.27f, -0.63f, -0.73f);

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

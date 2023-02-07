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
#include <Windows.h>

typedef struct Platform_Game_State
{
    Graphic_Pipline g_pipline;

    Camera cam;

    Texture* textures;
    Font font;
    Events* mouse_evt;
} Platform_Game_State;

static Platform_Game_State pl_g_state;

void init_platform_game(Region_Alloc* region, VkDevice device,
                        VkPhysicalDevice physical_device, VkCommandPool command_pool,
                        VkQueue graphic_queue, const Swap_Chain_attrib& swap_chain,
                        uint32 num_semaphores)
{

    pl_g_state.textures = dyn_arrayP(region, 2, Texture);

    create_texture(device, physical_device, command_pool, graphic_queue, true,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/default.png",
                   &pl_g_state.textures[0]);

    get_head(pl_g_state.textures)->size++;

    pl_g_state.g_pipline.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    create_graphics_pipeline(
        region, device, swap_chain.color_format, swap_chain.render_pass,
        swap_chain.sample_count, "Syntics/res/terrain.vert.spv",
        "Syntics/res/terrain.frag.spv", swap_chain.extent_2D.width,
        swap_chain.extent_2D.height, VK_CULL_MODE_NONE,
        size_arr(pl_g_state.textures), NULL, &pl_g_state.g_pipline);

    pl_g_state.g_pipline.vert_buffer.data = dyn_arrayP(region, (1000) * 4, Vertex);

    pl_g_state.g_pipline.idx_buffer.data = dyn_arrayP(region, (1000) * 6, uint32);

    pl_g_state.g_pipline.vert_buffer.size_bytes =
        capacity_arr(pl_g_state.g_pipline.vert_buffer.data) * sizeof(Vertex);
    create_vertex_buffer(device, physical_device, command_pool, graphic_queue,
                         &pl_g_state.g_pipline.vert_buffer);

    generate_indices(&pl_g_state.g_pipline.idx_buffer.data, 0, 1000);

    pl_g_state.g_pipline.idx_buffer.size_bytes =
        size_arr(pl_g_state.g_pipline.idx_buffer.data) * sizeof(uint32);
    pl_g_state.g_pipline.idx_buffer.curr_size =
        size_arr(pl_g_state.g_pipline.idx_buffer.data);
    create_index_buffer(device, physical_device, command_pool, graphic_queue,
                        &pl_g_state.g_pipline.idx_buffer);

    pl_g_state.g_pipline.uniform_buffers =
        region_mallocP(region, num_semaphores, Uniform_Buffer);
    pl_g_state.g_pipline.descriptors.desc_sets =
        region_mallocP(region, num_semaphores, VkDescriptorSet);

    for_range(i, num_semaphores)
    {
        pl_g_state.g_pipline.uniform_buffers[i].size_bytes = (uint32)sizeof(MVP);

        create_uniform_buffer(device, physical_device,
                              &pl_g_state.g_pipline.uniform_buffers[i]);
    }

    create_descriptors(region, device, &pl_g_state.g_pipline.descriptors,
                       num_semaphores, pl_g_state.g_pipline.set_layout,
                       pl_g_state.textures, size_arr(pl_g_state.textures),
                       pl_g_state.g_pipline.uniform_buffers);

    pl_g_state.cam.position = v3f(52.0f, 15.15f, -10.5f);
    pl_g_state.cam.orientation = v3f(0.0f, 0.0f, -1.0f);

    pl_g_state.cam.speed = 10.0f;

    pl_g_state.cam.mvp.model = scale(mat4i(1.0f), v3f(1.0f, 1.0f, 1.0f));
    pl_g_state.cam.mvp.view = view(
        pl_g_state.cam.position,
        pl_g_state.cam.position + pl_g_state.cam.orientation, pl_g_state.cam.up);

    pl_g_state.cam.mvp.light_pos = Vec3(1.0f, 5.0f, 0.5f);

    subscribe(&pl_g_state.mouse_evt, EVT_MOUSE);
}

static float translucentcy = 0.8f;
static void update_gui(Region_Alloc* region, float dt)
{
    back_bord_begin("TTTT", Vec2(100.0f));
    {
        gridd_begin(2, 1);
        {
            add_text("Translucentcy: ");
            add_input_float(translucentcy, 0.0f, 1.0f);
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
            static char temp[60] = {};
            static float count = 1.0f;
            if (count >= 0.1f)
            {
                uint32 fps = (uint32)(1.0f / dt);
                float milli = dt * 1000.0f;
                sprintf(temp, "Milli: %f | FPS: %u", milli, fps);
                count = 0.0f;
            }
            count += dt;
            add_text(temp);
        }
    }
    back_bord_end();
    back_bord_begin("Terminal", Vec2(500.0f, 100.0f));
    {
        add_terminal(250.0f, 200.0f);
    }
    back_bord_end();
}

void recreate_platform_game(Region_Alloc* region, const Application_State& app_state)
{
    recreate_graphic_pipline(region, app_state, "Syntics/res/terrain.vert.spv",
                             "Syntics/res/terrain.frag.spv", pl_g_state.g_pipline,
                             size_arr(pl_g_state.textures), NULL);
}

void update_platform_game(Region_Alloc* region, VkDevice device,
                          const Vec2& dimensions, uint32 semaphore_idx, float dt)
{
#if 1
    gui_update_begin(region, dimensions, semaphore_idx, dt, translucentcy);
    {
        update_gui(region, dt);
    }
    gui_update_end();

    if (!gui_focus())
#endif
    {
        update_camera(&pl_g_state.cam, pl_g_state.mouse_evt, dt);
    }
    pl_g_state.cam.mvp.view = view(
        pl_g_state.cam.position,
        pl_g_state.cam.position + pl_g_state.cam.orientation, pl_g_state.cam.up);
    pl_g_state.cam.mvp.proj =
        perspective(radians(53.0f), dimensions.x / dimensions.y, 0.1f, 100.0f);

    update_uniform_buffers(device,
                           pl_g_state.g_pipline.uniform_buffers[semaphore_idx],
                           &pl_g_state.cam.mvp, sizeof(pl_g_state.cam.mvp));
}

void render_platform_game(VkCommandBuffer command_buffer, uint32 semaphore_idx)
{
    bind_and_draw_graphics_pipline(
        command_buffer, pl_g_state.g_pipline.descriptors.desc_sets[semaphore_idx], 0,
        pl_g_state.g_pipline.idx_buffer.curr_size, pl_g_state.g_pipline);
}

void destroy_platform_game(VkDevice device, uint32 num_semaphores)
{
    vkDestroyPipelineLayout(device, pl_g_state.g_pipline.layout, NULL);
    vkDestroyPipeline(device, pl_g_state.g_pipline.pipeline, NULL);
    vkDestroyDescriptorSetLayout(device, pl_g_state.g_pipline.set_layout, NULL);
    destroy_buffer(device, pl_g_state.g_pipline.vert_buffer.buffer,
                   pl_g_state.g_pipline.vert_buffer.buffer_memory);
    destroy_buffer(device, pl_g_state.g_pipline.idx_buffer.buffer,
                   pl_g_state.g_pipline.idx_buffer.buffer_memory);

    vkDestroyDescriptorPool(device, pl_g_state.g_pipline.descriptors.desc_pool,
                            NULL);

    for (uint32 i = 0; i < num_semaphores; i++)
    {
        destroy_buffer(device, pl_g_state.g_pipline.uniform_buffers[i].buffer,
                       pl_g_state.g_pipline.uniform_buffers[i].buffer_memory);
    }
    for (uint32 i = 0; i < size_arr(pl_g_state.textures); i++)
    {
        destroy_texture(device, pl_g_state.textures[i]);
    }
}


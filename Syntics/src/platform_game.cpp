#include "logging.h"
#include "math/matrix.h"
#include "region_alloc.h"
#include "font.h"
#include "camera.h"
#include "buffers.h"
#include "swap_chain.h"
#include "gui.h"
#include "event_system.h"
#include "random.h"
#include "render_util.h"
#include "vulkan_types.h"
#include <math.h>

void draw_pipeline(void (*draw_callback)(void* data, VkCommandBuffer command_buffer,
                                         u32 semaphore_idx),
                   void* data);

void subscribe_recreate_callback(
    void (*rc_callback)(void* data, Region_Alloc* region,
                        const Application_State& app_state),
    void* data);

void subscribe_destroy_callback(void (*destroy_callback)(void* data, VkDevice device,
                                                         u32 num_semaphores),
                                void* data);

typedef struct Platform_Game_State
{
    Graphic_Pipline g_pipline;

    Camera cam;

    Texture* textures;
    Font font;
    Events* mouse_evt;
    Events* wheel_evt;

    u32 static_index = 0;
} Platform_Game_State;

static Platform_Game_State pl_g_state;

static u32 NUM_RECTS = 10000;
static u32 NUM_VERTICES = NUM_RECTS * 4;
static u32 NUM_INDICES = NUM_RECTS * 6;
static u32 num_rects = 0;

static void recreate_platform_game(void* data, Region_Alloc* region,
                                   const Application_State& app_state)
{
    recreate_graphic_pipline(region, app_state, "Syntics/res/platform_game.vert.spv",
                             "Syntics/res/platform_game.frag.spv",
                             pl_g_state.g_pipline, size_arr(pl_g_state.textures),
                             NULL);

    gui_recreate(region);
}

static void destroy_platform_game(void* data, VkDevice device, u32 num_semaphores)
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

    for (u32 i = 0; i < num_semaphores; i++)
    {
        destroy_buffer(device, pl_g_state.g_pipline.uniform_buffers[i].buffer,
                       pl_g_state.g_pipline.uniform_buffers[i].buffer_memory);
    }
    for (u32 i = 0; i < size_arr(pl_g_state.textures); i++)
    {
        destroy_texture(device, pl_g_state.textures[i]);
    }

    destroy_gui(device, num_semaphores);
}

void init_platform_game(Region_Alloc* region, VkDevice device,
                        VkPhysicalDevice physical_device, VkCommandPool command_pool,
                        VkQueue graphic_queue, const Swap_Chain_attrib& swap_chain,
                        u32 num_semaphores)
{

    pl_g_state.textures = dyn_arrayP(region, 2, Texture);

    create_texture(device, physical_device, command_pool, graphic_queue, true,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/default.png",
                   &pl_g_state.textures[0]);

    get_head(pl_g_state.textures)->size++;

    pl_g_state.g_pipline.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    create_graphics_pipeline(
        region, device, swap_chain.color_format, swap_chain.render_pass,
        swap_chain.sample_count, "Syntics/res/platform_game.vert.spv",
        "Syntics/res/platform_game.frag.spv", swap_chain.extent_2D.width,
        swap_chain.extent_2D.height, VK_CULL_MODE_NONE,
        size_arr(pl_g_state.textures), NULL, &pl_g_state.g_pipline);

    pl_g_state.g_pipline.vert_buffer.data = dyn_arrayP(region, NUM_VERTICES, Vertex);

    pl_g_state.g_pipline.idx_buffer.data = dyn_arrayP(region, NUM_INDICES, uint32);

    pl_g_state.g_pipline.vert_buffer.size_bytes =
        capacity_arr(pl_g_state.g_pipline.vert_buffer.data) * sizeof(Vertex);
    create_vertex_buffer(device, physical_device, command_pool, graphic_queue,
                         &pl_g_state.g_pipline.vert_buffer);

    generate_indices(&pl_g_state.g_pipline.idx_buffer.data, 0, NUM_RECTS);

    pl_g_state.g_pipline.idx_buffer.size_bytes =
        size_arr(pl_g_state.g_pipline.idx_buffer.data) * sizeof(uint32);
    pl_g_state.g_pipline.idx_buffer.curr_size = 0;
    // size_arr(pl_g_state.g_pipline.idx_buffer.data);
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

    pl_g_state.cam.position = v3f(0.0f, 0.0f, 0.0f);
    pl_g_state.cam.orientation = v3f(0.0f, 0.0f, 0.0f);
    pl_g_state.cam.mvp.model = mat4i(1.0f);
    pl_g_state.cam.mvp.view = mat4i(1.0f);

    pl_g_state.cam.speed = 200.0f;

    subscribe(&pl_g_state.mouse_evt, EVT_MOUSE);
    subscribe(&pl_g_state.wheel_evt, EVT_WHEEL);

    subscribe_recreate_callback(recreate_platform_game, NULL);
    subscribe_destroy_callback(destroy_platform_game, NULL);

    gui_init(region, device, physical_device, command_pool, graphic_queue,
             swap_chain, num_semaphores);
}

static f32 translucentcy = 0.8f;
static void update_gui(Region_Alloc* region, f32 dt)
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
            static f32 count = 1.0f;
            if (count >= 0.1f)
            {
                u32 fps = (uint32)(1.0f / dt);
                f32 milli = dt * 1000.0f;
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
    back_bord_begin("Graph", Vec2(800.0f, 100.0f));
    {
        add_graph(dt * 1000.0f, "Milli per frame", 20.0f, 10.0f, 5.0f, dt);
    }
    back_bord_end();
}

static void update_internal_cam(Camera* cam, f32 dt)
{
    cam->velocity = 0.0f;
    if (is_key_pressed(SYNT_W_PRESSED))
    {
        cam->velocity.y = -1.0f;
    }
    if (is_key_pressed(SYNT_A_PRESSED))
    {
        cam->velocity.x = 1.0f;
    }
    if (is_key_pressed(SYNT_S_PRESSED))
    {
        cam->velocity.y = 1.0f;
    }
    if (is_key_pressed(SYNT_D_PRESSED))
    {
        cam->velocity.x = -1.0f;
    }
    static f32 old_speed = cam->speed;
    if (is_key_pressed(SYNT_SHIFT_PRESSED))
    {
        cam->speed = old_speed * 4.0f;
    }
    else if (!is_key_pressed(SYNT_SHIFT_PRESSED))
    {
        cam->speed = old_speed;
    }
    cam->velocity *= cam->speed * dt;
    cam->position += cam->velocity;
}

static void render_platform_game(void* data, VkCommandBuffer command_buffer,
                                 u32 semaphore_idx)
{
    Index_Buffer* idx = &pl_g_state.g_pipline.idx_buffer;
    idx->curr_size = num_rects * 6;
    bind_and_draw_graphics_pipline(
        command_buffer, pl_g_state.g_pipline.descriptors.desc_sets[semaphore_idx], 0,
        idx->curr_size, pl_g_state.g_pipline);
}

void update_platform_game(Region_Alloc* region, VkDevice device,
                          const Vec2& dimensions, u32 semaphore_idx, f32 dt)
{
#if 1
    gui_update_begin(region, dimensions, semaphore_idx, dt, translucentcy);
    {
        update_gui(region, dt);
    }
    gui_update_end();
#endif

    static Vec2 extra_dim = Vec2(0.0);

    Events* we = pl_g_state.wheel_evt;
    if (we->activated)
    {
        extra_dim += we->wheel_evt.z_delta * -0.5f;
    }

    Camera* cam = &pl_g_state.cam;
    update_internal_cam(cam, dt);
    cam->mvp.proj = ortho(0, dimensions.y, dimensions.x, 0, -1.0f, 1.0f);
    cam->mvp.model = translate(mat4i(1.0f), cam->position);
    update_uniform_buffers(device,
                           pl_g_state.g_pipline.uniform_buffers[semaphore_idx],
                           &cam->mvp, sizeof(cam->mvp));

    Vertex_Buffer* vert = &pl_g_state.g_pipline.vert_buffer;
    num_rects = 0;
    get_head(vert->data)->size = 0;

    Rect3D plat;
    plat.pos = Vec3(10.0f, 100.0f, -1.0f);
    plat.size = Vec2(1000.0f, 40.0f);
    plat.color = Vec4(1.0f);
    quad(&vert->data, &num_rects, plat);

    map_copy_mem(device, &pl_g_state.g_pipline.vert_buffer.buffer_memory,
                 pl_g_state.g_pipline.vert_buffer.size_bytes,
                 pl_g_state.g_pipline.vert_buffer.data);

    draw_pipeline(render_platform_game, NULL);
}


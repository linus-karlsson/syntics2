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
    Camera p_cam;

    Texture* textures;
    Font font;
    Events* mouse_evt;
    Events* wheel_evt;

    u32 static_index = 0;
    u32 num_semaphores = 0;
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

    pl_g_state.num_semaphores = num_semaphores;
    pl_g_state.g_pipline.uniform_buffers =
        region_mallocP(region, num_semaphores * 2, Uniform_Buffer);
    pl_g_state.g_pipline.descriptors.desc_sets =
        region_mallocP(region, num_semaphores * 2, VkDescriptorSet);

    for_range(i, num_semaphores * 2)
    {
        pl_g_state.g_pipline.uniform_buffers[i].size_bytes = (uint32)sizeof(MVP);

        create_uniform_buffer(device, physical_device,
                              &pl_g_state.g_pipline.uniform_buffers[i]);
    }

    create_descriptors(region, device, &pl_g_state.g_pipline.descriptors,
                       num_semaphores * 2, pl_g_state.g_pipline.set_layout,
                       pl_g_state.textures, size_arr(pl_g_state.textures),
                       pl_g_state.g_pipline.uniform_buffers);

    pl_g_state.cam.pos = v3f(0.0f, 0.0f, 0.0f);
    pl_g_state.cam.ori = v3f(0.0f, 0.0f, 0.0f);
    pl_g_state.cam.mvp.model = mat4i(1.0f);
    pl_g_state.cam.mvp.view = mat4i(1.0f);

    pl_g_state.cam.speed = 30.0f;

    pl_g_state.p_cam.pos = v3f(0.0f, 0.0f, 0.0f);
    pl_g_state.p_cam.ori = v3f(0.0f, 0.0f, 0.0f);
    pl_g_state.p_cam.mvp.model = mat4i(1.0f);
    pl_g_state.p_cam.mvp.view = mat4i(1.0f);

    pl_g_state.p_cam.speed = 5.0f;

    subscribe(&pl_g_state.mouse_evt, EVT_MOUSE);
    subscribe(&pl_g_state.wheel_evt, EVT_WHEEL);

    subscribe_recreate_callback(recreate_platform_game, NULL);
    subscribe_destroy_callback(destroy_platform_game, NULL);

    gui_init(region, device, physical_device, command_pool, graphic_queue,
             swap_chain, num_semaphores);
}

static f32 translucentcy = 1.0f;
static b32 show_graph = 0;
static void update_gui(Region_Alloc* region, f32 dt)
{
    back_bord_begin("TTTT", V2(100.0f));
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
        gridd_end();
        gridd_begin(1, 1);
        {
            if (add_button("Graph"))
            {
                show_graph = show_graph ? false : true;
            }
        }
        gridd_end();
    }
    back_bord_end();
    back_bord_begin("Terminal", V2(500.0f, 100.0f));
    {
        add_terminal(250.0f, 200.0f);
    }
    back_bord_end();
    if (show_graph)
    {
        back_bord_begin("Graph", V2(800.0f, 100.0f));
        {
            add_graph(dt * 1000.0f, "Milli per frame", 20.0f, 10.0f, 5.0f, dt);
        }
        back_bord_end();
    }
}

static void calculate_pos(Camera* cam, const V3& acc, f32 dt)
{
    cam->pos = (acc * 0.5f * dt * dt) + 2 * cam->vel + cam->pos;
    cam->vel = acc * dt + cam->vel;
}

static void update_internal_cam(Camera* cam, f32 dt)
{
    V3 acc = 0;
    cam->speed = 30.0f;
    if (is_key_pressed(SYNT_A_PRESSED))
    {
        acc.x = -1.0f;
    }
    if (is_key_pressed(SYNT_D_PRESSED))
    {
        acc.x = 1.0f;
    }
#if 0
    if (!acc.x && !acc.y)
    {
        acc *= 0.707106781187f;
    }
#endif
    if (is_key_pressed(SYNT_SHIFT_PRESSED))
    {
        cam->speed *= 4.0f;
    }
    acc.x *= cam->speed;
    acc.x -= 7.0f * cam->vel.x;
    calculate_pos(cam, acc, dt);
#if 0
    static f32 sec = 0;
    if ((sec += dt) >= 0.5f)
    {
        print_camera(*cam);
        sec = 0;
    }
#endif
}

static float speed = 70.0f;
static void follow_player(Camera* cam, const V3& player_pos, f32 dt)
{
    V3 acc = 0;
    cam->speed = speed;
    float p_x = player_pos.x - 700.0f;
    float neg_cam_p = -cam->pos.x;
    float abs_val = abs_f32(neg_cam_p - p_x);
    if (abs_val < 40.0f)
    {
        speed = 40.0f;
    }
    else if (neg_cam_p < p_x)
    {
        acc.x = -1.0f;
    }
    else if (neg_cam_p > p_x)
    {
        acc.x = 1.0f;
    }
#if 0
    if (!acc.x && !acc.y)
    {
        acc *= 0.707106781187f;
    }
#endif
    if (is_key_pressed(SYNT_SHIFT_PRESSED))
    {
        cam->speed *= 4.0f;
    }
    acc.x *= cam->speed;
    acc.x -= 7.0f * cam->vel.x;
    calculate_pos(cam, acc, dt);
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
                          const V2& dimensions, u32 semaphore_idx, f32 dt)
{
#if 1
    gui_update_begin(region, dimensions, semaphore_idx, dt, translucentcy);
    {
        update_gui(region, dt);
    }
    gui_update_end();
#endif

    static V2 extra_dim = V2(0.0);

    Events* we = pl_g_state.wheel_evt;
    if (we->activated)
    {
        extra_dim += we->wheel_evt.z_delta * -0.2f;
    }

    Vertex_Buffer* vert = &pl_g_state.g_pipline.vert_buffer;
    num_rects = 0;
    get_head(vert->data)->size = 0;

    // Player cam
    Camera* p_cam = &pl_g_state.p_cam;
    update_internal_cam(p_cam, dt);
    V2 player_size = V2(40.0f, 100.0f);
    V3 player_pos = V3(p_cam->pos.x, 200.0f, -1.0f);
    quad(&vert->data, &num_rects, player_pos, player_size);

    // Background cam
    Camera* cam = &pl_g_state.cam;
    cam->mvp.proj = ortho(0, dimensions.y, dimensions.x, 0, -1.0f, 1.0f);
    follow_player(cam, p_cam->pos, dt);
    cam->mvp.model = translate(mat4i(1.0f), cam->pos);
    update_uniform_buffers(device,
                           pl_g_state.g_pipline.uniform_buffers[semaphore_idx],
                           &cam->mvp, sizeof(cam->mvp));

    V3 pos = V3(10.0f, 100.0f, -1.0f);
    V2 size = V2(3000.0f, 40.0f);
    V4 color_l = V4(0.0f, 1.0f, 0.0f, 1.0f);
    V4 color_r = V4(1.0f, 0.0f, 0.0f, 1.0f);
    quad_s_gradiant_l_r(&vert->data, &num_rects, pos, size, color_l, color_r);

    map_copy_mem(device, &pl_g_state.g_pipline.vert_buffer.buffer_memory,
                 pl_g_state.g_pipline.vert_buffer.size_bytes,
                 pl_g_state.g_pipline.vert_buffer.data);

    draw_pipeline(render_platform_game, NULL);
}


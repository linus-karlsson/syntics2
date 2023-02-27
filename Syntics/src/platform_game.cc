#include "defines.h"
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
#include "file_reading.h"
#include "collision.h"
#include "random.h"
#include "entity.h"
#include "noise.h"
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
    // TODO: this does not need to be cameras
    Dynamic_Entity p_e;
    Dynamic_Entity* b_es;
    V4* b_c_t;
    V4* b_c_b;

    Texture* textures;
    Font font;
    Events* mouse_evt;
    Events* wheel_evt;

    Rect2D* rects;
    Rect2D player_rect;

    u8* level_array;
    u32 level_witdth;
    u32 level_height;

    u32 static_index = 0;
    u32 num_semaphores = 0;
} Platform_Game_State;

static Platform_Game_State pl_g_state;

static const u32 NUM_RECTS = 10000;
static const u32 NUM_VERTICES = NUM_RECTS * 4;
static const u32 NUM_INDICES = NUM_RECTS * 6;
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

u32 min(u32 first, u32 second)
{
    return first < second ? first : second;
}

static void load_level(Region_Alloc* region, const char* path)
{
    File_Attrib file;
    read_file(file, NULL, path, "r");
    // TODO: This includes scrap
    if (region && !pl_g_state.level_array)
    {
        pl_g_state.level_array = dyn_arrayP(region, file.size, u8);
    }
    get_head(pl_g_state.level_array)->size = 0;
    u32 width = 0;
    u32 height = 0;
    u32 size = capacity_arr(pl_g_state.level_array);
    for_range(i, min(size, file.size))
    {
        if (file.buffer[i] == '0')
        {
            synt_push(pl_g_state.level_array, 0);
        }
        else if (file.buffer[i] == '1')
        {
            synt_push(pl_g_state.level_array, 1);
        }
        else if (file.buffer[i] == '\n')
        {
            // TODO: the -1 is for some other caracter
            if (!width) width = i - 1;
            height++;
            continue;
        }
    }
    pl_g_state.level_witdth = width;
    pl_g_state.level_height = height;
    free_file(file);
}

void init_platform_game(Region_Alloc* region, VkDevice device,
                        VkPhysicalDevice physical_device, VkCommandPool command_pool,
                        VkQueue graphic_queue, const Swap_Chain_attrib& swap_chain,
                        u32 num_semaphores)
{

    load_level(region, "level_create.synt");

    pl_g_state.rects = dyn_arrayP(
        region, pl_g_state.level_height * pl_g_state.level_witdth, Rect2D);

    pl_g_state.textures = dyn_arrayP(region, 2, Texture);

    create_texture(device, physical_device, command_pool, graphic_queue, true,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/default.png",
                   &pl_g_state.textures[0]);

    get_head(pl_g_state.textures)->size++;

    create_texture(device, physical_device, command_pool, graphic_queue, true,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/Circle.png",
                   &pl_g_state.textures[1]);

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

    pl_g_state.p_e.pos = v3f(100.0f, 200.0f, -1.0f);

    pl_g_state.b_es = dyn_arrayP(region, 20, Dynamic_Entity);
    pl_g_state.b_c_t = dyn_arrayP(region, 20, V4);
    pl_g_state.b_c_b = dyn_arrayP(region, 20, V4);
    f32 start = 20.0f;
    f32 z = -1.21f;
    for_range(i, 20)
    {
        pl_g_state.b_es[i].pos += rand_f32(0.0f, 300.0f);
        pl_g_state.b_es[i].pos.z = z;
        pl_g_state.b_es[i].speed = sy_lerp(0.5f, 10.0f, start / 20.0f);
        pl_g_state.b_c_t[i] = V4(rand_f32(0.0f, 1.0f), rand_f32(0.0f, 1.0f),
                                 rand_f32(0.0f, 1.0f), 1.0f);
        pl_g_state.b_c_b[i] = V4(rand_f32(0.0f, 1.0f), rand_f32(0.0f, 1.0f),
                                 rand_f32(0.0f, 1.0f), 1.0f);
        start -= 1.0f;
        z += 0.01f;
    }

    subscribe(&pl_g_state.mouse_evt, EVT_MOUSE);
    subscribe(&pl_g_state.wheel_evt, EVT_WHEEL);

    subscribe_recreate_callback(recreate_platform_game, NULL);
    subscribe_destroy_callback(destroy_platform_game, NULL);

    gui_init(region, device, physical_device, command_pool, graphic_queue,
             swap_chain, num_semaphores);
}

static f32 translucentcy = 1.0f;
static b32 show_graph = 0;
static b32 reload_level = 0;
static f32 g_dist_ = 0;
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
        gridd_begin(3, 1);
        {
            if (add_button("Graph"))
            {
                show_graph = show_graph ? false : true;
            }
            if (add_button("Reload level"))
            {
                reload_level = true;
            }
            add_input_float(g_dist_, 0.0f, 100.0f);
        }
        gridd_end();
    }
    back_bord_end();
    back_bord_begin("Terminal", V2(100.0f, 300.0f));
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

static V3 calculate_pos(Dynamic_Entity* entity, const V3& acc, f32 dt)
{
    V3 pos = (acc * 0.5f * dt * dt) + 2 * entity->vel + entity->pos;
    entity->vel = acc * dt + entity->vel;
    return pos;
}

static void update_internal_cam(Dynamic_Entity* entity, f32 dt)
{
    V3 acc = 0;
    f32 speed = 30.0f;
    if (is_key_pressed(SYNT_A_PRESSED))
    {
        acc.x = -1.0f;
    }
    if (is_key_pressed(SYNT_D_PRESSED))
    {
        acc.x = 1.0f;
    }
    acc.y = -9.81f;
    static b32 clicked = false;
    if (is_key_pressed(SYNT_W_PRESSED))
    {
        if (!clicked)
        {
            entity->vel.y = 8.0f;
        }
        clicked = true;
    }
    else
    {
        clicked = false;
    }
    // f32 len = v3_len(acc);
    static b32 clicked2 = false;
    if (is_key_pressed(SYNT_SHIFT_PRESSED))
    {
        if (!clicked2)
        {
            speed *= 40.0f;
        }
        clicked2 = true;
    }
    else
    {
        clicked2 = false;
    }
    acc.x *= speed;
    acc.x -= 9.0f * entity->vel.x;

    V2 contact_point(0.0f, 0.0f);
    V2 contact_normal(0.0f, 0.0f);
    f32 contact_time(0.0f);
    Rect2D* r = pl_g_state.rects;
    u32 size = size_arr(r);
    for_range(i, size)
    {
        if (dynamic_ray_rect_unsafe(pl_g_state.player_rect, r[i], contact_normal,
                                    contact_normal, contact_time, dt, -200.0f,
                                    200.0f))
        {
            V3 n = V3(contact_normal.x, contact_normal.y, 0.0f);
            entity->vel = entity->vel - (1.5f * dot(entity->vel, n) * n);
            // acc.y -= 12.0f * entity->vel.y;
            break;
        }
    }
    entity->pos = calculate_pos(entity, acc, dt);
}

static void follow_position_pp(V3& pos, const V3& target, V3& last_vel, f32 dt,
                               f32 per_distance_speed = 3.7f, f32 max_speed = 0.0f)
{
    f32 distance = distance_v3(pos, target);

    V3 dir = normalize(target - pos);
    f32 speed = distance * per_distance_speed;

    dir.x *= per_distance_speed;
    dir.y *= per_distance_speed;
    if (distance > 40.0f)
    {
        dir.x -= 1.0f * last_vel.x;
        dir.y -= 1.0f * last_vel.y;
    }
    pos.x = (dir.x * 0.5f * dt * dt) + 2 * last_vel.x + pos.x;
    pos.y = (dir.y * 0.5f * dt * dt) + 2 * last_vel.y + pos.y;
    last_vel = dir * dt + last_vel;
}
static V3 follow_position(const V3& pos, const V3& target, f32 dt,
                          f32 per_distance_speed = 3.7f, f32 max_speed = 0.0f)
{
    V3 result_vel = 0;
    f32 distance = distance_v3(pos, target);

    if (distance > 5.0f)
    {
        V3 dir = normalize(target - pos);
        f32 speed;
        if (max_speed)
        {
            speed = fminf(distance * per_distance_speed, max_speed);
        }
        else
        {
            speed = distance * per_distance_speed;
        }

        result_vel = dir * speed;
    }
    return result_vel;
}

static void follow_player_cam(Camera* cam, const V3& player_pos, f32 dt)
{
    V3 pos = V3(player_pos.x - 600.0f, player_pos.y - 400.0f, player_pos.z);
    V3 negated_cam_pos = cam->pos * -1.0f;
    cam->vel = follow_position(negated_cam_pos, pos, dt);
    cam->pos -= cam->vel * dt;
    cam->pos.z = -0.1f;
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

#define BLOCK_H 40.0f
#define BLOCK_W 40.0f

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
    Rect2D* rects = pl_g_state.rects;
    num_rects = 0;
    get_head(vert->data)->size = 0;
    get_head(rects)->size = 0;

    // Player cam
    Dynamic_Entity* p_e = &pl_g_state.p_e;

    Dynamic_Entity* butters = pl_g_state.b_es;
    u32 butters_size = capacity_arr(butters);
    for_range(i, butters_size)
    {
        V3 target_p =
            V3(p_e->pos.x + BLOCK_W * 0.5f, p_e->pos.y + BLOCK_H * 0.5f, p_e->pos.z);
        Dynamic_Entity* b = &butters[i];
        follow_position_pp(b->pos, target_p, b->vel, dt, b->speed, 100.0f);
        quad_gradiant_t_b(&vert->data, &num_rects, b->pos, V2(10.0f),
                          pl_g_state.b_c_t[i], pl_g_state.b_c_b[i], 1.0f);
    }

    Rect2D* p_rect = &pl_g_state.player_rect;
    V2 player_size = V2(BLOCK_W, BLOCK_H);
    V4 color_t = V4(0.0f, 0.0f, 1.0f, 1.0f);
    V4 color_b = V4(0.0f, 1.0f, 0.0f, 1.0f);
    *p_rect = quad_gradiant_t_b(&vert->data, &num_rects, p_e->pos, player_size,
                                color_t, color_b, 1.0f);
    p_rect->vel = V2(p_e->vel.x, p_e->vel.y);

    // Background cam
    Camera* cam = &pl_g_state.cam;
    cam->mvp.proj = ortho(0, dimensions.y, dimensions.x, 0, -1.0f, 1.0f);
    cam->mvp.model = translate(mat4i(1.0f), cam->pos);
    update_uniform_buffers(device,
                           pl_g_state.g_pipline.uniform_buffers[semaphore_idx],
                           &cam->mvp, sizeof(cam->mvp));

    if (reload_level)
    {
        load_level(NULL, "level_create.synt");
        reload_level = false;
    }

    u8* data = pl_g_state.level_array;
    int h_i = 0;
    for (int i = pl_g_state.level_height - 1; i >= 0; i--)
    {
        for_range(j, pl_g_state.level_witdth)
        {
            if (data[(i * pl_g_state.level_witdth) + j])
            {
                V3 pos = V3((float)j * BLOCK_W, (float)h_i * BLOCK_H, -1.0f);
                static V2 size = V2(BLOCK_W, BLOCK_H);
                static V4 color_l = V4(0.0f, 1.0f, 0.0f, 1.0f);
                static V4 color_r = V4(1.0f, 0.0f, 0.0f, 1.0f);
                if (j == 0 || j == pl_g_state.level_witdth - 1)
                {
                    synt_push(rects, quad_gradiant_t_b(&vert->data, &num_rects, pos,
                                                       size, color_l, color_r));
                }
                else
                {
                    synt_push(rects, quad_gradiant_l_r(&vert->data, &num_rects, pos,
                                                       size, color_l, color_r));
                }
            }
        }
        h_i++;
    }
    update_internal_cam(p_e, dt);
    follow_player_cam(cam, p_e->pos, dt);

    map_copy_mem(device, &pl_g_state.g_pipline.vert_buffer.buffer_memory,
                 pl_g_state.g_pipline.vert_buffer.size_bytes,
                 pl_g_state.g_pipline.vert_buffer.data);

    draw_pipeline(render_platform_game, NULL);
}


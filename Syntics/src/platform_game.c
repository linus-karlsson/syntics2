#include "defines.h"
#include "logging.h"
#include "math/matrix.h"
#include "math/transforms.h"
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
                        const Application_State* app_state),
    void* data);

void subscribe_destroy_callback(void (*destroy_callback)(void* data, VkDevice device,
                                                         u32 num_semaphores),
                                void* data);

typedef struct Z_Sorting
{
    u32 index;
    f32 z;
} Z_Sorting;

typedef struct Platform_Game_State
{
    Graphic_Pipline g_pipline;
    Vertex* temp_storage;
    Z_Sorting* z_sort;

    Camera_2D cam;
    // TODO: this does not need to be cameras
    Dynamic_Entity_2D* p_e;
    Dynamic_Entity_2D* f_e;
    Dynamic_Entity_2D* b_es;
    V4* b_c_t;
    V4* b_c_b;
    Rect2D* b_rects;

    Texture* textures;
    Font font;
    Events* mouse_evt;
    Events* wheel_evt;

    Rect2D* rects;
    Rect2D player_rect;
    Rect2D friend_rect;

    u8* level_array;
    u32 level_witdth;
    u32 level_height;

    u32 static_index;
    u32 num_semaphores;
} Platform_Game_State;

static Platform_Game_State pl_g_state = { 0 };

#define VERTICES_PER_RECT 4
#define INDICES_PER_RECT 6
#define NUM_PARTICLES 0

#define NUM_RECTS 10000
static const u32 NUM_VERTICES = NUM_RECTS * VERTICES_PER_RECT;
static const u32 NUM_INDICES = NUM_RECTS * INDICES_PER_RECT;
static u32 num_rects = 0;

static void recreate_platform_game(void* data, Region_Alloc* region,
                                   const Application_State* app_state)
{
    recreate_graphic_pipline_ap(
        region, app_state, "Syntics/res/platform_game.vert.spv",
        "Syntics/res/platform_game.frag.spv", &pl_g_state.g_pipline,
        size_arr(pl_g_state.textures), NULL);

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
        destroy_texture(device, &pl_g_state.textures[i]);
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
    read_file(&file, NULL, path, "r");
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
    free_file(&file);
}

void init_platform_game(Region_Alloc* region, VkDevice device,
                        VkPhysicalDevice physical_device, VkCommandPool command_pool,
                        VkQueue graphic_queue, const Swap_Chain_attrib* swap_chain,
                        u32 num_semaphores)
{
    load_level(region, "level_create.synt");
    init_entity(region);

    pl_g_state.rects = dyn_arrayP(
        region, pl_g_state.level_height * pl_g_state.level_witdth, Rect2D);

    pl_g_state.textures = dyn_arrayP(region, 2, Texture);

    create_texture_path(device, physical_device, command_pool, graphic_queue, true,
                        VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/default.png",
                        &pl_g_state.textures[0]);

    get_head(pl_g_state.textures)->size++;

    create_texture_path(device, physical_device, command_pool, graphic_queue, true,
                        VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/Circle.png",
                        &pl_g_state.textures[1]);

    get_head(pl_g_state.textures)->size++;

    pl_g_state.g_pipline.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    create_graphics_pipeline(
        region, device, swap_chain->render_pass, swap_chain->sample_count,
        "Syntics/res/platform_game.vert.spv", "Syntics/res/platform_game.frag.spv",
        swap_chain->extent_2D.width, swap_chain->extent_2D.height, VK_CULL_MODE_NONE,
        size_arr(pl_g_state.textures), NULL, &pl_g_state.g_pipline);

    pl_g_state.g_pipline.vert_buffer.data = dyn_arrayP(region, NUM_VERTICES, Vertex);
    pl_g_state.temp_storage = dyn_arrayP(region, NUM_VERTICES, Vertex);
    pl_g_state.z_sort = dyn_arrayP(region, NUM_RECTS, Z_Sorting);

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

    Camera_2D* cam = &pl_g_state.cam;
    cam->pos = v2f(0.0f, 0.0f);
    cam->ori = v3f(0.0f, 0.0f, 0.0f);
    cam->mvp.model = m4i(1.0f);
    cam->mvp.view = m4i(1.0f);

    cam->speed = 30.0f;

    pl_g_state.p_e = add_dyn_entity();
    Dynamic_Entity_2D* p_e = pl_g_state.p_e;
    p_e->pos = v2f(100.0f, 200.0f);
    p_e->z = -1.0f;

    pl_g_state.f_e = add_dyn_entity();
    Dynamic_Entity_2D* f_e = pl_g_state.f_e;
    f_e->pos = v2f(100.0f, 200.0f);
    f_e->z = -1.3f;
    f_e->speed = 10.0f;

#if 0
    if (NUM_PARTICLES)
    {
        pl_g_state.b_es = dyn_arrayP(region, NUM_PARTICLES, Dynamic_Entity_2D);
        pl_g_state.b_c_t = dyn_arrayP(region, NUM_PARTICLES, V4);
        pl_g_state.b_c_b = dyn_arrayP(region, NUM_PARTICLES, V4);
        pl_g_state.b_rects = dyn_arrayP(region, NUM_PARTICLES, Rect2D);
        f32 start = (f32)NUM_PARTICLES;
        f32 z = -1.3f;
        for_range(i, NUM_PARTICLES)
        {
            v2_s_add_equal(&pl_g_state.b_es[i].pos, rand_f32(100.0f, 300.0f));
            pl_g_state.b_es[i].z = z;
            pl_g_state.b_es[i].speed =
                10.0f; // sy_lerp(0.5f, 10.0f, start / (f32)NUM_PARTICLES);
            pl_g_state.b_c_t[i] = v4f(rand_f32(0.0f, 1.0f), rand_f32(0.0f, 1.0f),
                                      rand_f32(0.0f, 1.0f), 1.0f);
            pl_g_state.b_c_b[i] = v4f(rand_f32(0.0f, 1.0f), rand_f32(0.0f, 1.0f),
                                      rand_f32(0.0f, 1.0f), 1.0f);
            start -= 1.0f;
            z += 0.01f;
        }
    }
#endif

    subscribe(&pl_g_state.mouse_evt, EVT_MOUSE);
    subscribe(&pl_g_state.wheel_evt, EVT_WHEEL);

    subscribe_recreate_callback(recreate_platform_game, NULL);
    subscribe_destroy_callback(destroy_platform_game, NULL);

    gui_init(region, device, physical_device, command_pool, graphic_queue,
             swap_chain, num_semaphores);
}

static f32 translucentcy = 0.8f;
static b32 show_graph = 0;
static b32 reload_level = 0;
static b8 edit_mode = 0;
static f32 g_dist_ = 0;
static void update_gui(Region_Alloc* region, f32 dt)
{
    back_bord_begin("TTTT", v2i(100.0f));
    {
        gridd_begin(2, 1);
        {
            add_text("Translucentcy: ");
            add_input_float(&translucentcy, 0.0f, 1.0f);
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
            static char temp[60] = { 0 };
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
        gridd_begin(2, 2);
        {
            if (add_button("Graph"))
            {
                b_switch(show_graph);
            }
            if (add_button("Reload level"))
            {
                reload_level = true;
            }
            if (add_button("Edit mode"))
            {
                b_switch(edit_mode);
            }
            add_input_float(&g_dist_, 0.0f, 2000.0f);
        }
        gridd_end();
        entity_watch_window();
    }
    back_bord_end();
    back_bord_begin("Terminal", v2f(100.0f, 300.0f));
    {
        add_terminal(250.0f, 200.0f);
    }
    back_bord_end();
    if (show_graph)
    {
        back_bord_begin("Graph", v2f(800.0f, 100.0f));
        {
            add_graph(dt * 1000.0f, "Milli per frame", 20.0f, 10.0f, 5.0f, dt);
        }
        back_bord_end();
    }
}

static V2 calculate_pos(Dynamic_Entity_2D* entity, V2 acc, f32 dt)
{
    V2 pos = v2_add(v2_s_multi(acc, 0.5f * dt * dt),
                    v2_add(v2_s_multi(entity->vel, 2), entity->pos));
    entity->vel = v2_add(v2_s_multi(acc, dt), entity->vel);
    return pos;
}

static void update_camera_game(Camera_2D* cam, f32 dt)
{
    static b8 first_clicked = true;
    if (is_any_button_pressed() && !gui_focus())
    {
        int16 mouse_x, mouse_y;
        get_pos(&mouse_x, &mouse_y);

        static int16 last_x = 0;
        static int16 last_y = 0;

        u16 width, height;
        get_window_size(&width, &height);

        f32 movement_x = 0.0f;
        f32 movement_y = 0.0f;

        if (!first_clicked)
        {
            movement_x = (float)((mouse_x - last_x));
            movement_y = (float)((mouse_y - last_y));
        }
        else
            first_clicked = false;

        cam->pos.x += movement_x;
        cam->pos.y -= movement_y;

        last_x = mouse_x;
        last_y = mouse_y;
    }
    else
    {
        first_clicked = true;
    }
}

static b8 hit_ground = true;
static void update_position(Dynamic_Entity_2D* entity, const Rect2D* rect, V2 acc,
                            f32 dt)
{
    acc.x -= 9.0f * entity->vel.x;
    V2 contact_normal = v2f(0.0f, 0.0f);
    Rect2D* r = pl_g_state.rects;
    u32 size = size_arr(r);
    for_range(i, size)
    {
        if (dynamic_ray_rect_unsafe_d(rect, &r[i], &contact_normal, dt, -200.0f,
                                      200.0f))
        {
            V2 n = v2f(contact_normal.x, contact_normal.y);
            entity->vel =
                v2_sub(entity->vel, v2_s_multi(n, 1.5f * v2_dot(entity->vel, n)));
            // acc.y -= 12.0f * entity->vel.y;
            if (n.y == 1.0f && n.x == 0.0f)
            {
                hit_ground = true;
            }
            break;
        }
    }
    entity->pos = calculate_pos(entity, acc, dt);
}

static void entity_movement(Dynamic_Entity_2D* entity, const Rect2D* rect, f32 dt)
{
    V2 acc = v2d();
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
    if (is_key_pressed(SYNT_W_PRESSED))
    {
        if (entity->vel.y > 8.0f)
        {
            hit_ground = false;
        }
        if (hit_ground)
        {
            entity->vel.y += 80.0f * dt;
        }
    }
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
    update_position(entity, rect, acc, dt);
}

static void follow_position_pp(V2* pos, V2* last_vel, const Rect2D* rect, V2 target,
                               f32 dt, f32 per_distance_speed, f32 max_speed)
{
    f32 dis = v2_distance(*pos, target);

    V2 dir = v2_normalize(v2_sub(target, *pos));
    f32 speed = dt * fminf(dis * per_distance_speed, max_speed);

    v2_s_multi_equal(&dir, speed);
    if (dis > 60.0f)
    {
        v2_sub_equal(&dir, v2_s_multi(*last_vel, 3.0f));
    }
#if 1
    Rect2D* r = pl_g_state.rects;
    u32 r_size = size_arr(r);
    V2 contact_normal = v2f(0.0f, 0.0f);
    for_range(i, r_size)
    {
        if (dynamic_ray_rect_unsafe_d(rect, &r[i], &contact_normal, dt, -200.0f,
                                      200.0f))
        {
            V2 n = v2f(contact_normal.x, contact_normal.y);
            *last_vel =
                v2_sub(*last_vel, v2_s_multi(n, 1.5f * v2_dot(*last_vel, n)));
            break;
        }
    }
#endif
    *pos = v2_add(v2_s_multi(dir, 0.5f * dt * dt),
                  v2_add(v2_s_multi(*last_vel, 2), *pos));
    *last_vel = v2_add(v2_s_multi(dir, dt), *last_vel);
}
static V2 follow_position(V2 pos, V2 target, f32 dt, f32 per_distance_speed,
                          f32 max_speed)
{
    V2 result_vel = v2d();
    f32 dis = v2_distance(pos, target);

    if (dis > 5.0f)
    {
        V2 dir = v2_normalize(v2_sub(target, pos));
        f32 speed;
        if (max_speed)
        {
            speed = fminf(dis * per_distance_speed, max_speed);
        }
        else
        {
            speed = dis * per_distance_speed;
        }

        result_vel = v2_s_multi(dir, speed);
    }
    return result_vel;
}

#define BLOCK_H 40.0f
#define BLOCK_W 40.0f

static void follow_player_cam(Camera_2D* cam, V2 player_pos, V2 dim, f32 dt)
{
    V2 half_dim = v2_s_multi(dim, 0.5f);
    half_dim.x -= BLOCK_W * 0.5f;
    half_dim.y -= BLOCK_H * 0.5f;
    V2 pos = v2_sub(player_pos, half_dim);
    V2 negated_cam_pos = v2_s_multi(cam->pos, -1.0f);
    cam->vel = follow_position(negated_cam_pos, pos, dt, 3.7f, 0.0f);
    v2_sub_equal(&cam->pos, v2_s_multi(cam->vel, dt));
    cam->pos.x = clampf32(cam->pos.x,
                          (-(float)pl_g_state.level_witdth * BLOCK_W) + dim.x, 0.0f);
    cam->pos.y = clampf32(cam->pos.y,
                          (-(float)pl_g_state.level_height * BLOCK_H) + dim.y, 0.0f);

    cam->z = -0.1f;
}

static void render_platform_game(void* data, VkCommandBuffer command_buffer,
                                 u32 semaphore_idx)
{
    Index_Buffer* idx = &pl_g_state.g_pipline.idx_buffer;
    idx->curr_size = num_rects * 6;
    bind_and_draw_graphics_pipline(
        command_buffer, pl_g_state.g_pipline.descriptors.desc_sets[semaphore_idx], 0,
        idx->curr_size, &pl_g_state.g_pipline);
}

static void push_z(u32 i, f32 z)
{
    Z_Sorting thing = { i, z };
    synt_push(pl_g_state.z_sort, thing);
}

// TODO: faster sorting
static void bubble_sort_on_z(Z_Sorting** z_sort, u32 size)
{
    for_range(i, size - 1)
    {
        for_range(j, size - i - 1)
        {
            Z_Sorting* first = (*z_sort) + j;
            Z_Sorting* second = first + 1;
            if (first->z > second->z)
            {
                Z_Sorting temp = *first;
                *first = *second;
                *second = temp;
            }
        }
    }
}

void update_platform_game(Region_Alloc* region, VkDevice device, V2 dimensions,
                          u32 semaphore_idx, f32 dt)
{

    V2 extra_dim = v2d();

    Events* we = pl_g_state.wheel_evt;
    if (we->activated)
    {
        v2_s_add_equal(&extra_dim, (f32)we->wheel_evt.z_delta * -0.2f);
    }

    Vertex_Buffer* vert = &pl_g_state.g_pipline.vert_buffer;
    Vertex* t_storage = pl_g_state.temp_storage;
    Z_Sorting* z_sort = pl_g_state.z_sort;
    Rect2D* rects = pl_g_state.rects;
    num_rects = 0;
    get_head(vert->data)->size = 0;
    get_head(t_storage)->size = 0;
    get_head(z_sort)->size = 0;
    get_head(rects)->size = 0;

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
                V3 pos = v3f((float)j * BLOCK_W, (float)h_i * BLOCK_H, -1.0f);
                V2 size = v2f(BLOCK_W, BLOCK_H);
                V4 color_l = v4f(0.0f, 1.0f, 0.0f, 1.0f);
                V4 color_r = v4f(1.0f, 0.0f, 0.0f, 1.0f);
                if (j == 0 || j == pl_g_state.level_witdth - 1)
                {
                    synt_push(rects,
                              quad_gradiant_t_b_d2(&t_storage, &num_rects, pos, size,
                                                   color_l, color_r));
                }
                else
                {
                    synt_push(rects,
                              quad_gradiant_l_r_d2(&t_storage, &num_rects, pos, size,
                                                   color_l, color_r));
                }
            }
        }
        h_i++;
    }
    // NOTE to skip level for z sorting
    const u32 level_size = size_arr(t_storage);

    Camera_2D* cam = &pl_g_state.cam;

    // Player cam
    Dynamic_Entity_2D* p_e = pl_g_state.p_e;
    Rect2D* p_rect = &pl_g_state.player_rect;

    Dynamic_Entity_2D* f_e = pl_g_state.f_e;
    Rect2D* f_rect = &pl_g_state.friend_rect;

    static b8 friend_ctrl = false;

    static b8 cill = 0;
    if (is_key_clicked(&cill, SYNT_F_PRESSED))
    {
        friend_ctrl = friend_ctrl ? false : true;
    }

    V4 color_t = v4f(0.0f, 0.0f, 1.0f, 1.0f);
    V4 color_b = v4f(0.0f, 1.0f, 0.0f, 1.0f);
    V2 target_p = v2f(p_e->pos.x + BLOCK_W * 0.5f, p_e->pos.y + BLOCK_H * 0.5f);

    if (edit_mode)
    {
        update_camera_game(cam, dt);
    }
    else if (friend_ctrl)
    {
        entity_movement(f_e, f_rect, dt);
        V2 acc = v2f(0.0f, -9.82f);
        update_position(p_e, p_rect, acc, dt);
        follow_player_cam(cam, f_e->pos, dimensions, dt);
    }
    else
    {
        entity_movement(p_e, p_rect, dt);
        follow_position_pp(&f_e->pos, &f_e->vel, f_rect, target_p, dt, f_e->speed,
                           2000.0f);
        follow_player_cam(cam, p_e->pos, dimensions, dt);
    }
    cam->mvp.proj = ortho(0, dimensions.y, dimensions.x, 0, -1.0f, 1.0f);
    cam->mvp.model = m4_translate(m4i(1.0f), v3_v2f(cam->pos, cam->z));
    update_uniform_buffers(device,
                           &pl_g_state.g_pipline.uniform_buffers[semaphore_idx],
                           &cam->mvp, sizeof(cam->mvp));

    *f_rect = quad_gradiant_t_b(&t_storage, &num_rects, v3_v2f(f_e->pos, f_e->z),
                                v2i(10.0f), color_t, color_b, 1.0f);
    f_e->size = f_rect->size;
    f_rect->vel = f_e->vel;

    V2 c_n = v2f(0.0f, 0.0f);
    static b8 b_hits = false;
    static b8 side = false;
    if (dynamic_ray_rect_unsafe_d(f_rect, p_rect, &c_n, dt, -200.0f, 200.0f))
    {
        if (!b_hits)
        {
            f_e->z = side ? f_e->z - 0.5f : f_e->z + 0.5f;
        }
        b_hits = true;
    }
    else
    {
        if (b_hits)
        {
            side = side ? false : true;
        }
        b_hits = false;
    }

#if 0
    Dynamic_Entity_2D* butters = pl_g_state.b_es;
    Rect2D* b_r = pl_g_state.b_rects;
    u32 butters_size = capacity_arr(butters);
    for_range(i, butters_size)
    {
        V2 target_p = V2(p_e->pos.x + BLOCK_W * 0.5f, p_e->pos.y + BLOCK_H * 0.5f);
        Dynamic_Entity_2D* b = &butters[i];
        follow_position_pp(b->pos, b->vel, b_r[i], target_p, dt, b->speed, 100.0f);
        b_r[i] =
            quad_gradiant_t_b(&t_storage, &num_rects, V3(b->pos, b->z), V2(10.0f),
                              pl_g_state.b_c_t[i], pl_g_state.b_c_b[i], 1.0f);
        b_r[i].vel = V2(b->vel.x, b->vel.y);

        V2 c_n = V2(0.0f, 0.0f);
        static b8 b_hits[NUM_PARTICLES] = {};
        static b8 side[NUM_PARTICLES] = {};
        if (dynamic_ray_rect_unsafe(b_r[i], *p_rect, c_n, dt, -200.0f, 200.0f))
        {
            if (!b_hits[i])
            {
                b->z = side[i] ? b->z - 0.5f : b->z + 0.5f;
            }
            b_hits[i] = true;
        }
        else
        {
            if (b_hits[i])
            {
                side[i] = side[i] ? false : true;
            }
            b_hits[i] = false;
        }
    }
#endif

    V2 player_size = v2f(BLOCK_W, BLOCK_H);
    color_t = v4f(0.0f, 0.0f, 1.0f, 1.0f);
    color_b = v4f(0.0f, 1.0f, 0.0f, 1.0f);
    *p_rect = quad_gradiant_t_b(&t_storage, &num_rects, v3_v2f(p_e->pos, p_e->z),
                                player_size, color_t, color_b, 1.0f);
    p_rect->vel = p_e->vel;

    // Background cam

    const u32 data_size = size_arr(t_storage);
    ASSERT(data_size % 4 == 0, "");
    ASSERT(level_size % 4 == 0, "");
    for (u32 i = level_size; i < data_size; i += 4)
    {
        push_z(i, t_storage[i].pos.z);
    }
    bubble_sort_on_z(&z_sort, size_arr(z_sort));
    u32 sort_size = size_arr(z_sort);
    for_range(i, sort_size)
    {
        for_range(j, 4)
        {
            u32 index = (i * 4) + j;
            ASSERT(index < data_size, "");
            vert->data[level_size + index] = t_storage[z_sort[i].index + j];
        }
    }
    for_range(i, level_size)
    {
        vert->data[i] = t_storage[i];
    }

    map_copy_mem(device, &pl_g_state.g_pipline.vert_buffer.buffer_memory,
                 pl_g_state.g_pipline.vert_buffer.size_bytes,
                 pl_g_state.g_pipline.vert_buffer.data);

    draw_pipeline(render_platform_game, NULL);

#if 1
    gui_update_begin(region, dimensions, semaphore_idx, dt, translucentcy);
    {
        update_gui(region, dt);
    }
    gui_update_end();
#endif
}


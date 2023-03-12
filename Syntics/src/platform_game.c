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
#include <stddef.h>
#include <string.h>
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

    V2 mouse_pos;

    Rect2D* level_rects;
    Rect2D player_rect;
    Rect2D friend_rect;

    u8* level_array;
    u32 level_width;
    u32 level_height;

    u32 static_index;
    u32 num_semaphores;
} Platform_Game_State;

static Platform_Game_State pl_g_state = { 0 };

#define VERTICES_PER_RECT 4
#define INDICES_PER_RECT 6
#define NUM_PARTICLES 0

#define BLOCK_H 40.0f
#define BLOCK_W BLOCK_H

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
    destroy_buffer(device, pl_g_state.g_pipline.vert_buffer.buffer);
    destroy_buffer(device, pl_g_state.g_pipline.idx_buffer.buffer);

    vkDestroyDescriptorPool(device, pl_g_state.g_pipline.descriptors.desc_pool,
                            NULL);

#if 0
    for_range(i, num_semaphores)
    {
        destroy_buffer(device, pl_g_state.g_pipline.uniform_buffers[i].buffer);
    }
#endif
    for_range(i, size_arr(pl_g_state.textures))
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
    pl_g_state.level_width = width;
    pl_g_state.level_height = height;
    free_file(&file);
}

static u32 g_level_size = 0;
static void update_render_level()
{
    Vertex* t_storage = pl_g_state.temp_storage;
    Rect2D* rects = pl_g_state.level_rects;
    u8* data = pl_g_state.level_array;
    int h_i = 0;
    for (int i = pl_g_state.level_height - 1; i >= 0; i--)
    {
        for_range(j, pl_g_state.level_width)
        {
            if (data[(i * pl_g_state.level_width) + j])
            {
                V3 pos = v3f((float)j * BLOCK_W, (float)h_i * BLOCK_H, -1.0f);
                V2 size = v2f(BLOCK_W, BLOCK_H);
                V4 color_l = v4f(0.0f, 1.0f, 0.0f, 1.0f);
                V4 color_r = v4f(1.0f, 0.0f, 0.0f, 1.0f);
                if (j == 0 || j == pl_g_state.level_width - 1)
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
    g_level_size = size_arr(t_storage);
}

static u32 pos_to_tile(V2 pos)
{
    V2 world_space = v2_sub(pos, pl_g_state.cam.pos);

    f32 scalar = 1.0f / BLOCK_H;
    v2_s_multi_equal(&world_space, scalar);

    u32 x = (u32)floorf(world_space.x);
    u32 y = (u32)floorf(world_space.y);

    y = ((pl_g_state.level_height - 1) * (pl_g_state.level_width)) -
        (y * pl_g_state.level_width);

    u32 res = (y + x);
    return res;
}

void init_platform_game(Region_Alloc* region, VkDevice device,
                        VkPhysicalDevice physical_device, VkCommandPool command_pool,
                        VkQueue graphic_queue, const Swap_Chain_attrib* swap_chain,
                        u32 num_semaphores)
{
    init_entity(region);

    Quad2D q;
    q.pos = v2f(2.0f, 2.0f);
    q.points[0] = p2f(1.0f, 2.0f);
    q.points[1] = p2f(2.0f, 1.0f);
    q.points[2] = p2f(3.0f, 1.0f);
    q.points[3] = p2f(2.0f, 4.0f);

    Quad2D q1;
    q1.pos = v2f(5.0f, 2.0f);
    q1.points[0] = p2f(4.0f, 2.0f);
    q1.points[1] = p2f(4.0f, 1.0f);
    q1.points[2] = p2f(6.0f, 2.0f);
    q1.points[3] = p2f(5.0f, 4.0f);

    b8 res = quad_lines(&q, &q1);

    if (res)
    {
        SY_ERROR("Yeee baby");
    }

    pl_g_state.textures = dyn_arrayP(region, 3, Texture);

    create_texture_path(device, physical_device, command_pool, graphic_queue, true,
                        VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/default.png",
                        &pl_g_state.textures[0]);

    get_head(pl_g_state.textures)->size++;

    create_texture_path(device, physical_device, command_pool, graphic_queue, true,
                        VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/Circle.png",
                        &pl_g_state.textures[1]);

    get_head(pl_g_state.textures)->size++;

    create_texture_path(device, physical_device, command_pool, graphic_queue, false,
                        VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/ArialWhiteSmall.png",
                        &pl_g_state.textures[2]);

    get_head(pl_g_state.textures)->size++;

    pl_g_state.font = load_font_file(region, "Syntics/res/ArialWhiteSmall.fnt");
    pl_g_state.font.tex_index = 2;

    pl_g_state.g_pipline.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    create_graphics_pipeline(
        region, device, swap_chain->render_pass, swap_chain->sample_count,
        "Syntics/res/platform_game.vert.spv", "Syntics/res/platform_game.frag.spv",
        swap_chain->extent_2D.width, swap_chain->extent_2D.height, VK_CULL_MODE_NONE,
        size_arr(pl_g_state.textures), NULL, &pl_g_state.g_pipline);

#if 0
    pl_g_state.g_pipline.vert_buffer.data = dyn_arrayP(region, NUM_VERTICES, Vertex);
    pl_g_state.g_pipline.vert_buffer.buffer.size_bytes =
        capacity_arr(pl_g_state.g_pipline.vert_buffer.data) * sizeof(Vertex);

    create_vertex_buffer(device, physical_device, &pl_g_state.g_pipline.vert_buffer);
#endif

    // TODO: Dunno if this is smart. i'm writing directly into the buffer.
    init_graphics_pipeline_test(
        region, device, physical_device, NUM_VERTICES, num_semaphores,
        pl_g_state.textures, size_arr(pl_g_state.textures), &pl_g_state.g_pipline);

    pl_g_state.temp_storage = dyn_arrayP(region, NUM_VERTICES, Vertex);
    pl_g_state.z_sort = dyn_arrayP(region, NUM_RECTS, Z_Sorting);

    pl_g_state.g_pipline.idx_buffer.data = dyn_arrayT(region, NUM_INDICES, u32);
    generate_indices(&pl_g_state.g_pipline.idx_buffer.data, 0, NUM_RECTS);

    pl_g_state.g_pipline.idx_buffer.buffer.size_bytes =
        size_arr(pl_g_state.g_pipline.idx_buffer.data) * sizeof(u32);
    pl_g_state.g_pipline.idx_buffer.curr_size = 0;
    create_index_buffer_local(device, physical_device, command_pool, graphic_queue,
                              &pl_g_state.g_pipline.idx_buffer);

    region_pop(region, NUM_INDICES, u32, TEMP_ARRAY);

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

    load_level(region, "level_create.synt");
    pl_g_state.level_rects =
        dyn_arrayP(region, pl_g_state.level_height * pl_g_state.level_width, Rect2D);
    update_render_level();
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
static b32 show_graph = false;
static b32 reload_level = false;
static b32 edit_mode = true;
static b32 play_edit_mode = false;
static f32 g_dist_ = 0;

static b32 g_show_e = false;
static Dynamic_Entity_2D* entity_to_show = NULL;

static V4 colorddd = { 0 };
static void update_gui(Region_Alloc* region, f32 dt, V2 dimensions, u32 fps)
{
    back_bord_begin("TTTT", v2i(100.0f));
    {
        gridd_begin(2, 1);
        {
            add_text("Translucentcy: ");
            add_input_float_d(&translucentcy, 0.0f, 1.0f);
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
                f32 milli = dt * 1000.0f;
                sprintf(temp, "Milli: %f | FPS: %u", milli, fps);
                count = 0.0f;
            }
            count += dt;
            add_text(temp);
        }
        gridd_end();
        gridd_begin(2, 3);
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
            if (add_button("Play Edit mode"))
            {
                b_switch(play_edit_mode);
                b_switch(edit_mode);
            }
            add_input_float_d(&g_dist_, 0.0f, 2000.0f);
        }
        gridd_end();
        gridd_begin(4, 1);
        {
            add_input_float_d(&colorddd.x, 0.0f, 1.0f);
            add_input_float_d(&colorddd.y, 0.0f, 1.0f);
            add_input_float_d(&colorddd.z, 0.0f, 1.0f);
            add_input_float_d(&colorddd.w, 0.0f, 1.0f);
        }
        gridd_end();

        if (g_show_e && entity_to_show)
        {
            edit_show_entity(entity_to_show, "Player");
        }
    }
    back_bord_end();
    back_bord_begin("Terminal", v2f(500.0f, 100.0f));
    {
        add_terminal(250.0f, 200.0f);
    }
    back_bord_end();
    if (show_graph && g_show_e && entity_to_show)
    {
        back_bord_begin("Graph", v2f(800.0f, 100.0f));
        {
            add_graph(entity_to_show->vel.x, "Entity vel x", 20.0f, -20.0f, 5.0f,
                      dt);
        }
        back_bord_end();
    }
}

static void entity_select(V2 dimensions)
{
    Dynamic_Entity_2D* e = NULL;
    Vertex* t_storage = pl_g_state.temp_storage;
    Camera_2D* cam = &pl_g_state.cam;

    V2 mouse_pos_world = pl_g_state.mouse_pos;

    v2_sub_equal(&mouse_pos_world, cam->pos);
    u32 i = 0;
    // b32 any_hit = false;
    while ((e = iterate_entities(&i)))
    {
        if (point_in_entity_2d(mouse_pos_world, e))
        {
            V4 t_color = v4f(0.1f, 0.1f, 0.1f, translucentcy);
            V4 b_color = v4f(0.05f, 0.05f, 0.05f, translucentcy);
            V2 drop_down_size = v2f(210.0f, 50.0f);
            mouse_pos_world.y -= drop_down_size.y;

            char buffer[100] = { 0 };
            sprintf(buffer, "Pos: (x:%.2f, y:%.2f)\nVel: (x:%.2f, y:%.2f)", e->pos.x,
                    e->pos.y, e->vel.x, e->vel.y);

            u32 buffer_len = (u32)strlen(buffer);

            num_rects += text_2D(
                pl_g_state.font, -1.0f, buffer, buffer_len,
                v3f(mouse_pos_world.x + 12.0f, mouse_pos_world.y + 40.0f, 0.0f),
                v4i(1.0f), 1.0f, NULL, NULL, &t_storage);

            quad_s_gradiant_t_b_d2(&t_storage, &num_rects,
                                   v3_v2f(mouse_pos_world, -0.001f), drop_down_size,
                                   b_color, t_color);

            // any_hit = true;
            static b8 clicked_lock = true;
            if (is_any_button_clicked(&clicked_lock))
            {
                g_show_e = true;
                entity_to_show = e;
            }
            break;
        }
    }
#if 0
    if (!any_hit && !gui_focus())
    {
        static b8 clicked_lock = true;
        if (is_any_button_clicked(&clicked_lock))
        {
            g_show_e = false;
        }
    }
#endif
}

static V2 calculate_pos(Dynamic_Entity_2D* entity, V2 acc, f32 dt)
{
    synt_LOG_Term("%f\n", dt);
    V2 pos = v2_add(v2_s_multi(acc, 0.5f * dt * dt),
                    v2_add(v2_s_multi(entity->vel, dt), entity->pos));
    entity->vel = v2_add(v2_s_multi(acc, dt), entity->vel);
    return pos;
}

static void update_camera_game(Camera_2D* cam, f32 dt)
{
    static b8 first_clicked = true;
    if (is_any_button_pressed() && !gui_focus())
    {
        if (is_key_pressed(SYNT_KEY_SHIFT))
        {
            u8* data = pl_g_state.level_array;
            u32 index = pos_to_tile(pl_g_state.mouse_pos);
            u32 l_d_capacity = capacity_arr(data);
            ASSERT(index < l_d_capacity, "index to flipping high");
            if (pl_g_state.mouse_evt->mouse_evt.button_evt.button ==
                SYNT_LEFT_BUTTON)
            {
                data[index] = 1;
            }
            else
            {
                data[index] = 0;
            }
        }
        else
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
    }
    else
    {
        first_clicked = true;
    }
}

static void update_position(Dynamic_Entity_2D* entity, const Rect2D* rect, V2 acc,
                            f32 dt)
{
    acc.x -= 2.0f * entity->vel.x;
    V2 contact_normal = v2f(0.0f, 0.0f);
    V2 contact_point = v2d();
    f32 contact_time = 0.0f;
    Rect2D* r = pl_g_state.level_rects;
    u32 size = size_arr(r);
    for_range(i, size)
    {
        if (dynamic_ray_rect_unsafe(rect, &r[i], &contact_point, &contact_normal,
                                    &contact_time, dt, -5.0f, 5.0f))
        {
            V2 n = v2f(contact_normal.x, contact_normal.y);
            entity->vel =
                v2_sub(entity->vel, v2_s_multi(n, 1.5f * v2_dot(entity->vel, n)));
            // acc.y -= 12.0f * entity->vel.y;
            break;
        }
    }
    entity->pos = calculate_pos(entity, acc, dt);
}

static void entity_movement(Dynamic_Entity_2D* entity, const Rect2D* rect, f32 dt)
{
    V2 acc = v2d();
    f32 speed = 90.0f;
    if (is_key_pressed(SYNT_KEY_A))
    {
        acc.x = -10.0f;
    }
    if (is_key_pressed(SYNT_KEY_D))
    {
        acc.x = 10.0f;
    }
    acc.y = -9.81f * speed;
    static b8 clicked1 = true;
    if (is_key_clicked(&clicked1, SYNT_KEY_W))
    {
        entity->vel.y = 1000.0f;
    }
    static b8 clicked2 = true;
    if (is_key_clicked(&clicked2, SYNT_KEY_SHIFT))
    {
        speed *= 40.0f;
    }
    acc.x *= speed;
    update_position(entity, rect, acc, dt);
}

static void follow_position_pp(V2* pos, V2* last_vel, const Rect2D* rect, V2 target,
                               f32 dt, f32 per_distance_speed, f32 max_speed)
{
    f32 dis = v2_distance(*pos, target);

    V2 dir = v2_normalize(v2_sub(target, *pos));
    f32 speed = dis * per_distance_speed;

    v2_s_multi_equal(&dir, speed);
    if (dis > 60.0f)
    {
        v2_sub_equal(&dir, v2_s_multi(*last_vel, 3.0f));
    }
#if 1
    Rect2D* r = pl_g_state.level_rects;
    u32 r_size = size_arr(r);
    V2 contact_normal = v2f(0.0f, 0.0f);
    for_range(i, r_size)
    {
        if (dynamic_ray_rect_unsafe_d(rect, &r[i], &contact_normal, dt, -1.0f, 1.0f))
        {
            V2 n = v2f(contact_normal.x, contact_normal.y);
            *last_vel =
                v2_sub(*last_vel, v2_s_multi(n, 1.5f * v2_dot(*last_vel, n)));
            break;
        }
    }
#endif
    *pos = v2_add(v2_s_multi(dir, 0.5f * dt * dt),
                  v2_add(v2_s_multi(*last_vel, dt), *pos));
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
                          (-(float)pl_g_state.level_width * BLOCK_W) + dim.x, 0.0f);
    cam->pos.y = clampf32(cam->pos.y,
                          (-(float)pl_g_state.level_height * BLOCK_H) + dim.y, 0.0f);

    cam->z = -0.1f;
}

static void render_platform_game(void* data, VkCommandBuffer command_buffer,
                                 u32 semaphore_idx)
{
    vkCmdPushConstants(command_buffer, pl_g_state.g_pipline.layout,
                       VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MVP),
                       &pl_g_state.cam.mvp);

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
                          u32 semaphore_idx, f32 dt, u32 fps)
{

    V2 extra_dim = v2d();

    Events* we = pl_g_state.wheel_evt;
    if (we->activated)
    {
        v2_s_add_equal(&extra_dim, (f32)we->wheel_evt.z_delta * -0.2f);
    }
    pl_g_state.mouse_pos.x = (f32)pl_g_state.mouse_evt->mouse_evt.move_evt.pos_x;
    pl_g_state.mouse_pos.y =
        dimensions.y - (f32)pl_g_state.mouse_evt->mouse_evt.move_evt.pos_y;

    Vertex_Buffer* vert = &pl_g_state.g_pipline.vert_buffer;
    Vertex* t_storage = pl_g_state.temp_storage;
    Z_Sorting* z_sort = pl_g_state.z_sort;
    get_head(z_sort)->size = 0;
    get_head(t_storage)->size = 0;
    get_head(pl_g_state.level_rects)->size = 0;
    num_rects = 0;
    get_head(vert->data)->size = 0;
    // NOTE to skip level for z sorting
    if (reload_level)
    {
        load_level(NULL, "level_create.synt");
        reload_level = false;
    }
    update_render_level();

    Camera_2D* cam = &pl_g_state.cam;

    // Player cam
    Dynamic_Entity_2D* p_e = pl_g_state.p_e;
    Rect2D* p_rect = &pl_g_state.player_rect;

    Dynamic_Entity_2D* f_e = pl_g_state.f_e;
    Rect2D* f_rect = &pl_g_state.friend_rect;

    static b8 friend_ctrl = false;

    static b8 f_key_lock = 0;
    if (is_key_clicked(&f_key_lock, SYNT_KEY_F))
    {
        b_switch(friend_ctrl);
    }
    static b8 e_key_lock = 0;
    if (is_key_clicked(&e_key_lock, SYNT_KEY_E))
    {
        b_switch(edit_mode);
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
        V2 acc = v2f(0.0f, -9.82f * 50.0f);
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

#if 0
    copy_data_buffer(&pl_g_state.g_pipline.uniform_buffers[semaphore_idx].buffer,
                     &cam->mvp, sizeof(cam->mvp));
#endif

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
    p_e->size = p_rect->size;
    p_rect->vel = p_e->vel;

    if (edit_mode || play_edit_mode)
    {
        entity_select(dimensions);
#if 1
        gui_update_begin(region, dimensions, semaphore_idx, dt, translucentcy);
        {
            update_gui(region, dt, dimensions, fps);
        }
        gui_update_end();
#endif
    }

    // Background cam

    const u32 data_size = size_arr(t_storage);
    ASSERT(data_size % 4 == 0, "");
    ASSERT(g_level_size % 4 == 0, "");
    for (u32 i = g_level_size; i < data_size; i += 4)
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
            vert->data[g_level_size + index] = t_storage[z_sort[i].index + j];
        }
    }
    for_range(i, g_level_size)
    {
        vert->data[i] = t_storage[i];
    }

    // Writing directly into the buffer.
#if 0
    Vertex_Buffer* vb = &pl_g_state.g_pipline.vert_buffer;
    copy_data_buffer(&vb->buffer, vb->data, vb->buffer.size_bytes);
#endif

    draw_pipeline(render_platform_game, NULL);
}


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
#include <stdlib.h>
#include <math.h>

void draw_pipeline(void (*draw_callback)(void* data, VkCommandBuffer command_buffer,
                                         u32 semaphore_idx),
                   void* data);

void subscribe_recreate_callback(void (*rc_callback)(void* data, Region_Alloc* region,
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
    Graphic_Pipline g_pipeline;
    Graphic_Pipline coll_g_pipeline;
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
    
    Region_Alloc frame_region;
    
    Polygon2D* coll_shapes;
    
    Texture* textures;
    Font font;
    Events* mouse_evt;
    Events* wheel_evt;
    Events* key_evt;
    
    V2 mouse_pos;
    
    Rect2D* level_rects;
    Rect2D player_rect;
    Rect2D friend_rect;
    
    u8* level_array;
    u32 level_width;
    u32 level_height;
    
    u32 num_semaphores;
} Platform_Game_State;

typedef struct Poly_Undo
{
    u32* ids;
    V2* points;
    V2* pos;
} Poly_Undo;

typedef struct Collision_Edit_State
{
    V2 presist_offset;
    
    u32 pol_index;
    u32 p_index;
    
    b32 point_selected;
    b32 poly_selected;
    
    Poly_Undo undo;
} Collision_Edit_State;

static Platform_Game_State pl_g_state = { 0 };
static Collision_Edit_State c_e_g_state = { 0 };

#define VERTICES_PER_RECT 4
#define INDICES_PER_RECT 6
#define NUM_PARTICLES 0

#define BLOCK_H 40.0f
#define BLOCK_W BLOCK_H

#define NUM_RECTS 10000
static const u32 NUM_VERTICES = NUM_RECTS * VERTICES_PER_RECT;
static const u32 NUM_INDICES = NUM_RECTS * INDICES_PER_RECT;
static u32 num_rects = 0;
#define COLLISION_SIZE 20000
#define COLLISION_UNDO_SIZE 1000

b32 is_undo_full(Poly_Undo* p_u)
{
    return !_check_array_size(p_u->pos) || !_check_array_size(p_u->points);
}

void flush_undo(Poly_Undo* p_u, const Polygon2D* polys)
{
    u32 size_to_remove = (u32)(size_arr(p_u->ids) * 0.5f);
    for_range(i, size_to_remove)
    {
        u32 index = synt_pop(p_u->ids);
        ASSERT(index < get_head(polys)->size, "flush_undo");
        u32 points_to_remove = polys[synt_pop(p_u->ids)].n_sides;
        get_head(p_u->points)->size -= points_to_remove;
    }
    get_head(p_u->pos)->size -= size_to_remove;
}

void save_undo(Poly_Undo* p_u, const Polygon2D* pols, u32 i)
{
    const Polygon2D* pol = &pols[i];
    if (is_undo_full(p_u))
    {
        flush_undo(p_u, pols);
    }
    synt_push(p_u->ids, i);
    synt_push(p_u->pos, pol->pos);
    for_range(points, pol->n_sides)
    {
        synt_push(p_u->points, pol->points[points]);
    }
}

static b32 g_render_collision = true;
static void recreate_platform_game(void* data, Region_Alloc* region,
                                   const Application_State* app_state)
{
    recreate_graphic_pipline_ap(region, app_state, "Syntics/res/platform_game.vert.spv",
                                "Syntics/res/platform_game.frag.spv",
                                &pl_g_state.g_pipeline, size_arr(pl_g_state.textures),
                                NULL);
    
    if (g_render_collision)
    {
        recreate_graphic_pipline_ap(
                                    region, app_state, "Syntics/res/platform_game.vert.spv",
                                    "Syntics/res/gui_graph.frag.spv", &pl_g_state.coll_g_pipeline, 1, NULL);
    }
    
    gui_recreate(region);
}

static V2 calculate_centroid(const Polygon2D* p)
{
    V2 res = v2d();
    for_range(i, p->n_sides)
    {
        v2_add_equal(&res, p->points[i]);
    }
    ASSERT(p->n_sides, "calculate_centroid");
    f32 scalar = 1.0f / p->n_sides;
    v2_s_multi_equal(&res, scalar);
    return res;
}

static void poly_save_to_file()
{
    
    char buffer[4096] = { 0 };
    
    u32 size = size_arr(pl_g_state.coll_shapes);
    
    u32 len = 0;
    for_range(i, size)
    {
        Polygon2D* shape = &pl_g_state.coll_shapes[i];
        
        sprintf_s(buffer + len, sizeof(buffer) - len, "id,%u\nn,%u\n", shape->id,
                  shape->n_sides);
        
        for_range(j, shape->n_sides)
        {
            len = (u32)strlen(buffer);
            sprintf_s(buffer + len, sizeof(buffer) - len, "p,%u,x,%f,y,%f\n", j,
                      shape->points[j].x, shape->points[j].y);
        }
        len = (u32)strlen(buffer);
        buffer[len++] = '\n';
        buffer[len++] = '\n';
    }
    
    buffer[len] = '\0';
    
    write_entire_file("saved_geometry.txt", buffer);
}

static void destroy_platform_game(void* data, VkDevice device, u32 num_semaphores)
{
    poly_save_to_file();
    destroy_graphic_pipeline(device, 0, &pl_g_state.g_pipeline);
    destroy_graphic_pipeline(device, 0, &pl_g_state.coll_g_pipeline);
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
    i32 h_i = 0;
    for (i32 i = pl_g_state.level_height - 1; i >= 0; i--)
    {
        for_range(j, pl_g_state.level_width)
        {
            if (data[(i * pl_g_state.level_width) + j])
            {
                V3 pos = v3f((f32)j * BLOCK_W, (f32)h_i * BLOCK_H, -1.0f);
                V2 size = v2f(BLOCK_W, BLOCK_H);
                V4 color_l = v4f(0.0f, 1.0f, 0.0f, 1.0f);
                V4 color_r = v4f(1.0f, 0.0f, 0.0f, 1.0f);
                if (j == 0 || j == pl_g_state.level_width - 1)
                {
                    synt_push(rects, quad_gradiant_t_b_d2(&t_storage, &num_rects, pos,
                                                          size, color_l, color_r));
                }
                else
                {
                    synt_push(rects, quad_gradiant_l_r_d2(&t_storage, &num_rects, pos,
                                                          size, color_l, color_r));
                }
            }
        }
        h_i++;
    }
    g_level_size = size_arr(t_storage);
}
static void move_polygon_offest(Polygon2D* p, V2 pos, V2 p_offset)
{
    V2 offset = v2_add(pos, p_offset);
    for_range(i, p->n_sides)
    {
        V2 centroid_vec = v2_sub(p->points[i], p->pos);
        p->points[i] = v2_add(offset, centroid_vec);
    }
    p->pos = offset;
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

static Polygon2D poly2D_region(Region_Alloc* region, u32 n_sides)
{
    static u32 id = 0;
    Polygon2D res = { 0 };
    res.points = region_mallocP(region, n_sides, V2);
    res.normals = region_mallocP(region, n_sides, V2);
    res.n_sides = n_sides;
    res.id = id++;
    return res;
}

#define SEPERATOR(x) (((x) == ' ') || ((x) == '\t') || ((x) == '\n') || ((x) == ','))

static i32 read_word(const File_Attrib* file, u32* i, char* buffer)
{
    i32 buffer_i = 0;
    b32 written = false;
    while ((*i) < file->size)
    {
        if (!SEPERATOR(file->buffer[(*i)]))
        {
            buffer[buffer_i++] = file->buffer[(*i)++];
            written = true;
        }
        else
        {
            if (!written) (*i)++;
            break;
        }
    }
    buffer[buffer_i] = '\0';
    if ((*i) >= file->size) buffer_i = -1;
    return buffer_i;
}

internal i32 remove_miss(const File_Attrib* file, u32* i, char* buffer)
{
    i32 res = 0;
    do
    {
        res = read_word(file, i, buffer);
    } while (!res);
    
    return res;
}

#define END_OF_FILE -1

static V2 read_x_y(const File_Attrib* file, u32* i, char* buffer)
{
    V2 res = v2d();
    for_range(j, 2)
    {
        i32 read = 0;
        read = remove_miss(file, i, buffer);
        if (read == END_OF_FILE) return res;
        
        for_range(k, (u32)read)
        {
            if (buffer[k] == 'x')
            {
                read = remove_miss(file, i, buffer);
                if (read == END_OF_FILE) return res;
                res.x = (f32)atof(buffer);
                break;
            }
            else if (buffer[k] == 'y')
            {
                read = remove_miss(file, i, buffer);
                if (read == END_OF_FILE) return res;
                res.y = (f32)atof(buffer);
                break;
            }
        }
    }
    return res;
}

static void parse_shape_file(Region_Alloc* region)
{
    stack_begin_scope();
    
    File_Attrib file = { 0 };
    read_file(&file, get_stack(), "saved_geometry.txt", "r");
    char buffer[40] = { 0 };
    Polygon2D p = { 0 };
    u32 count = 0;
    for_range(i, file.size)
    {
        if (remove_miss(&file, &i, buffer) == END_OF_FILE) continue;
        
        if (!strcmp(buffer, "id"))
        {
            if (count++ > 0)
            {
                p.pos = calculate_centroid(&p);
                synt_push(pl_g_state.coll_shapes, p);
                memset(&p, 0, sizeof(p));
            }
            if (remove_miss(&file, &i, buffer) == END_OF_FILE) continue;
            
            p.id = (u32)atoi(buffer);
        }
        else if (!strcmp(buffer, "n"))
        {
            if (remove_miss(&file, &i, buffer) == END_OF_FILE) continue;
            
            p.n_sides = (u32)atoi(buffer);
            ASSERT(closed_interval(3, p.n_sides, 15), "Polygon too small or too big");
            p = poly2D_region(region, p.n_sides);
        }
        else if (!strcmp(buffer, "p"))
        {
            ASSERT(p.n_sides != 0, "sides not correct");
            if (remove_miss(&file, &i, buffer) == END_OF_FILE) continue;
            
            u32 index = atoi(buffer);
            ASSERT(index < p.n_sides, "parse p");
            p.points[index] = read_x_y(&file, &i, buffer);
        }
    }
    if (count)
    {
        p.pos = calculate_centroid(&p);
        synt_push(pl_g_state.coll_shapes, p);
    }
    
    stack_end_scope();
}

void init_platform_game(Region_Alloc* region, VkDevice device,
                        VkPhysicalDevice physical_device, VkCommandPool command_pool,
                        VkQueue graphic_queue, const Swap_Chain_attrib* swap_chain,
                        u32 num_semaphores)
{
    stack_begin_scope();
    
    init_entity(region);
    
    init_region(&pl_g_state.frame_region, MEGABYTE(2));
    
    c_e_g_state.undo.ids = dyn_arrayP(region, COLLISION_UNDO_SIZE, u32);
    c_e_g_state.undo.pos = dyn_arrayP(region, COLLISION_UNDO_SIZE, V2);
    c_e_g_state.undo.points = dyn_arrayP(region, COLLISION_UNDO_SIZE * 5, V2);
    
    const char* paths[] = {
        "Syntics/res/default.png",
        "Syntics/res/Circle.png",
        "Syntics/res/ArialWhiteSmall.png",
    };
    u32 num_text = sy_SIZE(paths);
    pl_g_state.textures = dyn_arrayP(region, num_text, Texture);
    
    for_range(i, num_text)
    {
        create_texture_path(device, physical_device, command_pool, graphic_queue, true,
                            VK_FORMAT_R8G8B8A8_SRGB, paths[i], &pl_g_state.textures[i]);
    }
    get_head(pl_g_state.textures)->size = num_text;
    
    pl_g_state.font = load_font_file(region, "Syntics/res/ArialWhiteSmall.fnt");
    pl_g_state.font.tex_index = 2;
    
    Graphic_Pipline* g_p = &pl_g_state.g_pipeline;
    g_p->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    g_p->cull_mode = VK_CULL_MODE_FRONT_BIT;
    g_p->poly_mode = VK_POLYGON_MODE_FILL;
    create_graphics_pipeline(
                             device, swap_chain->render_pass, swap_chain->sample_count,
                             "Syntics/res/platform_game.vert.spv", "Syntics/res/platform_game.frag.spv",
                             swap_chain->extent_2D.width, swap_chain->extent_2D.height, num_text, NULL, g_p);
    
    init_graphics_pipeline(region, device, physical_device, NUM_VERTICES, num_semaphores,
                           pl_g_state.textures, size_arr(pl_g_state.textures), g_p);
    
    pl_g_state.temp_storage = dyn_arrayP(region, NUM_VERTICES, Vertex);
    pl_g_state.z_sort = dyn_arrayP(region, NUM_RECTS, Z_Sorting);
    
    g_p->idx_buffer.data = dyn_arrayT(region, NUM_INDICES, u32);
    generate_indices(&g_p->idx_buffer.data, 0, NUM_RECTS);
    
    g_p->idx_buffer.buffer.size_bytes = size_arr(g_p->idx_buffer.data) * sizeof(u32);
    g_p->idx_buffer.curr_size = 0;
    create_index_buffer_local(device, physical_device, command_pool, graphic_queue,
                              &g_p->idx_buffer);
    
    region_pop(region, NUM_INDICES, u32, TEMP_ARRAY);
    
    Graphic_Pipline* coll_g_p = &pl_g_state.coll_g_pipeline;
    coll_g_p->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    coll_g_p->cull_mode = VK_CULL_MODE_FRONT_BIT;
    coll_g_p->poly_mode = VK_POLYGON_MODE_FILL;
    create_graphics_pipeline(
                             device, swap_chain->render_pass, swap_chain->sample_count,
                             "Syntics/res/platform_game.vert.spv", "Syntics/res/gui_graph.frag.spv",
                             swap_chain->extent_2D.width, swap_chain->extent_2D.height, 1, NULL, coll_g_p);
    
    init_graphics_pipeline(region, device, physical_device, COLLISION_SIZE,
                           num_semaphores, pl_g_state.textures, 1, coll_g_p);
    
    coll_g_p->idx_buffer.data = dyn_arrayP(region, COLLISION_SIZE, u32);
    
    coll_g_p->idx_buffer.buffer.size_bytes =
        capacity_arr(coll_g_p->idx_buffer.data) * sizeof(u32);
    create_index_buffer_visible(device, physical_device, &coll_g_p->idx_buffer);
    
    pl_g_state.coll_shapes = dyn_arrayP(region, (u32)(COLLISION_SIZE * 0.4f), Polygon2D);
    
    parse_shape_file(region);
    
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
    
    subscribe(&pl_g_state.mouse_evt, EVT_MOUSE);
    subscribe(&pl_g_state.key_evt, EVT_KEY);
    
    subscribe_recreate_callback(recreate_platform_game, NULL);
    subscribe_destroy_callback(destroy_platform_game, NULL);
    
    gui_init(region, device, physical_device, command_pool, graphic_queue, swap_chain,
             num_semaphores, true);
    
    stack_end_scope();
}

static f32 translucentcy = 1.0f;
static b32 show_graph = true;
static b32 reload_level = false;
static b32 edit_mode = true;
static b32 play_edit_mode = false;
static b32 wire_frame = false;
static f32 g_dist_ = 5.0f;
static f32 g_dist_1 = 90.0f;

static b32 g_show_e = false;
static Dynamic_Entity_2D* entity_to_show = NULL;

static void update_gui(Region_Alloc* region, const Application_State* app_state, f32 dt,
                       V2 dimensions)
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
                sprintf_s(temp, sizeof(temp), "Milli: %f | FPS: %u", milli, app_state->fps);
                count = 0.0f;
            }
            count += dt;
            add_text(temp);
        }
        gridd_end();
        gridd_begin(2, 5);
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
            add_input_float_d(&g_dist_, 2.0f, 80.0f);
            add_input_float_d(&g_dist_1, 0.0f, 80.0f);
            if (add_button("Wire frame"))
            {
                if (!wire_frame)
                {
                    pl_g_state.coll_g_pipeline.poly_mode = VK_POLYGON_MODE_LINE;
                    wire_frame = true;
                }
                else
                {
                    pl_g_state.coll_g_pipeline.poly_mode = VK_POLYGON_MODE_FILL;
                    wire_frame = false;
                }
                if (g_render_collision)
                {
                    recreate_graphic_pipline_ap(region, app_state,
                                                "Syntics/res/platform_game.vert.spv",
                                                "Syntics/res/gui_graph.frag.spv",
                                                &pl_g_state.coll_g_pipeline, 1, NULL);
                }
            }
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
    if (show_graph)
    {
        back_bord_begin("Graph", v2f(800.0f, 100.0f));
        {
            add_graph(dt * 1000.0f, "Milliseconds per frame", 20.0f, 10.0f, 5.0f, dt);
        }
        back_bord_end();
    }
}

internal void entity_select(V2 dimensions)
{
    Vertex* t_storage = pl_g_state.temp_storage;
    Camera_2D* cam = &pl_g_state.cam;
    
    V2 mouse_pos_world = pl_g_state.mouse_pos;
    
    v2_sub_equal(&mouse_pos_world, cam->pos);
    
    u32 i = 0;
    Dynamic_Entity_2D* e = iterate_entities(&i);
    for (; e; e = iterate_entities(&i))
    {
        if (point_in_entity_2d(mouse_pos_world, e))
        {
            V4 t_color = v4f(0.1f, 0.1f, 0.1f, translucentcy);
            V4 b_color = v4f(0.05f, 0.05f, 0.05f, translucentcy);
            V2 drop_down_size = v2f(210.0f, 50.0f);
            mouse_pos_world.y -= drop_down_size.y;
            
            char buffer[100] = { 0 };
            sprintf_s(buffer, sizeof(buffer),
                      "Pos: (x:%.2f, y:%.2f)\nVel: (x:%.2f, y:%.2f)", e->pos.x, e->pos.y,
                      e->vel.x, e->vel.y);
            
            u32 buffer_len = (u32)strlen(buffer);
            
            num_rects +=
                text_2D(pl_g_state.font, -1.0f, buffer, buffer_len,
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

static void update_camera_game(Camera_2D* cam, f32 dt)
{
    static b8 first_clicked = true;
    if (is_any_button_pressed() && !gui_focus() && !c_e_g_state.point_selected &&
        !c_e_g_state.poly_selected)
    {
        if (is_key_pressed(SYNT_KEY_SHIFT))
        {
            u8* data = pl_g_state.level_array;
            u32 index = pos_to_tile(pl_g_state.mouse_pos);
            u32 l_d_capacity = capacity_arr(data);
            ASSERT(index < l_d_capacity, "index to flipping high");
            if (pl_g_state.mouse_evt->mouse_evt.button_evt.button == SYNT_LEFT_BUTTON)
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

static void move_polygon(Polygon2D* p, V2 pos)
{
    calculate_centroid(p);
    V2 d = v2_sub(pos, p->pos);
    for_range(i, p->n_sides)
    {
        p->points[i] = v2_add(p->points[i], d);
    }
    p->pos = pos;
}

void test_collision(Dynamic_Entity_2D* entity, Rect2D* target, V2 pos)
{
    if (point_in_rect(pos, &pl_g_state.player_rect))
    {
        static b8 lock = true;
        if (is_any_button_clicked(&lock))
        {
            c_e_g_state.presist_offset = v2_sub(entity->pos, pos);
        }
        if (is_any_button_pressed())
        {
            entity->pos = v2_add(pos, c_e_g_state.presist_offset);
            V2 normal = v2d();
            if (rect_in_rect_normal(&pl_g_state.player_rect, target, &normal))
            {
                synt_LOG_Term("(x: %f, y: %f)\n", normal.x, normal.y);
            }
        }
    }
}

static V2 calculate_pos(Dynamic_Entity_2D* entity, V2 acc, f32 dt)
{
    V2 pos = v2_add(v2_s_multi(acc, 0.5f * dt * dt),
                    v2_add(v2_s_multi(entity->vel, dt), entity->pos));
    
    entity->vel = v2_add(v2_s_multi(acc, dt), entity->vel);
    
    return pos;
}

static b32 hit_ground = false;

static void update_position(Dynamic_Entity_2D* entity, Rect2D* rect, V2 acc, f32 dt)
{
    entity->vel.x -= 3.0f * entity->vel.x * dt;
    b32 hit = false;
    rect->pos = calculate_pos(entity, acc, dt);
    
    Rect2D* r = pl_g_state.level_rects;
    u32 r_size = size_arr(r);
    V2 n = v2d();
    
    for_range(i, r_size)
    {
        if (dynamic_ray_rect_unsafe_d(rect, &r[i], &n, dt, -1.0f, 1.0f))
        {
            entity->vel =
                v2_sub(entity->vel, v2_s_multi(n, 1.5f * v2_dot(entity->vel, n)));
            hit = true;
            break;
        }
    }
    if (!hit)
    {
        hit_ground = false;
        entity->pos = rect->pos;
    }
}

static void entity_movement(Dynamic_Entity_2D* entity, Rect2D* rect, f32 dt)
{
    V2 acc = v2d();
    f32 speed = 80.0f;
    if (is_key_pressed(SYNT_KEY_A))
    {
        acc.x = -10.0f;
    }
    if (is_key_pressed(SYNT_KEY_D))
    {
        acc.x = 10.0f;
    }
    if (!hit_ground)
    {
        acc.y = -9.81f;
    }
    static b8 clicked1 = true;
    if (is_key_clicked(&clicked1, SYNT_KEY_W))
    {
        entity->vel.y = 10.0f * speed;
    }
    static b8 clicked2 = true;
    if (is_key_clicked(&clicked2, SYNT_KEY_SHIFT))
    {
        speed *= 40.0f;
    }
    acc.x *= speed;
    acc.y *= speed;
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
            *last_vel = v2_sub(*last_vel, v2_s_multi(n, 2.0f * v2_dot(*last_vel, n)));
            break;
        }
    }
#endif
    *pos =
        v2_add(v2_s_multi(dir, 0.5f * dt * dt), v2_add(v2_s_multi(*last_vel, dt), *pos));
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
    cam->pos.x =
        clampf32(cam->pos.x, (-(float)pl_g_state.level_width * BLOCK_W) + dim.x, 0.0f);
    cam->pos.y =
        clampf32(cam->pos.y, (-(float)pl_g_state.level_height * BLOCK_H) + dim.y, 0.0f);
    
    cam->z = -0.1f;
}

static void render_platform_game(void* data, VkCommandBuffer command_buffer,
                                 u32 semaphore_idx)
{
    if (g_render_collision)
    {
        vkCmdPushConstants(command_buffer, pl_g_state.coll_g_pipeline.layout,
                           VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MVP),
                           &pl_g_state.cam.mvp);
        
        Index_Buffer* idx = &pl_g_state.coll_g_pipeline.idx_buffer;
        idx->curr_size = size_arr(idx->data);
        bind_and_draw_graphics_pipline(
                                       command_buffer,
                                       pl_g_state.coll_g_pipeline.descriptors.desc_sets[semaphore_idx], 0,
                                       idx->curr_size, &pl_g_state.coll_g_pipeline);
    }
    vkCmdPushConstants(command_buffer, pl_g_state.g_pipeline.layout,
                       VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MVP), &pl_g_state.cam.mvp);
    
    Index_Buffer* idx = &pl_g_state.g_pipeline.idx_buffer;
    idx->curr_size = num_rects * 6;
    bind_and_draw_graphics_pipline(
                                   command_buffer, pl_g_state.g_pipeline.descriptors.desc_sets[semaphore_idx], 0,
                                   idx->curr_size, &pl_g_state.g_pipeline);
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

void update_platform_game(Region_Alloc* region, const Application_State* app_state,
                          VkDevice device, V2 dimensions, u32 semaphore_idx, f32 dt)
{
    Region_Alloc* frame_region = &pl_g_state.frame_region;
    reset_region(frame_region);
    
    pl_g_state.mouse_pos.x = (f32)pl_g_state.mouse_evt->mouse_evt.move_evt.pos_x;
    pl_g_state.mouse_pos.y =
        dimensions.y - (f32)pl_g_state.mouse_evt->mouse_evt.move_evt.pos_y;
    
#if 0  
    Converting to Normalized device coordinates:
    ndc_x = (2.0 * pixel_x) / window_width - 1.0
        ndc_y = 1.0 - (2.0 * pixel_y) / window_height 
    {
        Do the transforms ...
    }
    Convert back:
    pixel_x = ((ndc_x + 1.0) * 0.5f) * window_width
        pixel_y = (1.0 - ndc_y) * window_height * 0.5f
        
        // Might be like this:
    
        Converting to Normalized device coordinates:
    ndc_x = (2.0f * pixel_x) / window_width - 1.0f
        ndc_y = (2.0f * pixel_y) / window_height - 1.0f
    {
        Do the transforms ...
    }
    Convert back:
    pixel_x = ((ndc_x + 1.0) * 0.5f) * window_width
        pixel_y = ((ndc_x + 1.0) * 0.5f) * window_height
#endif
    
    Vertex_Buffer* vert = &pl_g_state.g_pipeline.vert_buffer;
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
        entity_select(dimensions);
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
    if (g_render_collision)
    {
        Vertex_Buffer* vert2 = &pl_g_state.coll_g_pipeline.vert_buffer;
        Index_Buffer* idx = &pl_g_state.coll_g_pipeline.idx_buffer;
        get_head(vert2->data)->size = 0;
        get_head(idx->data)->size = 0;
        
#if 0
        Polygon2D* pols = pl_g_state.coll_shapes;
        V2 m_world_space = v2_sub(pl_g_state.mouse_pos, pl_g_state.cam.pos);
        if (c_e_g_state.point_selected)
        {
            pols[c_e_g_state.pol_index].pos =
                calculate_centroid(&pols[c_e_g_state.pol_index]);
            pols[c_e_g_state.pol_index].points[c_e_g_state.p_index] = m_world_space;
        }
        u32 size = size_arr(pols);
        
        V4* color = dyn_arrayT(frame_region, size, V4);
        
        static b8 clicked_lock = true;
        for_range(i, 0)
        {
            color[i] = v4i(0.0f);
            color[i].g = 1.0f;
            color[i].a = 1.0f;
            
            Polygon2D* pol = &pols[i];
            for_range(j, pol->n_sides)
            {
                if (point_in_point_d(m_world_space, pol->points[j]))
                {
                    if (is_any_button_clicked(&clicked_lock))
                    {
                        b_switch(c_e_g_state.point_selected);
                        if (c_e_g_state.point_selected)
                        {
                            c_e_g_state.p_index = j;
                            c_e_g_state.pol_index = i;
                            save_undo(&c_e_g_state.undo, pols, i);
                        }
                    }
                    break;
                }
            }
            if (!c_e_g_state.point_selected && clicked_lock &&
                point_SAT(m_world_space, pol))
            {
                color[i].b = 1.0f;
                static b8 lock = true;
                if (is_any_button_clicked(&lock))
                {
                    c_e_g_state.presist_offset = v2_sub(pol->pos, m_world_space);
                }
                if (is_any_button_pressed())
                {
                    if (!c_e_g_state.poly_selected)
                    {
                        c_e_g_state.pol_index = i;
                    }
                    c_e_g_state.poly_selected = true;
                }
                else
                {
#if 0 
                    // If the position should not be undo
                    if (c_e_g_state.poly_selected)
                    {
                        Poly_Undo* undo = &c_e_g_state.undo;
                        u32 undo_size = size_arr(undo->ids);
                        u32 point_iter = 0;
                        for_range(k, undo_size)
                        {
                            u32 index = undo->ids[k];
                            if (index == i)
                            {
                                for_range(d, pol->n_sides)
                                {
                                    V2 centroid_vec = v2_sub(
                                                             undo->points[point_iter], undo->pos[k]);
                                    
                                    undo->points[point_iter++] =
                                        v2_add(m_world_space, centroid_vec);
                                }
                                undo->pos[k] = m_world_space;
                            }
                            else
                            {
                                point_iter += pols[index].n_sides;
                            }
                        }
                    }
#endif
                    c_e_g_state.poly_selected = false;
                }
            }
            if (!is_poly2d_convex(pols[i]))
            {
                color[i].r = 1.0f;
                color[i].g = 0.0f;
                color[i].b = 0.0f;
            }
        }
        if (c_e_g_state.poly_selected)
        {
            move_polygon_offest(&pols[c_e_g_state.pol_index], m_world_space,
                                c_e_g_state.presist_offset);
        }
        
        if (is_key_pressed(SYNT_KEY_CTRL))
        {
            Events* evt = pl_g_state.key_evt;
            if (evt->activated && is_key_pressed(SYNT_KEY_Z))
            {
                Poly_Undo* undo = &c_e_g_state.undo;
                if (get_head(undo->ids)->size > 0)
                {
                    u32 index = synt_pop(undo->ids);
                    ASSERT(index <= size_arr(pl_g_state.coll_shapes),
                           "undo something that doesn't exist!");
                    Polygon2D* to_undo = &pl_g_state.coll_shapes[index];
                    for_range(points, to_undo->n_sides)
                    {
                        to_undo->points[points] = synt_pop(undo->points);
                    }
                    to_undo->pos = synt_pop(undo->pos);
                    to_undo->pos = calculate_centroid(to_undo);
                }
            }
        }
#if 0
        V2 displacement = pols[0].pos;
        if (polygon2D_lines_static(&pols[0], &pols[1], &displacement))
        {
            color[0].x = 0.0f;
            color[0].y = 0.0f;
            color[0].z = 1.0f;
            color[1].x = 0.0f;
            color[1].y = 0.0f;
            color[1].z = 1.0f;
        }
        
        // move_polygon(&pols[0], displacement);
#endif
        
        for_range(i, 0)
        {
            polygon2D_draw_lines(&vert2->data, &idx->data, pols[i], -0.5f, color[i],
                                 0.0f);
        }
        
        static b8 clicked_lock1 = true;
        if (is_key_clicked(&clicked_lock1, SYNT_KEY_V))
        {
            poly_save_to_file();
        }
        
#endif
        
        V3 test_pos = v3f(400.0f, 400.0f, -1.0f);
        V2 test_size = v2f(300.f, 200.0f);
        
        square_rounded_corners(&vert2->data, &idx->data, test_pos, test_size, v4i(1.0f),
                               g_dist_1, (u32)g_dist_, 0.0f);
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
    
    // test_collision(p_e, &r, pl_g_state.mouse_pos);
    
    V2 player_size = v2f(BLOCK_W, BLOCK_H);
    color_t = v4f(0.0f, 0.0f, 1.0f, 1.0f);
    color_b = v4f(0.0f, 1.0f, 0.0f, 1.0f);
    *p_rect = quad_gradiant_t_b(&t_storage, &num_rects, v3_v2f(p_e->pos, p_e->z),
                                player_size, color_t, color_b, 1.0f);
    p_e->size = p_rect->size;
    p_rect->vel = p_e->vel;
    
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
    u32 size_to_copy = 0;
    for_range(i, sort_size)
    {
        for_range(j, 4)
        {
            u32 index = (i * 4) + j;
            ASSERT(index < data_size, "");
            vert->data[g_level_size + index] = t_storage[z_sort[i].index + j];
            size_to_copy++;
        }
    }
    for_range(i, g_level_size)
    {
        size_to_copy++;
        vert->data[i] = t_storage[i];
    }
    
#if 1
    u32 size_bytes = size_to_copy * sizeof(Vertex);
    copy_data_buffer(&vert->buffer, vert->data, size_bytes);
    
    Vertex_Buffer* vb = &pl_g_state.coll_g_pipeline.vert_buffer;
    size_bytes = size_arr(vb->data) * sizeof(Vertex);
    copy_data_buffer(&vb->buffer, vb->data, size_bytes);
    
    Index_Buffer* ib = &pl_g_state.coll_g_pipeline.idx_buffer;
    size_bytes = size_arr(ib->data) * sizeof(u32);
    copy_data_buffer(&ib->buffer, ib->data, size_bytes);
#endif
    
    draw_pipeline(render_platform_game, NULL);
    
    if (edit_mode || play_edit_mode)
    {
#if 1
        gui_update_begin(region, dimensions, semaphore_idx, dt, translucentcy);
        {
            update_gui(region,app_state, dt, dimensions);
        }
        gui_update_end();
#endif
    }
}


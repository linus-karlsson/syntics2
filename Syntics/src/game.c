#include "game.h"
#include "logging.h"
#include "region_alloc.h"
#include "font.h"
#include "camera.h"
#include "buffers.h"
#include "swap_chain.h"
#include "gui.h"
#include "event_system.h"
#include "file_reading.h"
#include "vulkan_types.h"
#include "obj_load.h"
#include "noise.h"
#include "render_util.h"
#include "random.h"
#include <math.h>

global const char* OBJ_PATH = "Syntics/res/kiha32/kiha32.obj";

typedef struct Render_Test_State
{
    Graphic_Pipline main_g_pipeline;
    Graphic_Pipline figur_g_pipeline;

    Camera_3D cam;
    Camera_3D figur_cam;

    Texture* textures;
    Font font;
    Events* mouse_evt;

} Render_Test_State;

typedef struct Cube
{
    Vertex verts[8];
} Cube;

global Render_Test_State test;

#define DEFAULT_TEXTURE 0
#define OBJ_TEXTURE 1

internal void load_vertices_indices(Region_Alloc* region,
                                    Graphic_Pipline* graphic_pipline)
{
#if 0
    tinyobj_attrib_t attrib;
    tinyobj_shape_t shapes;
    tinyobj_material_t materials;
    size_t num_shapes;
    size_t num_materials;

    tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials, &num_materials,
                      &warn, &err, OBJ_PATH)

        uint32_t sum = 0;
    for (const auto& shape : shapes)
        sum += (uint32_t)shape.mesh.indices.size();

    graphic_pipline->vert_buffer.data = dyn_array(region, sum, Vertex, TEMP_ARRAY);
    graphic_pipline->idx_buffer.data = dyn_array(region, sum, uint32, TEMP_ARRAY);

    u32idx = 0;
    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            synt::Vertex vertex = {};

            vertex.pos = { attrib.vertices[3 * index.vertex_index + 0],
                           attrib.vertices[3 * index.vertex_index + 1],
                           attrib.vertices[3 * index.vertex_index + 2], 1.0f };

            vertex.tex_coords = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
            };

            vertex.color = { 1.0f, 1.0f, 1.0f, 1.0f };

            vertex.tex_index = 0.0f;

            synt_push(graphic_pipline->vert_buffer.data, vertex);
            synt_push(graphic_pipline->idx_buffer.data, idx++);
        }
    }

#else
    // TODO: fix small glitches.
    Obj_Load_Attrib loader;

    load_model(&loader, OBJ_PATH);

    u32 size = size_arr(loader.indices);

    graphic_pipline->vert_buffer.data = dyn_arrayP(region, size * 3, Vertex);
    graphic_pipline->idx_buffer.data = dyn_arrayP(region, size * 3, u32);

    u32 idx = 0;
    for (u32 i = 0; i < size; i++)
    {
        for (u32 j = 0; j < 3; j++)
        {
            Vertex vertex = { 0 };

            vertex.pos = loader.verts[loader.indices[i].vertex_index[j]];

            vertex.color = v4f(1.0f, 1.0f, 1.0f, 1.0f);

            vertex.tex_coords.x =
                loader.tex_coords[loader.indices[i].texture_index[j]].x;
            vertex.tex_coords.y =
                1.0f - loader.tex_coords[loader.indices[i].texture_index[j]].y;

            // printf("(x: %f, y: %f, z: %f)\n", vertex.pos.x, vertex.pos.y,
            // vertex.pos.z);

            vertex.tex_index = OBJ_TEXTURE;

            synt_push(graphic_pipline->vert_buffer.data, vertex);
            synt_push(graphic_pipline->idx_buffer.data, idx++);
        }
    }
#endif
}

#if 1
#define CHUNK_SIZE_X 100
#define CHUNK_SIZE_Y 1
#define CHUNK_SIZE_Z 100

#define CHUNK_SIZE CHUNK_SIZE_X* CHUNK_SIZE_Y* CHUNK_SIZE_Z

global const f32 QUAD_WIDTH = 0.5f;
global const f32 QUAD_DEPTH = 0.5f;
global const f32 OFFSET_INCREASE = 0.1f;

global f32 freq = 0.41f;
global f32 grain = 0.36f;
global f32 oct = 3.0f;
global f32 max_height = 8.0f;

f32 round_down_to_half(f32 value)
{
    return value - fmodf(value, 0.5f);
}

internal void generate_terrain(f32 x_off, f32 z_off)
{
    Vertex_Buffer* vert = &test.main_g_pipeline.vert_buffer;
    for_range(z, CHUNK_SIZE_Z)
    {
        f32 ix_off = x_off;
        for_range(x, CHUNK_SIZE_X)
        {
#if 0
            f32 y_noise = (sy_value_noise2d(ix_off, z_off, freq, grain, (i32)oct) *
                           max_height);
            y_noise = round_down_to_half(y_noise);
            for_range(y, CHUNK_SIZE_Y)
            {
                cube(&vert->data,
                     v3f(0.0f + (0.5f * x), y_noise + (0.5f * y), 0.0f + (0.5f * z)),
                     v3i(0.5f), v4i(1.0f), DEFAULT_TEXTURE);
            }
#else
            f32 y_noise = (sy_value_noise2d(ix_off, z_off, freq, grain, (i32)oct) *
                           max_height);

            V3 pos = v3f(x * QUAD_WIDTH, y_noise, z * QUAD_DEPTH);
            // f32 colorf = y_noise / max_height;
            V4 color = v4i(1.0f);
            f32 tex_index = DEFAULT_TEXTURE;

            Vertex vertex = vertex_create(pos, v3f(0.0f, 1.0f, 0.0f),
                                          v2f(0.0f, 0.0f), color, tex_index);

            synt_push(vert->data, vertex);

#endif
            ix_off += 0.1f;
        }
        z_off += 0.1f;
    }
}
#endif

internal void generate_normal()
{
    Vertex_Buffer* vert = &test.main_g_pipeline.vert_buffer;
    u32 size = size_arr(vert->data);
    for (u32 i = 0; i < size - CHUNK_SIZE_X - 1; i += 1)
    {
        V3 pos = vert->data[i].pos;
        V3 next_pos0 = vert->data[i + CHUNK_SIZE_X].pos;
        V3 next_pos1 = vert->data[i + 1].pos;
        V3 side0 = v3_sub(next_pos0, pos);
        V3 side1 = v3_sub(next_pos1, pos);
        V3 normal = v3_normalize(v3_cross(side0, side1));
        vert->data[i].normal = normal;
    }
}

global V3 g_light_pos = { 0.0, 1.0, 0.0 };

internal void render_game(void* data, VkCommandBuffer command_buffer,
                          u32 semaphore_idx)
{
#if 1
    vkCmdPushConstants(command_buffer, test.main_g_pipeline.layout,
                       VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(V3), &g_light_pos);
#endif
#if 1

    Index_Buffer* idx = &test.main_g_pipeline.idx_buffer;
    bind_and_draw_graphics_pipline(
        command_buffer, test.main_g_pipeline.descriptors.desc_sets[semaphore_idx], 0,
        idx->curr_size, &test.main_g_pipeline);
#endif

    Index_Buffer* idx2 = &test.figur_g_pipeline.idx_buffer;
    bind_and_draw_graphics_pipline(
        command_buffer, test.figur_g_pipeline.descriptors.desc_sets[semaphore_idx],
        0, idx2->curr_size, &test.figur_g_pipeline);
}

internal void recreate_game(void* data, Region_Alloc* region,
                            const Application_State* app_state)
{
    recreate_graphic_pipline_ap(region, app_state, "Syntics/res/game.vert.spv",
                                "Syntics/res/game.frag.spv", &test.main_g_pipeline,
                                size_arr(test.textures), NULL);
    gui_recreate(region);
}

internal void destroy_game(void* data, VkDevice device, u32 num_semaphores)
{
    destroy_graphic_pipeline(device, num_semaphores, &test.main_g_pipeline);
    destroy_graphic_pipeline(device, num_semaphores, &test.figur_g_pipeline);

    for (u32 i = 0; i < size_arr(test.textures); i++)
    {
        destroy_texture(device, test.textures[i]);
    }

    destroy_gui(device, num_semaphores);
}

void init_game(Region_Alloc* region, VkDevice device,
               VkPhysicalDevice physical_device, VkCommandPool command_pool,
               VkQueue graphic_queue, const Swap_Chain_attrib* swap_chain,
               u32 num_semaphores)
{

    const char* paths[] = {
        [DEFAULT_TEXTURE] = "Syntics/res/default.png",
        [OBJ_TEXTURE] = "Syntics/res/kiha32/1591184735691.png",
    };
    u32 num_text = sy_SIZE(paths);
    test.textures = dyn_arrayP(region, num_text, Texture);

    for_range(i, num_text)
    {
        create_texture_path(device, physical_device, command_pool, graphic_queue,
                            true, VK_FORMAT_R8G8B8A8_SRGB, paths[i],
                            &test.textures[i]);
    }
    get_head(test.textures)->size = num_text;

    Graphic_Pipline* g_p = &test.main_g_pipeline;
    g_p->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    g_p->cull_mode = VK_CULL_MODE_NONE;
    g_p->poly_mode = VK_POLYGON_MODE_FILL;
    create_graphics_pipeline(device, swap_chain->render_pass,
                             swap_chain->sample_count, "Syntics/res/game.vert.spv",
                             "Syntics/res/game.frag.spv",
                             swap_chain->extent_2D.width,
                             swap_chain->extent_2D.height, num_text, NULL, g_p);

#if 1
    u32 size = 8 * CHUNK_SIZE;
    init_graphics_pipeline(region, device, physical_device, size, num_semaphores,
                           test.textures, num_text, g_p);

    generate_terrain(0.0f, 0.0f);
    generate_normal();

    copy_data_buffer(&g_p->vert_buffer.buffer, g_p->vert_buffer.data,
                     g_p->vert_buffer.buffer.size_bytes);

    Index_Buffer* idx = &g_p->idx_buffer;
    idx->data = dyn_arrayP(region, 2 * CHUNK_SIZE, u32);

    int32 I = 0;
    int32 step_value = 1;
    for_range(i, CHUNK_SIZE_Z - 1)
    {
        for_range(j, CHUNK_SIZE_X)
        {
            synt_push(idx->data, (CHUNK_SIZE_X * i) + I);
            synt_push(idx->data, (CHUNK_SIZE_X * (i + 1)) + I);

            I += step_value;
        }
        step_value *= -1;
        I += step_value;
    }
#if 0
    g_p->idx_buffer.data = dyn_arrayP(region, 36 * CHUNK_SIZE, u32);
    cube_indices(&g_p->idx_buffer.data, CHUNK_SIZE);
#endif

    g_p->idx_buffer.buffer.size_bytes = size_arr(g_p->idx_buffer.data) * sizeof(u32);
    g_p->idx_buffer.curr_size = size_arr(g_p->idx_buffer.data);
    create_index_buffer_local(device, physical_device, command_pool, graphic_queue,
                              &g_p->idx_buffer);
#else
    load_vertices_indices(region, g_p);

    g_p->vert_buffer.buffer.size_bytes =
        size_arr(g_p->vert_buffer.data) * sizeof(Vertex);
    create_vertex_buffer_local(device, physical_device, command_pool, graphic_queue,
                               &g_p->vert_buffer);

    g_p->idx_buffer.buffer.size_bytes =
        size_arr(g_p->idx_buffer.data) * sizeof(uint32);
    g_p->idx_buffer.curr_size = size_arr(g_p->idx_buffer.data);
    create_index_buffer_local(device, physical_device, command_pool, graphic_queue,
                              &g_p->idx_buffer);

#endif
    Graphic_Pipline* f_g_p = &test.figur_g_pipeline;
    f_g_p->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    f_g_p->cull_mode = VK_CULL_MODE_NONE;
    f_g_p->poly_mode = VK_POLYGON_MODE_FILL;
    create_graphics_pipeline(device, swap_chain->render_pass,
                             swap_chain->sample_count, "Syntics/res/game.vert.spv",
                             "Syntics/res/game.frag.spv",
                             swap_chain->extent_2D.width,
                             swap_chain->extent_2D.height, num_text, NULL, f_g_p);

    init_gp(region, device, physical_device, num_semaphores,
                           test.textures, size_arr(test.textures), f_g_p);

    load_vertices_indices(region, f_g_p);

    f_g_p->vert_buffer.buffer.size_bytes =
        size_arr(f_g_p->vert_buffer.data) * sizeof(Vertex);
    create_vertex_buffer_local(device, physical_device, command_pool, graphic_queue,
                               &f_g_p->vert_buffer);

    f_g_p->idx_buffer.buffer.size_bytes =
        size_arr(f_g_p->idx_buffer.data) * sizeof(uint32);
    f_g_p->idx_buffer.curr_size = size_arr(f_g_p->idx_buffer.data);
    create_index_buffer_local(device, physical_device, command_pool, graphic_queue,
                              &f_g_p->idx_buffer);

    test.cam = cam_3di(4.0f, 5.0f);
    test.figur_cam = cam_3di(2000.0f, 5.0f);

    subscribe(&test.mouse_evt, EVT_MOUSE);

    subscribe_recreate_callback(recreate_game, NULL);
    subscribe_destroy_callback(destroy_game, NULL);

    gui_init(region, device, physical_device, command_pool, graphic_queue,
             swap_chain, num_semaphores, true);
}

global f32 translucentcy = 0.8f;
global f32 testing = 0.1f;
global b32 wire_frame = false;

global V3 scaling_value = { 1.0f, 1.0f, 1.0f };

internal void update_gui(Region_Alloc* region, const Application_State* app_state,
                         f32 dt, V2 dimensions)
{
    back_bord_begin("First thing", v2f(10.0f, 10.0f));
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
            if (add_button("Wire Frame"))
            {
                if (!wire_frame)
                {
                    test.main_g_pipeline.poly_mode = VK_POLYGON_MODE_LINE;
                }
                else
                {
                    test.main_g_pipeline.poly_mode = VK_POLYGON_MODE_FILL;
                }
                b_switch(wire_frame);
                recreate_game(NULL, region, app_state);
            }
        }
        gridd_end();
        gridd_begin(1, 1);
        {
            add_text("Position (x, y, z) This is a test");
        }
        gridd_end();

        gridd_begin(3, 1);
        {
            add_input_float(&scaling_value.x, -100.0f, 100.0f, 3.0f);
            add_input_float(&scaling_value.y, -100.0f, 100.0f, 3.0f);
            add_input_float(&scaling_value.z, -100.0f, 100.0f, 3.0f);
        }
        gridd_end();

        gridd_begin(1, 1);
        {
            add_text("Freq --- Grain --- Oct --- Max Height");
        }
        gridd_end();

        gridd_begin(4, 1);
        {
            add_input_float(&freq, 0.0f, 10.0f, 1.0f);
            add_input_float(&grain, 0.0f, 10.0f, 1.0f);
            add_input_float(&oct, 0.0f, 10.0f, 1.0f);
            add_input_float(&max_height, 0.0f, 20.0f, 2.0f);
        }
        gridd_end();

        gridd_begin(1, 1);
        {
            presist char temp[60] = { 0 };
            presist f32 count = 1.0f;
            if (count >= 0.1f)
            {
                f32 milli = dt * 1000.0f;
                sprintf_s(temp, sizeof(temp), "Milli: %f | FPS: %u", milli,
                          app_state->fps);
                count = 0.0f;
            }
            count += dt;
            add_text(temp);
        }
        gridd_end();
        gridd_begin(1, 1);
        {
            presist char temp[200] = { 0 };
            presist f32 count = 1.0f;
            if (count >= 0.1f)
            {
                sprintf_s(
                    temp, sizeof(temp),
                    "|%f,%f,%f,%f|\n|%f,%f,%f,%f|\n|%f,%f,%f,%f|\n|%f,%f,%f,%f|\n\n",
                    test.figur_cam.mvp.model.data[0][0],
                    test.figur_cam.mvp.model.data[1][0],
                    test.figur_cam.mvp.model.data[2][0],
                    test.figur_cam.mvp.model.data[3][0],
                    test.figur_cam.mvp.model.data[0][1],
                    test.figur_cam.mvp.model.data[1][1],
                    test.figur_cam.mvp.model.data[2][1],
                    test.figur_cam.mvp.model.data[3][1],
                    test.figur_cam.mvp.model.data[0][2],
                    test.figur_cam.mvp.model.data[1][2],
                    test.figur_cam.mvp.model.data[2][2],
                    test.figur_cam.mvp.model.data[3][2],
                    test.figur_cam.mvp.model.data[0][3],
                    test.figur_cam.mvp.model.data[1][3],
                    test.figur_cam.mvp.model.data[2][3],
                    test.figur_cam.mvp.model.data[3][3]);
                count = 0.0f;
            }
            count += dt;
            add_text(temp);
        }
        gridd_end();
    }
    back_bord_end();

    back_bord_begin("Terminal", v2f(500.0f, 100.0f));
    {
        add_terminal(250.0f, 200.0f);
    }
    back_bord_end();
}

#define sample_count 1000

internal b8 record(f32 dt)
{
    presist b8 p_pressed = false;
    presist b8 q_pressed = false;
    presist u32 count_rec = 0;
    presist u32 count_play = 0;
    presist M4 rec[sample_count] = { 0 };
    presist f32 sec = 0.0f;
    presist const f32 sample_time = MILLISECONDS(15.0f);

    presist b8 q_clicked = false;
    if (q_clicked)
    {
        if (!q_pressed)
        {
            count_rec = 0;
            q_pressed = true;
        }
        else
        {
            synt_LOG_Term("Stop Rec\n");
            q_pressed = false;
        }
    }
    presist b8 first_clicked = true;
    q_clicked = is_key_clicked(&first_clicked, SYNT_KEY_Q);
    if (q_pressed && !p_pressed)
    {
        sec += dt;
        if (sec >= sample_time)
        {
            if (count_rec < sample_count)
            {
                synt_LOG_Term("Rec: %u / %u\n", count_rec + 1, sample_count);
                rec[count_rec++] = test.cam.mvp.view;
            }
            else
            {
                q_clicked = true;
            }
            sec = 0.0f;
        }
    }
    presist b8 first_clicked1 = true;
    if (is_key_clicked(&first_clicked1, SYNT_KEY_P))
    {
        if (!p_pressed)
        {
            synt_LOG_Term("Start Playing\n");
            p_pressed = true;
        }
        else
        {
            synt_LOG_Term("Stop Playing\n");
            p_pressed = false;
        }
        count_play = 0;
    }
    if (p_pressed)
    {
        sec += dt;
        if (sec >= sample_time)
        {
            test.cam.mvp.view = rec[count_play++];
            count_play %= count_rec;
            sec = 0.0f;
        }
    }
    return p_pressed;
}

void update_game(Region_Alloc* region, const Application_State* app_state,
                 VkDevice device, V2 dimensions, u32 semaphore_idx, f32 dt)
{
    presist b8 off_the_ground = true;

    if (!gui_focus())
    {
        update_camera(&test.cam, test.mouse_evt, dt, off_the_ground);
    }

    if (!record(dt))
    {
        test.cam.mvp.view =
            view(test.cam.pos, v3_add(test.cam.pos, test.cam.ori), test.cam.up);
    }

    presist f32 rotation = 45.0f;

    test.cam.mvp.proj =
        perspective(radians(rotation), dimensions.x / dimensions.y, 0.1f, 100.0f);

    test.figur_cam.mvp.proj =
        perspective(radians(rotation), dimensions.x / dimensions.y, 0.1f, 100.0f);

    if (is_key_pressed(SYNT_KEY_G))
    {
        rotation += 200.0f * dt;
    }
    if (is_key_pressed(SYNT_KEY_F))
    {
        rotation -= 200.0f * dt;
    }

    presist b8 gravity = false;
    presist b8 first_clicked = true;
    if (is_key_clicked(&first_clicked, SYNT_KEY_T))
    {
        b_switch(gravity);
    }

#if 0
cube_increment = 0.5
offset_increment = 0.1

def calculate_noise_height(x, z):
    x_offset = x / cube_increment * offset_increment
    z_offset = z / cube_increment * offset_increment

#Pass the modified offsets to your noise function
    noise_height = noise_function(x_offset, z_offset)

    return noise_height
#endif
#if 0
    V3 x_z = v3f((test.cam.pos.x * OFFSET_INCREASE) / QUAD_WIDTH, 0.0f,
                 (test.cam.pos.z * OFFSET_INCREASE) / QUAD_DEPTH);

    x_z.y = (sy_value_noise2d(x_z.x, x_z.z, freq, grain, (i32)oct) * max_height);

    presist f32 sec_off_ground = 0.0f;

    f32 extra_padding = 0.5f;
    if (test.cam.pos.y <= x_z.y + extra_padding)
    {
        test.cam.pos.y = x_z.y + extra_padding;

        V3 first_point = v3f(test.cam.pos.x + 0.1f, 0.0f, test.cam.pos.z);
        V3 x_z0 = v3f((first_point.x * OFFSET_INCREASE) / QUAD_WIDTH, 0.0f,
                      (first_point.z * OFFSET_INCREASE) / QUAD_DEPTH);
        first_point.y =
            (sy_value_noise2d(x_z0.x, x_z0.z, freq, grain, (i32)oct) * max_height) +
            extra_padding;

        V3 second_point = v3f(test.cam.pos.x, 0.0f, test.cam.pos.z + 0.1f);
        V3 x_z1 = v3f((second_point.x * OFFSET_INCREASE) / QUAD_WIDTH, 0.0f,
                      (second_point.z * OFFSET_INCREASE) / QUAD_DEPTH);
        second_point.y =
            (sy_value_noise2d(x_z1.x, x_z1.z, freq, grain, (i32)oct) * max_height) +
            extra_padding;

        V3 side0 = v3_sub(first_point, test.cam.pos);
        V3 side1 = v3_sub(second_point, test.cam.pos);
        V3 normal = v3_normalize(v3_cross(side1, side0));

        f32 angle = v3_angle(test.cam.vel, normal);

        test.cam.vel.x -= test.cam.vel.x * (5.0f * angle * dt);
        test.cam.vel.z -= test.cam.vel.z * (5.0f * angle * dt);

        // PRINT_V3(test.cam.vel);

        sec_off_ground = 0.0f;
        off_the_ground = false;
    }
    else
    {
        sec_off_ground += dt;
    }
    if (sec_off_ground >= 0.1f)
    {
        off_the_ground = true;
        test.cam.vel.x -= 5.0f * test.cam.vel.x * dt;
        test.cam.vel.z -= 5.0f * test.cam.vel.z * dt;
    }
#endif

#if 0
    Vertex_Buffer* vert = &test.g_pipeline.vert_buffer;
    get_head(vert->data)->size = 0;
    generate_terrain(x_off, z_off);
    copy_data_buffer(&vert->buffer, vert->data, vert->buffer.size_bytes);
#endif

#if 1
    copy_data_buffer(&test.main_g_pipeline.uniform_buffers[semaphore_idx].buffer,
                     &test.cam.mvp, sizeof(test.cam.mvp));

    if (is_key_pressed(SYNT_KEY_LEFT))
    {
        test.figur_cam.pos.x -= 3.0f * dt;
    }
    if (is_key_pressed(SYNT_KEY_UP))
    {
        test.figur_cam.pos.z -= 3.0f * dt;
    }
    if (is_key_pressed(SYNT_KEY_RIGHT))
    {
        test.figur_cam.pos.x += 3.0f * dt;
    }
    if (is_key_pressed(SYNT_KEY_DOWN))
    {
        test.figur_cam.pos.z += 3.0f * dt;
    }

#if 0
    test.figur_cam.mvp.view =
        view(test.figur_cam.pos, v3_add(test.figur_cam.pos, test.figur_cam.ori),
             test.figur_cam.up);
#endif

    MVP final_mvp = test.figur_cam.mvp;
    final_mvp.view = test.cam.mvp.view; 
    final_mvp.model = m4_scale(m4i(1.0f),scaling_value);

    copy_data_buffer(&test.figur_g_pipeline.uniform_buffers[semaphore_idx].buffer,
                     &final_mvp, sizeof(final_mvp));
#endif

    draw_pipeline(render_game, NULL);

    gui_update_begin(region, dimensions, semaphore_idx, dt, translucentcy);
    {
        update_gui(region, app_state, dt, dimensions);
    }
    gui_update_end();
}

#if 0
void render_render_testing(VkCommandBuffer command_buffer, u32semaphore_idx)
{
    bind_and_draw_graphics_pipline(
        command_buffer, test.g_pipeline.descriptors.desc_sets[semaphore_idx],
        test.g_pipeline);
}
#endif


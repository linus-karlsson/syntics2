#include "render_testing.h"
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

#define MAIN_PIPELINE 0
#define UI_PIPELINE 1

static const char* OBJ_PATH = "Syntics/res/kiha32/kiha32.obj";
static const char* PNG_PATH = "Syntics/res/kiha32/1591184735691.png";

typedef struct Render_Test_State
{
    Graphic_Pipline g_pipeline;

    Camera_3D cam;

    Texture* textures;
    Font font;
    Events* mouse_evt;

} Render_Test_State;

static Render_Test_State test;

#if 0
#include <tiny-obj/tiny_obj_loader.h>

static void load_vertices_indices(Region_Alloc* region,
                                  Graphic_Pipline* graphic_pipline)
{
#if 1
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, OBJ_PATH))
        synt::ERROR((warn + err).c_str());

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

#endif

    // TODO: fix small glitches.
#if 0
    Obj_Load_Attrib loader;

    loader.load_model(OBJ_PATH);

    u32size = size_arr(loader.indices);

    Temp_Alloc<Vertex> vertex_buffer(region, size * 3);
    Temp_Alloc<uint32> index_buffer(region, size * 3);

    u32idx = 0;
    for (uint32_t i = 0; i < size; i++)
    {
        for (uint32_t j = 0; j < 3; j++)
        {
            Vertex vertex = {};

            vertex.pos = loader.verts[loader.indices[i].vertex_index[j]];

            vertex.color = { 1.0f, 1.0f, 1.0f, 1.0f };

            vertex.tex_coords.x =
                loader.tex_coords[loader.indices[i].texture_index[j]].x;
            vertex.tex_coords.y =
                1.0f - loader.tex_coords[loader.indices[i].texture_index[j]].y;

            // printf("(x: %f, y: %f, z: %f)\n", vertex.pos.x, vertex.pos.y,
            // vertex.pos.z);

            vertex.tex_index = 0.0f;

            vertex_buffer.push_back(vertex);
            index_buffer.push_back(idx++);
        }
    }

    graphic_pipline->vert_buffer.data = vertex_buffer.data;
    graphic_pipline->idx_buffer.data  = index_buffer.data;

    graphic_pipline->vert_buffer.size_bytes = vertex_buffer.size() * sizeof(Vertex);
    create_vertex_buffer(device, phy_device, com_pool, graphic_queue,
                         &graphic_pipline->vert_buffer);

    graphic_pipline->idx_buffer.size_bytes = index_buffer.size() * sizeof(uint32);
    graphic_pipline->idx_buffer.curr_size  = index_buffer.size();
    create_index_buffer(device, phy_device, com_pool, graphic_queue,
                        &graphic_pipline->idx_buffer);

#endif
}
#endif

internal void render_game(void* data, VkCommandBuffer command_buffer,
                          u32 semaphore_idx)
{
    vkCmdPushConstants(command_buffer, test.g_pipeline.layout,
                       VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MVP), &test.cam.mvp);

    Index_Buffer* idx = &test.g_pipeline.idx_buffer;
    bind_and_draw_graphics_pipline(
        command_buffer, test.g_pipeline.descriptors.desc_sets[semaphore_idx], 0,
        idx->curr_size, &test.g_pipeline);
}

internal void recreate_game(void* data, Region_Alloc* region,
                            const Application_State* app_state)
{
    recreate_graphic_pipline_ap(region, app_state, "Syntics/res/game.vert.spv",
                                "Syntics/res/game.frag.spv", &test.g_pipeline,
                                size_arr(test.textures), NULL);
    gui_recreate(region);
}

internal void destroy_game(void* data, VkDevice device, u32 num_semaphores)
{
    destroy_graphic_pipeline(device, 0, &test.g_pipeline);

    for (u32 i = 0; i < size_arr(test.textures); i++)
    {
        destroy_texture(device, test.textures[i]);
    }

    destroy_gui(device, num_semaphores);
}

#define DEFAULT_TEXTURE 0

void init_game(Region_Alloc* region, VkDevice device,
               VkPhysicalDevice physical_device, VkCommandPool command_pool,
               VkQueue graphic_queue, const Swap_Chain_attrib* swap_chain,
               u32 num_semaphores)
{

    const char* paths[] = {
        [DEFAULT_TEXTURE] = "Syntics/res/default.png",
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

    Graphic_Pipline* g_p = &test.g_pipeline;
    g_p->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    g_p->cull_mode = VK_CULL_MODE_NONE;
    g_p->poly_mode = VK_POLYGON_MODE_FILL;
    create_graphics_pipeline(device, swap_chain->render_pass,
                             swap_chain->sample_count, "Syntics/res/game.vert.spv",
                             "Syntics/res/game.frag.spv",
                             swap_chain->extent_2D.width,
                             swap_chain->extent_2D.height, num_text, NULL, g_p);

    Vertex verts[8] = {
        { { 0.5f, 0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }, DEFAULT_TEXTURE },
        { { 0.5f, 0.5f - 0.5f, -0.5f },
          { 1.0f, 0.0f, 0.0f, 1.0f },
          { 0.0f, 1.0f },
          DEFAULT_TEXTURE },
        { { 0.5f + 0.5f, 0.5f - 0.5f, -0.5f },
          { 1.0f, 0.0f, 0.0f, 1.0f },
          { 1.0f, 1.0f },
          DEFAULT_TEXTURE },
        { { 0.5f + 0.5f, 0.5f, -0.5f },
          { 1.0f, 0.0f, 0.0f, 1.0f },
          { 1.0f, 0.0f },
          DEFAULT_TEXTURE },
        { { 0.5f, 0.5f, -1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }, DEFAULT_TEXTURE },
        { { 0.5f, 0.5f - 0.5f, -1.0f },
          { 0.0f, 1.0f, 0.0f, 1.0f },
          { 0.0f, 1.0f },
          DEFAULT_TEXTURE },
        { { 0.5f + 0.5f, 0.5f - 0.5f, -1.0f },
          { 0.0f, 1.0f, 0.0f, 1.0f },
          { 1.0f, 1.0f },
          DEFAULT_TEXTURE },
        { { 0.5f + 0.5f, 0.5f, -1.0f },
          { 0.0f, 1.0f, 0.0f, 1.0f },
          { 1.0f, 0.0f },
          DEFAULT_TEXTURE }
    };
    init_graphics_pipeline(region, device, physical_device, sy_SIZE(verts),
                           num_semaphores, test.textures, num_text, g_p);

    u32 size = sy_SIZE(verts);
    for (u32 i = 0; i < size; i++)
    {
        synt_push(g_p->vert_buffer.data, verts[i]);
    }
    u32 size_bytes = size * sizeof(Vertex);
    copy_data_buffer(&g_p->vert_buffer.buffer, g_p->vert_buffer.data, size_bytes);

    u32 idnc[] = { 0, 1, 2, 2, 3, 0, 3, 2, 6, 6, 7, 3, 7, 6, 5, 5, 4, 7,
                   4, 5, 1, 1, 0, 4, 4, 0, 3, 3, 7, 4, 1, 5, 6, 6, 2, 1 };

    g_p->idx_buffer.data = dyn_arrayP(region, sy_SIZE(idnc), u32);

    size = sy_SIZE(idnc);
    for (u32 i = 0; i < size; i++)
    {
        synt_push(g_p->idx_buffer.data, idnc[i]);
    }
    g_p->idx_buffer.buffer.size_bytes = size_arr(g_p->idx_buffer.data) * sizeof(u32);
    g_p->idx_buffer.curr_size = size;
    create_index_buffer_local(device, physical_device, command_pool, graphic_queue,
                              &g_p->idx_buffer);

#if 0
    g_p->uniform_buffers = region_mallocP(region, num_semaphores, Uniform_Buffer);
    g_p->descriptors.desc_sets =
        region_mallocP(region, num_semaphores, VkDescriptorSet);

    for_range(i, num_semaphores)
    {
        g_p->uniform_buffers[i].size_bytes = (uint32)sizeof(MVP);

        create_uniform_buffer(device, physical_device, &g_p->uniform_buffers[i]);
    }

    create_descriptors(region, device, &g_p->descriptors, num_semaphores,
                       g_p->set_layout, test.textures, size_arr(test.textures),
                       g_p->uniform_buffers);
#endif

    test.cam = cam_3dd();

    subscribe(&test.mouse_evt, EVT_MOUSE);

    subscribe_recreate_callback(recreate_game, NULL);
    subscribe_destroy_callback(destroy_game, NULL);

    gui_init(region, device, physical_device, command_pool, graphic_queue,
             swap_chain, num_semaphores, true);
}

global f32 translucentcy = 1.0f;
global f32 testing = 0.1f;

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
            add_text("Position (x, y, z) This is a test");
        }
        gridd_end();

        gridd_begin(3, 1);
        {
            add_input_float(&test.cam.pos.x, -100.0f, 100.0f, 3.0f);
            add_input_float(&test.cam.pos.y, -100.0f, 100.0f, 3.0f);
            add_input_float(&test.cam.pos.z, -100.0f, 100.0f, 3.0f);
        }
        gridd_end();

        gridd_begin(1, 1);
        {
            static char temp[60] = { 0 };
            static f32 count = 1.0f;
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
    }
    back_bord_end();

    back_bord_begin("Second thing", v2f(800.0f, 10.0f));
    {
        gridd_begin(1, 1);
        {
            add_input_float(&testing, -10.0f, 10.0f, 3.0f);
        }
        gridd_end();
    }
    back_bord_end();
    back_bord_begin("Terminal", v2f(500.0f, 100.0f));
    {
        add_terminal(250.0f, 200.0f);
    }
    back_bord_end();
#if 0
        back_bord_begin("Graph", v2f(800.0f, 100.0f));
        {
            add_graph(dt * 1000.0f, "Milliseconds per frame", 20.0f, 10.0f, 5.0f,
                      dt);
        }
        back_bord_end();
#endif
}

void update_game(Region_Alloc* region, const Application_State* app_state,
                 VkDevice device, V2 dimensions, u32 semaphore_idx, f32 dt)
{
    if (!gui_focus())
    {
        update_camera(&test.cam, test.mouse_evt, dt);
    }

    test.cam.mvp.view =
        view(test.cam.pos, v3_add(test.cam.pos, test.cam.ori), test.cam.up);
    test.cam.mvp.proj =
        perspective(radians(53.0f), dimensions.x / dimensions.y, -0.9f, 100.0f);

#if 0
    update_uniform_buffers(device, test.g_pipeline.uniform_buffers[semaphore_idx],
                           &test.cam.mvp, sizeof(test.cam.mvp));
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


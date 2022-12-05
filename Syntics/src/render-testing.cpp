#include "render-testing.h"
#include "logging.h"
#include "region_alloc.h"
#include "font.h"
#include "camera.h"
#include "buffers.h"
#include "swap_chain.h"
#include "gui.h"
#include "event_system.h"
#include <tiny-obj/tiny_obj_loader.h>

#define MAIN_PIPELINE 0
#define UI_PIPELINE 1

static const char* OBJ_PATH = "Syntics/res/kiha32/kiha32.obj";
static const char* PNG_PATH = "Syntics/res/kiha32/1591184735691.png";

namespace synt {

typedef struct Render_Test_State
{
    Graphic_Pipline g_pipline;

    Camera cam;

    Texture* textures;
    Font font;
    Events* mouse_evt;

} Render_Test_State;

static Render_Test_State test;

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

    graphic_pipline->vert_buffer.data =
        dyn_array((*region), sum, Vertex, TEMP_ARRAY);
    graphic_pipline->idx_buffer.data = dyn_array((*region), sum, uint32, TEMP_ARRAY);

    uint32 idx = 0;
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

    uint32 size = size_arr(loader.indices);

    Temp_Alloc<Vertex> vertex_buffer(region, size * 3);
    Temp_Alloc<uint32> index_buffer(region, size * 3);

    uint32 idx = 0;
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

void init_render_testing(Region_Alloc* region, VkDevice device,
                         VkPhysicalDevice physical_device,
                         VkCommandPool command_pool, VkQueue graphic_queue,
                         const Swap_Chain_attrib& swap_chain, uint32 num_semaphores)
{

    test.textures = dyn_arrayP((*region), 2, Texture);

    create_texture(device, physical_device, command_pool, graphic_queue, true,
                   VK_FORMAT_R8G8B8A8_SRGB, PNG_PATH, &test.textures[0]);

    get_head(test.textures)->size++;

    create_texture(device, physical_device, command_pool, graphic_queue, true,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/default.png",
                   &test.textures[1]);

    get_head(test.textures)->size++;

    test.g_pipline.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    create_graphics_pipeline(
        region, device, swap_chain.color_format, swap_chain.render_pass,
        swap_chain.sample_count, "Syntics/res/vert.spv", "Syntics/res/frag.spv",
        swap_chain.extent_2D.width, swap_chain.extent_2D.height,
        VK_CULL_MODE_BACK_BIT, size_arr(test.textures), &test.g_pipline);

    load_vertices_indices(region, &test.g_pipline);

    test.cam.position    = synt::v3f(-7.0f, 6.0f, 11.0f);
    test.cam.orientation = synt::v3f(0.5f, -0.5f, -1.0f);

#if 1
    test.g_pipline.vert_buffer.size_bytes =
        size_arr(test.g_pipline.vert_buffer.data) * sizeof(Vertex);
    create_vertex_buffer(device, physical_device, command_pool, graphic_queue,
                         &test.g_pipline.vert_buffer);

    test.g_pipline.idx_buffer.size_bytes =
        size_arr(test.g_pipline.idx_buffer.data) * sizeof(uint32);
    test.g_pipline.idx_buffer.curr_size = size_arr(test.g_pipline.idx_buffer.data);
    create_index_buffer(device, physical_device, command_pool, graphic_queue,
                        &test.g_pipline.idx_buffer);

    region_pop((*region), capacity_arr(test.g_pipline.idx_buffer.data), uint32,
               TEMP_ARRAY);
    region_pop((*region), capacity_arr(test.g_pipline.vert_buffer.data), Vertex,
               TEMP_ARRAY);
#endif
    test.g_pipline.uniform_buffers =
        region_mallocP((*region), num_semaphores, Uniform_Buffer);
    test.g_pipline.descriptors.desc_sets =
        region_mallocP((*region), num_semaphores, VkDescriptorSet);

    for_range(i, num_semaphores)
    {
        test.g_pipline.uniform_buffers[i].size_bytes = (uint32)sizeof(MVP);

        create_uniform_buffer(device, physical_device,
                              &test.g_pipline.uniform_buffers[i]);
    }

#if 1
    create_descriptors(region, device, &test.g_pipline.descriptors, num_semaphores,
                       test.g_pipline.set_layout, test.textures,
                       size_arr(test.textures), test.g_pipline.uniform_buffers);

    test.cam.speed = 2.0f;

    test.cam.mvp.model = scale(mat4i(1.0f), v3f(1.0f, 1.0f, 1.0f));
    test.cam.mvp.view  = synt::view(
         test.cam.position, test.cam.position + test.cam.orientation, test.cam.up);
#endif

    subscribe(&test.mouse_evt, EVT_MOUSE);

#if 0
    Vertex verts[8] = {
        { { 0.5f, 0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }, 1.0f },
        { { 0.5f, 0.5f - 0.5f, -0.5f },
          { 1.0f, 0.0f, 0.0f, 1.0f },
          { 0.0f, 1.0f },
          1.0f },
        { { 0.5f + 0.5f, 0.5f - 0.5f, -0.5f },
          { 1.0f, 0.0f, 0.0f, 1.0f },
          { 1.0f, 1.0f },
          1.0f },
        { { 0.5f + 0.5f, 0.5f, -0.5f },
          { 1.0f, 0.0f, 0.0f, 1.0f },
          { 1.0f, 0.0f },
          1.0f },
        { { 0.5f, 0.5f, -1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }, 1.0f },
        { { 0.5f, 0.5f - 0.5f, -1.0f },
          { 0.0f, 1.0f, 0.0f, 1.0f },
          { 0.0f, 1.0f },
          1.0f },
        { { 0.5f + 0.5f, 0.5f - 0.5f, -1.0f },
          { 0.0f, 1.0f, 0.0f, 1.0f },
          { 1.0f, 1.0f },
          1.0f },
        { { 0.5f + 0.5f, 0.5f, -1.0f },
          { 0.0f, 1.0f, 0.0f, 1.0f },
          { 1.0f, 0.0f },
          1.0f }
    };

    render_state.g_piplines[0].vert_buffer.data =
        dyn_arrayT((*region), sy_SIZE(verts), Vertex);

    for (uint32 i = 0; i < sy_SIZE(verts); i++)
    {
        synt_push(render_state.g_piplines[0].vert_buffer.data, verts[i]);
    }

    uint32 idnc[] = { 0, 1, 2, 2, 3, 0, 3, 2, 6, 6, 7, 3, 7, 6, 5, 5, 4, 7,
                      4, 5, 1, 1, 0, 4, 4, 0, 3, 3, 7, 4, 1, 5, 6, 6, 2, 1 };

    render_state.g_piplines[0].idx_buffer.data =
        dyn_arrayT((*region), sy_SIZE(idnc), uint32);

    for (uint32 i = 0; i < sy_SIZE(idnc); i++)
    {
        synt_push(render_state.g_piplines[0].idx_buffer.data, idnc[i]);
    }

#endif

#if 0
    Vertex verts[] = {
        { { -0.5f, -0.5f, -1.0f },
          { 1.0f, 0.0f, 0.0f, 1.0f },
          { 0.0f, 1.0f },
          1.0f },
        { { -0.5f + 0.5f, -0.5f, -1.0f },
          { 0.0f, 1.0f, 0.0f, 1.0f },
          { 1.0f, 1.0f },
          1.0f },
        { { -0.5f + 0.5f, -0.5f + 0.5f, -1.0f },
          { 0.0f, 0.0f, 1.0f, 1.0f },
          { 1.0f, 0.0f },
          1.0f },
        { { -0.5f, -0.5f + 0.5f, -1.0f },
          { 1.0f, 1.0f, 1.0f, 1.0f },
          { 1.0f, 0.0f },
          1.0f },
    };

    render_state.g_piplines[0].vert_buffer.data =
        dyn_arrayT((*region), sy_SIZE(verts), Vertex);

    uint32 idnc[sy_SIZE(verts)];

    for (uint32 i = 0; i < sy_SIZE(verts); i++)
    {
        synt_push(render_state.g_piplines[0].vert_buffer.data, verts[i]);
        idnc[i] = i;
    }

    render_state.g_piplines[0].idx_buffer.data =
        dyn_arrayT((*region), sy_SIZE(idnc), uint32);

    for (uint32 i = 0; i < sy_SIZE(idnc); i++)
    {
        synt_push(render_state.g_piplines[0].idx_buffer.data, idnc[i]);
    }

    render_state.cam.position    = synt::v3f(0.0f, 0.0f, 1.0f);
    render_state.cam.orientation = synt::v3f(0.0f, 0.0f, -1.0f);
#endif
}

static uint32 FPS = 0;

static void update_gui(Region_Alloc* region, float dt)
{
    static char fps_buffer[10]   = "FPS: ";
    static char milli_buffer[20] = {};
    back_bord_begin("First thing", Vec2(10.0f, 10.0f));
    {
        gridd_begin(1, 1);
        {
            add_text("Position (x, y, z) This is a test");
        }
        gridd_end();

        gridd_begin(3, 1);
        {
            add_input_float(test.cam.position.x, -100.0f, 100.0f);
            add_input_float(test.cam.position.y, -100.0f, 100.0f);
            add_input_float(test.cam.position.z, -100.0f, 100.0f);
        }
        gridd_end();

        static float sec = 0.1f;
        sec += dt;
        if (sec >= 0.1f)
        {
            sprintf(fps_buffer + 5, "%u", FPS);
            sprintf(milli_buffer, "%f", dt * 1000);
            sprintf(milli_buffer + strlen(milli_buffer), " ms");
            sec = 0.0f;
        }
        gridd_begin(2, 1);
        {
            add_text(fps_buffer);
            add_text(milli_buffer);
        }
        gridd_end();
    }
    back_bord_end();

    back_bord_begin("Second thing", Vec2(800.0f, 10.0f));
    {
        gridd_begin(3, 2);
        {
            if (add_button("+"))
            {
                test.cam.position.x += 0.2;
            }
            if (add_button("Clic")) synt_LOG("Click me\n");
            if (add_button("ddsss")) synt_LOG("dd\n");
            if (add_button("Hllo")) synt_LOG("Hllo\n");
            if (add_button("dss")) synt_LOG("dd\n");
            if (add_button("Hllo")) synt_LOG("Hllo\n");
        }
        gridd_end();

        gridd_begin(1, 1);
        {
            add_text("Position (x, y, z)");
        }
        gridd_end();

        gridd_begin(3, 1);
        {
            add_input_float(test.cam.position.x, -100.0f, 100.0f);
            add_input_float(test.cam.position.y, -100.0f, 100.0f);
            add_input_float(test.cam.position.z, -100.0f, 100.0f);
        }
        gridd_end();

        gridd_begin(2, 1);
        {
            add_text(fps_buffer);
            add_text(milli_buffer);
        }
        gridd_end();
    }
    back_bord_end();
}

void render_testing_recreate(Region_Alloc* region,
                             const Application_State& app_state)
{
    recreate_graphic_pipline(region, app_state, "Syntics/res/vert.spv",
                             "Syntics/res/frag.spv", test.g_pipline,
                             size_arr(test.textures));
}

void render_testing_update(Region_Alloc* region, VkDevice device,
                           const Vec2& dimensions, uint32 semaphore_idx, float dt)
{
    gui_update_begin(region, device, dimensions, semaphore_idx, dt);
    {
        update_gui(region, dt);
    }
    gui_update_end(region, device);

    if (!gui_focus())
    {
        update_camera(&test.cam, test.mouse_evt, dt);
    }

    test.cam.mvp.view = synt::view(
        test.cam.position, test.cam.position + test.cam.orientation, test.cam.up);
    test.cam.mvp.proj =
        perspective(radians(53.0f), dimensions.x / dimensions.y, 0.1f, 100.0f);

    update_uniform_buffers(device, test.g_pipline.uniform_buffers[semaphore_idx],
                           &test.cam.mvp, sizeof(test.cam.mvp));
}

void render_render_testing(VkCommandBuffer command_buffer, uint32 semaphore_idx)
{
    bind_and_draw_graphics_pipline(
        command_buffer, test.g_pipline.descriptors.desc_sets[semaphore_idx],
        test.g_pipline);
}

void render_testing_destroy(VkDevice device, uint32 num_semaphores)
{
    vkDestroyPipelineLayout(device, test.g_pipline.layout, NULL);
    vkDestroyPipeline(device, test.g_pipline.pipeline, NULL);
    vkDestroyDescriptorSetLayout(device, test.g_pipline.set_layout, NULL);
    destroy_buffer(device, test.g_pipline.vert_buffer.buffer,
                   test.g_pipline.vert_buffer.buffer_memory);
    destroy_buffer(device, test.g_pipline.idx_buffer.buffer,
                   test.g_pipline.idx_buffer.buffer_memory);

    vkDestroyDescriptorPool(device, test.g_pipline.descriptors.desc_pool, NULL);

    for (uint32 i = 0; i < num_semaphores; i++)
    {
        destroy_buffer(device, test.g_pipline.uniform_buffers[i].buffer,
                       test.g_pipline.uniform_buffers[i].buffer_memory);
    }
    for (uint32 i = 0; i < size_arr(test.textures); i++)
    {
        destroy_texture(device, test.textures[i]);
    }
}

} // namespace synt

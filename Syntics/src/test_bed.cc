#include "defines.h"
#include "logging.h"
#include "vulkan_types.h"
#include "gui.h"
#include "render_util.h"
#include "region_alloc.h"
#include "buffers.h"
#include "swap_chain.h"
#include "event_system.h"
#include "camera.h"

typedef struct Test_State
{
    Graphic_Pipeline triangle_list_pipeline;
    Graphic_Pipeline line_list_pipeline;
    Vertex_Index_Buffer vert_idx;
    Vertex_Index_Buffer gridd_vert_idx;
    sygui::Window_Handle* win_handles;

    Camera_3D cam;
    M4 global_model;

    M4 rotate_model;

    Texture* textures;

    Events* mouse_evt;

} Test_State;

global Test_State g_state = {};

internal void destroy_test(void* data, VkDevice device, u32 num_semaphores)
{
    destroy_graphic_pipeline(device, num_semaphores,
                             &g_state.triangle_list_pipeline);
    destroy_graphic_pipeline(device, num_semaphores,
                             &g_state.line_list_pipeline);

    destroy_buffer(device, g_state.vert_idx.vert.buffer);
    destroy_buffer(device, g_state.vert_idx.idx.buffer);

    destroy_buffer(device, g_state.gridd_vert_idx.vert.buffer);
    destroy_buffer(device, g_state.gridd_vert_idx.idx.buffer);

    for (u32 i = 0; i < size_arr(g_state.textures); i++)
    {
        destroy_texture(device, g_state.textures[i]);
    }

    sygui::destroy(device, num_semaphores);
}

internal void render_test_bed(void* data, VkCommandBuffer command_buffer,
                              u32 semaphore_idx)
{
    V2* dimensions = (V2*)data;
    VkViewport view_port = {};
    view_port.x = 0.0f;
    view_port.y = 0.0f;
    view_port.width = dimensions->width;
    view_port.height = dimensions->height;
    view_port.maxDepth = 1.0f;

    VkRect2D scissor_internal = { { (i32)view_port.x, (i32)view_port.y },
                                  { (u32)view_port.width, (u32)view_port.height } };

    vkCmdSetViewport(command_buffer, 0, 1, &view_port);
    vkCmdSetScissor(command_buffer, 0, 1, &scissor_internal);

    bind_graphics_pipline(command_buffer, g_state.triangle_list_pipeline,
                          semaphore_idx);

    bind_vertex_index_buffer(command_buffer, g_state.vert_idx);

    push_model(command_buffer, g_state.triangle_list_pipeline.layout,
               g_state.global_model);

    vkCmdDrawIndexed(command_buffer, 6, 1, 0, 0, 0);

    push_model(command_buffer, g_state.triangle_list_pipeline.layout,
               g_state.global_model);

    vkCmdDrawIndexed(command_buffer, g_state.vert_idx.idx.curr_size - 6, 1, 6, 0, 0);

    // Gridd

    bind_graphics_pipline(command_buffer, g_state.line_list_pipeline,
                          semaphore_idx);

    bind_vertex_index_buffer(command_buffer, g_state.gridd_vert_idx);

    push_model(command_buffer, g_state.line_list_pipeline.layout,
               g_state.rotate_model);

    vkCmdDrawIndexed(command_buffer, g_state.gridd_vert_idx.idx.curr_size, 1, 0, 0,
                     0);
}

internal u32 gridd_using_line_list(Vertex* vertices, u32 vertex_offset, u32* indices,
                                   u32 index_offset, V3 middle_pos, V2 spacing,
                                   u32 lines_width_count, u32 lines_height_count,
                                   V4 color, f32 tex_index)
{
    u32 vert_offset = vertex_offset;
    assert(lines_height_count > 0);
    assert(lines_width_count > 0);

    V2 total_size = {};
    total_size.width = lines_width_count * spacing.width;
    total_size.height = lines_height_count * spacing.height;

    V3 current_pos = middle_pos - v3_v2(total_size * 0.5f);
    V3 saved_pos = current_pos;
    current_pos.x += spacing.x * 0.5f;

    Vertex vert = {};
    vert.color = color;
    vert.tex_index = tex_index;
    for (u32 i = 0; i < lines_width_count; i++)
    {
        vert.pos = current_pos;
        val(vertices, vert_offset++) = vert;
        vert.pos.y += total_size.height;
        val(vertices, vert_offset++) = vert;
        current_pos.x += spacing.x;
    }
    current_pos = saved_pos;
    current_pos.y += spacing.y * 0.5f;

    for (u32 i = 0; i < lines_height_count; i++)
    {
        vert.pos = current_pos;
        val(vertices, vert_offset++) = vert;
        vert.pos.x += total_size.width;
        val(vertices, vert_offset++) = vert;
        current_pos.y += spacing.y;
    }
    return vert_offset - vertex_offset;
}

void init_test_bed(Region_Alloc* region, VkDevice device,
                   VkPhysicalDevice physical_device, VkCommandPool command_pool,
                   VkQueue graphic_queue, const Swap_Chain_Attrib* swap_chain,
                   u32 num_semaphores)
{
    g_state.win_handles = dyn_array_callocP(region, 10, sygui::Window_Handle);

    const char* paths[] = {
        "Syntics/res/default.png",
    };

    u32 num_text = sy_SIZE(paths);
    g_state.textures = dyn_arrayP(region, num_text, Texture);

    create_textures_path(device, physical_device, command_pool, graphic_queue, true,
                         num_text, paths, g_state.textures);

    get_head(g_state.textures)->size = num_text;

    { // Triangle list
        Graphic_Pipeline* g_p = &g_state.triangle_list_pipeline;
        *g_p = gp_default1(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        create_graphics_pipeline_deluxe(
            region, device, physical_device, num_semaphores,
            "Syntics/res/shaders/test_bed.vert.spv",
            "Syntics/res/shaders/test_bed.frag.spv", *swap_chain, g_state.textures,
            num_text, g_p);
    }

    { // Line list
        Graphic_Pipeline* g_p = &g_state.line_list_pipeline;
        *g_p = gp_default1(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
        g_p->line_width = 2.0f;
        create_graphics_pipeline_deluxe(
            region, device, physical_device, num_semaphores,
            "Syntics/res/shaders/test_bed.vert.spv",
            "Syntics/res/shaders/test_bed.frag.spv", *swap_chain, g_state.textures,
            num_text, g_p);
    }

    {
        Vertex_Index_Buffer* vert_idx = &g_state.vert_idx;
        Vertex_Buffer* vert = &vert_idx->vert;
        Index_Buffer* idx = &vert_idx->idx;

        vert->data = dyn_arrayP(region, 2000, Vertex);
        idx->data = dyn_arrayP(region, 2000, u32);

        quad_middle(vert->data, v3f(0.0f, 0.0f, 0.0f), v2i(0.07f),
                    v4f(1.0f, 0.0f, 0.0f, 1.0f), 0);

        V3 poes[3] = {};
        poes[0] = v3f(-0.5f, -0.5f, 0.0f);
        poes[1] = v3f(0.0f, v3_len(poes[0]), 0.0f);
        poes[2] = v3f(0.5f, -0.5f, 0.0f);

        for (u32 i = 0; i < sy_SIZE(poes); i++)
        {
            quad_middle(vert->data, poes[i], v2i(0.07f), v4ic(1.0f), 0);
        }
        generate_indices(idx->data, 0, sy_SIZE(poes) + 1);

        idx->curr_size = size_arr(idx->data);
        create_vertex_index_buffer_default(device, physical_device, command_pool,
                                           graphic_queue, VERTEX_INDEX_LOCAL_LOCAL,
                                           &g_state.vert_idx);
    }

    {
        Vertex_Index_Buffer* vert_idx = &g_state.gridd_vert_idx;
        Vertex_Buffer* vert = &vert_idx->vert;
        Index_Buffer* idx = &vert_idx->idx;

        vert->data = dyn_arrayP(region, 2000, Vertex);
        idx->data = dyn_arrayP(region, 2000, u32);

        u32 size = gridd_using_line_list(vert->data, 0, idx->data, 0, v3d(),
                                         v2i(0.2f), 10, 10, v4i(1.0f), 0);

        get_head(vert->data)->size += size;

        for (u32 i = 0; i < size; i++)
        {
            synt_push(idx->data, i);
        }
        idx->curr_size = size_arr(idx->data);
        create_vertex_index_buffer_default(device, physical_device, command_pool,
                                           graphic_queue, VERTEX_INDEX_LOCAL_LOCAL,
                                           vert_idx);
    }
    g_state.cam = cam_3di(4.0f, 5.0f);
    g_state.cam.pos.z = 1.0f;
    g_state.global_model = m4i(1.0f);

    subscribe(&g_state.mouse_evt, EVT_MOUSE);

    subscribe_destroy_callback(destroy_test, NULL);

    sygui::init(region, device, physical_device, command_pool, graphic_queue,
                swap_chain, num_semaphores, true);

    g_state.win_handles[0] = sygui::create_window();
    g_state.win_handles[1] = sygui::create_window();
}

global f32 translucentcy = 0.8f;
global b32 wire_frame = false;
global f32 rot_speed = 1.0f;
global b8 should_rotate = false;

internal void update_gui(Region_Alloc* region, const Application_State* app_state,
                         f32 dt, V2 dimensions)
{
    sygui::begin_pane(g_state.win_handles[0], "First thing", v2f(10.0f, 10.0f));
    {
        sygui::begin_gridd(2, 1);
        {
            sygui::add_text("Translucentcy: ");
            sygui::add_input_float_d(&translucentcy, 0.0f, 1.0f);
        }
        sygui::end_gridd();
        sygui::begin_gridd(4, 1);
        {
            if (sygui::add_button("OFF"))
            {
                translucentcy = 0.0f;
            }
            if (sygui::add_button("Low"))
            {
                translucentcy = 0.2f;
            }
            if (sygui::add_button("High"))
            {
                translucentcy = 0.8f;
            }
            if (sygui::add_button("Fill"))
            {
                translucentcy = 1.0f;
            }
        }
        sygui::end_gridd();
        sygui::begin_gridd(1, 1);
        {
            if (sygui::add_button("Wire Frame"))
            {
                if (!wire_frame)
                {
                    g_state.triangle_list_pipeline.poly_mode = VK_POLYGON_MODE_LINE;
                }
                else
                {
                    g_state.triangle_list_pipeline.poly_mode = VK_POLYGON_MODE_FILL;
                }
                b_switch(wire_frame);
                recreate_graphic_pipline_ap(app_state,
                                            "Syntics/res/shaders/test_bed.vert.spv",
                                            "Syntics/res/shaders/test_bed.frag.spv",
                                            &g_state.triangle_list_pipeline,
                                            size_arr(g_state.textures), NULL);
            }
        }
        sygui::end_gridd();
        sygui::begin_gridd(1, 1);
        {
            sygui::add_text("Position (x, y, z) This is a test");
        }
        sygui::end_gridd();

        sygui::begin_gridd(2, 1);
        {
            if (sygui::add_button("Should Rotate"))
            {
                b_switch(should_rotate);
            }
            sygui::add_input_float(&rot_speed, 0.0f, 100.0f, 3.0f);
        }
        sygui::end_gridd();

        sygui::begin_gridd(1, 1);
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
            sygui::add_text(temp);
        }
        sygui::end_gridd();
    }
    sygui::end_pane();

    sygui::begin_pane(g_state.win_handles[1], "Terminal", v2f(500.0f, 100.0f));
    {
        sygui::add_terminal(250.0f, 200.0f);
    }
    sygui::end_pane();
}

global V2 preserved_dimensions = {};
void update_test_bed(Region_Alloc* region, const Application_State* app_state,
                     V2 dimensions, u32 semaphore_idx, f32 dt)
{
    preserved_dimensions = dimensions;

    if (!sygui::is_focus())
    {
        update_camera(&g_state.cam, g_state.mouse_evt, dt, true, true);
    }
    g_state.cam.vp.view =
        view(g_state.cam.pos, g_state.cam.pos + g_state.cam.ori, g_state.cam.up);

    f32 rotation = 45.0f;
    g_state.cam.vp.proj =
        perspective(radians(rotation), dimensions.x / dimensions.y, 0.1f, 100.0f);

    presist f32 rot = rot_speed;
    if (should_rotate)
    {
        rot += rot_speed * dt;
    }
    g_state.rotate_model = m4_rotate(rot, Z);

    copy_data_buffer(
        &g_state.triangle_list_pipeline.uniform_buffers[semaphore_idx].buffer,
        &g_state.cam.vp, sizeof(g_state.cam.vp));

    copy_data_buffer(
        &g_state.line_list_pipeline.uniform_buffers[semaphore_idx].buffer,
        &g_state.cam.vp, sizeof(g_state.cam.vp));

    draw_pipeline(render_test_bed, (void*)&preserved_dimensions);

    sygui::begin_update(region, dimensions, semaphore_idx, dt, translucentcy);
    {
        update_gui(region, app_state, dt, dimensions);
    }
    sygui::end_update();
}

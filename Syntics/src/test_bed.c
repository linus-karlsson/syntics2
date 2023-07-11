
#define DEFAULT_TEXTURE_TEST 0
#define FONT_TEXTURE_TEST 1

typedef struct Test_State
{
    Graphic_Pipeline triangle_list_pipeline;
    Graphic_Pipeline line_list_pipeline;

    Vertex_Index_Buffer vert_idx;
    Vertex_Index_Buffer gridd_vert_idx;

    Vertex_Index_Buffer menu_vert_idx;

    Window_Handle* win_handles;

    Font font;

    Camera_3D cam;

    VP menu_vp;

    M4 global_model;

    M4 rotate_model;

    Texture* textures;

    Events* mouse_evt;

} Test_State;

global Test_State g_state_TEST = { 0 };

internal void destroy_test(void* data, VkDevice device, u32 num_semaphores)
{
    destroy_graphic_pipeline(device, num_semaphores,
                             &g_state_TEST.triangle_list_pipeline);
    destroy_graphic_pipeline(device, num_semaphores,
                             &g_state_TEST.line_list_pipeline);

#if 0
    destroy_buffer(device, g_state_TEST.vert_idx.vert.buffer);
    destroy_buffer(device, g_state_TEST.vert_idx.idx.buffer);

    destroy_buffer(device, g_state_TEST.gridd_vert_idx.vert.buffer);
    destroy_buffer(device, g_state_TEST.gridd_vert_idx.idx.buffer);
#endif
    destroy_buffer(device, g_state_TEST.menu_vert_idx.vert.buffer);
    destroy_buffer(device, g_state_TEST.menu_vert_idx.idx.buffer);

    for (u32 i = 0; i < size_arr(g_state_TEST.textures); i++)
    {
        destroy_texture(device, g_state_TEST.textures[i]);
    }

    destroy(device, num_semaphores);
}

internal void draw(VkCommandBuffer command_buffer, u32 offset, u32 count)
{
    vkCmdDrawIndexed(command_buffer, count, 1, offset, 0, 0);
}

internal void render_test_bed(void* data, VkCommandBuffer command_buffer,
                              u32 semaphore_idx)
{
    V2* dimensions = (V2*)data;
    VkViewport view_port = { 0 };
    view_port.x = 0.0f;
    view_port.y = 0.0f;
    view_port.width = dimensions->width;
    view_port.height = dimensions->height;
    view_port.maxDepth = 1.0f;

    VkRect2D scissor_internal = { { (i32)view_port.x, (i32)view_port.y },
                                  { (u32)view_port.width, (u32)view_port.height } };

    vkCmdSetViewport(command_buffer, 0, 1, &view_port);
    vkCmdSetScissor(command_buffer, 0, 1, &scissor_internal);

    bind_graphics_pipline(command_buffer, &g_state_TEST.triangle_list_pipeline,
                          semaphore_idx);

    bind_vertex_index_buffer1(command_buffer, &g_state_TEST.menu_vert_idx);

    push_model(command_buffer, g_state_TEST.triangle_list_pipeline.layout,
               g_state_TEST.global_model);

    draw(command_buffer, 0, g_state_TEST.menu_vert_idx.idx.curr_size);

#if 0
    bind_vertex_index_buffer(command_buffer, g_state_TEST.vert_idx);

    push_model(command_buffer, g_state_TEST.triangle_list_pipeline.layout,
               g_state_TEST.rotate_model);

    vkCmdDrawIndexed(command_buffer, 6, 1, 0, 0, 0);

    push_model(command_buffer, g_state_TEST.triangle_list_pipeline.layout,
               g_state_TEST.global_model);

    vkCmdDrawIndexed(command_buffer, g_state_TEST.vert_idx.idx.curr_size - 6, 1, 6, 0, 0);

    // Gridd

#if 1
    bind_graphics_pipline(command_buffer, g_state_TEST.line_list_pipeline, semaphore_idx);

    bind_vertex_index_buffer(command_buffer, g_state_TEST.gridd_vert_idx);

    push_model(command_buffer, g_state_TEST.line_list_pipeline.layout,
               g_state_TEST.rotate_model);

    vkCmdDrawIndexed(command_buffer, g_state_TEST.gridd_vert_idx.idx.curr_size, 1, 0, 0,
                     0);
#endif
#endif
}

internal u32 circle(Vertex* vertices, u32 vertex_offset, u32* indices,
                    u32 index_offset, u32* indices_count, V3 middle_pos,
                    u32 triangle_count, f32 radius, V4 color, f32 tex_index)
{
    assert(triangle_count > 0);

    u32 vert_offset = vertex_offset;
    u32 middle_index = vert_offset;
    u32 vertex_count = (triangle_count * 4);
    u32 count = 0;

    f32 angle = 0.0f;
    f32 angle_increase = 90.0f / triangle_count;

    Vertex vert = { 0 };
    vert.color = color;
    vert.tex_index = tex_index;
    vert.pos = middle_pos;
    val(vertices, vert_offset++) = vert;

    for (u32 i = 0; i < 4; i++)
    {
        for (u32 j = 0; j < triangle_count; j++)
        {
            vert.pos.x = cosf(radians(angle)) * radius;
            vert.pos.y = sinf(radians(angle)) * radius;
            val(vertices, vert_offset++) = vert;
            angle += angle_increase;

            count++;
            val(indices, index_offset++) = middle_index;
            val(indices, index_offset++) = vertex_offset + count;
            val(indices, index_offset++) =
                vertex_offset + (count % vertex_count) + 1;
        }
    }

    if (indices_count)
    {
        *indices_count += 3 * 4 * triangle_count;
    }
    u32 size = vertex_count + 1;
    return size;
}

global b8 file_changed = false;
global HANDLE file_change_handle;
HANDLE start_semaphore;

unsigned long looking_for_file_changes(void* data)
{
    char* path_to_detect = (char*)data;
    for (;;)
    {
        WaitForSingleObject(start_semaphore, INFINITE);
        file_change_handle = FindFirstChangeNotification(
            path_to_detect, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);

        assert(file_change_handle != INVALID_HANDLE_VALUE);

        WaitForSingleObject(file_change_handle, INFINITE);

        file_changed = true;
    }
}

void init_test_bed(Region_Alloc* region, VkDevice device,
                   VkPhysicalDevice physical_device, VkCommandPool command_pool,
                   VkQueue graphic_queue, const Swap_Chain_Attrib* swap_chain,
                   u32 num_semaphores)
{
    start_semaphore = CreateSemaphore(NULL, 0, 1, NULL);

    const char* p = "Syntics/res/shaders/spv";
    char* path_to_detect = extend_path(region, p, (u32)strlen(p));

    thread_create(path_to_detect, looking_for_file_changes, 0, NULL);
    ReleaseSemaphore(start_semaphore, 1, 0);

    g_state_TEST.win_handles = dyn_array_callocP(region, 10, Window_Handle);

    const char* paths[] = {
        [DEFAULT_TEXTURE_TEST] = "Syntics/res/default.png",
        [FONT_TEXTURE_TEST] = "Syntics/res/Purisa.png",
    };
    u32 num_text = sy_SIZE(paths);
    g_state_TEST.textures = dyn_arrayP(region, num_text, Texture);

    create_textures_path(device, physical_device, command_pool, graphic_queue, true,
                         num_text, paths, g_state_TEST.textures);

    get_head(g_state_TEST.textures)->size = num_text;

    g_state_TEST.font = load_font_file(region, "Syntics/res/Purisa.fnt");
    g_state_TEST.font.tex_index = FONT_TEXTURE_TEST;

    { // Triangle list
        Graphic_Pipeline* g_p = &g_state_TEST.triangle_list_pipeline;
        *g_p = gp_default1(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        create_graphics_pipeline_deluxe(
            region, device, physical_device, num_semaphores,
            "Syntics/res/shaders/spv/test_bed.vert.spv",
            "Syntics/res/shaders/spv/test_bed.frag.spv", swap_chain,
            g_state_TEST.textures, num_text, g_p);
    }

    { // Line list
        Graphic_Pipeline* g_p = &g_state_TEST.line_list_pipeline;
        *g_p = gp_default1(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
        g_p->line_width = 2.0f;
        create_graphics_pipeline_deluxe(
            region, device, physical_device, num_semaphores,
            "Syntics/res/shaders/spv/test_bed.vert.spv",
            "Syntics/res/shaders/spv/test_bed.frag.spv", swap_chain,
            g_state_TEST.textures, num_text, g_p);
    }
#if 0

    {
        Vertex_Index_Buffer* vert_idx = &g_state_TEST.vert_idx;
        Vertex_Buffer* vert = &vert_idx->vert;
        Index_Buffer* idx = &vert_idx->idx;

        vert->data = dyn_arrayP(region, 2000, Vertex);
        idx->data = dyn_arrayP(region, 2000, u32);

        V3 poes[3] = {};
        poes[0] = v3f(-0.5f, -0.5f, 0.0f);
        poes[1] = v3f(0.0f, v3_len(poes[0]), 0.0f);
        poes[2] = v3f(0.5f, -0.5f, 0.0f);
        quad_middle(vert->data, v3f(0.0f, 0.0f, 0.0f), v2i(0.07f),
                    v4f(1.0f, 0.0f, 0.0f, 1.0f), 0);

        for (u32 i = 0; i < sy_SIZE(poes); i++)
        {
            quad_middle(vert->data, poes[i], v2i(0.07f), v4ic(1.0f), 0);
        }
        generate_indices(idx->data, 0, sy_SIZE(poes) + 1);

        get_head(vert->data)->size =
            circle(vert->data, size_arr(vert->data), idx->data, size_arr(idx->data),
                   &get_head(idx->data)->size, v3d(), 20, 0.5f, v4i(1.0f), 0);

        idx->curr_size = size_arr(idx->data);
        create_vertex_index_buffer_default(device, physical_device, command_pool,
                                           graphic_queue, VERTEX_INDEX_LOCAL_LOCAL,
                                           &g_state_TEST.vert_idx);
    }

    {
        Vertex_Index_Buffer* vert_idx = &g_state_TEST.gridd_vert_idx;
        Vertex_Buffer* vert = &vert_idx->vert;
        Index_Buffer* idx = &vert_idx->idx;

        vert->data = dyn_arrayP(region, 2000, Vertex);
        idx->data = dyn_arrayP(region, 2000, u32);

        u32 size = gridd_using_line_list(vert->data, 0, idx->data, 0, v3d(),
                                         v2i(0.2f), 10, 10, v4i(1.0f), 0);

        get_head(vert->data)->size += size;
        get_head(idx->data)->size += size;

        idx->curr_size = size_arr(idx->data);
        create_vertex_index_buffer_default(device, physical_device, command_pool,
                                           graphic_queue, VERTEX_INDEX_LOCAL_LOCAL,
                                           vert_idx);
    }
#endif

    {
        Vertex_Index_Buffer* vert_idx = &g_state_TEST.menu_vert_idx;
        Vertex_Buffer* vert = &vert_idx->vert;
        Index_Buffer* idx = &vert_idx->idx;

        vert->data = dyn_arrayP(region, 2000, Vertex);
        idx->data = dyn_arrayP(region, 2000, u32);

        V2 dimensions =
            v2f((f32)swap_chain->extent_2D.width, (f32)swap_chain->extent_2D.height);

        V2 padding = v2f(300.0f, 200.0f);
        V2 back_bord_size = v2f(dimensions.width - padding.x * 2.0f,
                                dimensions.height - padding.y * 2.0f);
        f32 k = 0.4f;
        f32 a = 0.4f;

        square_rounded_corners(vert->data, idx->data, v3_v2(padding), back_bord_size,
                               v4f(k, k, k, a), 20.0f, 8, DEFAULT_TEXTURE_TEST);

        u32 offset = size_arr(vert->data);
        u32 quad_count = 0;
        // Options
        {
            const char* buffers[] = {
                "Play New Game",
                "Play Saved Game",
                "Settings",
                "Quit",
            };
            u32 options_count = sy_SIZE(buffers);

            for (u32 i = 0; i < options_count; i++)
            {
                const char* current_buffer = buffers[i];
                u32 len = (u32)strlen(current_buffer);
                f32 x_advance =
                    text_x_advance(g_state_TEST.font, current_buffer, len, 1.0f);

                V3 position = v3_v2(padding);
                position.x += (back_bord_size.x * 0.5f) - (x_advance * 0.5f);
                position.y += (i * 150.0f) + 30.0f;
                quad_count += text_2D(g_state_TEST.font, 1.0f, current_buffer, len,
                                      position,v4i(1.0f), 1.0f, NULL, NULL,
                                      vert->data);
            }
        }
        generate_indices(idx->data, offset, quad_count);

        idx->curr_size = size_arr(idx->data);
        create_vertex_index_buffer_default1(device, physical_device, command_pool,
                                            graphic_queue, VERTEX_INDEX_LOCAL_LOCAL,
                                            vert_idx);
    }

    g_state_TEST.cam = cam_3di(4.0f, 5.0f);
    g_state_TEST.cam.pos.z = 1.0f;
    g_state_TEST.global_model = m4i(1.0f);

    subscribe(&g_state_TEST.mouse_evt, EVT_MOUSE);

    subscribe_destroy_callback(destroy_test, NULL);

    init(region, device, physical_device, command_pool, graphic_queue, swap_chain,
         num_semaphores, true);

    g_state_TEST.win_handles[0] = create_window();
    g_state_TEST.win_handles[1] = create_window();
}

global f32 test_translucentcy = 0.8f;
global b32 test_wire_frame = false;
global f32 rot_speed = 1.0f;
global b8 should_rotate = false;

void test_update_gui(Region_Alloc* region, const Application_State* app_state,
                     f32 dt, V2 dimensions)
{
    begin_pane(g_state_TEST.win_handles[0], "First thing", v2f(10.0f, 10.0f));
    {
        begin_gridd(2, 1);
        {
            add_text("test_Translucentcy: ");
            add_input_float_d(&test_translucentcy, 0.0f, 1.0f);
        }
        end_gridd();
        begin_gridd(4, 1);
        {
            if (add_button("OFF"))
            {
                test_translucentcy = 0.0f;
            }
            if (add_button("Low"))
            {
                test_translucentcy = 0.2f;
            }
            if (add_button("High"))
            {
                test_translucentcy = 0.8f;
            }
            if (add_button("Fill"))
            {
                test_translucentcy = 1.0f;
            }
        }
        end_gridd();
        begin_gridd(1, 1);
        {
            if (add_button("Wire Frame"))
            {
                if (!test_wire_frame)
                {
                    g_state_TEST.triangle_list_pipeline.poly_mode =
                        VK_POLYGON_MODE_LINE;
                }
                else
                {
                    g_state_TEST.triangle_list_pipeline.poly_mode =
                        VK_POLYGON_MODE_FILL;
                }
                b_switch(test_wire_frame);
                recreate_graphic_pipline_ap(app_state,
                                            "Syntics/res/shaders/test_bed.vert.spv",
                                            "Syntics/res/shaders/test_bed.frag.spv",
                                            &g_state_TEST.triangle_list_pipeline,
                                            size_arr(g_state_TEST.textures), NULL);
            }
        }
        end_gridd();
        begin_gridd(1, 1);
        {
            add_text("Position (x, y, z) This is a test");
        }
        end_gridd();

        begin_gridd(2, 1);
        {
            if (add_button("Should Rotate"))
            {
                b_switch(should_rotate);
            }
            add_input_float(&rot_speed, 0.0f, 100.0f, 3.0f);
        }
        end_gridd();

        begin_gridd(1, 1);
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
        end_gridd();
    }
    end_pane();

    begin_pane(g_state_TEST.win_handles[1], "Terminal", v2f(500.0f, 100.0f));
    {
        add_terminal(250.0f, 200.0f);
    }
    end_pane();
}

internal void recreate_gps(const Application_State* app_state)
{
    recreate_graphic_pipline_ap(
        app_state, "Syntics/res/shaders/spv/test_bed.vert.spv",
        "Syntics/res/shaders/spv/test_bed.frag.spv",
        &g_state_TEST.triangle_list_pipeline, size_arr(g_state_TEST.textures), NULL);

    recreate_graphic_pipline_ap(
        app_state, "Syntics/res/shaders/spv/test_bed.vert.spv",
        "Syntics/res/shaders/spv/test_bed.frag.spv",
        &g_state_TEST.line_list_pipeline, size_arr(g_state_TEST.textures), NULL);
}

void update_test_bed(Region_Alloc* region, const Application_State* app_state,
                     V2 dimensions, u32 semaphore_idx, f32 dt)
{

    presist b8 file_change_counter = false;
    if (file_changed)
    {
        // TODO: Because more than one file gets compile each time this function gets
        // called multiple times
        recreate_gps(app_state);
        file_changed = false;
        ReleaseSemaphore(start_semaphore, 1, 0);
    }
    presist V2 preserved_dimensions = { 0 };
    preserved_dimensions = dimensions;

#if 0
    if (!is_focus())
    {
        update_camera(&g_state_TEST.cam, g_state_TEST.mouse_evt, dt, true, true);
    }
    g_state_TEST.cam.vp.view =
        view(g_state_TEST.cam.pos, g_state_TEST.cam.pos + g_state_TEST.cam.ori, g_state_TEST.cam.up);

    f32 rotation = 45.0f;
    g_state_TEST.cam.vp.proj =
        perspective(radians(rotation), dimensions.x / dimensions.y, 0.0f, 100.0f);
#endif

    g_state_TEST.menu_vp.view = m4i(1.0f);
    g_state_TEST.menu_vp.proj =
        ortho(0.0f, dimensions.width, 0.0f, dimensions.height, -1.0f, 1.0f);

    presist f32 rot = 0.0f;
    if (should_rotate)
    {
        rot += rot_speed * dt;
    }
    g_state_TEST.rotate_model = m4_rotate(rot, Z);

    copy_data_buffer(
        &g_state_TEST.triangle_list_pipeline.uniform_buffers[semaphore_idx].buffer,
        &g_state_TEST.menu_vp, sizeof(g_state_TEST.menu_vp));

#if 0
    copy_data_buffer(
        &g_state_TEST.line_list_pipeline.uniform_buffers[semaphore_idx].buffer,
        &g_state_TEST.cam.vp, sizeof(g_state_TEST.cam.vp));
#endif

    draw_pipeline(render_test_bed, (void*)&preserved_dimensions);

    begin_update(region, dimensions, semaphore_idx, dt, test_translucentcy);
    {
        test_update_gui(region, app_state, dt, dimensions);
    }
    end_update();
}

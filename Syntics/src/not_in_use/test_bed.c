
#define DEFAULT_TEXTURE_TEST 0
#define FONT_TEXTURE_TEST 1

typedef struct Color_Animation
{
    V3 start_color;
    V3 end_color;
    V3* current_start_color;
    V3* current_end_color;
    f32 duration;
    f32 sec;
    f32 process;
    u32 current_id;
} Color_Animation;

typedef struct Test_State
{
    Graphic_Pipeline triangle_list_pipeline;
    Graphic_Pipeline line_list_pipeline;

    Vertex_Index_Buffer vert_idx;
    Vertex_Index_Buffer gridd_vert_idx;

    Vertex_Index_Buffer menu_vert_idx;

    Gui_Context gui_ctx;
    Window_Handle* win_handles;

    Font font;

    Camera_3D cam;

    VP menu_vp;
    M4 global_model;
    M4 rotate_model;
    AABB_2D* aabb_options;
    u32* vertex_options_offset;
    u32* vertex_options_count;
    Color_Animation colors;

    Texture* textures;

    Events* mouse_evt;

    V4 base_font_color;

    b32 should_render_game;
} Test_State;

global Test_State g_state_TEST = { 0 };

internal void test_bed_destroy(void* data, VkDevice device, u32 num_semaphores)
{
    graphic_pipeline_destroy(device, num_semaphores,
                             &g_state_TEST.triangle_list_pipeline);
    graphic_pipeline_destroy(device, num_semaphores,
                             &g_state_TEST.line_list_pipeline);

#if 0
    syntics_vulkan_buffer_destroy(device, g_state_TEST.vert_idx.vert.buffer);
    syntics_vulkan_buffer_destroy(device, g_state_TEST.vert_idx.idx.buffer);

    syntics_vulkan_buffer_destroy(device, g_state_TEST.gridd_vert_idx.vert.buffer);
    syntics_vulkan_buffer_destroy(device, g_state_TEST.gridd_vert_idx.idx.buffer);
#endif
    syntics_vulkan_buffer_destroy(device, g_state_TEST.menu_vert_idx.vert.buffer);
    syntics_vulkan_buffer_destroy(device, g_state_TEST.menu_vert_idx.idx.buffer);

    for (u32 i = 0; i < array_size(g_state_TEST.textures); i++)
    {
        syntics_vulkan_texture_destroy(device, g_state_TEST.textures[i]);
    }
    gui_destroy(&g_state_TEST.gui_ctx, device, num_semaphores);
}

void test_bed_render(void* data, VkCommandBuffer command_buffer,
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
                                  { (u32)view_port.width,
                                    (u32)view_port.height } };

    vkCmdSetViewport(command_buffer, 0, 1, &view_port);
    vkCmdSetScissor(command_buffer, 0, 1, &scissor_internal);

    graphics_pipline_bind(command_buffer, &g_state_TEST.triangle_list_pipeline,
                          semaphore_idx);

    syntics_vulkan_vertex_index_buffer_bind1(command_buffer, &g_state_TEST.menu_vert_idx);

    syntics_vulkan_push_constant(command_buffer, g_state_TEST.triangle_list_pipeline.layout,
                  &g_state_TEST.global_model, sizeof(M4));

    syntics_vulkan_draw(command_buffer, 0, g_state_TEST.menu_vert_idx.idx.curr_size);

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
    graphics_pipline_bind(command_buffer, g_state_TEST.line_list_pipeline, semaphore_idx);

    bind_vertex_index_buffer(command_buffer, g_state_TEST.gridd_vert_idx);

    push_model(command_buffer, g_state_TEST.line_list_pipeline.layout,
               g_state_TEST.rotate_model);

    vkCmdDrawIndexed(command_buffer, g_state_TEST.gridd_vert_idx.idx.curr_size, 1, 0, 0,
                     0);
#endif
#endif
}

u32 circle(Vertex* vertices, u32 vertex_offset, u32* indices, u32 index_offset,
           u32* indices_count, V3 middle_pos, u32 triangle_count, f32 radius,
           V4 color, f32 tex_index)
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
    array_val(vertices, vert_offset++) = vert;

    for (u32 i = 0; i < 4; i++)
    {
        for (u32 j = 0; j < triangle_count; j++)
        {
            vert.pos.x = (f32)cos((f64)radians(angle)) * radius;
            vert.pos.y = (f32)sin((f64)radians(angle)) * radius;
            array_val(vertices, vert_offset++) = vert;
            angle += angle_increase;

            count++;
            array_val(indices, index_offset++) = middle_index;
            array_val(indices, index_offset++) = vertex_offset + count;
            array_val(indices, index_offset++) =
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

void test_bed_recreate_gps(void* data, const Application_State* app_state)
{
    graphic_pipline_ap_recreate(app_state,
                                "Syntics/res/shaders/spv/test_bed.vert.spv",
                                "Syntics/res/shaders/spv/test_bed.frag.spv",
                                &g_state_TEST.triangle_list_pipeline,
                                array_size(g_state_TEST.textures), NULL);

    graphic_pipline_ap_recreate(app_state,
                                "Syntics/res/shaders/spv/test_bed.vert.spv",
                                "Syntics/res/shaders/spv/test_bed.frag.spv",
                                &g_state_TEST.line_list_pipeline,
                                array_size(g_state_TEST.textures), NULL);
}

#define NEW_GAME_OPTION_TEST 0
#define SAVED_GAME_OPTION_TEST 1
#define SETTINGS_OPTION_TEST 2
#define QUIT_OPTION_TEST 3

global V4 g_color;

global u32 g_offset;

void test_bed_init(Region_Alloc* region, VkDevice device,
                   VkPhysicalDevice physical_device, VkCommandPool command_pool,
                   VkQueue graphic_queue, const Swap_Chain_Attrib* swap_chain,
                   const Platform* platform, Render_State* render_state,
                   u32 num_semaphores)
{
    g_state_TEST.win_handles = region_array_calloc(region, 10, Window_Handle);

    const char* paths[] = {
        [DEFAULT_TEXTURE_TEST] = "Syntics/res/default.png",
        [FONT_TEXTURE_TEST] = "Syntics/res/Purisa.png",
    };
    u32 num_text = sy_SIZE(paths);
    g_state_TEST.textures = region_array(region, num_text, Texture);

    syntics_vulkan_textures_path_create(device, physical_device, command_pool, graphic_queue,
                         true, num_text, paths, g_state_TEST.textures);

    region_array_head(g_state_TEST.textures)->size = num_text;

    g_state_TEST.font = font_file_load(region, "Syntics/res/Purisa.fnt");
    g_state_TEST.font.tex_index = FONT_TEXTURE_TEST;

    { // Triangle list
        Graphic_Pipeline* g_p = &g_state_TEST.triangle_list_pipeline;
        *g_p = gp_default1(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        graphics_pipeline_create_deluxe(
            region, device, physical_device, num_semaphores,
            "Syntics/res/shaders/spv/test_bed.vert.spv",
            "Syntics/res/shaders/spv/test_bed.frag.spv", swap_chain,
            g_state_TEST.textures, num_text, g_p);
    }

    { // Line list
        Graphic_Pipeline* g_p = &g_state_TEST.line_list_pipeline;
        *g_p = gp_default1(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
        g_p->line_width = 2.0f;
        graphics_pipeline_create_deluxe(
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

        vert->data = region_array(region, 2000, Vertex);
        idx->data = region_array(region, 2000, u32);

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
            circle(vert->data, array_size(vert->data), idx->data, array_size(idx->data),
                   &get_head(idx->data)->size, v3d(), 20, 0.5f, v4i(1.0f), 0);

        idx->curr_size = array_size(idx->data);
        create_vertex_index_buffer_default(device, physical_device, command_pool,
                                           graphic_queue, VERTEX_INDEX_LOCAL_LOCAL,
                                           &g_state_TEST.vert_idx);
    }

    {
        Vertex_Index_Buffer* vert_idx = &g_state_TEST.gridd_vert_idx;
        Vertex_Buffer* vert = &vert_idx->vert;
        Index_Buffer* idx = &vert_idx->idx;

        vert->data = region_array(region, 2000, Vertex);
        idx->data = region_array(region, 2000, u32);

        u32 size = gridd_using_line_list(vert->data, 0, idx->data, 0, v3d(),
                                         v2i(0.2f), 10, 10, v4i(1.0f), 0);

        get_head(vert->data)->size += size;
        get_head(idx->data)->size += size;

        idx->curr_size = array_size(idx->data);
        create_vertex_index_buffer_default(device, physical_device, command_pool,
                                           graphic_queue, VERTEX_INDEX_LOCAL_LOCAL,
                                           vert_idx);
    }
#endif

    {
        Vertex_Index_Buffer* vert_idx = &g_state_TEST.menu_vert_idx;
        Vertex_Buffer* vert = &vert_idx->vert;
        Index_Buffer* idx = &vert_idx->idx;

        vert->array = vertex_array_create(region, 2000);
        idx->array = u32_array_create(region, 2000);

        V2 dimensions = v2f((f32)swap_chain->extent_2D.width,
                            (f32)swap_chain->extent_2D.height);

        V2 padding = v2f(300.0f, 200.0f);
        V2 back_bord_size = v2f(dimensions.width - padding.x * 2.0f,
                                dimensions.height - padding.y * 2.0f);

        square_rounded_corners(&vert->array, &idx->array, 0, v3_v2(padding),
                               back_bord_size, v4f(0.09f, 0.247f, 0.268f, 0.5f),
                               20.0f, 8, DEFAULT_TEXTURE_TEST);

        g_offset = vert->array.size;
        u32 quad_count = 0;
        // Options
        {
            g_state_TEST.base_font_color =
                v4f(sy_RGB(100.0f), sy_RGB(216.0f), sy_RGB(14.0f), 1.0f);

            const char* buffers[] = {
                [NEW_GAME_OPTION_TEST] = "Play New Game",
                [SAVED_GAME_OPTION_TEST] = "Play Saved Game",
                [SETTINGS_OPTION_TEST] = "Settings",
                [QUIT_OPTION_TEST] = "Quit",
            };
            u32 options_count = sy_SIZE(buffers);

            g_state_TEST.aabb_options =
                region_array_calloc(region, options_count, AABB_2D);

            g_state_TEST.vertex_options_offset =
                region_array_calloc(region, options_count, u32);

            g_state_TEST.vertex_options_count =
                region_array_calloc(region, options_count, u32);

            g_state_TEST.colors.start_color =
                v3f(sy_RGB(156.0f), sy_RGB(216.0f), sy_RGB(14.0f));

            g_state_TEST.colors.end_color =
                v3f(sy_RGB(246.0f), sy_RGB(62.0f), sy_RGB(14.0f));

            g_state_TEST.colors.current_start_color =
                &g_state_TEST.colors.start_color;

            g_state_TEST.colors.current_end_color =
                &g_state_TEST.colors.end_color;
            g_state_TEST.colors.duration = 1.0f;

            for (u32 i = 0; i < options_count; i++)
            {
                const char* current_buffer = buffers[i];
                assert(current_buffer);
                u32 len = (u32)strlen(current_buffer);
                f32 x_advance = text_x_advance(g_state_TEST.font,
                                               current_buffer, len, 1.0f);

                V3 position = v3_v2(padding);
                position.x += (back_bord_size.x * 0.5f) - (x_advance * 0.5f);
                position.y += (i * 150.0f) + 30.0f;
                u32 text_count =
                    text_2D(g_state_TEST.font, 1.0f, current_buffer, len,
                            position, g_state_TEST.base_font_color, 1.0f, NULL,
                            NULL, &vert->array);

                g_state_TEST.vertex_options_offset[i] =
                    (quad_count * 4) + g_offset;
                g_state_TEST.vertex_options_count[i] = text_count * 4;

                quad_count += text_count;

                AABB_2D aabb = {
                    .min = v2_v3(position),
                    .size = v2f(x_advance, (f32)g_state_TEST.font.line_height),
                    .id = i,
                };
                region_array_push(g_state_TEST.aabb_options, aabb);
            }
        }
        indices_generate(&idx->array, g_offset, quad_count);

        idx->curr_size = idx->array.size;
        syntics_vulkan_vertex_index_buffer_create_default1(
            device, physical_device, command_pool, graphic_queue,
            VERTEX_INDEX_VISIBLE_LOCAL, vert_idx);
    }
    g_state_TEST.cam = cam_3di(4.0f, 5.0f);
    g_state_TEST.cam.pos.z = 1.0f;
    g_state_TEST.global_model = m4i(1.0f);

    event_subscribe(&g_state_TEST.mouse_evt, EVT_MOUSE);

    subscribe_recreate_gp_callback(render_state, test_bed_recreate_gps, NULL);
    subscribe_destroy_callback(render_state, test_bed_destroy, NULL);

    gui_init(region, device, physical_device, command_pool, graphic_queue,
             swap_chain, platform, num_semaphores, 2, true,
             &g_state_TEST.gui_ctx);

    g_state_TEST.win_handles[0] = window_create(&g_state_TEST.gui_ctx);
    g_state_TEST.win_handles[1] = window_create(&g_state_TEST.gui_ctx);
}

global f32 translucentcy_TEST = 0.8f;
global b32 wire_frame_TEST = false;
global f32 rot_speed_TEST = 1.0f;
global b8 should_rotate_TEST = false;

void test_update_gui(Region_Alloc* region, const Application_State* app_state,
                     f32 dt, V2 dimensions)
{
    Ui_Window* win0 =
        window_begin(&g_state_TEST.gui_ctx, g_state_TEST.win_handles[0],
                     "First thing", v2f(10.0f, 10.0f));
    {

        window_gridd_begin(win0, 2, 1);
        {
            window_text_add(win0, "Translucentcy_TEST: ");
            window_input_float_add_d(win0, &translucentcy_TEST, 0.0f, 1.0f);
        }
        window_gridd_end(win0);
        window_gridd_begin(win0, 4, 1);
        {
            if (window_button_add(win0, "OFF"))
            {
                translucentcy_TEST = 0.0f;
            }
            if (window_button_add(win0, "Low"))
            {
                translucentcy_TEST = 0.2f;
            }
            if (window_button_add(win0, "High"))
            {
                translucentcy_TEST = 0.8f;
            }
            if (window_button_add(win0, "Fill"))
            {
                translucentcy_TEST = 1.0f;
            }
        }
        window_gridd_end(win0);
        window_gridd_begin(win0, 1, 1);
        {
            if (window_button_add(win0, "Wire Frame"))
            {
                if (!wire_frame_TEST)
                {
                    g_state_TEST.triangle_list_pipeline.poly_mode =
                        VK_POLYGON_MODE_LINE;
                }
                else
                {
                    g_state_TEST.triangle_list_pipeline.poly_mode =
                        VK_POLYGON_MODE_FILL;
                }
                b_switch(wire_frame_TEST);
                graphic_pipline_ap_recreate(
                    app_state, "Syntics/res/shaders/spv/test_bed.vert.spv",
                    "Syntics/res/shaders/spv/test_bed.frag.spv",
                    &g_state_TEST.triangle_list_pipeline,
                    array_size(g_state_TEST.textures), NULL);
            }
        }
        window_gridd_end(win0);
        window_gridd_begin(win0, 1, 1);
        {
            window_text_add(win0, "Position (x, y, z) This is a test");
        }
        window_gridd_end(win0);

        window_gridd_begin(win0, 4, 1);
        {
            window_input_float_add_d(win0, &g_color.r, 0.0f, 1.0f);
            window_input_float_add_d(win0, &g_color.g, 0.0f, 1.0f);
            window_input_float_add_d(win0, &g_color.b, 0.0f, 1.0f);
            window_input_float_add_d(win0, &g_color.a, 0.0f, 1.0f);
        }
        window_gridd_end(win0);

        window_gridd_begin(win0, 2, 1);
        {
            if (window_button_add(win0, "Should Rotate"))
            {
                b_switch(should_rotate_TEST);
            }
            window_input_float_add(win0, &rot_speed_TEST, 0.0f, 100.0f, 3.0f);
        }
        window_gridd_end(win0);

        window_gridd_begin(win0, 1, 1);
        {
            presist char temp[60] = { 0 };
            presist f32 count = 1.0f;
            if (count >= 0.1f)
            {
                f32 milli = dt * 1000.0f;
                sysprintf(temp, sizeof(temp), "Milli: %f | FPS: %u", milli,
                          app_state->fps);
                count = 0.0f;
            }
            count += dt;
            window_text_add(win0, temp);
        }
        window_gridd_end(win0);
    }
    window_end(&win0);

    Ui_Window* win1 =
        window_begin(&g_state_TEST.gui_ctx, g_state_TEST.win_handles[1],
                     "Terminal", v2f(500.0f, 100.0f));
    {
        terminal_add(&g_state_TEST.gui_ctx, terminal_ptr_get(), win1, 250.0f,
                     200.0f);
    }
    window_end(&win1);
}

V4 animate_colors(f32 dt)
{
    Color_Animation* animation = &g_state_TEST.colors;
    V4 result =
        v4_v3f(v3_lerp(*animation->current_start_color,
                       *animation->current_end_color, animation->process),
               1.0f);
    animation->sec += dt;
    if (animation->sec >= animation->duration)
    {
        V3* temp_color = animation->current_start_color;
        animation->current_start_color = animation->current_end_color;
        animation->current_end_color = temp_color;
        animation->sec = 0.0f;
    }
    animation->process = animation->sec / animation->duration;
    return result;
}

void color_change(u32 id, V4 new_color)
{
    Vertex_Buffer* vb = &g_state_TEST.menu_vert_idx.vert;

    Vertex* starting_vertex = vertex_array_val_ptr(
        &vb->array, array_val(g_state_TEST.vertex_options_offset, id));

    const u32 vertex_count = array_val(g_state_TEST.vertex_options_count, id);

    for (u32 i = 0; i < vertex_count; i++)
    {
        starting_vertex[i].color = new_color;
    }
    syntics_vulkan_buffer_copy_data(&vb->buffer, vb->array.data, vb->buffer.size_bytes);
}

void test_bed_new_game(u32 id, b8 any_button_clicked, f32 dt)
{
    color_change(id, animate_colors(dt));
    if (any_button_clicked)
    {
        g_state_TEST.should_render_game = true;
    }
}

void test_bed_saved_game(u32 id, b8 any_button_clicked, f32 dt)
{
    color_change(id, animate_colors(dt));
}

void test_bed_settings(u32 id, b8 any_button_clicked, f32 dt)
{
    color_change(id, animate_colors(dt));
}

void test_bed_quit(u32 id, b8 any_button_clicked, f32 dt)
{
    color_change(id, animate_colors(dt));
    if (any_button_clicked)
    {
        quit_event();
    }
}

void test_bed_process_options(u32 id, b8 any_button_clicked, f32 dt)
{
    switch (id)
    {
        case NEW_GAME_OPTION_TEST:
        {
            test_bed_new_game(id, any_button_clicked, dt);
            break;
        }
        case SAVED_GAME_OPTION_TEST:
        {
            test_bed_saved_game(id, any_button_clicked, dt);
            break;
        }
        case SETTINGS_OPTION_TEST:
        {
            test_bed_settings(id, any_button_clicked, dt);
            break;
        }
        case QUIT_OPTION_TEST:
        {
            test_bed_quit(id, any_button_clicked, dt);
            break;
        }
        default:
        {
            assert(false);
        }
    }
}

void test_bed_update(Region_Alloc* region, Frame_Data* frame,
                     const Application_State* app_state,
                     Render_State* render_state, V2 dimensions,
                     u32 semaphore_idx, f32 dt)
{
#if 0
    if (g_state_TEST.should_render_game)
    {
        if (is_key_pressed(SYNT_KEY_T))
        {
            g_state_TEST.should_render_game = false;
        }
        else
        {
            game_update(region, app_state, render_state, dimensions, semaphore_idx,
                        dt);
            return;
        }
    }
#endif
    presist V2 preserved_dimensions = { 0 };
    preserved_dimensions = dimensions;

    V2 mouse_pos;
    {
        i16 x, y;
        syntics_platform_mouse_get_pos(&x, &y);
        mouse_pos = v2f((f32)x, (f32)y);
    }

    {
        presist b8 first = true;
        b8 clicked = is_any_button_clicked(&first);

        AABB_2D* aabbs = g_state_TEST.aabb_options;
        const u32 option_aabb_count = array_size(g_state_TEST.aabb_options);
        for (u32 i = 0; i < option_aabb_count; i++)
        {
            if (point_in_aabb_2d(mouse_pos, &aabbs[i]))
            {
                if (g_state_TEST.colors.current_id != i + 1)
                {
                    g_state_TEST.colors.current_start_color =
                        &g_state_TEST.colors.start_color;

                    g_state_TEST.colors.current_end_color =
                        &g_state_TEST.colors.end_color;

                    g_state_TEST.colors.sec = 0.0f;
                    g_state_TEST.colors.process = 0.0f;
                }
                g_state_TEST.colors.current_id = i + 1;
                test_bed_process_options(i, clicked, dt);
            }
            else
            {
                color_change(i, g_state_TEST.base_font_color);
            }
        }
    }

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
    if (should_rotate_TEST)
    {
        rot += rot_speed_TEST * dt;
    }
    g_state_TEST.rotate_model = m4_rotate(rot, Z);

    syntics_vulkan_buffer_copy_data(
        &g_state_TEST.triangle_list_pipeline.uniform_buffers[semaphore_idx]
             .buffer,
        &g_state_TEST.menu_vp, sizeof(g_state_TEST.menu_vp));

#if 0
    syntics_vulkan_buffer_copy_data(
        &g_state_TEST.line_list_pipeline.uniform_buffers[semaphore_idx].buffer,
        &g_state_TEST.cam.vp, sizeof(g_state_TEST.cam.vp));
#endif
    render_callback(render_state, test_bed_render,
                    (void*)&preserved_dimensions);
    g_state_TEST.gui_ctx.translucentcy = translucentcy_TEST;
    gui_update_begin(&g_state_TEST.gui_ctx, dimensions, semaphore_idx, dt);
    {
        test_update_gui(region, app_state, dt, dimensions);
    }
    gui_update_end(&g_state_TEST.gui_ctx, render_state, frame);
}

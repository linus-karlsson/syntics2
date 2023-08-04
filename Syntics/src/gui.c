#ifndef SY_INCLUDES // only for clangd 
#include "syntics.h"
#endif

#define MAX_SPACE 10000
#define BUTTON_SIZE_MULTI 8.3f

// NOTE: it starts at 2 because the first two AABBs is reserved to the back
// board(meaning the plate behind all the components of a window) and the retractable
// button.
#define AABB_START 2
#define AABB_INDEX array_size(win->_aabbs) + AABB_START

#define X_START 11.0f
#define Y_START 25.0f

#define AABBS_COUNT_GUI 100

#define LEFT_SIDE_HIT 0
#define RIGHT_SIDE_HIT 1
#define BOTTOM_HIT 2

#define LEFT_UPPER_HIT 3
#define LEFT_LOWER_HIT 4

#define RIGHT_UPPER_HIT 5
#define RIGHT_LOWER_HIT 6

#define RESIZE_LEFT 1
#define RESIZE_RIGHT 2
#define RESIZE_TOP 3
#define RESIZE_BUTTOM 4
#define RESIZE_BOTH_RIGHT 5

#define BORDER_THICKNESS 1.0f

#define PADDING 9.0f
#define INDICES_PER_QAUD 6
#define VERTEX_PER_QUAD 4

#define TOTAL_NUM_WINS 3

#define DEFAULT_TEXURE 0
#define TEXT_TEXURE 1

#define HEADER_HEIGHT 30

#define QUADS_PER_WINDOW 2000
#define INDICES_PER_WINDOW QUADS_PER_WINDOW * 6
#define VERTICES_PER_WINDOW QUADS_PER_WINDOW * 4
#define TERM_BUFFER_SIZE QUADS_PER_WINDOW
#define GRAPH_BUFFER_SIZE 1000

#define DEFAULT_TEXTURE_GUI 0
#define FONT_TEXTURE_GUI 1

#define WIN_RETRACTED BIT_1
#define WIN_FIRST BIT_2
#define WIN_GRIDD_START BIT_3
#define WIN_DYN_RESIZE BIT_4
#define WIN_PRESIST_HOLD BIT_5
#define WIN_RESIZE_HOLD BIT_6
#define WIN_TERM BIT_7
#define WIN_GRAPH BIT_8

global b8 ui_hit_GUI;
global b8 ui_hold_GUI;
global b8 ui_input_active_GUI;

Ui_Window ui_win(u32 id)
{
    Ui_Window res = { 0 };
    res._id = id;
    res._start.x = X_START;
    res._start.y = Y_START;
    res._offset.x = res._start.x;
    res._show = 0;
    set_bit(res._flags, WIN_FIRST);
    set_bit(res._flags, WIN_DYN_RESIZE);

    memset(res._input_floats, 0, sizeof(res._input_floats));
    memset(res._input_texts, 0, sizeof(res._input_texts));
    return res;
}

Gui_Context gui(void)
{
    Gui_Context res = { 0 };
    res._cam = cam_3dd();
    res._triangle_list_pipeline.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    res._line_strip_pipeline.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    return res;
}

Hover_Clicked hover_clicked_create(const Ui_Window* win, u32 aabb_index)
{
    Hover_Clicked res = { 0 };
    if (win->_window_index == win->_const_gui_ctx->_window_event_index)
    {
        res.clicked =
            aabb_index == win->_const_gui_ctx->_hover_clicked_index.clicked;
        res.hover = aabb_index == win->_const_gui_ctx->_hover_clicked_index.hover;
    }
    return res;
}

u32 binary_file_parse(Gui_Context* ctx)
{
    stack_begin_scope(stack);

    const char* full_path = path_extend_d1("saved_gui.synt");
    File_Attrib file = { 0 };
    file_read(&file, stack_get(), full_path, "rb");

    Ui_Window* win = NULL;
    f32* values = (f32*)(file.buffer + sizeof(u32));
    u32 num_windows = *((u32*)file.buffer);
    for (u32 i = 0; i < num_windows; i++)
    {
        ASSERT(i < TOTAL_NUM_WINS, "Saved file for gui is wrong");
        win = &ctx->_ui_wins[i];
        win->_recreate = 1;
        unset_bit(win->_flags, WIN_FIRST);

        win->_start.x = *(values + 0 + (4 * i));
        win->_start.y = *(values + 1 + (4 * i));
        win->_dimensions.width = *(values + 2 + (4 * i));
        win->_dimensions.height = *(values + 3 + (4 * i));
    }

    stack_end_scope(stack);
    return num_windows;
}

void binary_file_save(const Gui_Context* ctx)
{
    stack_begin_scope(stack);
    u32 size = sizeof(u32) + (ctx->_wins_count * sizeof(f32) * 4);
    u8* buffer = stack_array(size, u8);

    *((u32*)buffer) = ctx->_wins_count;
    f32* values = (f32*)(buffer + sizeof(u32));
    for (u32 i = 0; i < ctx->_wins_count; i++)
    {
        Ui_Window* win = &ctx->_ui_wins[i];
        *(values + 0 + (4 * i)) = win->_start.x;
        *(values + 1 + (4 * i)) = win->_start.y;
        *(values + 2 + (4 * i)) = win->_dimensions.width;
        *(values + 3 + (4 * i)) = win->_dimensions.height;
    }
    char* full_path = path_extend_d1("saved_gui.synt");
    file_write_entire(full_path, (char*)buffer, size);
    stack_end_scope(stack);
}


void gui_init(Region_Alloc* region, VkDevice device,
              VkPhysicalDevice physical_device, VkCommandPool command_pool,
              VkQueue graphic_queue, const Swap_Chain_Attrib* swap_chain,
              const Platform* platform, u32 num_semaphores, b32 use_save,
              Gui_Context* ctx)
{
    stack_begin_scope(gui_init_stack);

    assert(ctx);
    *ctx = gui();
    ctx->_lookup_table = region_malloc_struct(region, Lookup_Table);
    *ctx->_lookup_table = lookup_table_create(region, TOTAL_NUM_WINS);
    ctx->_ui_wins = region_array(region, TOTAL_NUM_WINS, Ui_Window);
    ctx->_win_handles = region_array(region, TOTAL_NUM_WINS, Lookup_Key);
    ctx->_free_handles = region_array(region, TOTAL_NUM_WINS, u32);
    ctx->_render_order = region_array(region, TOTAL_NUM_WINS, u32);

    ctx->font_color = v4i(1.0f);
    ctx->font = font_file_load(region, "Syntics/res/ArialWhiteSmall.fnt");
    ctx->font.tex_index = FONT_TEXTURE_GUI;

    event_subscribe(&ctx->key_evt, EVT_KEY);
    event_subscribe(&ctx->mouse_evt, EVT_MOUSE);
    event_subscribe(&ctx->wheel_evt, EVT_WHEEL);

    ctx->region = region;

    const char* paths[] = {
        [DEFAULT_TEXTURE_GUI] = "Syntics/res/default.png",
        [FONT_TEXTURE_GUI] = "Syntics/res/ArialWhiteSmall.png",
    };
    u32 num_text = sy_SIZE(paths);
    ctx->_textures = region_array(region, num_text, Texture);
    textures_path_create(device, physical_device, command_pool, graphic_queue, 0,
                         num_text, paths, ctx->_textures);
    array_head(ctx->_textures)->size = num_text;

    ctx->_device = device;
    ctx->_const_swap_chain = swap_chain;
    ctx->_const_platform = platform;

    { // Triangle list
        Graphic_Pipeline* g_p = &ctx->_triangle_list_pipeline;
        *g_p =
            gp_default2(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT);
        graphics_pipeline_create_deluxe(region, device, physical_device,
                                        num_semaphores,
                                        "Syntics/res/shaders/spv/gui.vert.spv",
                                        "Syntics/res/shaders/spv/gui.frag.spv",
                                        swap_chain, ctx->_textures, num_text, g_p);
    }

#if 1
    { // Line strip
        Graphic_Pipeline* g_p = &ctx->_line_strip_pipeline;
        *g_p = gp_default2(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, VK_CULL_MODE_BACK_BIT);
        graphics_pipeline_create_deluxe(region, device, physical_device,
                                        num_semaphores,
                                        "Syntics/res/shaders/spv/gui.vert.spv",
                                        "Syntics/res/shaders/spv/gui_graph.frag.spv",
                                        swap_chain, ctx->_textures, num_text, g_p);
    }
#endif

    { // Main
        Vertex_Buffer* vert = &ctx->_main_vert_idx.vert;
        Index_Buffer* idx = &ctx->_main_vert_idx.idx;

        vert->array = vertex_array_create(region, MAX_SPACE * VERTEX_PER_QUAD);
        idx->array = u32_array_create(stack_get(), MAX_SPACE * INDICES_PER_QAUD);

        indices_generate(&idx->array, 0, MAX_SPACE);

        vertex_index_buffer_create_default1(
            device, physical_device, command_pool, graphic_queue,
            VERTEX_INDEX_VISIBLE_LOCAL, &ctx->_main_vert_idx);
    }
    { // Terminal
        Vertex_Buffer* vert = &ctx->_terminal_vert_idx.vert;
        Index_Buffer* idx = &ctx->_terminal_vert_idx.idx;

        const u32 term_buffer_size = terminal_buffer_size_get();
        vert->array =
            vertex_array_create(region, term_buffer_size * VERTEX_PER_QUAD);
        idx->array =
            u32_array_create(stack_get(), term_buffer_size * INDICES_PER_QAUD);

        indices_generate(&idx->array, 0, term_buffer_size);

        vertex_index_buffer_create_default1(
            device, physical_device, command_pool, graphic_queue,
            VERTEX_INDEX_VISIBLE_LOCAL, &ctx->_terminal_vert_idx);
    }


#if 0
    { // Graph 
        Vertex_Buffer* vert = &ctx->_graph_vert_idx.vert;
        Index_Buffer* idx = &ctx->_graph_vert_idx.idx;

        vert->array = vertex_array_create(region, GRAPH_BUFFER_SIZE);
        idx->array = u32_array_create(stack_get(), GRAPH_BUFFER_SIZE);

        for (u32 i = 0; i < GRAPH_BUFFER_SIZE; i++)
        {
            u32_array_push(&idx->array, i);
        }

        vertex_index_buffer_create_default1(
            device, physical_device, command_pool, graphic_queue,
            VERTEX_INDEX_VISIBLE_LOCAL, &ctx->_graph_vert_idx);
    }
#endif

    for (u32 i = 0; i < TOTAL_NUM_WINS; i++)
    {
        ctx->_ui_wins[i] = ui_win(0);
        ctx->_ui_wins[i]._aabbs = region_array(region, AABBS_COUNT_GUI, AABB_2D);

        ctx->_ui_wins[i]._vertex_array.data = vertex_array_val_ptr(
            &ctx->_main_vert_idx.vert.array, (i * VERTICES_PER_WINDOW));
        ctx->_ui_wins[i]._vertex_array.size = 0;
        ctx->_ui_wins[i]._vertex_array._capacity = VERTICES_PER_WINDOW;

        ctx->_ui_wins[i].font_color = v4i(1.0f);
        ctx->_ui_wins[i].translucentcy = &ctx->translucentcy;
        ctx->_ui_wins[i]._const_gui_ctx = ctx;
        ctx->_render_order[i] = i;
    }
    if (use_save)
    {
        binary_file_parse(ctx);
    }
    ctx->_cam.pos = v3f(0.0f, 0.0f, 0.0f);
    ctx->_cam.ori = v3f(0.0f, 0.0f, 0.0f);
    ctx->_cam.vp.view = m4i(1.0f);

    stack_end_scope(gui_init_stack);
}

void gui_draw(VkCommandBuffer command_buffer, const VkViewport* view_port,
              const VkRect2D* scissor, u32 index_offset, u32 num_indices)
{
    vkCmdSetViewport(command_buffer, 0, 1, view_port);
    vkCmdSetScissor(command_buffer, 0, 1, scissor);
    vkCmdDrawIndexed(command_buffer, num_indices, 1, index_offset, 0, 0);
}

global u32 samples_GUI = 0;
void gui_render(void* data, VkCommandBuffer command_buffer, u32 semaphore_idx)
{
    Gui_Context* ctx = (Gui_Context*)data;
    assert(ctx);

    M4 model_matrix = m4i(1.0f);
    Graphic_Pipeline* trianle_gp = &ctx->_triangle_list_pipeline;
    graphics_pipline_bind(command_buffer, trianle_gp, semaphore_idx);
    push_constant(command_buffer, trianle_gp->layout, &model_matrix, sizeof(model_matrix));
    vertex_index_buffer1_bind(command_buffer, &ctx->_main_vert_idx);

    VkViewport view_port = { 0 };
    view_port.width = (f32)ctx->_const_swap_chain->extent_2D.width;
    view_port.height = (f32)ctx->_const_swap_chain->extent_2D.height;
    view_port.maxDepth = 1.0f;

    VkRect2D whole_screen_scissor = { { (i32)view_port.x, (i32)view_port.y },
                                      { (u32)view_port.width,
                                        (u32)view_port.height } };

    for (u32 i = 0; i < ctx->_wins_count; i++)
    {
        const Ui_Window* win = &ctx->_ui_wins[ctx->_render_order[i]];
        if (win->_show)
        {
// NOTE: if ever in use i do need to rebind the pipeline and so on.
#if 0  
            if (check_bit(win->flags, WIN_GRAPH) && samples_GUI != 0)
            {
                // TODO: because it is a different pipeline, render after the
                // all other windows. Which makes the line appear on top of other
                // windows
                gui_draw(command_buffer, semaphore_idx, &_graph_scissor,
                     ctx->graph_g_pipeline, 0, samples);
            }
#endif
            gui_draw(command_buffer, &view_port, &win->_scissor, win->_index_offset,
                     win->_num_indices);
            if (check_bit(win->_flags, WIN_TERM))
            {
                vertex_index_buffer1_bind(command_buffer, &ctx->_terminal_vert_idx);

                Terminal_Attrib* term = terminal_ptr_get();
                assert(term->buffer);
                gui_draw(command_buffer, &view_port, &term->scissor, 0,
                         term->num_indices);

                vertex_index_buffer1_bind(command_buffer, &ctx->_main_vert_idx);
            }
        }
    }
    if (ctx->_blue_rects_index_offset)
    {
        gui_draw(command_buffer, &view_port, &whole_screen_scissor,
                 ctx->_blue_rects_index_offset,
                 ctx->_docking_display_quad_count * INDICES_PER_QAUD);
    }
}

// NOTE: not needed because of dynamic scissor and view port
void recreate(Region_Alloc* region)
{
}

void gui_update_begin(Gui_Context* ctx, V2 dimensions, u32 semaphore_idx, f32 delta)
{
    ctx->dt = delta;
    ctx->_cam.vp.proj = ortho(0.0f, dimensions.x, 0.0f, dimensions.y, -1.0f, 1.0f);

#if 1
    data_buffer_copy(
        &ctx->_triangle_list_pipeline.uniform_buffers[semaphore_idx].buffer,
        &ctx->_cam.vp, sizeof(ctx->_cam.vp));

    data_buffer_copy(
        &ctx->_line_strip_pipeline.uniform_buffers[semaphore_idx].buffer,
        &ctx->_cam.vp, sizeof(ctx->_cam.vp));
#endif

    ctx->dimensions = dimensions;
    ctx->mouse_pos = v2f((f32)ctx->mouse_evt->mouse_evt.move_evt.pos_x,
                         (f32)ctx->mouse_evt->mouse_evt.move_evt.pos_y);

    ctx->_hover_clicked_index.hover = 0;
    ctx->_hover_clicked_index.clicked = 0;
    static b8 first_clicked = 1;
    const b8 button_clicked = is_any_button_clicked(&first_clicked);
    const u8 action = ctx->mouse_evt->mouse_evt.button_evt.action;
    static b8 should_update = 1;

    ui_input_active_GUI = 0;

    if (should_update)
    {
        for (i32 i = ctx->_wins_count; i >= 0; i--)
        {
            const u32 window_index = ctx->_render_order[i];
            const Ui_Window* win = &ctx->_ui_wins[window_index];

            const u32 aabb_count = array_size(win->_aabbs);
            for (i32 j = aabb_count; j >= 0; j--)
            {
                const AABB_2D* current_aabb = &win->_aabbs[j];
                ui_hit_GUI = point_in_aabb_2d(ctx->mouse_pos, current_aabb);

                if (ui_hit_GUI)
                {
                    // Used to know witch window is having events
                    ctx->_window_event_index = window_index;
                    ctx->_hover_clicked_index.hover = j + AABB_START;
                    if (button_clicked)
                    {
                        ctx->_hover_clicked_index.clicked = j + AABB_START;
                        ui_hold_GUI = 1;
                    }
                    break;
                }
            }
            if (ui_hit_GUI)
            {
                break;
            }
        }
    }
    if (!ui_hit_GUI && action)
    {
        should_update = 0;
        ctx->_hover_clicked_index.clicked = 1;
    }
    else
    {
        should_update = 1;
    }
    if (!action)
    {
        ui_hold_GUI = 0;
    }
    for (u32 i = 0; i < ctx->_wins_count; i++)
    {
        Ui_Window* win = ctx->_ui_wins + i;
        win->_highest_high = (u32)win->_g.y;
        win->_g.x = 0;
        win->_g.y = 0;
        win->_gridd.dimensions[0] = 0;
        win->_gridd.dimensions[1] = 0;
        win->_input_f32_index = 0;
        win->_input_text_index = 0;
        win->_show = 0;
        win->_vertex_array.size = 0;
        array_head(win->_aabbs)->size = 0;
        unset_bit(win->_flags, WIN_TERM);
    }
    ctx->_main_vert_idx.vert.array.size = 0;

    ctx->_terminal_vert_idx.vert.array.size = 0;

    ctx->_docking_display_quad_count = 0;
    ctx->_win_hold_idx = 0;
    ctx->_entity_open_idx = 0;
}

static void dock_blue_set(Gui_Context* ctx, u32 side_hit, V2 pos, V2 size,
                          V2 docked_pos, V2 docked_size)
{
    if (!ctx->_dock_hit[side_hit])
    {
        ctx->_blue_rects[side_hit] = quad_d1(
            &ctx->_docking_display_vertex_array, &ctx->_docking_display_quad_count,
            v3f(pos.x, pos.y, -0.05f), size, v4f(0.1f, 0.1f, 1.0f, 0.5f));
    }
    else
    {
        ctx->_dock_resized_rect = quad_d1(&ctx->_docking_display_vertex_array,
                                          &ctx->_docking_display_quad_count,
                                          v3f(docked_pos.x, docked_pos.y, -0.05f),
                                          docked_size, v4f(0.1f, 0.1f, 1.0f, 0.5f));
        ctx->_dock_resized_rect.id = side_hit;
    }
}

void gui_update_end(Gui_Context* ctx, Render_State* render_state)
{
    if (ctx->_top_bar_presist_hold)
    {
        ctx->_docking_display_vertex_array.data =
            vertex_array_val_ptr(&ctx->_main_vert_idx.vert.array,
                                 (ctx->_wins_count * VERTICES_PER_WINDOW));
        ctx->_docking_display_vertex_array.size = 0;
        ctx->_docking_display_vertex_array._capacity = 3 * VERTEX_PER_QUAD;

        ctx->_blue_rects_index_offset = INDICES_PER_WINDOW * ctx->_wins_count;

        const Ui_Window* win = &ctx->_ui_wins[ctx->_win_hold_idx - 1];
        const V2 blue_side_size = v2f(60.0f, 100.0f);
        f32 fullscreen_offset = 0.0f;
#if 0
        if (!is_fullscreen())
        {
            // This is the size of the top bar.
            fullscreen_offset = 20.0f;
        }
#endif
        const V2 docked_side_pos =
            v2f(win->_dimensions.x, ctx->dimensions.y - fullscreen_offset);
        dock_blue_set(ctx, LEFT_SIDE_HIT,
                      v2f(40.0f, (ctx->dimensions.y * 0.5f) - 50.0f), blue_side_size,
                      v2f(0.0f, fullscreen_offset), docked_side_pos);
        dock_blue_set(
            ctx, RIGHT_SIDE_HIT,
            v2f(ctx->dimensions.x - 100.0f, (ctx->dimensions.y * 0.5f) - 50.0f),
            blue_side_size,
            v2f(ctx->dimensions.x - win->_dimensions.x, fullscreen_offset),
            docked_side_pos);
        dock_blue_set(
            ctx, BOTTOM_HIT,
            v2f((ctx->dimensions.x * 0.5f) - 50.0f, ctx->dimensions.y - 100.0f),
            v2f(100.0f, 60.0f), v2f(0.0f, ctx->dimensions.y - win->_dimensions.y),
            v2f(ctx->dimensions.x, win->_dimensions.y));
    }
    else
    {
        ctx->_blue_rects_index_offset = 0;
    }
    ctx->_win_dock_hit_idx = 0;
    for (u32 i = 0; i < TOTAL_DOCK_HIT_GUI; i++)
    {
        ctx->_dock_hit[i] = point_in_aabb_2d(ctx->mouse_pos, &ctx->_blue_rects[i]);
        if (ctx->_dock_hit[i])
        {
            ctx->_win_dock_hit_idx = ctx->_win_hold_idx;
            break;
        }
    }

    Vertex_Buffer* vb0 = &ctx->_main_vert_idx.vert;
    data_buffer_copy(&vb0->buffer, vb0->array.data, vb0->buffer.size_bytes);

    Vertex_Buffer* vb1 = &ctx->_graph_vert_idx.vert;
    data_buffer_copy(&vb1->buffer, vb1->array.data, vb1->buffer.size_bytes);

    Vertex_Buffer* vb2 = &ctx->_terminal_vert_idx.vert;
    data_buffer_copy(&vb2->buffer, vb2->array.data, vb2->buffer.size_bytes);

    ctx->_wins_count = ctx->_num_wins_frame;
    ctx->_num_wins_frame = 0;

    render_callback(render_state, gui_render, ctx);
}

void change_size(f32* win_dim_to_change, f32* pos_to_change, f32* presist_offset,
                 f32 win_dim, f32 mouse_pos)
{
    f32 change = (*presist_offset - mouse_pos);
    if (win_dim < *win_dim_to_change)
    {
        *pos_to_change -= change;
    }
    *win_dim_to_change += change;
    *presist_offset = mouse_pos;
}

void resize_both_set(Ui_Window* win, V2* presist_offset, V2 mouse_pos)
{
    *presist_offset = mouse_pos;
    set_bit(win->_flags, WIN_RESIZE_HOLD);
}

void resize_set(Ui_Window* win, f32* presist_offset, f32 mouse_pos)
{
    *presist_offset = mouse_pos;
    set_bit(win->_flags, WIN_RESIZE_HOLD);
}

Window_Handle window_create(Gui_Context* ctx)
{
    Lookup_Key key = entry_add(ctx->_lookup_table, ctx->_wins_count);
    u32 index = ctx->_wins_count;
    u32 free_indices = array_size(ctx->_free_handles);
    if (free_indices)
    {
        index = array_pop(ctx->_free_handles);
    }
    array_val(ctx->_win_handles, index) = key;
    array_val(ctx->_ui_wins, ctx->_wins_count)._id = key._row.index;

    ctx->_render_order[ctx->_wins_count++] = index;
    return (Window_Handle)&ctx->_win_handles[index];
}

void window_free(Gui_Context* ctx, Window_Handle handle)
{
    Lookup_Key* key = (Lookup_Key*)handle;
    u32 index = entry_remove(ctx->_lookup_table, *key);

    if (index == 0) return;

    for (u32 i = 0; i < TOTAL_NUM_WINS; i++)
    {
        if (ctx->_win_handles[i]._row.index == key->_row.index &&
            ctx->_win_handles[i]._row.ref_value == key->_row.ref_value)
        {
            array_push(ctx->_free_handles, i);
            break;
        }
    }
    u32 updated_index = ctx->_wins_count - 1;
    if (index != ctx->_wins_count - 1)
    {
        Ui_Window* update_window = ctx->_ui_wins + index;
        *update_window = array_val(ctx->_ui_wins, ctx->_wins_count - 1);
        entry_index_change(ctx->_lookup_table, update_window->_id, index);
    }
    u32 saved_pos = 0;
    for (u32 i = 0; i < ctx->_wins_count; i++)
    {
        if (ctx->_render_order[i] == updated_index)
        {
            ctx->_render_order[i] = index;
        }
        else if (ctx->_render_order[i] == index)
        {
            saved_pos = i;
        }
    }
    for (u32 i = saved_pos; i < ctx->_wins_count - 1; i++)
    {
        ctx->_render_order[i] = ctx->_render_order[i + 1];
    }
    ctx->_wins_count--;
}

Ui_Window* window_begin(Gui_Context* ctx, Window_Handle handle, const char* title,
                        V2 pos)
{
    Lookup_Key* key = (Lookup_Key*)handle;
    u32 index = table_index(ctx->_lookup_table, *key);
    if (index == 0)
    {
        SY_ERROR("Window handle not created");
    }
    Ui_Window* win = &ctx->_ui_wins[--index];
    assert(!win->_active && "Forgot to call end_window()");
    win->_active = true;

    win->_window_index = index;

    if (win->_show == 1)
    {
        SY_ERROR("Window handle already used");
    }
    win->_index_offset = INDICES_PER_WINDOW * index;
    win->_num_indices = 0;
    win->_show = 1;
    unset_bit(win->_flags, WIN_TERM);
    unset_bit(win->_flags, WIN_GRAPH);
    if (check_bit(win->_flags, WIN_FIRST))
    {
        win->_start.x = pos.x + X_START;
        win->_start.y = pos.y + Y_START;
        win->_recreate = 1;
        unset_bit(win->_flags, WIN_FIRST);
    }

    u32 c_rect_index = AABB_INDEX;

    Hover_Clicked hc = hover_clicked_create(win, c_rect_index);
    // Hover_Clicked retract_button =
    // hover_clicked_create(win,_rect_index + 1);
    Hover_Clicked top_bar = hover_clicked_create(win, c_rect_index + 2);
    Hover_Clicked resize_right = { 0 };
    Hover_Clicked resize_left = { 0 };
    Hover_Clicked resize_top = { 0 };
    Hover_Clicked resize_bottom = { 0 };
    Hover_Clicked resize_both_right = { 0 };
    if (!check_bit(win->_flags, WIN_RETRACTED))
    {
        resize_right = hover_clicked_create(win, c_rect_index + 3);
        resize_left = hover_clicked_create(win, c_rect_index + 4);
        resize_top = hover_clicked_create(win, c_rect_index + 5);
        resize_bottom = hover_clicked_create(win, c_rect_index + 6);
        resize_both_right = hover_clicked_create(win, c_rect_index + 7);
    }

    const f32 title_bar_size = 20.0f;

    if (top_bar.clicked)
    {
        u32 saved_pos = 0;
        for (u32 i = 0; i < ctx->_wins_count; i++)
        {
            if (ctx->_render_order[i] == index)
            {
                saved_pos = i;
                break;
            }
        }
        for (u32 i = saved_pos; i < ctx->_wins_count - 1; i++)
        {
            ctx->_render_order[i] = ctx->_render_order[i + 1];
        }
        ctx->_render_order[ctx->_wins_count - 1] = index;
        if (win->_docked)
        {
            win->_start.x =
                (ctx->mouse_pos.x - (win->_size_cache.x * 0.5f)) + X_START;
            win->_dimensions = win->_size_cache;
            win->_docked = 0;
        }
        win->_presist_offset.x = ctx->mouse_pos.x - (win->_start.x);
        win->_presist_offset.y = ctx->mouse_pos.y - (win->_start.y);
        set_bit(win->_flags, WIN_PRESIST_HOLD);
    }
    else if (resize_left.clicked)
    {
        resize_set(win, &win->_presist_offset.x, ctx->mouse_pos.x);
        ctx->_resize_idx = RESIZE_LEFT;
    }
    else if (resize_right.clicked)
    {
        resize_set(win, &win->_presist_offset.x,
                   ctx->mouse_pos.x - win->_dimensions.x);
        ctx->_resize_idx = RESIZE_RIGHT;
    }
    else if (resize_top.clicked)
    {
        resize_set(win, &win->_presist_offset.y, ctx->mouse_pos.y);
        ctx->_resize_idx = RESIZE_TOP;
    }
    else if (resize_bottom.clicked)
    {
        resize_set(win, &win->_presist_offset.y,
                   ctx->mouse_pos.y - win->_dimensions.y);
        ctx->_resize_idx = RESIZE_BUTTOM;
    }
    else if (resize_both_right.clicked)
    {
        resize_both_set(win, &win->_presist_offset,
                        v2_sub(ctx->mouse_pos, win->_dimensions));
        ctx->_resize_idx = RESIZE_BOTH_RIGHT;
    }
    if (check_bit(win->_flags, WIN_PRESIST_HOLD))
    {
        win->_start.x = ctx->mouse_pos.x - win->_presist_offset.x;
        win->_start.y = ctx->mouse_pos.y - win->_presist_offset.y;
        win->_is_holding = 1;
        ctx->_top_bar_presist_hold = 1;
        ctx->_win_hold_idx = win->_window_index + 1;
        unset_bit(win->_flags, WIN_DYN_RESIZE);

        win->_recreate = 1;
    }
    if (!ui_hold_GUI)
    {
        if (hc.hover || top_bar.hover)
        {
            platform_cursor_change(ctx->_const_platform, SYNT_NORMAL_CURSOR);
        }
        unset_bit(win->_flags, WIN_PRESIST_HOLD);
        unset_bit(win->_flags, WIN_RESIZE_HOLD);
        win->_is_holding = 0;
        ctx->_top_bar_presist_hold = 0;
        set_bit(win->_flags, WIN_DYN_RESIZE);
    }
    if (ctx->_win_dock_hit_idx - 1 == win->_window_index)
    {
        if (!ui_hold_GUI)
        {
            win->_start.x = ctx->_dock_resized_rect.min.x + X_START;
            win->_start.y = ctx->_dock_resized_rect.min.y + Y_START;
            if (!win->_docked)
            {
                win->_size_cache = win->_dimensions;
                win->_docked = 1;
            }
            win->_dimensions.x = ctx->_dock_resized_rect.size.x;
            win->_dimensions.y = ctx->_dock_resized_rect.size.y;
        }
    }

#define REZIZE_BAR_SIZE 10.0f

    win->_start.x = clampf32(win->_start.x, X_START,
                             (ctx->dimensions.x) - (win->_dimensions.x - X_START));

    win->_start.y = clampf32(win->_start.y, Y_START,
                             (ctx->dimensions.y) - (win->_dimensions.y - Y_START));

    f32 wide = 0;
    f32 high = 0;
    if (check_bit(win->_flags, WIN_DYN_RESIZE))
    {
        wide = win->_biggest_wide + REZIZE_BAR_SIZE - (win->_start.x - X_START);
        high = ((f32)win->_highest_high * 33.0f) + Y_START + win->_extra_hight;
    }
    if (check_bit(win->_flags, WIN_RESIZE_HOLD))
    {
        win->_is_holding = 1;
        win->_recreate = 1;
        if (ctx->_resize_idx == RESIZE_LEFT)
        {
            change_size(&win->_dimensions.x, &win->_start.x, &win->_presist_offset.x,
                        wide, ctx->mouse_pos.x);
        }
        else if (ctx->_resize_idx == RESIZE_RIGHT)
        {
            win->_dimensions.x = ctx->mouse_pos.x - win->_presist_offset.x;
        }
        else if (ctx->_resize_idx == RESIZE_TOP)
        {
            change_size(&win->_dimensions.y, &win->_start.y, &win->_presist_offset.y,
                        high, ctx->mouse_pos.y);
        }
        else if (ctx->_resize_idx == RESIZE_BUTTOM)
        {
            win->_dimensions.y = ctx->mouse_pos.y - win->_presist_offset.y;
        }
        else if (ctx->_resize_idx == RESIZE_BOTH_RIGHT)
        {
            win->_dimensions.x = ctx->mouse_pos.x - win->_presist_offset.x;
            win->_dimensions.y = ctx->mouse_pos.y - win->_presist_offset.y;
        }
    }
    if (wide > win->_dimensions.x)
    {
        win->_dimensions.x = wide;
        win->_recreate = 1;
    }
    if (high > win->_dimensions.y)
    {
        win->_dimensions.y = high;
        win->_recreate = 1;
    }
#if 0
    if (retract_button.clicked)
    {
        switch_bit(win->_flags, WIN_RETRACTED);
    }
    if (check_bit(win->_flags, WIN_RETRACTED))
    {
        win->_dimensions.y = title_bar_size;
    }
#endif

    // TODO: this is for fullscreen mode, still sucks ass
    win->_dimensions = v2f(clampf32(win->_dimensions.x, 0.0f, ctx->dimensions.x),
                           clampf32(win->_dimensions.y, 0.0f, ctx->dimensions.y));

    if (!check_bit(win->_flags, WIN_RETRACTED) && !ui_hold_GUI)
    {
        if (resize_right.hover || resize_left.hover)
        {
            platform_cursor_change(ctx->_const_platform, SYNT_RESIZE_H_CURSOR);
        }
        else if (resize_top.hover || resize_bottom.hover)
        {
            platform_cursor_change(ctx->_const_platform, SYNT_RESIZE_V_CURSOR);
        }
        else if (resize_both_right.hover)
        {
            platform_cursor_change(ctx->_const_platform, SYNT_RESIZE_NW_CURSOR);
        }
    }

    Vertex_Array* vert = &win->_vertex_array;

    V4 back_bord_color = v4f(0.03f, 0.03f, 0.03f, ctx->translucentcy);
    V3 back_bord_pos = v3f(win->_start.x - X_START, win->_start.y - Y_START, 0.0f);

    AABB_2D back_r = quad_d1(vert, &win->_num_indices, back_bord_pos,
                             win->_dimensions, back_bord_color);
    back_r.id = win->_window_index;
    array_push(win->_aabbs, back_r);

    AABB_2D retract_rect = quad(
        vert, &win->_num_indices,
        v3f(back_bord_pos.x + 10.0f, back_bord_pos.y, 0.0f), v2i(title_bar_size),
        v4f(0.0f, 0.0f, 0.0f, ctx->translucentcy * 0.22f), DEFAULT_TEXURE);
    array_push(win->_aabbs, retract_rect);

    if (win->_recreate)
    {
        win->_scissor.offset.x =
            (u32)clampf32(back_r.min.x, 0.0f, ctx->dimensions.x);
        i32 diff_x = back_r.min.x < 0.0f ? (i32)back_r.min.x : 0;
        win->_scissor.extent.width =
            (u32)clampf32(back_r.size.x + diff_x + 1, 0.0f, ctx->dimensions.x);

        win->_scissor.offset.y = (i32)clampf32_low(back_r.min.y, 0.0f);
        win->_scissor.extent.height =
            (u32)clampf32(back_r.size.y + 1, 0.0f, ctx->dimensions.x);

        win->_recreate = 0;
    }

    V4 border_color = v4f(0.5f, 0.0f, 0.033f, ctx->translucentcy);

    V2 border_H_size = v2f(win->_dimensions.x, BORDER_THICKNESS);
    V2 border_V_size = v2f(BORDER_THICKNESS,
                           win->_dimensions.y - title_bar_size - BORDER_THICKNESS);

    back_bord_pos.y += title_bar_size;

    quad_s(vert, &win->_num_indices, back_bord_pos, border_V_size, border_color,
           DEFAULT_TEXURE, 1.0f);

    back_bord_pos.x += border_H_size.x - BORDER_THICKNESS;

    quad_s(vert, &win->_num_indices, back_bord_pos, border_V_size, border_color,
           DEFAULT_TEXURE, 1.0f);

    back_bord_pos.x -= border_H_size.width - BORDER_THICKNESS;
    back_bord_pos.y += border_V_size.height;

    quad_s(vert, &win->_num_indices, back_bord_pos, border_H_size, border_color,
           DEFAULT_TEXURE, 1.0f);

    // Top bar
    array_push(win->_aabbs,
               quad_s_gradiant_d2(
                   vert, &win->_num_indices,
                   v3f(win->_start.x - X_START, win->_start.y - Y_START, 0.0f),
                   v2f(win->_dimensions.x, title_bar_size),
                   v4f(0.8f, 0.0f, 0.03f, ctx->translucentcy), 0.35f));
    array_back(win->_aabbs)->min.x += title_bar_size + 10.0f;
    array_back(win->_aabbs)->size.x -= title_bar_size + 10.0f;
    array_back(win->_aabbs)->id = win->_window_index;

    if (!check_bit(win->_flags, WIN_RETRACTED))

    {
        AABB_2D r_resize_right = { 0 };
        r_resize_right.min = v2f((win->_start.x - 18.0f) + win->_dimensions.x,
                                 win->_start.y - Y_START);
        r_resize_right.size = v2f(8.0f, win->_dimensions.y - 10.0f);
        r_resize_right.id = win->_window_index;

        AABB_2D r_resize_left = { 0 };
        r_resize_left.min = v2f((win->_start.x - X_START), win->_start.y - Y_START);
        r_resize_left.size = v2f(8.0f, win->_dimensions.y);
        r_resize_left.id = win->_window_index;

        AABB_2D r_resize_top = { 0 };
        r_resize_top.min = v2f((win->_start.x - X_START), (win->_start.y - 37.0f));
        r_resize_top.size = v2f(win->_dimensions.x, 8.0f);
        r_resize_top.id = win->_window_index;

        AABB_2D r_resize_bottom = { 0 };
        r_resize_bottom.min = v2f((win->_start.x - X_START),
                                  (win->_start.y - 32.0f) + win->_dimensions.y);
        r_resize_bottom.size = v2f(win->_dimensions.x - 10.0f, 8.0f);
        r_resize_bottom.id = win->_window_index;

        AABB_2D r_resize_both_right = { 0 };
        r_resize_both_right.min = v2f(r_resize_right.min.x, r_resize_bottom.min.y);
        r_resize_both_right.size = v2i(10.0f);
        r_resize_both_right.id = win->_window_index;

        array_push(win->_aabbs, r_resize_right);
        array_push(win->_aabbs, r_resize_left);
        array_push(win->_aabbs, r_resize_top);
        array_push(win->_aabbs, r_resize_bottom);
        array_push(win->_aabbs, r_resize_both_right);
    }

    if (title && *title)
    {
        u32 title_len = (u32)strlen(title);
        win->_num_indices +=
            text_2D(ctx->font, 1.0f, title, (u32)strlen(title),
                    v3f(win->_start.x - X_START + (win->_dimensions.x / 2.0f) -
                            ((title_len * BUTTON_SIZE_MULTI) / 2),
                        win->_start.y - 22.0f, 0.0f),
                    ctx->font_color, 1.0f, NULL, NULL, vert);
    }

    win->_biggest_wide = 0;

    ctx->_num_wins_frame++;
    return win;
}

void window_end(Ui_Window** win)
{
    (*win)->_num_indices *= INDICES_PER_QAUD;
    (*win)->_gridd.dimensions[0] = 0;
    (*win)->_gridd.dimensions[1] = 0;
    (*win)->_active = false;
    *win = NULL;
}

void window_gridd_begin(Ui_Window* win, u32 x, u32 y)
{
    if (!x) x = 1;
    if (!y) y = 1;

    win->_gridd.dimensions[0] = (f32)x;
    win->_gridd.dimensions[1] += (f32)y;
    set_bit(win->_flags, WIN_GRIDD_START);

    if (win->_biggest_wide < x)
    {
        win->_biggest_wide = (f32)x;
    }
    win->_g.x = 0.0f;
    win->_offset.x = win->_start.x;
}

void window_gridd_end(Ui_Window* win)
{
    if (win->_g.x != 0.0f)
    {
        ++win->_g.y;
    }
    unset_bit(win->_flags, WIN_GRIDD_START);
}

static void set_biggest_wide(Ui_Window* win)
{
    f32 wide = win->_offset.x + win->_last_button_width;
    if (wide > win->_biggest_wide)
    {
        win->_biggest_wide = wide;
    }
    win->_offset.x = win->_start.x;
}

static void misc_update(Ui_Window* win)
{
    if (++win->_g.x == win->_gridd.dimensions[0])
    {
        win->_g.x = 0.0f;
        set_biggest_wide(win);

        if (++win->_g.y >= win->_gridd.dimensions[1])
        {
            unset_bit(win->_flags, WIN_GRIDD_START);
            win->_last_button_width = 0;
        }
    }
}

static f32 calculate_text_advance(const Font* font, const char* buffer, u32 len)
{
    f32 x_advance = 0;
    for (u32 i = 0; i < len; i++)
    {
        Character curr_char = font->characters[(u32)buffer[i]];
        x_advance += (f32)curr_char.x_advance * 1.0f;
    }
    return x_advance;
}

static V4 hand_hover(const Ui_Window* win, V4 color, b32 hover, b8 ui_hold)
{
    if (hover && !ui_hold)
    {
        v4_s_multi_equal(&color, 1.8f);
        platform_cursor_change(win->_const_gui_ctx->_const_platform,
                               SYNT_HAND_CURSOR);
    }
    return color;
}

b8 window_button_add(Ui_Window* win, const char* text)
{
    assert(check_bit(win->_flags, WIN_GRIDD_START));
    if (check_bit(win->_flags, WIN_RETRACTED))
    {
        return 0;
    }
    win->_offset.y = win->_start.y + ((win->_g.y * 30.0f));

    const u32 aabb_index = AABB_INDEX;
    const Hover_Clicked hover_clicked = hover_clicked_create(win, aabb_index);

    const V4 button_color =
        hand_hover(win, v4f(0.7f, 0.0f, 0.033f, *win->translucentcy + 0.2f),
                   hover_clicked.hover, ui_hold_GUI);

#define PADDING_IN 12.0f

    const u32 len = (u32)strlen(text);
    const f32 button_width =
        calculate_text_advance(&win->_const_gui_ctx->font, text, len) + PADDING_IN;

    if (win->_g.x != 0) win->_offset.x += win->_last_button_width + PADDING;
    array_push(win->_aabbs,
               quad_s_gradiant_d1(&win->_vertex_array, &win->_num_indices,
                                  v3f(win->_offset.x, win->_offset.y, 0.0f),
                                  v2f(button_width, 20.0f), button_color));
    array_back(win->_aabbs)->id = win->_window_index;

    if (text && *text)
    {
        win->_num_indices += text_2D(
            win->_const_gui_ctx->font, 1.0f, text, len,
            v3f(win->_offset.x + (PADDING_IN * 0.61f), win->_offset.y + 2.0f, 0.0f),
            win->font_color, 1.0f, NULL, NULL, &win->_vertex_array);
    }

    win->_last_button_width = button_width;
    misc_update(win);

    return hover_clicked.clicked;
}

static b8 is_character_number(u16 key)
{
    switch (key)
    {
        case SYNT_KEY_0:
        case SYNT_KEY_1:
        case SYNT_KEY_2:
        case SYNT_KEY_3:
        case SYNT_KEY_4:
        case SYNT_KEY_5:
        case SYNT_KEY_6:
        case SYNT_KEY_7:
        case SYNT_KEY_8:
        case SYNT_KEY_9:
        case SYNT_KEY_PERIOD:
        case SYNT_KEY_MINUS:
        {
            return 1;
        }
        default:
        {
            return 0;
        }
    }
}

static b8 is_character_letter(u16 key)
{
    switch (key)
    {
        case SYNT_KEY_A:
        case SYNT_KEY_B:
        case SYNT_KEY_C:
        case SYNT_KEY_D:
        case SYNT_KEY_E:
        case SYNT_KEY_F:
        case SYNT_KEY_G:
        case SYNT_KEY_H:
        case SYNT_KEY_I:
        case SYNT_KEY_J:
        case SYNT_KEY_K:
        case SYNT_KEY_L:
        case SYNT_KEY_M:
        case SYNT_KEY_N:
        case SYNT_KEY_O:
        case SYNT_KEY_P:
        case SYNT_KEY_Q:
        case SYNT_KEY_R:
        case SYNT_KEY_S:
        case SYNT_KEY_T:
        case SYNT_KEY_U:
        case SYNT_KEY_V:
        case SYNT_KEY_W:
        case SYNT_KEY_X:
        case SYNT_KEY_Y:
        case SYNT_KEY_Z:
        {
            return 1;
        }
        default:
        {
            return 0;
        }
    }
}

#define input_focused(win, curr_input, clicked, allow_letters, cache_on_leave)      \
    _input_focused(win, &(curr_input)->input, (curr_input)->text,                   \
                   (curr_input)->last_text, sy_SIZE((curr_input)->text), clicked,   \
                   allow_letters, cache_on_leave)
static b8 _input_focused(Ui_Window* win, Input* curr_input, char* text,
                         char* last_text, u32 text_size, b8 clicked,
                         b8 allow_letters, b8 cache_on_leave)
{
    b8 result = 1;
    if (clicked || curr_input->presist_clicked)
    {
        curr_input->presist_clicked = 1;

        curr_input->curr_index =
            curr_input->highlight_on ? 0 : curr_input->buffer_size;

        Events* key_evt = win->_const_gui_ctx->key_evt;
        if (key_evt->activated && key_evt->key_evt.action)
        {
            curr_input->highlight_on = 0;

            u16 key = key_evt->key_evt.key;
            char letter;
            if (key == SYNT_KEY_ENTER)
            {
                curr_input->presist_clicked = 0;
                result = 0;
                curr_input->buffer_size = curr_input->curr_index;
            }
            else if (key == SYNT_KEY_BACKSPACE)
            {
                text[curr_input->curr_index != 0 ? --curr_input->curr_index : 0] =
                    '\0';
                curr_input->buffer_size = curr_input->curr_index;
            }
            else if (key != SYNT_KEY_CAPS)
            {
                b8 is_letter = 0;
                if (allow_letters)
                {
                    is_letter = is_character_letter(key);
                }
                b8 is_number = is_character_number(key);
                if (is_letter || is_number || key == SYNT_KEY_SPACE ||
                    key == SYNT_KEY_APOSTROPHE)
                {
                    letter = (char)code_to_ascii(key);
                    int repeats = 1;
                    if (!is_number)
                    {
                        if (key == SYNT_KEY_TAB)
                        {
                            letter = ' ';
                            repeats = 4;
                        }
                        else if (is_letter && !is_caps_on())
                        {
                            letter ^= 0x20;
                        }
                    }
                    for (int i = 0; i < repeats; i++)
                    {
                        if (curr_input->curr_index < text_size - 1)
                        {
                            text[curr_input->curr_index++] = letter;
                        }
                    }
                    text[curr_input->curr_index] = '\0';
                }
                curr_input->buffer_size = curr_input->curr_index;
            }
        }
        if (!clicked && win->_const_gui_ctx->_hover_clicked_index.clicked)
        {
            if (!cache_on_leave)
            {
                memcpy(text, last_text, text_size);
            }
            curr_input->presist_clicked = 0;

            curr_input->highlight_on = 0;
        }
    }
    ui_input_active_GUI |= curr_input->presist_clicked;
    return result;
}

#define render_input(win, curr_input, input_color, text_color, min)                 \
    _render_input(win, &(curr_input)->input, (curr_input)->text, input_color,       \
                  text_color, min)
u32 _render_input(Ui_Window* win, Input* curr_input, const char* text,
                  V4 input_color, V4 text_color, f32 min)
{
    win->_offset.y = win->_start.y + ((win->_g.y * 30.0f));

    f32 x_advance = 0;
    size_t len = strlen(text);
    for (size_t i = 0; i < len; i++)
    {
        Character curr_char = win->_const_gui_ctx->font.characters[(size_t)text[i]];
        x_advance += (f32)curr_char.x_advance * 1.0f;
    }

    f32 input_width = x_advance + 5.0f;

    if (input_width < min)
    {
        input_width = min;
    }
#if 0
    if (win->_last_button_width < min)
    {
        win->_last_button_width = min;
    }
#endif
    if (win->_g.x) win->_offset.x += win->_last_button_width + PADDING;

    array_push(win->_aabbs,
               quad_s_gradiant_d1(&win->_vertex_array, &win->_num_indices,
                                  v3f(win->_offset.x, win->_offset.y, 0.0f),
                                  v2f(input_width, 20.0f), input_color));
    array_back(win->_aabbs)->id = win->_window_index;

    if (curr_input->highlight_on && len > 0)
    {
        quad_d1(&win->_vertex_array, &win->_num_indices,
                v3f(win->_offset.x + 2.5f, win->_offset.y + 2.0f, 0.0f),
                v2f(x_advance, 16.0f), v4f(0.0f, 0.0f, 1.0f, 0.7f));
    }
#if 1
    // Blinking cursor
    else if (curr_input->presist_clicked)
    {
        curr_input->time += win->_const_gui_ctx->dt;
        if (curr_input->time >= 0.4f || curr_input->highlight_on)
        {
            quad_d1(
                &win->_vertex_array, &win->_num_indices,
                v3f(win->_offset.x + x_advance + 1.0f, win->_offset.y + 2.0f, 0.0f),
                v2f(2.0f, 16.0f), text_color);

            curr_input->time = curr_input->time >= 0.8f ? 0 : curr_input->time;
        }
    }
#else
    // Non blinking
    else if (curr_input->presist_clicked)
    {
        quad(&gui_ctx.g_pipline.vert_buffer.data, &win->_num_indices,
             { win->_offset.x + x_advance + 1.0f, win->_offset.y + 2.0f, -0.05f },
             V2(2.0f, 16.0f), text_color);
    }

#endif

    win->_num_indices +=
        text_2D(win->_const_gui_ctx->font, 1.0f, text, (u32)len,
                v3f(win->_offset.x + 3.0f, win->_offset.y + 2.0f, 0.0f), text_color,
                1.0f, NULL, NULL, &win->_vertex_array);

    win->_last_button_width = input_width;

    return (u32)len;
}

#define window_input_float_add_d(win, input, min, max)                              \
    window_input_float_add(win, input, min, max, (max - min) * 0.4f)
b8 window_input_float_add(Ui_Window* win, f32* input, f32 min, f32 max, f32 speed)
{
    if (!check_bit(win->_flags, WIN_GRIDD_START))
    {
        SY_ERROR("Gridd overflow or is not started\n");
        return 0;
    }
    if (check_bit(win->_flags, WIN_RETRACTED))
    {
        return 0;
    }
    const u32 aabb_index = AABB_INDEX;
    const Hover_Clicked hover_clicked = hover_clicked_create(win, aabb_index);

    Input_Float* curr_input = &win->_input_floats[win->_input_f32_index];

    curr_input->input.min = min;
    curr_input->input.max = max;

    if (curr_input->input.presist_hold ||
        ((hover_clicked.hover && ui_hold_GUI) && !win->_is_holding))
    {
        const int16 mouse_x =
            win->_const_gui_ctx->mouse_evt->mouse_evt.move_evt.pos_x;

        static int16 last_x = 0;

        b8 moved = 0;
        if (!hover_clicked.clicked)
        {
            if (last_x < mouse_x)
            {
                if (!curr_input->input.highlight_on)
                {
                    f32 multiplier = (f32)(mouse_x - last_x);
                    *input += speed * multiplier * win->_const_gui_ctx->dt;
                }
                moved = 1;
            }
            else if (last_x > mouse_x)
            {
                if (!curr_input->input.highlight_on)
                {
                    f32 multiplier = (f32)(last_x - mouse_x);
                    *input -= speed * multiplier * win->_const_gui_ctx->dt;
                }
                moved = 1;
            }
        }
        if (moved)
        {
            curr_input->input.frames_moved++;

            if (curr_input->input.frames_moved == 12)
            {
                curr_input->input.highlight_on = 0;
                curr_input->input.curr_index = 0;
                curr_input->input.presist_clicked = 0;
                curr_input->input.frames_moved = 0;
            }
            if (!curr_input->input.highlight_on)
            {
                *input = clampf32(*input, min, max);
                val_to_str(curr_input->text, "%f", *input);
            }
            memcpy(curr_input->last_text, curr_input->text,
                   sizeof(curr_input->last_text));
        }
        last_x = mouse_x;

        curr_input->input.presist_hold = 1;
        win->_is_holding = 1;
        platform_cursor_change(win->_const_gui_ctx->_const_platform,
                               SYNT_RESIZE_H_CURSOR);
    }
    if (!ui_hold_GUI)
    {
        if (curr_input->input.presist_hold)
        {
            platform_cursor_change(win->_const_gui_ctx->_const_platform,
                                   SYNT_NORMAL_CURSOR);
        }
        curr_input->input.presist_hold = 0;
        win->_is_holding = 0;
        curr_input->input.frames_moved = 0;
    }
    if (hover_clicked.clicked)
    {
        *input = clampf32(*input, min, max);
        val_to_str(curr_input->text, "%f", *input);
        curr_input->input.highlight_on = 1;
    }
    if (!input_focused(win, curr_input, hover_clicked.clicked, 0, 0))
    {
        *input = (f32)atof(curr_input->text);
        *input = clampf32(*input, min, max);
        val_to_str(curr_input->text, "%f", *input);

        memcpy(curr_input->last_text, curr_input->text,
               sizeof(curr_input->last_text));
    }
    V4 input_color = v4f(0.0f, 0.5f, 0.033f, *win->translucentcy + 0.3f);
    render_input(win, curr_input, input_color, win->font_color, 50.0f);
    assert(win->_input_f32_index < sy_SIZE(win->_input_floats));
    win->_input_f32_index++;
    misc_update(win);
    return hover_clicked.clicked;
}

b8 window_text_input_add(Ui_Window* win, char* ptr_to_text, u32* size)
{
    b8 result = 0;
    if (check_bit(win->_flags, WIN_RETRACTED))
    {
        return result;
    }
    Input_Text* curr_input = &win->_input_texts[win->_input_text_index];
    curr_input->input.max = 100;

    const u32 aabb_index = AABB_INDEX;
    const Hover_Clicked hover_clicked = hover_clicked_create(win, aabb_index);

    if (hover_clicked.clicked)
    {
        curr_input->input.highlight_on = curr_input->input.highlight_on ? 0 : 1;
    }
    result = !input_focused(win, curr_input, hover_clicked.clicked, 1, 1);

    V4 input_color = v4f(1.0f, 1.0f, 1.0f, *win->translucentcy);
    V4 text_color = v4f(0.0f, 0.0f, 0.0f, 1.0f);
    u32 len = render_input(win, curr_input, input_color, text_color, 100.0f);

    if (ptr_to_text)
    {
        memcpy(ptr_to_text, curr_input->text, len);
    }
    if (size)
    {
        *size = len;
    }

    assert(win->_input_text_index < sy_SIZE(win->_input_texts));
    win->_input_text_index++;
    misc_update(win);
    return result;
}

void window_text_add(Ui_Window* win, const char* text)
{
    if (check_bit(win->_flags, WIN_RETRACTED))
    {
        return;
    }
    win->_offset.y = win->_start.y + ((win->_g.y * 30.0f));
#if 0
    if (win->_last_button_width < 50.0f)
    {
        win->_last_button_width = 50.0f;
    }
#endif
    if (win->_g.x) win->_offset.x += win->_last_button_width + 10.0f;
    f32 x_advance = 0;
    if (text && *text)
    {
#if 0
        win->_num_indices += text_2D_ttf(ui_state.font_ttf, text,
                           v3f(win->_x_offset_button + 2.0f,
                                win->_START.y + 0.0f + (win->_g.y * 30.0f), -0.1f),
                           1.0f, &ui_state.g_pipline.vert_buffer.data);
#endif
        u32 len = (u32)strlen(text);
        win->_num_indices +=
            text_2D(win->_const_gui_ctx->font, 1.0f, text, len,
                    v3f(win->_offset.x + 2.0f, win->_offset.y + 2.0f, 0.0f),
                    win->font_color, 1.0f, NULL, &x_advance, &win->_vertex_array);
    }
    win->_last_button_width = x_advance;
    misc_update(win);
}

static u32 new_lines = 0;

static u32 buffer_flush(void** s_buffer, u32 size_bytes, f32 multiplier)
{
    ASSERT(multiplier < 1.0f, "");
    u32 new_size = (u32)((f32)(size_bytes)*multiplier);
    u32 bytes_to_remove = size_bytes - new_size;

    u8* ptr = (u8*)(*s_buffer) + bytes_to_remove;
    memcpy(*s_buffer, ptr, bytes_to_remove);

    return new_size;
}

static u32 graph_flush(Gui_Context* ctx)
{
    Vertex_Array* array = &ctx->_graph_vert_idx.vert.array;
    return array->size = buffer_flush((void**)&array->data,
                                      array->size * sizeof(Vertex), 0.75f) /
                         sizeof(Vertex);
}

static void terminal_flush(Terminal_Attrib* term)
{
    Array_Head* head = array_head(term->buffer);
    head->size = buffer_flush((void**)&term->buffer, head->size, 0.5f);

    new_lines = 0;
    for (u32 i = 0; i < head->size; i++)
    {
        if (term->buffer[i] == '\n')
        {
            new_lines++;
        }
    }
}

void terminal_add(Gui_Context* ctx, Terminal_Attrib* term, Ui_Window* win, f32 width,
                  f32 height)
{
    if (check_bit(win->_flags, WIN_RETRACTED))
    {
        return;
    }
    char* buffer = term->buffer;

    Vertex_Array* vert = &win->_vertex_array;

    win->_gridd.dimensions[0] = 0;
    win->_gridd.dimensions[1] = 0;

    window_gridd_begin(win, 3, 1);

    if (window_button_add(win, "Auto"))
    {
        term->auto_scroll = 1;
    }
    static u32 idx_ = 0;
    char temp[][6] = { "Stop", "Start" };
    if (window_button_add(win, temp[idx_]))
    {
        if (term->init)
        {
            sy_print("Printing stopped\n");
            term->init = 0;
        }
        else
        {
            term->init = 1;
            sy_print("Printing Starts...\n");
        }
        idx_++;
        idx_ %= 2;
    }
    if (window_button_add(win, "Flush"))
    {
        terminal_flush(term);
    }

    f32 extra_padding = 8.0f;

#if 0
    V3 pos =
        v3f(win->offset.x + extra_padding + BORDER_THICKNESS,
             win->start.y + 2.0f + BORDER_THICKNESS + (win->g.y * 30.0f), -0.1f);
#endif

    V3 top_left =
        v3f(win->_offset.x - BORDER_THICKNESS,
            win->_start.y + 2.0f + (win->_g.y * 30.0f) - BORDER_THICKNESS, 0.0f);

    f32 part_above_termnal = top_left.y + BORDER_THICKNESS + 5.0f + extra_padding -
                             (win->_start.y - HEADER_HEIGHT);

    static b8 first = 1;
    if (first)
    {
        term->dimensions = v2f(width, height);
        first = 0;
        if (term->dimensions.x > win->_dimensions.x)
        {
            win->_dimensions.x = term->dimensions.x;
        }
    }
    else
    {
        term->dimensions.x = win->_dimensions.x - 20.0f;
        term->dimensions.y = win->_dimensions.y - part_above_termnal;
#if 0
        if (term->dimensions.y + part_above_termnal >= win->_dimensions.y)
        {
            term->dimensions.y = win->_dimensions.y - part_above_termnal;
        }
#endif
    }

    V2 term_H_size = v2f(term->dimensions.x, BORDER_THICKNESS);
    V2 term_V_size =
        v2f(BORDER_THICKNESS, term->dimensions.y + BORDER_THICKNESS + extra_padding);

    V3 term_pos =
        v3f(top_left.x + BORDER_THICKNESS, top_left.y + BORDER_THICKNESS, 0.0f);

    term->scissor.offset.x = (int32)clampf32_low(term_pos.x, 0.0f);
    term->scissor.offset.y = (int32)clampf32_low(term_pos.y, 0.0f);
    term->scissor.extent.width =
        (u32)clampf32_low(term->dimensions.x - BORDER_THICKNESS, 0.0f);
    term->scissor.extent.height =
        (u32)clampf32_low(term->dimensions.y + extra_padding, 0.0f);

    // If term window->should not be clipped to back bord
#if 0
    const b8 clicked = aabb_index == _index_clicked;
    const b8 hover = aabb_index == _index_hover;

    if (clicked)
    {
        term->presist_offset.y = ui_state.mouse_pos.y - term->dimensions.y;
    }
    if (hover)
    {
        change_cursor(SYNT_RESIZE_V_CURSOR);
    }

    if (term->presist_hold || ((hover && _ui_hold) && !_is_holding))
    {
        change_cursor(SYNT_RESIZE_V_CURSOR);

        term->presist_hold = 1;
        _is_holding = 1;

        f32 new_dim = ui_state.mouse_pos.y - term->presist_offset.y;
        if (new_dim + part_above_termnal < win->_dimensions.y)
        {
            term->dimensions.y = new_dim;
        }
    }
    if (!_ui_hold)
    {
        term->presist_hold = 0;
    }
#endif

    V4 border_color = v4f(0.5f, 0.0f, 0.033f, ctx->translucentcy);
    border_add_s_d1(vert, &win->_num_indices, border_color, top_left,
                    v2f(term_H_size.x, term_V_size.y), BORDER_THICKNESS);

    const u32 aabb_index = AABB_INDEX;

    const Hover_Clicked hover_clicked_terminal =
        hover_clicked_create(win, aabb_index);

    array_push(win->_aabbs,
               quad_d1(vert, &win->_num_indices, term_pos,
                       v2f((f32)term->scissor.extent.width,
                           term_V_size.y - BORDER_THICKNESS),
                       v4f(0.005f, 0.005f, 0.005f, ctx->translucentcy)));
    array_back(win->_aabbs)->id = win->_window_index;

    // Text moving upp

    f32 line_height = 0;
    f32 buffer_height = 0;
    static f32 buffer_diff = 0;

    if (hover_clicked_terminal.hover)
    {
        platform_cursor_change(ctx->_const_platform, SYNT_NORMAL_CURSOR);
        if (ctx->wheel_evt->activated)
        {
            term->auto_scroll = 0;
            buffer_diff += ((f32)ctx->wheel_evt->wheel_evt.z_delta * 0.3f);
        }
    }
    if (term->auto_scroll)
    {
        line_height = (f32)ctx->font.line_height * 1.0f;
        buffer_height = line_height * (f32)new_lines;
        buffer_diff = term->dimensions.y - (buffer_height);
    }
    term->num_indices = 0;
    Vertex_Array* term_array = &ctx->_terminal_vert_idx.vert.array;
    u32 buffer_size = array_size(buffer);
    term_pos.x += extra_padding;
    term_pos.y += buffer_diff + extra_padding;
    term->num_indices += text_2D(ctx->font, 1.0f, buffer, buffer_size, term_pos,
                                 ctx->font_color, 1.0f, NULL, NULL, term_array);
    term->num_indices *= INDICES_PER_QAUD;

    win->_last_button_width = width;
    win->_extra_hight = (u32)height;
    misc_update(win);
    set_bit(win->_flags, WIN_TERM);
}

static f32 graph_sec = 1.0f;

static f32 y_values_pixels[GRAPH_BUFFER_SIZE];
static f32 y_values[GRAPH_BUFFER_SIZE];

static f32 max_value = 0.0f;
static f32 min_value = 0.0f;

static b32 graph_stop = 0;

void graph_add(Gui_Context* ctx, Ui_Window* win, f32 value, const char* y_title,
               f32 y_max, f32 y_min, f32 sample_rate, f32 dt)
{
    if (check_bit(win->_flags, WIN_RETRACTED))
    {
        return;
    }
    window_gridd_begin(win, 1, 2);

    window_text_add(win, y_title);

    win->_offset.y = win->_start.y + ((win->_g.y * 30.0f));

    f32 extra_padding = 0.0f;
    V3 top_left = v3f(win->_offset.x - BORDER_THICKNESS,
                      win->_offset.y + extra_padding - BORDER_THICKNESS, 0.0f);

    V2 h_size = v2f(win->_dimensions.x - 100.0f, BORDER_THICKNESS);
    V2 v_size = v2f(BORDER_THICKNESS, 140.0f);

    Vertex_Array* vert = &win->_vertex_array;

    V4 border_color = v4f(0.5f, 0.0f, 0.033f, ctx->translucentcy);
    border_add_s_d1(vert, &win->_num_indices, border_color, top_left,
                    v2f(h_size.x, v_size.y), BORDER_THICKNESS);

    V3 graph_pos = v3f(top_left.x + BORDER_THICKNESS, top_left.y + BORDER_THICKNESS,
                       top_left.z);
    V2 graph_size =
        v2f(h_size.x - (BORDER_THICKNESS * 2), v_size.y - (BORDER_THICKNESS * 2));

    const u32 aabb_index = AABB_INDEX;
    const Hover_Clicked hover_clicked_graph = hover_clicked_create(win, aabb_index);

    if (hover_clicked_graph.clicked)
    {
        graph_stop = graph_stop ? 0 : 1;
    }

    array_push(win->_aabbs,
               quad_d1(vert, &win->_num_indices, graph_pos, graph_size,
                       v4f(0.005f, 0.005f, 0.005f, ctx->translucentcy)));
    array_back(win->_aabbs)->id = win->_window_index;

    ctx->_graph_scissor.offset.x = (i32)clampf32_low(graph_pos.x, 0.0f);
    ctx->_graph_scissor.offset.y = (i32)clampf32_low(graph_pos.y, 0.0f);
    ctx->_graph_scissor.extent.width = (u32)clampf32_low(graph_size.x, 0.0f);
    ctx->_graph_scissor.extent.height = (u32)clampf32_low(graph_size.y, 0.0f);

    Vertex_Array* graph_vert = &ctx->_graph_vert_idx.vert.array;

    if (y_min >= y_max)
    {
        SY_ERROR("Passing y_min that is grater or equal to y_max, in add_graph()");
    }
    graph_sec += dt;

    static V3 sample_pos = { 0 };

    static const f32 x_advance_per_sec = 20.0f;

    static char buffer[10] = { 0 };

    sample_pos = v3f(top_left.x + h_size.x - 5.0f, sample_pos.y, top_left.z);

    f32 mouse_x = ctx->mouse_pos.x;
    f32 y_value_under_mouse = 0.0f;
    V3 interperlated_pos = v3f(mouse_x, top_left.y, top_left.z);
    for (u32 i = 0; i < samples_GUI; i++)
    {
        graph_vert->data[i].pos.x =
            sample_pos.x -
            ((x_advance_per_sec / sample_rate) * (samples_GUI - 1 - i));
        graph_vert->data[i].pos.y = top_left.y + v_size.y - y_values_pixels[i];
        graph_vert->data[i].pos.z = sample_pos.z;

        if (hover_clicked_graph.hover && i > 0)
        {
            f32 x_values[2] = { graph_vert->data[i - 1].pos.x,
                                graph_vert->data[i].pos.x };
            if (mouse_x <= x_values[1] && mouse_x >= x_values[0])
            {
                f32 normalized = sy_normalize_f32(mouse_x, x_values[0], x_values[1]);

                y_value_under_mouse =
                    sy_lerp(y_values[i - 1], y_values[i], normalized);

                interperlated_pos.x = mouse_x;
                interperlated_pos.y = sy_lerp(graph_vert->data[i - 1].pos.y,
                                              graph_vert->data[i].pos.y, normalized);
            }
        }
    }

    if (graph_sec * sample_rate >= 1.0f)
    {
        if (!graph_stop)
        {
            if (max_value < value)
            {
                max_value = value;
            }
            if (min_value > value)
            {
                min_value = value;
            }
            value = clampf32(value, y_min, y_max);
            // Normalize value and scale it

            f32 stepping_pixels = (value - y_min) / (y_max - y_min) * v_size.y;
            y_values_pixels[samples_GUI] = stepping_pixels;
            y_values[samples_GUI] = value;
            sample_pos.y = top_left.y + v_size.y - stepping_pixels;
            if (samples_GUI == 1)
            {
                y_values_pixels[samples_GUI - 1] = stepping_pixels;
                y_values[samples_GUI - 1] = value;
                graph_vert->data[0].pos = sample_pos;
                min_value = value;
                max_value = value;
            }

            Vertex vertex = { 0 };
            vertex.pos = sample_pos;
            vertex.color = v4i(1.0f);
            Array_Head* head = array_head(graph_vert->data);
            if (head->size >= head->capacity)
            {
                samples_GUI = graph_flush(ctx);
            }
            graph_vert->data[head->size++] = vertex;
            samples_GUI++;
        }
        graph_sec = 0;
        f32_to_str(buffer, 7, y_values[samples_GUI - 1]);
    }

    char buffer_max[10] = { 0 };
    char buffer_min[10] = { 0 };
    f32_to_str(buffer_min, 7, y_min);
    f32_to_str(buffer_max, 7, y_max);

    f32 x_pos_num = top_left.x + h_size.x + 3.0f;
    win->_num_indices += text_2D(
        ctx->font, 1.0f, buffer, (u32)strlen(buffer),
        v3f(x_pos_num, graph_vert->data[samples_GUI - 1].pos.y - 8.0f, sample_pos.z),
        ctx->font_color, 1.0f, NULL, NULL, vert);

    win->_num_indices +=
        text_2D(ctx->font, 1.0f, buffer_max, (u32)strlen(buffer_max),
                v3f(x_pos_num, top_left.y - 3.0f, sample_pos.z), ctx->font_color,
                1.0f, NULL, NULL, vert);

    win->_num_indices +=
        text_2D(ctx->font, 1.0f, buffer_min, (u32)strlen(buffer_min),
                v3f(x_pos_num, top_left.y + v_size.y - 13.0f, sample_pos.z),
                ctx->font_color, 1.0f, NULL, NULL, vert);

    if (hover_clicked_graph.hover && !ui_hold_GUI)
    {
        char buffer_value_under_mouse[10] = { 0 };
        f32_to_str(buffer_value_under_mouse, 7, y_value_under_mouse);
        win->_num_indices +=
            text_2D(ctx->font, 1.0f, buffer_value_under_mouse,
                    (u32)strlen(buffer_value_under_mouse),
                    v3f(mouse_x + 5.0f, top_left.y + 10.0f, sample_pos.z),
                    ctx->font_color, 1.0f, NULL, NULL, vert);

        f32 small_square_size = 10.0f;
        interperlated_pos.x -= small_square_size * 0.5f;
        interperlated_pos.y -= small_square_size * 0.5f;
        interperlated_pos.z = sample_pos.z;
        border_add_s_d0(vert, &win->_num_indices, border_color, interperlated_pos,
                        v2i(small_square_size));

        quad_d1(vert, &win->_num_indices, v3f(mouse_x, top_left.y, sample_pos.z),
                v_size, border_color);
    }

    win->_g.y += v_size.y / 35.0f;
    set_bit(win->_flags, WIN_GRAPH);
    win->_last_button_width = 340.0f;
    misc_update(win);
    window_gridd_end(win);

    window_gridd_begin(win, 2, 1);
    {
        char buffer_max_value[20] = "Max: ";
        char buffer_min_value[20] = "|  Min: ";

        f32_to_str_offset(buffer_max_value, 5, 7, max_value);
        f32_to_str_offset(buffer_min_value, 8, 7, min_value);
        window_text_add(win, buffer_max_value);
        window_text_add(win, buffer_min_value);
    }
    window_gridd_end(win);
}

#if 0
b8 g_open[10] = { 0 };

static b8 entity_showcase(Gui_Context* ctx, Ui_Window* win, Dynamic_Entity_2D* e,
                          char* name)
{
    Vertex_Array* vert = &win->_vertex_array;

    u32 drop_idx = ctx->_entity_open_idx++;

    win->_offset.y = win->_start.y + ((win->_g.y * 30.0f));

    const Hover_Clicked hover_clicked = hover_clicked_create(win, AABB_INDEX);

    if (hover_clicked.clicked)
    {
        b_switch(g_open[drop_idx]);
    }

    V4 color = hand_hover(win, v4f(0.1f, 0.1f, 0.1f, ctx->translucentcy),
                          hover_clicked.hover, ctx->_ui_hold);

    V2 size = v2f(200.0f, 20.0f);
    u32 name_len = (u32)strlen(name);
    f32 name_width = calculate_text_advance(&ctx->font, name, name_len) + PADDING_IN;
    if (name_width >= size.x)
    {
        f32 avg_char_size = name_width / name_len;
        f32 diff = name_width - size.x;
        u32 char_to_remove = (u32)(diff / avg_char_size);
        u32 i = name_len - 1;
        for (u32 j = 0; j < char_to_remove; j++)
        {
            name[i--] = '\0';
            name_len--;
        }
        for (u32 j = 0; j < 3; j++)
        {
            name[i--] = '.';
        }
    }

    array_push(win->_aabbs,
               quad_s_gradiant_d1(vert, &win->_num_indices,
                                  v3f(win->_offset.x, win->_offset.y, 0.0f), size,
                                  color));
    array_back(win->_aabbs)->id = win->_window_index;

    if (name && *name)
    {
        win->_num_indices += text_2D(
            ctx->font, 1.0f, name, name_len,
            v3f(win->_offset.x + (PADDING_IN * 0.61f), win->_offset.y + 2.0f, 0.0f),
            ctx->font_color, 1.0f, NULL, NULL, vert);
    }
    win->_g.y++;

    win->_last_button_width = size.x;
    set_biggest_wide(win);

    return g_open[drop_idx];
}

void edit_show_entity(Dynamic_Entity_2D* e, char* name)
{
    Ui_Window* win = &gui_ctx._ui_wins[gui_ctx._win_idx];
    Vertex_Buffer* vert = &gui_ctx._main_vert_idx.vert;
    if (showcase_entity(e, win, name))
    {
        char buffer[50] = { 0 };
        val_to_str(buffer, "Pos: (x:%.2f, y:%.2f)", e->movement->pos.x,
                   e->movement->pos.y);
        win->offset.y = win->start.y + ((win->g.y * 30.0f));
        u32 buffer_len = (u32)strlen(buffer);

        win->num_indices +=
            text_2D(gui_ctx.font, 1.0f, buffer, buffer_len,
                    v3f(win->offset.x + 2.0f, win->offset.y + 2.0f, 0.0f),
                    gui_ctx.font_color, 1.0f, NULL, NULL, vert->data);

        win->g.y++;

        window_gridd_begin(4, 1);
        {
            add_text("x:");
            add_input_float(&e->movement->pos.x, 0.0f, 5000.0f, 100.0f);
            add_text("y:");
            add_input_float(&e->movement->pos.y, 0.0f, 5000.0f, 100.0f);
        }
        window_gridd_end();

        val_to_str(buffer, "Vel: (x:%.2f, y:%.2f)", e->movement->vel.x,
                   e->movement->vel.y);
        win->offset.y = win->start.y + ((win->g.y * 30.0f));
        buffer_len = (u32)strlen(buffer);

        win->num_indices +=
            text_2D(gui_ctx.font, 1.0f, buffer, buffer_len,
                    v3f(win->offset.x + 2.0f, win->offset.y + 2.0f, 0.0f),
                    gui_ctx.font_color, 1.0f, NULL, NULL, vert->data);

        win->g.y += 1.5f;
    }
}

void show_entity(Dynamic_Entity_2D* e, char* name)
{
    Ui_Window* win = &gui_ctx._ui_wins[gui_ctx._win_idx];
    Vertex_Buffer* vert = &gui_ctx._main_vert_idx.vert;
    if (showcase_entity(e, win, name))
    {
        win->offset.y = win->start.y + ((win->g.y * 30.0f));

        char buffer[100] = { 0 };
        val_to_str(buffer, "Pos: (x:%.2f, y:%.2f)\nVel: (x:%.2f, y:%.2f)",
                   e->movement->pos.x, e->movement->pos.y, e->movement->vel.x,
                   e->movement->vel.y);

        u32 buffer_len = (u32)strlen(buffer);

        win->num_indices +=
            text_2D(gui_ctx.font, 1.0f, buffer, buffer_len,
                    v3f(win->offset.x + 2.0f, win->offset.y + 2.0f, 0.0f),
                    gui_ctx.font_color, 1.0f, NULL, NULL, vert->data);

        win->g.y += 1.5f;
    }
}

void entity_watch_window(void)
{
    Ui_Window* win = &gui_ctx._ui_wins[gui_ctx._win_idx];
    Vertex_Buffer* vert = &gui_ctx._main_vert_idx.vert;

    u32 count = 0;
    u32 i = 0;
    for (Dynamic_Entity_2D e = iterate_entities(&i); e.movement;
         e = iterate_entities(&i))
    {
        win->offset.y = win->start.y + ((win->g.y * 30.0f));
        V3 pos = v3f(win->offset.x, win->offset.y, 0.0f);
        win->g.y++;
        char buffer[100] = { 0 };
        val_to_str(buffer, "Entity%d: pos: (x:%.2f, y:%.2f), vel: (x:%.2f, y:%.2f)",
                   count++, e.movement->pos.x, e.movement->pos.y, e.movement->vel.x,
                   e.movement->vel.y);

        u32 len = (u32)strlen(buffer);
        f32 button_width = calculate_text_advance(buffer, len) + PADDING_IN;

        V4 button_color = v4f(0.7f, 0.0f, 0.033f, ctx->translucentcy);
        quad_s_gradiant_d1(vert->data, &win->num_indices, pos,
                           v2f(button_width, 20.0f), button_color);

        win->num_indices += text_2D(
            gui_ctx.font, 1.0f, buffer, len,
            v3f(win->offset.x + (PADDING_IN * 0.61f), win->offset.y + 2.0f, 0.0f),
            gui_ctx.font_color, 1.0f, NULL, NULL, vert->data);
    }
}
#endif

void gui_destroy(Gui_Context* ctx, VkDevice device, u32 num_semaphores)
{
    binary_file_save(ctx);
    graphic_pipeline_destroy(device, num_semaphores, &ctx->_triangle_list_pipeline);
    graphic_pipeline_destroy(device, num_semaphores, &ctx->_line_strip_pipeline);

    buffer_destroy(device, ctx->_main_vert_idx.vert.buffer);
    buffer_destroy(device, ctx->_main_vert_idx.idx.buffer);

    buffer_destroy(device, ctx->_graph_vert_idx.vert.buffer);
    buffer_destroy(device, ctx->_graph_vert_idx.idx.buffer);

    for (u32 i = 0; i < array_size(ctx->_textures); i++)
    {
        texture_destroy(device, ctx->_textures[i]);
    }
}

b8 is_focus()
{
    return ui_hit_GUI || ui_hold_GUI || ui_input_active_GUI;
}

void sy_print_text(Terminal_Attrib* term, char* text)
{
    if (term->init)
    {
        Array_Head* head = array_head(term->buffer);
        char* temp_text = text;
        for (; *temp_text != '\0'; temp_text++)
        {
#if 1
            if (*temp_text == '\n')
            {
                new_lines++;
            }
#endif
            term->buffer[head->size++] = *temp_text;
            if (head->size >= head->capacity)
            {
                terminal_flush(term);
            }
        }
    }
}


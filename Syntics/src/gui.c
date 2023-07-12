
void draw_pipeline(void (*draw_callback)(void* data, VkCommandBuffer command_buffer,
                                         u32 semaphore_idx),
                   void* data);

#define MAX_SPACE 10000
#define RECTS_START 2
#define RECT_INDEX array_size(gui_context.rects) + RECTS_START
#define BUTTON_SIZE_MULTI 8.3f
#define Y_START_SHADOW ui_wins[win_idx].Y_START + 2.0f
#define X_START 11.0f
#define Y_START 25.0f

#define DEFAULT_TEXTURE_GUI 0
#define FONT_TEXTURE_GUI 1

typedef struct Terminal_Attrib
{
    V2 dimensions;
    VkRect2D scissor;
    u32 index_offset;
    u32 num_indices;
    u32 presist_offset_x;
    u32 presist_offset_y;

    b8 auto_scroll;
    b8 presist_hold; // PADDING: 2 bytes
} Terminal_Attrib;

Terminal_Attrib term_attrib(void)
{
    Terminal_Attrib res = { 0 };
    res.auto_scroll = 1;
    return res;
}

typedef struct Input
{
    u32 curr_index;
    u32 buffer_size;
    u32 frames_moved;

    f32 max;
    f32 min;
    f32 time;

    b8 presist_clicked;
    b8 presist_hold;
    b8 highlight_on; // PADDING: 1 byte
} Input;

typedef struct Input_Text
{
    Input input;
    char text[100];
    char last_text[100];
} Input_Text;

typedef struct Input_Float
{
    Input input;
    char text[15];
    char last_text[15]; // PADDING: 2 bytes
} Input_Float;

Input_Text input_text(void)
{
    Input_Text res = { 0 };
    return res;
}

Input_Float input_float(void)
{
    Input_Float res = { 0 };
    return res;
}

typedef struct Gridd
{
    f32 dimensions[2];
} Gridd;

typedef struct Ui_Window
{
    Input_Float input_floats[10];
    Input_Text input_texts[10];

    VkRect2D scissor;
    Gridd gridd;

    u32 id;

    u32 input_f32_index;
    u32 input_text_index;
    u32 title_len;
    u32 index_offset;
    u32 num_indices;
    u32 extra_hight;
    u32 highest_high;

    V2 dimensions;
    V2 g;
    V2 start;
    V2 offset;
    V2 presist_offset;
    V2 size_cache;

    f32 biggest_wide;
    f32 last_button_width;

#define WIN_RETRACTED BIT_1
#define WIN_FIRST BIT_2
#define WIN_GRIDD_START BIT_3
#define WIN_DYN_RESIZE BIT_4
#define WIN_PRESIST_HOLD BIT_5
#define WIN_RESIZE_HOLD BIT_6
#define WIN_TERM BIT_7
#define WIN_GRAPH BIT_8
    b8 flags;
    b8 docked;
    b8 recreate;
    b8 show;
} Ui_Window;

Ui_Window ui_win(u32 id)
{
    Ui_Window res = { 0 };
    res.id = id;
    res.start.x = X_START;
    res.start.y = Y_START;
    res.offset.x = res.start.x;
    res.show = 0;
    set_bit(res.flags, WIN_FIRST);
    set_bit(res.flags, WIN_DYN_RESIZE);

    u32 size_f32 = (u32)sy_SIZE(res.input_floats);
    for (u32 i = 0; i < size_f32; i++)
    {
        res.input_floats[i] = input_float();
    }
    u32 size_text = (u32)sy_SIZE(res.input_texts);
    for (u32 i = 0; i < size_text; i++)
    {
        res.input_texts[i] = input_text();
    }
    return res;
}

typedef struct Gui
{
    Graphic_Pipeline triangle_list_pipeline;
    Graphic_Pipeline line_strip_pipeline;

    Vertex_Index_Buffer main_vert_idx;
    Vertex_Index_Buffer graph_vert_idx;

    const Swap_Chain_Attrib* swap_chain;
    VkDevice device;
    Region_Alloc* region;

    Events* mouse_evt;
    Events* wheel_evt;
    Events* key_evt;

    Font font;
    Texture* textures;
    Rect2D* rects;

    V2 dimensions;
    V2 mouse_pos;

    Camera_3D cam;

    char* terminal_buffer;
} Gui;

Gui gui(void)
{
    Gui res = { 0 };
    res.cam = cam_3dd();
    res.triangle_list_pipeline.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    res.line_strip_pipeline.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    return res;
}

#define LEFT_SIDE_HIT 0
#define RIGHT_SIDE_HIT 1
#define BOTTOM_HIT 2

#define TOTAL_HIT 3

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

global f32 g_translucentcy = 1.0f;

#define IDX_OFFSET (num_ui_rects * INDICES_PER_RECT)
#define INDICES_PER_RECT 6
#define VERTEX_PER_RECT 4

#define TOTAL_NUM_WINS 3
global Gui gui_context = { 0 };
global Ui_Window* ui_wins = NULL;
global Lookup_Key* win_handles = NULL;
global u32* free_handles = NULL;
global u32* render_order = NULL;

global u32 num_wins = 0;
global Ui_Window* curr_win;

global Terminal_Attrib term = { 0 };

global u32 win_idx = 0;
global u32 num_wins_frame = 0;
global u32 index_hover = 0;
global u32 index_clicked = 0;
global u32 num_ui_rects = 0;
global u32 win_hold_idx = 0;
global u32 win_dock_hit_idx = 0;
global u32 blue_rects_index_offset = 0;
global u32 win_idx_resize_hover = 0;
global u32 resize_idx = 0;
global u32 extra_term = 0;

global u32 focused_index = 0;
global u32 g_entity_open_idx = 0;

global b8 ui_hit = 0;
global b8 ui_hold = 0;
global b8 ui_input_active = 0;
global b8 top_bar_presist_hold = 0;
global b8 is_holding = 0;
global b8 dock_hit[TOTAL_HIT] = { 0 };
global b8 terminal_buffer_init = 0;

global f32 g_dt = 0;

global Rect2D blue_rects[TOTAL_HIT] = { 0 };
global Rect2D dock_resized_rect = { 0 };
global VkRect2D graph_scissor = { 0 };

global V4 font_color;

#define DEFAULT_TEXURE 0
#define TEXT_TEXURE 1

#define HEADER_HEIGHT 30

#define RECTS_PER_WINDOW 2000
#define INDICES_PER_WINDOW RECTS_PER_WINDOW * 6
#define VERTICES_PER_WINDOW RECTS_PER_WINDOW * 4
#define TERM_BUFFER_SIZE RECTS_PER_WINDOW - 10
#define GRAPH_BUFFER_SIZE 1000

global Lookup_Table* lookup_table_GUI = NULL;

typedef struct Hover_Clicked
{
    b32 clicked;
    b32 hover;
} Hover_Clicked;

static Hover_Clicked get_hover_clicked(u32 index)
{
    Hover_Clicked res;
    res.clicked = index == index_clicked;
    res.hover = index == index_hover;
    return res;
}

internal u32 parse_file_binary(void)
{
    stack_begin_scope();

    const char* full_path = extend_path_d1("saved_gui.synt");
    File_Attrib file = { 0 };
    read_file(&file, get_stack(), full_path, "rb");

    Ui_Window* win = NULL;
    f32* values = (f32*)(file.buffer + sizeof(u32));
    u32 num_windows = *((u32*)file.buffer);
    for (u32 i = 0; i < num_windows; i++)
    {
        ASSERT(i < TOTAL_NUM_WINS, "Saved file for gui is wrong");
        win = &ui_wins[i];
        win->recreate = 1;
        unset_bit(win->flags, WIN_FIRST);

        win->start.x = *(values + 0 + (4 * i));
        win->start.y = *(values + 1 + (4 * i));
        win->dimensions.width = *(values + 2 + (4 * i));
        win->dimensions.height = *(values + 3 + (4 * i));
    }

    stack_end_scope();
    return num_windows;
}

internal void save_file_binary(void)
{
    stack_begin_scope();
    u32 size = sizeof(u32) + (win_idx * sizeof(f32) * 4);
    u8* buffer = stack_array(size, u8);

    *((u32*)buffer) = win_idx;
    f32* values = (f32*)(buffer + sizeof(u32));
    for (u32 i = 0; i < win_idx; i++)
    {
        Ui_Window* win = &ui_wins[i];
        *(values + 0 + (4 * i)) = win->start.x;
        *(values + 1 + (4 * i)) = win->start.y;
        *(values + 2 + (4 * i)) = win->dimensions.width;
        *(values + 3 + (4 * i)) = win->dimensions.height;
    }
    char* full_path = extend_path_d1("saved_gui.synt");
    write_entire_file(full_path, (char*)buffer, size);
    stack_end_scope();
}

typedef void* Window_Handle;

void gui_init(Region_Alloc* region, VkDevice device, VkPhysicalDevice physical_device,
          VkCommandPool command_pool, VkQueue graphic_queue,
          const Swap_Chain_Attrib* swap_chain, u32 num_semaphores, b32 use_save)
{
    stack_begin_scope();

    lookup_table_GUI = region_malloc_struct(region, Lookup_Table);
    *lookup_table_GUI = lookup_table_create(region, TOTAL_NUM_WINS);
    ui_wins = region_arrayP(region, TOTAL_NUM_WINS, Ui_Window);
    win_handles = region_arrayP(region, TOTAL_NUM_WINS, Lookup_Key);
    free_handles = region_arrayP(region, TOTAL_NUM_WINS, u32);
    render_order = region_arrayP(region, TOTAL_NUM_WINS, u32);

    for (u32 i = 0; i < TOTAL_NUM_WINS; i++)
    {
        ui_wins[i] = ui_win(0);
        render_order[i] = i;
    }

    if (!terminal_buffer_init)
    {
        gui_context = gui();
        gui_context.terminal_buffer = region_arrayP(region, TERM_BUFFER_SIZE, char);
        terminal_buffer_init = 1;
    }
    font_color = v4i(1.0f);
    term = term_attrib();

    if (use_save)
    {
        parse_file_binary();
    }

    subscribe(&gui_context.key_evt, EVT_KEY);
    subscribe(&gui_context.mouse_evt, EVT_MOUSE);
    subscribe(&gui_context.wheel_evt, EVT_WHEEL);

    gui_context.region = region;

    const char* paths[] = {
        [DEFAULT_TEXTURE_GUI] = "Syntics/res/default.png",
        [FONT_TEXTURE_GUI] = "Syntics/res/ArialWhiteSmall.png",
    };
    u32 num_text = sy_SIZE(paths);
    gui_context.textures = region_arrayP(region, num_text, Texture);
    create_textures_path(device, physical_device, command_pool, graphic_queue, 0,
                         num_text, paths, gui_context.textures);
    array_head(gui_context.textures)->size = num_text;

    gui_context.device = device;
    gui_context.swap_chain = swap_chain;

    { // Triangle list
        Graphic_Pipeline* g_p = &gui_context.triangle_list_pipeline;
        *g_p =
            gp_default2(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT);
        create_graphics_pipeline_deluxe(
            region, device, physical_device, num_semaphores,
            "Syntics/res/shaders/spv/gui.vert.spv",
            "Syntics/res/shaders/spv/gui.frag.spv", swap_chain, gui_context.textures,
            num_text, g_p);
    }

    { // Line strip
        Graphic_Pipeline* g_p = &gui_context.line_strip_pipeline;
        *g_p = gp_default2(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, VK_CULL_MODE_BACK_BIT);
        create_graphics_pipeline_deluxe(
            region, device, physical_device, num_semaphores,
            "Syntics/res/shaders/spv/gui.vert.spv",
            "Syntics/res/shaders/spv/gui_graph.frag.spv", swap_chain,
            gui_context.textures, num_text, g_p);
    }

    { // Main
        Vertex_Buffer* vert = &gui_context.main_vert_idx.vert;
        Index_Buffer* idx = &gui_context.main_vert_idx.idx;

        vert->data = region_arrayP(region, MAX_SPACE * VERTEX_PER_RECT, Vertex);
        idx->data = region_arrayP(get_stack(), MAX_SPACE * INDICES_PER_RECT, u32);

        generate_indices(idx->data, 0, MAX_SPACE);

        create_vertex_index_buffer_default1(
            device, physical_device, command_pool, graphic_queue,
            VERTEX_INDEX_VISIBLE_LOCAL, &gui_context.main_vert_idx);
    }

    { // Graph pipeline;
        Vertex_Buffer* vert = &gui_context.graph_vert_idx.vert;
        Index_Buffer* idx = &gui_context.graph_vert_idx.idx;

        vert->data = region_arrayP(region, GRAPH_BUFFER_SIZE, Vertex);
        idx->data = region_arrayP(get_stack(), GRAPH_BUFFER_SIZE, u32);

        for (u32 i = 0; i < GRAPH_BUFFER_SIZE; i++)
        {
            array_push(idx->data, i);
        }

        create_vertex_index_buffer_default1(
            device, physical_device, command_pool, graphic_queue,
            VERTEX_INDEX_VISIBLE_LOCAL, &gui_context.graph_vert_idx);
    }

    gui_context.font = load_font_file(region, "Syntics/res/ArialWhiteSmall.fnt");
    gui_context.font.tex_index = FONT_TEXTURE_GUI;

    num_ui_rects = 1000;
    gui_context.rects = region_arrayP(region, num_ui_rects, Rect2D);
    num_ui_rects = 0;

    gui_context.cam.pos = v3f(0.0f, 0.0f, 0.0f);
    gui_context.cam.ori = v3f(0.0f, 0.0f, 0.0f);
    gui_context.cam.vp.view = m4i(1.0f);

    stack_end_scope();
}

void init_terminal(Region_Alloc* region)
{
    if (!terminal_buffer_init)
    {
        gui_context = gui();
        gui_context.terminal_buffer = region_arrayP(region, TERM_BUFFER_SIZE, char);
        terminal_buffer_init = 1;
    }
}

void gui_draw(VkCommandBuffer command_buffer, const VkViewport* view_port,
              const VkRect2D* scissor, u32 index_offset, u32 num_indices)
{
    vkCmdSetViewport(command_buffer, 0, 1, view_port);
    vkCmdSetScissor(command_buffer, 0, 1, scissor);
    vkCmdDrawIndexed(command_buffer, num_indices, 1, index_offset, 0, 0);
}

static u32 samples_GUI = 0;
void gui_render(void* data, VkCommandBuffer command_buffer, u32 semaphore_idx)
{
    M4 model_matrix = m4i(1.0f);
    Graphic_Pipeline* trianle_gp = &gui_context.triangle_list_pipeline;
    bind_graphics_pipline(command_buffer, trianle_gp, semaphore_idx);
    push_model(command_buffer, trianle_gp->layout, model_matrix);
    bind_vertex_index_buffer1(command_buffer, &gui_context.main_vert_idx);

    VkViewport view_port = { 0 };
    view_port.width = (f32)gui_context.swap_chain->extent_2D.width;
    view_port.height = (f32)gui_context.swap_chain->extent_2D.height;
    view_port.maxDepth = 1.0f;

    VkRect2D whole_screen_scissor = { { (i32)view_port.x, (i32)view_port.y },
                                      { (u32)view_port.width,
                                        (u32)view_port.height } };

    for (u32 i = 0; i < num_wins; i++)
    {
        const Ui_Window* win = &ui_wins[render_order[i]];
        if (win->show)
        {
// NOTE: if ever in use i do need to rebind the pipeline and so on.
#if 0  
            if (check_bit(win->flags, WIN_GRAPH) && samples_GUI != 0)
            {
                // TODO: because it is a different pipeline, render after the
                // all other windows. Which makes the line appear on top of other
                // windows
                gui_draw(command_buffer, semaphore_idx, &graph_scissor,
                     gui_context.graph_g_pipeline, 0, samples);
            }
#endif
            gui_draw(command_buffer, &view_port, &win->scissor, win->index_offset,
                     win->num_indices);
            if (check_bit(win->flags, WIN_TERM))
            {
                gui_draw(command_buffer, &view_port, &term.scissor,
                         term.index_offset, term.num_indices);
            }
        }
    }
    if (blue_rects_index_offset)
    {
        gui_draw(command_buffer, &view_port, &whole_screen_scissor,
                 blue_rects_index_offset, IDX_OFFSET);
    }
}

// NOTE: not needed because of dynamic scissor and view port
void recreate(Region_Alloc* region)
{
}

void begin_update(Region_Alloc* region, V2 dimensions, u32 semaphore_idx, f32 delta,
                  f32 translucentcy)
{
    g_translucentcy = translucentcy;
    g_dt = delta;
    gui_context.cam.vp.proj =
        ortho(0.0f, dimensions.x, 0.0f, dimensions.y, -1.0f, 1.0f);

#if 1
    copy_data_buffer(
        &gui_context.triangle_list_pipeline.uniform_buffers[semaphore_idx].buffer,
        &gui_context.cam.vp, sizeof(gui_context.cam.vp));

    copy_data_buffer(
        &gui_context.line_strip_pipeline.uniform_buffers[semaphore_idx].buffer,
        &gui_context.cam.vp, sizeof(gui_context.cam.vp));
#endif

    gui_context.dimensions = dimensions;
    gui_context.mouse_pos =
        v2f((f32)gui_context.mouse_evt->mouse_evt.move_evt.pos_x,
            (f32)gui_context.mouse_evt->mouse_evt.move_evt.pos_y);

    index_hover = 0;
    index_clicked = 0;
    static b8 first_clicked = 1;
    const b8 button_clicked = is_any_button_clicked(&first_clicked);
    const u8 action = gui_context.mouse_evt->mouse_evt.button_evt.action;
    static b8 should_update = 1;

    ui_input_active = 0;

    if (should_update)
    {
#if 1
        for (int i = array_size(gui_context.rects) - 1; i >= 0; i--)
        {
            Rect2D* curr_r = &gui_context.rects[i];
            if (curr_r->id == focused_index)
            {
                ui_hit = point_in_rect(gui_context.mouse_pos, curr_r);
                if (ui_hit)
                {
                    index_hover = i + RECTS_START;

                    if (button_clicked)
                    {
                        focused_index = curr_r->id;
                        index_clicked = i + RECTS_START;
                        ui_hold = 1;
                    }
                    break;
                }
            }
        }
        if (!ui_hit)
#endif
        {
            for (int i = array_size(gui_context.rects) - 1; i >= 0; i--)
            {
                Rect2D* curr_r = &gui_context.rects[i];
                ui_hit = point_in_rect(gui_context.mouse_pos, curr_r);
                if (ui_hit)
                {
                    index_hover = i + RECTS_START;

                    if (button_clicked)
                    {
                        focused_index = curr_r->id;
                        index_clicked = i + RECTS_START;
                        ui_hold = 1;
                    }
                    break;
                }
            }
        }
    }
    if (!ui_hit && action)
    {
        should_update = 0;
        index_clicked = 1;
    }
    else
    {
        should_update = 1;
    }
    if (!action)
    {
        ui_hold = 0;
    }
    for (u32 i = 0; i < num_wins; i++)
    {
        ui_wins[i].highest_high = (u32)ui_wins[i].g.y;
        ui_wins[i].g.x = 0;
        ui_wins[i].g.y = 0;
        ui_wins[i].gridd.dimensions[0] = 0;
        ui_wins[i].gridd.dimensions[1] = 0;
        ui_wins[i].input_f32_index = 0;
        ui_wins[i].input_text_index = 0;
        ui_wins[i].show = 0;
    }
    array_head(gui_context.rects)->size = 0;
    array_head(gui_context.main_vert_idx.vert.data)->size = 0;

    num_ui_rects = 0;
    extra_term = 0;
    win_idx = 0;
    win_hold_idx = 0;

    g_entity_open_idx = 0;
}

static void set_dock_blue(u32 side_hit, V2 pos, V2 size, V2 docked_pos,
                          V2 docked_size)
{
    if (!dock_hit[side_hit])
    {
        blue_rects[side_hit] =
            quad_d1(gui_context.main_vert_idx.vert.data, &num_ui_rects,
                    v3f(pos.x, pos.y, -0.05f), size, v4f(0.1f, 0.1f, 1.0f, 0.5f));
    }
    else
    {
        dock_resized_rect =
            quad_d1(gui_context.main_vert_idx.vert.data, &num_ui_rects,
                    v3f(docked_pos.x, docked_pos.y, -0.05f), docked_size,
                    v4f(0.1f, 0.1f, 1.0f, 0.5f));
        dock_resized_rect.id = side_hit;
    }
}

void end_update(void)
{
    if (top_bar_presist_hold)
    {
        blue_rects_index_offset = INDICES_PER_WINDOW * (win_idx + extra_term);
        const Ui_Window* win = &ui_wins[win_hold_idx - 1];
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
            v2f(win->dimensions.x, gui_context.dimensions.y - fullscreen_offset);
        set_dock_blue(LEFT_SIDE_HIT,
                      v2f(40.0f, (gui_context.dimensions.y * 0.5f) - 50.0f),
                      blue_side_size, v2f(0.0f, fullscreen_offset), docked_side_pos);
        set_dock_blue(
            RIGHT_SIDE_HIT,
            v2f(gui_context.dimensions.x - 100.0f,
                (gui_context.dimensions.y * 0.5f) - 50.0f),
            blue_side_size,
            v2f(gui_context.dimensions.x - win->dimensions.x, fullscreen_offset),
            docked_side_pos);
        set_dock_blue(BOTTOM_HIT,
                      v2f((gui_context.dimensions.x * 0.5f) - 50.0f,
                          gui_context.dimensions.y - 100.0f),
                      v2f(100.0f, 60.0f),
                      v2f(0.0f, gui_context.dimensions.y - win->dimensions.y),
                      v2f(gui_context.dimensions.x, win->dimensions.y));
    }
    else
    {
        blue_rects_index_offset = 0;
    }
    win_dock_hit_idx = 0;
    for (u32 i = 0; i < TOTAL_HIT; i++)
    {
        dock_hit[i] = point_in_rect(gui_context.mouse_pos, &blue_rects[i]);
        if (dock_hit[i])
        {
            win_dock_hit_idx = win_hold_idx;
            break;
        }
    }

    Vertex_Buffer* vb0 = &gui_context.main_vert_idx.vert;
    copy_data_buffer(&vb0->buffer, vb0->data, vb0->buffer.size_bytes);

    Vertex_Buffer* vb1 = &gui_context.graph_vert_idx.vert;
    copy_data_buffer(&vb1->buffer, vb1->data, vb1->buffer.size_bytes);

    gui_context.main_vert_idx.idx.curr_size = IDX_OFFSET;

    num_wins = num_wins_frame;
    num_wins_frame = 0;

    draw_pipeline(gui_render, NULL);
}

static void change_size(f32* win_dim_to_change, f32* pos_to_change,
                        f32* presist_offset, f32 win_dim, f32 mouse_pos)
{
    f32 change = (*presist_offset - mouse_pos);
    if (win_dim < *win_dim_to_change)
    {
        *pos_to_change -= change;
    }
    *win_dim_to_change += change;
    *presist_offset = mouse_pos;
}

static void set_resice(Ui_Window* win, f32* presist_offset, f32 mouse_pos,
                       u32 resize_id)
{
    *presist_offset = mouse_pos;
    set_bit(win->flags, WIN_RESIZE_HOLD);
    resize_idx = resize_id;
}

Window_Handle create_window(void)
{
    // + 1 to keep the first entry empty for error checking
    Lookup_Key key = add_entry(lookup_table_GUI, num_wins + 1);
    u32 index = num_wins;
    u32 free_indices = array_size(free_handles);
    if (free_indices)
    {
        index = array_pop(free_handles);
    }
    val(win_handles, index) = key;
    val(ui_wins, num_wins).id = key._row.index;

    render_order[num_wins++] = index;
    return (Window_Handle)&win_handles[index];
}

void free_window(Window_Handle handle)
{
    Lookup_Key* key = (Lookup_Key*)handle;
    u32 index = remove_entry(lookup_table_GUI, *key);

    if (index == 0) return;

    for (u32 i = 0; i < TOTAL_NUM_WINS; i++)
    {
        if (win_handles[i]._row.index == key->_row.index &&
            win_handles[i]._row.ref_value == key->_row.ref_value)
        {
            array_push(free_handles, i);
            break;
        }
    }
    u32 updated_index = num_wins - 1;
    if (index != num_wins - 1)
    {
        Ui_Window* update_window = ui_wins + index;
        *update_window = val(ui_wins, num_wins - 1);
        cange_entry_index(lookup_table_GUI, update_window->id, index);
    }
    u32 saved_pos = 0;
    for (u32 i = 0; i < num_wins; i++)
    {
        if (render_order[i] == updated_index)
        {
            render_order[i] = index;
        }
        else if (render_order[i] == index)
        {
            saved_pos = i;
        }
    }
    for (u32 i = saved_pos; i < num_wins - 1; i++)
    {
        render_order[i] = render_order[i + 1];
    }
    num_wins--;
}

void begin_pane(Window_Handle handle, const char* title, V2 pos)
{
    Lookup_Key* key = (Lookup_Key*)handle;
    u32 index = table_index(lookup_table_GUI, *key);
    if (index == 0)
    {
        SY_ERROR("Window handle not created");
    }
    Ui_Window* win = &ui_wins[--index];
    if (win->show == 1)
    {
        SY_ERROR("Window handle already used");
    }
    curr_win = win;
    win->index_offset = INDICES_PER_WINDOW * (index + extra_term);
    win->num_indices = 0;
    win->title_len = (u32)strlen(title);
    win->show = 1;
    unset_bit(win->flags, WIN_TERM);
    unset_bit(win->flags, WIN_GRAPH);
    if (check_bit(win->flags, WIN_FIRST))
    {
        win->start.x = pos.x + X_START;
        win->start.y = pos.y + Y_START;
        win->recreate = 1;
        unset_bit(win->flags, WIN_FIRST);
    }

    u32 c_rect_index = RECT_INDEX;

    Hover_Clicked hc = get_hover_clicked(c_rect_index);
    // Hover_Clicked retract_button = get_hover_clicked(c_rect_index + 1);
    Hover_Clicked top_bar = get_hover_clicked(c_rect_index + 2);
    Hover_Clicked resize_right = { 0 };
    Hover_Clicked resize_left = { 0 };
    Hover_Clicked resize_top = { 0 };
    Hover_Clicked resize_bottom = { 0 };
    Hover_Clicked resize_both_right = { 0 };
    if (!check_bit(win->flags, WIN_RETRACTED))
    {
        resize_right = get_hover_clicked(c_rect_index + 3);
        resize_left = get_hover_clicked(c_rect_index + 4);
        resize_top = get_hover_clicked(c_rect_index + 5);
        resize_bottom = get_hover_clicked(c_rect_index + 6);
        resize_both_right = get_hover_clicked(c_rect_index + 7);
    }

    const f32 title_bar_size = 20.0f;

    if (top_bar.clicked)
    {
        u32 saved_pos = 0;
        for (u32 i = 0; i < num_wins; i++)
        {
            if (render_order[i] == index)
            {
                saved_pos = i;
                break;
            }
        }
        for (u32 i = saved_pos; i < num_wins - 1; i++)
        {
            render_order[i] = render_order[i + 1];
        }
        render_order[num_wins - 1] = index;
        if (win->docked)
        {
            win->start.x =
                (gui_context.mouse_pos.x - (win->size_cache.x * 0.5f)) + X_START;
            win->dimensions = win->size_cache;
            win->docked = 0;
        }
        win->presist_offset.x = gui_context.mouse_pos.x - (win->start.x);
        win->presist_offset.y = gui_context.mouse_pos.y - (win->start.y);
        set_bit(win->flags, WIN_PRESIST_HOLD);
    }
    else if (resize_left.clicked)
    {
        set_resice(win, &win->presist_offset.x, gui_context.mouse_pos.x,
                   RESIZE_LEFT);
    }
    else if (resize_right.clicked)
    {
        set_resice(win, &win->presist_offset.x,
                   gui_context.mouse_pos.x - win->dimensions.x, RESIZE_RIGHT);
    }
    else if (resize_top.clicked)
    {
        set_resice(win, &win->presist_offset.y, gui_context.mouse_pos.y, RESIZE_TOP);
    }
    else if (resize_bottom.clicked)
    {
        set_resice(win, &win->presist_offset.y,
                   gui_context.mouse_pos.y - win->dimensions.y, RESIZE_BUTTOM);
    }
    else if (resize_both_right.clicked)
    {
        set_resice(win, &win->presist_offset.x,
                   gui_context.mouse_pos.x - win->dimensions.x, RESIZE_BOTH_RIGHT);
        set_resice(win, &win->presist_offset.y,
                   gui_context.mouse_pos.y - win->dimensions.y, RESIZE_BOTH_RIGHT);
    }
    if (check_bit(win->flags, WIN_PRESIST_HOLD))
    {
        win->start.x = gui_context.mouse_pos.x - win->presist_offset.x;
        win->start.y = gui_context.mouse_pos.y - win->presist_offset.y;
        is_holding = 1;
        top_bar_presist_hold = 1;
        win_hold_idx = win_idx + 1;
        unset_bit(win->flags, WIN_DYN_RESIZE);

        win->recreate = 1;
    }
    if (!ui_hold)
    {
        if (hc.hover || top_bar.hover)
        {
            change_cursor(SYNT_NORMAL_CURSOR);
        }
        unset_bit(win->flags, WIN_PRESIST_HOLD);
        unset_bit(win->flags, WIN_RESIZE_HOLD);
        is_holding = 0;
        top_bar_presist_hold = 0;
        set_bit(win->flags, WIN_DYN_RESIZE);
    }
    if (win_dock_hit_idx - 1 == win_idx)
    {
        if (!ui_hold)
        {
            win->start.x = dock_resized_rect.pos.x + X_START;
            win->start.y = dock_resized_rect.pos.y + Y_START;
            if (!win->docked)
            {
                win->size_cache = win->dimensions;
                win->docked = 1;
            }
            win->dimensions.x = dock_resized_rect.size.x;
            win->dimensions.y = dock_resized_rect.size.y;
        }
    }

#define REZIZE_BAR_SIZE 10.0f

    win->start.x =
        clampf32(win->start.x, X_START,
                 (gui_context.dimensions.x) - (win->dimensions.x - X_START));

    win->start.y =
        clampf32(win->start.y, Y_START,
                 (gui_context.dimensions.y) - (win->dimensions.y - Y_START));

    f32 wide = 0;
    f32 high = 0;
    if (check_bit(win->flags, WIN_DYN_RESIZE))
    {
        wide = win->biggest_wide + REZIZE_BAR_SIZE - (win->start.x - X_START);
        high = ((f32)win->highest_high * 33.0f) + Y_START + win->extra_hight;
    }
    if (check_bit(win->flags, WIN_RESIZE_HOLD))
    {
        is_holding = 1;
        win->recreate = 1;
        if (resize_idx == RESIZE_LEFT)
        {
            change_size(&win->dimensions.x, &win->start.x, &win->presist_offset.x,
                        wide, gui_context.mouse_pos.x);
        }
        else if (resize_idx == RESIZE_RIGHT)
        {
            win->dimensions.x = gui_context.mouse_pos.x - win->presist_offset.x;
        }
        else if (resize_idx == RESIZE_TOP)
        {
            change_size(&win->dimensions.y, &win->start.y, &win->presist_offset.y,
                        high, gui_context.mouse_pos.y);
        }
        else if (resize_idx == RESIZE_BUTTOM)
        {
            win->dimensions.y = gui_context.mouse_pos.y - win->presist_offset.y;
        }
        else if (resize_idx == RESIZE_BOTH_RIGHT)
        {
            win->dimensions.x = gui_context.mouse_pos.x - win->presist_offset.x;
            win->dimensions.y = gui_context.mouse_pos.y - win->presist_offset.y;
        }
    }
    if (wide > win->dimensions.x)
    {
        win->dimensions.x = wide;
        win->recreate = 1;
    }
    if (high > win->dimensions.y)
    {
        win->dimensions.y = high;
        win->recreate = 1;
    }
#if 0
    if (retract_button.clicked)
    {
        switch_bit(win->flags, WIN_RETRACTED);
    }
    if (check_bit(win->flags, WIN_RETRACTED))
    {
        win->dimensions.y = title_bar_size;
    }
#endif

    // TODO: this is for fullscreen mode, still sucks ass
    win->dimensions =
        v2f(clampf32(win->dimensions.x, 0.0f, gui_context.dimensions.x),
            clampf32(win->dimensions.y, 0.0f, gui_context.dimensions.y));

    // TODO: This needs to be cleaned up, kinda buggy
    if (!check_bit(win->flags, WIN_RETRACTED) && !ui_hold)
    {
        if (resize_right.hover || resize_left.hover)
        {
            win_idx_resize_hover = win_idx + 1;
            change_cursor(SYNT_RESIZE_H_CURSOR);
        }
        else if (resize_top.hover || resize_bottom.hover)
        {
            change_cursor(SYNT_RESIZE_V_CURSOR);
        }
        else if (resize_both_right.hover)
        {
            change_cursor(SYNT_RESIZE_NW_CURSOR);
        }
        else if ((win_idx_resize_hover - 1 == win_idx) && !ui_hold)
        {
            change_cursor(SYNT_NORMAL_CURSOR);
            win_idx_resize_hover = 0;
        }
    }

    Vertex_Buffer* vert = &gui_context.main_vert_idx.vert;

    V4 back_bord_color = v4f(0.03f, 0.03f, 0.03f, g_translucentcy);
    V3 back_bord_pos = v3f(win->start.x - X_START, win->start.y - Y_START, 0.0f);

    Rect2D back_r = quad_d1(vert->data, &win->num_indices, back_bord_pos,
                            win->dimensions, back_bord_color);
    back_r.id = win_idx;
    array_push(gui_context.rects, back_r);

    Rect2D retract_rect = quad(
        vert->data, &win->num_indices,
        v3f(back_bord_pos.x + 10.0f, back_bord_pos.y, 0.0f), v2i(title_bar_size),
        v4f(0.0f, 0.0f, 0.0f, g_translucentcy * 0.22f), DEFAULT_TEXURE);
    array_push(gui_context.rects, retract_rect);

    if (win->recreate)
    {
        win->scissor.offset.x =
            (u32)clampf32(back_r.pos.x, 0.0f, gui_context.dimensions.x);
        i32 diff_x = back_r.pos.x < 0.0f ? (i32)back_r.pos.x : 0;
        win->scissor.extent.width = (u32)clampf32(back_r.size.x + diff_x + 1, 0.0f,
                                                  gui_context.dimensions.x);

        win->scissor.offset.y = (i32)clampf32_low(back_r.pos.y, 0.0f);
        win->scissor.extent.height =
            (u32)clampf32(back_r.size.y + 1, 0.0f, gui_context.dimensions.x);

        win->recreate = 0;
    }

    V4 border_color = v4f(0.5f, 0.0f, 0.033f, g_translucentcy);

    V2 border_H_size = v2f(win->dimensions.x, BORDER_THICKNESS);
    V2 border_V_size =
        v2f(BORDER_THICKNESS, win->dimensions.y - title_bar_size - BORDER_THICKNESS);

    back_bord_pos.y += title_bar_size;

    quad_s(vert->data, &win->num_indices, back_bord_pos, border_V_size, border_color,
           DEFAULT_TEXURE, 1.0f);

    back_bord_pos.x += border_H_size.x - BORDER_THICKNESS;

    quad_s(vert->data, &win->num_indices, back_bord_pos, border_V_size, border_color,
           DEFAULT_TEXURE, 1.0f);

    back_bord_pos.x -= border_H_size.width - BORDER_THICKNESS;
    back_bord_pos.y += border_V_size.height;

    quad_s(vert->data, &win->num_indices, back_bord_pos, border_H_size, border_color,
           DEFAULT_TEXURE, 1.0f);

    // Top bar
    array_push(
        gui_context.rects,
        quad_s_gradiant_d2(vert->data, &win->num_indices,
                           v3f(win->start.x - X_START, win->start.y - Y_START, 0.0f),
                           v2f(win->dimensions.x, title_bar_size),
                           v4f(0.8f, 0.0f, 0.03f, g_translucentcy), 0.35f));
    array_back(gui_context.rects)->pos.x += title_bar_size + 10.0f;
    array_back(gui_context.rects)->size.x -= title_bar_size + 10.0f;
    array_back(gui_context.rects)->id = win_idx;

    if (!check_bit(win->flags, WIN_RETRACTED))

    {
        Rect2D r_resize_right = { 0 };
        r_resize_right.pos =
            v2f((win->start.x - 18.0f) + win->dimensions.x, win->start.y - Y_START);
        r_resize_right.size = v2f(8.0f, win->dimensions.y - 10.0f);
        r_resize_right.id = win_idx;

        Rect2D r_resize_left = { 0 };
        r_resize_left.pos = v2f((win->start.x - X_START), win->start.y - Y_START);
        r_resize_left.size = v2f(8.0f, win->dimensions.y);
        r_resize_left.id = win_idx;

        Rect2D r_resize_top = { 0 };
        r_resize_top.pos = v2f((win->start.x - X_START), (win->start.y - 37.0f));
        r_resize_top.size = v2f(win->dimensions.x, 8.0f);
        r_resize_top.id = win_idx;

        Rect2D r_resize_bottom = { 0 };
        r_resize_bottom.pos = v2f((win->start.x - X_START),
                                  (win->start.y - 32.0f) + win->dimensions.y);
        r_resize_bottom.size = v2f(win->dimensions.x - 10.0f, 8.0f);
        r_resize_bottom.id = win_idx;

        Rect2D r_resize_both_right = { 0 };
        r_resize_both_right.pos = v2f(r_resize_right.pos.x, r_resize_bottom.pos.y);
        r_resize_both_right.size = v2i(10.0f);
        r_resize_both_right.id = win_idx;

        array_push(gui_context.rects, r_resize_right);
        array_push(gui_context.rects, r_resize_left);
        array_push(gui_context.rects, r_resize_top);
        array_push(gui_context.rects, r_resize_bottom);
        array_push(gui_context.rects, r_resize_both_right);
    }

    if (title && *title)
    {
        win->num_indices +=
            text_2D(gui_context.font, 1.0f, title, (u32)strlen(title),
                    v3f(win->start.x - X_START + (win->dimensions.x / 2.0f) -
                            ((win->title_len * BUTTON_SIZE_MULTI) / 2),
                        win->start.y - 22.0f, 0.0f),
                    font_color, 1.0f, NULL, NULL, vert->data);
    }

    win->biggest_wide = 0;

    num_wins_frame++;
}

static void move_to_next_chunk(u32* num_indices)
{
    ASSERT(*num_indices < RECTS_PER_WINDOW, "");

    array_head(gui_context.main_vert_idx.vert.data)->size +=
        (RECTS_PER_WINDOW - *num_indices) * VERTEX_PER_RECT;

    *num_indices *= INDICES_PER_RECT;
}
void end_pane(void)
{
    Ui_Window* win = &ui_wins[win_idx];
    if (!check_bit(win->flags, WIN_TERM))
    {
        move_to_next_chunk(&win->num_indices);
    }

    ++win_idx;

    win->gridd.dimensions[0] = 0;
    win->gridd.dimensions[1] = 0;
}

void begin_gridd(u32 x, u32 y)
{
    if (!x) x = 1;
    if (!y) y = 1;
    Ui_Window* win = &ui_wins[win_idx];

    win->gridd.dimensions[0] = (f32)x;
    win->gridd.dimensions[1] += (f32)y;
    set_bit(win->flags, WIN_GRIDD_START);

    if (win->biggest_wide < x)
    {
        win->biggest_wide = (f32)x;
    }
    win->g.x = 0.0f;
    win->offset.x = win->start.x;
}

void end_gridd(void)
{
    Ui_Window* win = &ui_wins[win_idx];
    if (win->g.x != 0.0f)
    {
        ++win->g.y;
    }
    unset_bit(win->flags, WIN_GRIDD_START);
}

static void set_biggest_wide(Ui_Window* win)
{
    f32 wide = win->offset.x + win->last_button_width;
    if (wide > win->biggest_wide)
    {
        win->biggest_wide = wide;
    }
    win->offset.x = win->start.x;
}

static void update_misc(void)
{
    Ui_Window* win = &ui_wins[win_idx];
    if (++win->g.x == win->gridd.dimensions[0])
    {
        win->g.x = 0.0f;
        set_biggest_wide(win);

        if (++win->g.y >= win->gridd.dimensions[1])
        {
            unset_bit(win->flags, WIN_GRIDD_START);
            win->last_button_width = 0;
        }
    }
}

static f32 calculate_text_advance(const char* buffer, u32 len)
{
    f32 x_advance = 0;
    for (u32 i = 0; i < len; i++)
    {
        Character curr_char = gui_context.font.characters[(u32)buffer[i]];
        x_advance += (f32)curr_char.x_advance * 1.0f;
    }
    return x_advance;
}

static V4 hand_hover(V4 color, b32 hover)
{
    if (hover && !ui_hold)
    {
        v4_s_multi_equal(&color, 1.8f);
        color.w = 1.0f;
        change_cursor(SYNT_HAND_CURSOR);
    }
    return color;
}

b8 add_button(const char* text)
{
    Ui_Window* win = &ui_wins[win_idx];
    if (!check_bit(win->flags, WIN_GRIDD_START))
    {
        SY_ERROR("Gridd overflow or is not started");
        return 0;
    }
    if (check_bit(win->flags, WIN_RETRACTED))
    {
        return 0;
    }
    win->offset.y = win->start.y + ((win->g.y * 30.0f));

    u32 rect_index = RECT_INDEX;
    const b8 clicked = rect_index == index_clicked;
    const b8 hover = rect_index == index_hover;

    V4 button_color = hand_hover(v4f(0.7f, 0.0f, 0.033f, g_translucentcy), hover);

#define PADDING_IN 12.0f

    u32 len = (u32)strlen(text);
    f32 button_width = calculate_text_advance(text, len) + PADDING_IN;

    if (win->g.x != 0) win->offset.x += win->last_button_width + PADDING;
    array_push(gui_context.rects,
              quad_s_gradiant_d1(gui_context.main_vert_idx.vert.data,
                                 &win->num_indices,
                                 v3f(win->offset.x, win->offset.y, 0.0f),
                                 v2f(button_width, 20.0f), button_color));
    array_back(gui_context.rects)->id = win_idx;

    if (text && *text)
    {
        win->num_indices += text_2D(
            gui_context.font, 1.0f, text, len,
            v3f(win->offset.x + (PADDING_IN * 0.61f), win->offset.y + 2.0f, 0.0f),
            font_color, 1.0f, NULL, NULL, gui_context.main_vert_idx.vert.data);
    }

    win->last_button_width = button_width;
    update_misc();

    return clicked;
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

#define input_focused(curr_input, clicked, allow_letters, cache_on_leave)           \
    _input_focused(&(curr_input)->input, (curr_input)->text,                        \
                   (curr_input)->last_text, sy_SIZE((curr_input)->text), clicked,   \
                   allow_letters, cache_on_leave)
static b8 _input_focused(Input* curr_input, char* text, char* last_text,
                         u32 text_size, b8 clicked, b8 allow_letters,
                         b8 cache_on_leave)
{
    b8 result = 1;
    if (clicked || curr_input->presist_clicked)
    {
        curr_input->presist_clicked = 1;

        curr_input->curr_index =
            curr_input->highlight_on ? 0 : curr_input->buffer_size;

        Events* key_evt = gui_context.key_evt;
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
        if (!clicked && index_clicked)
        {
            if (!cache_on_leave)
            {
                memcpy(text, last_text, text_size);
            }
            curr_input->presist_clicked = 0;

            curr_input->highlight_on = 0;
        }
    }
    ui_input_active |= curr_input->presist_clicked;
    return result;
}

#define render_input(curr_input, win, input_color, text_color, min)                 \
    _render_input(&(curr_input)->input, (curr_input)->text, win, input_color,       \
                  text_color, min)
static u32 _render_input(Input* curr_input, const char* text, Ui_Window* win,
                         V4 input_color, V4 text_color, f32 min)
{
    win->offset.y = win->start.y + ((win->g.y * 30.0f));

    f32 x_advance = 0;
    size_t len = strlen(text);
    for (size_t i = 0; i < len; i++)
    {
        Character curr_char = gui_context.font.characters[(size_t)text[i]];
        x_advance += (f32)curr_char.x_advance * 1.0f;
    }

    f32 input_width = x_advance + 5.0f;

    if (input_width < min)
    {
        input_width = min;
    }
#if 0
    if (win->last_button_width < min)
    {
        win->last_button_width = min;
    }
#endif
    if (win->g.x) win->offset.x += win->last_button_width + PADDING;

    Vertex_Buffer* vert = &gui_context.main_vert_idx.vert;

    array_push(gui_context.rects,
              quad_s_gradiant_d1(vert->data, &win->num_indices,
                                 v3f(win->offset.x, win->offset.y, 0.0f),
                                 v2f(input_width, 20.0f), input_color));
    array_back(gui_context.rects)->id = win_idx;

    if (curr_input->highlight_on && len > 0)
    {
        quad_d1(vert->data, &win->num_indices,
                v3f(win->offset.x + 2.5f, win->offset.y + 2.0f, 0.0f),
                v2f(x_advance, 16.0f), v4f(0.0f, 0.0f, 1.0f, 0.7f));
    }
#if 1
    // Blinking cursor
    else if (curr_input->presist_clicked)
    {
        curr_input->time += g_dt;
        if (curr_input->time >= 0.4f || curr_input->highlight_on)
        {
            quad_d1(
                vert->data, &win->num_indices,
                v3f(win->offset.x + x_advance + 1.0f, win->offset.y + 2.0f, 0.0f),
                v2f(2.0f, 16.0f), text_color);

            curr_input->time = curr_input->time >= 0.8f ? 0 : curr_input->time;
        }
    }
#else
    // Non blinking
    else if (curr_input->presist_clicked)
    {
        quad(&gui_context.g_pipline.vert_buffer.data, &win->num_indices,
             { win->offset.x + x_advance + 1.0f, win->offset.y + 2.0f, -0.05f },
             V2(2.0f, 16.0f), text_color);
    }

#endif

    win->num_indices +=
        text_2D(gui_context.font, 1.0f, text, (u32)len,
                v3f(win->offset.x + 3.0f, win->offset.y + 2.0f, 0.0f), text_color,
                1.0f, NULL, NULL, vert->data);

    win->last_button_width = input_width;

    return (u32)len;
}

#define add_input_float_d(input, min, max)                                          \
    add_input_float(input, min, max, (max - min) * 0.4f)
b8 add_input_float(f32* input, f32 min, f32 max, f32 speed)
{
    Ui_Window* win = &ui_wins[win_idx];
    if (!check_bit(win->flags, WIN_GRIDD_START))
    {
        SY_ERROR("Gridd overflow or is not started\n");
        return 0;
    }
    if (check_bit(win->flags, WIN_RETRACTED))
    {
        return 0;
    }
    u32 rect_index = array_size(gui_context.rects) + RECTS_START;
    const b8 clicked = rect_index == index_clicked;
    const b8 hover = rect_index == index_hover;

    Input_Float* curr_input = &win->input_floats[win->input_f32_index];

    curr_input->input.min = min;
    curr_input->input.max = max;

    if (curr_input->input.presist_hold || ((hover && ui_hold) && !is_holding))
    {
        const int16 mouse_x = gui_context.mouse_evt->mouse_evt.move_evt.pos_x;

        static int16 last_x = 0;

        b8 moved = 0;
        if (!clicked)
        {
            if (last_x < mouse_x)
            {
                if (!curr_input->input.highlight_on)
                {
                    f32 multiplier = (f32)(mouse_x - last_x);
                    *input += speed * multiplier * g_dt;
                }
                moved = 1;
            }
            else if (last_x > mouse_x)
            {
                if (!curr_input->input.highlight_on)
                {
                    f32 multiplier = (f32)(last_x - mouse_x);
                    *input -= speed * multiplier * g_dt;
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
        is_holding = 1;
        change_cursor(SYNT_RESIZE_H_CURSOR);
    }
    if (!ui_hold)
    {
        if (curr_input->input.presist_hold)
        {
            change_cursor(SYNT_NORMAL_CURSOR);
        }
        curr_input->input.presist_hold = 0;
        is_holding = 0;
        curr_input->input.frames_moved = 0;
    }
    if (clicked)
    {
        *input = clampf32(*input, min, max);
        val_to_str(curr_input->text, "%f", *input);
        curr_input->input.highlight_on = 1;
    }
    if (!input_focused(curr_input, clicked, 0, 0))
    {
        *input = (f32)atof(curr_input->text);
        *input = clampf32(*input, min, max);
        val_to_str(curr_input->text, "%f", *input);

        memcpy(curr_input->last_text, curr_input->text,
               sizeof(curr_input->last_text));
    }
    V4 input_color = v4f(0.0f, 0.5f, 0.033f, g_translucentcy);
    render_input(curr_input, win, input_color, font_color, 50.0f);
    win->input_f32_index++;
    update_misc();
    return clicked;
}

b8 add_input_text(char* ptr_to_text, u32* size)
{
    Ui_Window* win = &ui_wins[win_idx];
    b8 result = 0;
    if (check_bit(win->flags, WIN_RETRACTED))
    {
        return result;
    }
    Input_Text* curr_input = &win->input_texts[win->input_text_index];
    curr_input->input.max = 100;

    u32 rect_index = RECT_INDEX;
    const b8 clicked = rect_index == index_clicked;
    // const b8 hover = rect_index == index_hover;

    if (clicked)
    {
        curr_input->input.highlight_on = curr_input->input.highlight_on ? 0 : 1;
    }
    result = !input_focused(curr_input, clicked, 1, 1);

    V4 input_color = v4f(1.0f, 1.0f, 1.0f, g_translucentcy);
    V4 text_color = v4f(0.0f, 0.0f, 0.0f, 1.0f);
    u32 len = render_input(curr_input, win, input_color, text_color, 100.0f);

    if (ptr_to_text)
    {
        memcpy(ptr_to_text, curr_input->text, len);
    }
    if (size)
    {
        *size = len;
    }

    win->input_text_index++;
    update_misc();
    return result;
}

void add_text(const char* text)
{
    Ui_Window* win = &ui_wins[win_idx];
    if (check_bit(win->flags, WIN_RETRACTED))
    {
        return;
    }
    win->offset.y = win->start.y + ((win->g.y * 30.0f));
#if 0
    if (win->last_button_width < 50.0f)
    {
        win->last_button_width = 50.0f;
    }
#endif
    if (win->g.x) win->offset.x += win->last_button_width + 10.0f;
    f32 x_advance = 0;
    if (text && *text)
    {
#if 0
        win->num_indices += text_2D_ttf(ui_state.font_ttf, text,
                           v3f(win->x_offset_button + 2.0f,
                                win->START.y + 0.0f + (win->g.y * 30.0f), -0.1f),
                           1.0f, &ui_state.g_pipline.vert_buffer.data);
#endif
        u32 len = (u32)strlen(text);
        win->num_indices += text_2D(
            gui_context.font, 1.0f, text, len,
            v3f(win->offset.x + 2.0f, win->offset.y + 2.0f, 0.0f), font_color, 1.0f,
            NULL, &x_advance, gui_context.main_vert_idx.vert.data);
    }
    win->last_button_width = x_advance;
    update_misc();
}

static u32 new_lines = 0;

static u32 flush_buffer(void** s_buffer, u32 size_bytes, f32 multiplier)
{
    ASSERT(multiplier < 1.0f, "");
    u32 new_size = (u32)((f32)(size_bytes)*multiplier);
    u32 bytes_to_remove = size_bytes - new_size;

    u8* ptr = (u8*)(*s_buffer) + bytes_to_remove;
    memcpy(*s_buffer, ptr, bytes_to_remove);

    return new_size;
}

static u32 flush_graph(void)
{
    Vertex* buffer = gui_context.graph_vert_idx.vert.data;
    Array_Head* head = array_head(buffer);
    return head->size =
               flush_buffer((void**)&buffer, head->size * sizeof(Vertex), 0.75f) /
               sizeof(Vertex);
}

static void flush_terminal(void)
{
    Array_Head* head = array_head(gui_context.terminal_buffer);
    head->size =
        flush_buffer((void**)&gui_context.terminal_buffer, head->size, 0.5f);

    new_lines = 0;
    for (u32 i = 0; i < head->size; i++)
    {
        if (gui_context.terminal_buffer[i] == '\n')
        {
            new_lines++;
        }
    }
}

void add_terminal(f32 width, f32 height)
{
    Ui_Window* win = &ui_wins[win_idx];
    if (check_bit(win->flags, WIN_RETRACTED))
    {
        return;
    }
    extra_term += 1;
    char* buffer = gui_context.terminal_buffer;
    Vertex_Buffer* vert = &gui_context.main_vert_idx.vert;

    win->gridd.dimensions[0] = 0;
    win->gridd.dimensions[1] = 0;

    begin_gridd(3, 1);

    if (add_button("Auto"))
    {
        term.auto_scroll = 1;
    }
    static u32 idx_ = 0;
    char temp[][6] = { "Stop", "Start" };
    if (add_button(temp[idx_]))
    {
        if (terminal_buffer_init)
        {
            sy_print("Printing stopped\n");
            terminal_buffer_init = 0;
        }
        else
        {
            terminal_buffer_init = 1;
            sy_print("Printing Starts...\n");
        }
        idx_++;
        idx_ %= 2;
    }
    if (add_button("Flush"))
    {
        flush_terminal();
    }

    f32 extra_padding = 8.0f;

#if 0
    V3 pos =
        v3f(win->offset.x + extra_padding + BORDER_THICKNESS,
             win->start.y + 2.0f + BORDER_THICKNESS + (win->g.y * 30.0f), -0.1f);
#endif

    V3 top_left =
        v3f(win->offset.x - BORDER_THICKNESS,
            win->start.y + 2.0f + (win->g.y * 30.0f) - BORDER_THICKNESS, 0.0f);

    f32 part_above_termnal = top_left.y + BORDER_THICKNESS + 5.0f + extra_padding -
                             (win->start.y - HEADER_HEIGHT);

    static b8 first = 1;
    if (first)
    {
        term.dimensions = v2f(width, height);
        first = 0;
        if (term.dimensions.x > win->dimensions.x)
        {
            win->dimensions.x = term.dimensions.x;
        }
    }
    else
    {
        term.dimensions.x = win->dimensions.x - 20.0f;
        term.dimensions.y = win->dimensions.y - part_above_termnal;
#if 0
        if (term.dimensions.y + part_above_termnal >= win->dimensions.y)
        {
            term.dimensions.y = win->dimensions.y - part_above_termnal;
        }
#endif
    }

    V2 term_H_size = v2f(term.dimensions.x, BORDER_THICKNESS);
    V2 term_V_size =
        v2f(BORDER_THICKNESS, term.dimensions.y + BORDER_THICKNESS + extra_padding);

    V3 term_pos =
        v3f(top_left.x + BORDER_THICKNESS, top_left.y + BORDER_THICKNESS, 0.0f);

    term.scissor.offset.x = (int32)clampf32_low(term_pos.x, 0.0f);
    term.scissor.offset.y = (int32)clampf32_low(term_pos.y, 0.0f);
    term.scissor.extent.width =
        (u32)clampf32_low(term.dimensions.x - BORDER_THICKNESS, 0.0f);
    term.scissor.extent.height =
        (u32)clampf32_low(term.dimensions.y + extra_padding, 0.0f);

    // If term window should not be clipped to back bord
#if 0
    const b8 clicked = rect_index == index_clicked;
    const b8 hover = rect_index == index_hover;

    if (clicked)
    {
        term.presist_offset.y = ui_state.mouse_pos.y - term.dimensions.y;
    }
    if (hover)
    {
        change_cursor(SYNT_RESIZE_V_CURSOR);
    }

    if (term.presist_hold || ((hover && ui_hold) && !is_holding))
    {
        change_cursor(SYNT_RESIZE_V_CURSOR);

        term.presist_hold = 1;
        is_holding = 1;

        f32 new_dim = ui_state.mouse_pos.y - term.presist_offset.y;
        if (new_dim + part_above_termnal < win->dimensions.y)
        {
            term.dimensions.y = new_dim;
        }
    }
    if (!ui_hold)
    {
        term.presist_hold = 0;
    }
#endif

    V4 border_color = v4f(0.5f, 0.0f, 0.033f, g_translucentcy);
    add_border_s_d1(vert->data, &win->num_indices, border_color, top_left,
                    v2f(term_H_size.x, term_V_size.y), BORDER_THICKNESS);

    u32 rect_index = RECT_INDEX;
    // const b8 terminal_clicked = rect_index == index_clicked;
    const b8 terminal_hover = rect_index == index_hover;

    array_push(gui_context.rects,
              quad_d1(vert->data, &win->num_indices, term_pos,
                      v2f((f32)term.scissor.extent.width,
                          term_V_size.y - BORDER_THICKNESS),
                      v4f(0.005f, 0.005f, 0.005f, g_translucentcy)));
    array_back(gui_context.rects)->id = win_idx;

    move_to_next_chunk(&win->num_indices);

    term.index_offset = INDICES_PER_WINDOW * (win_idx + extra_term);
    term.num_indices = 0;

    // Text moving upp

    f32 line_height = 0;
    f32 buffer_height = 0;
    static f32 buffer_diff = 0;

    if (terminal_hover)
    {
        change_cursor(SYNT_NORMAL_CURSOR);
        if (gui_context.wheel_evt->activated)
        {
            term.auto_scroll = 0;
            buffer_diff += ((f32)gui_context.wheel_evt->wheel_evt.z_delta * 0.3f);
        }
    }
    if (term.auto_scroll)
    {
        line_height = (f32)gui_context.font.line_height * 1.0f;
        buffer_height = line_height * (f32)new_lines;
        buffer_diff = term.dimensions.y - (buffer_height);
    }
    // new_lines = 0;

    u32 buffer_size = array_size(buffer);
    term_pos.x += extra_padding;
    term_pos.y += buffer_diff + extra_padding;
    term.num_indices += text_2D(gui_context.font, 1.0f, buffer, buffer_size,
                                term_pos, font_color, 1.0f, NULL, NULL, vert->data);

    move_to_next_chunk(&term.num_indices);

    win->last_button_width = width;
    win->extra_hight = (u32)height;
    update_misc();
    set_bit(win->flags, WIN_TERM);
}

static f32 graph_sec = 1.0f;

static f32 y_values_pixels[GRAPH_BUFFER_SIZE];
static f32 y_values[GRAPH_BUFFER_SIZE];

static f32 max_value = 0.0f;
static f32 min_value = 0.0f;

static b32 graph_stop = 0;

void add_graph(f32 value, const char* y_title, f32 y_max, f32 y_min, f32 sample_rate,
               f32 dt)
{
    Ui_Window* win = &ui_wins[win_idx];
    if (check_bit(win->flags, WIN_RETRACTED))
    {
        return;
    }
    begin_gridd(1, 2);

    add_text(y_title);

    win->offset.y = win->start.y + ((win->g.y * 30.0f));

    f32 extra_padding = 0.0f;
    V3 top_left = v3f(win->offset.x - BORDER_THICKNESS,
                      win->offset.y + extra_padding - BORDER_THICKNESS, 0.0f);

    V2 h_size = v2f(win->dimensions.x - 100.0f, BORDER_THICKNESS);
    V2 v_size = v2f(BORDER_THICKNESS, 140.0f);

    Vertex_Buffer* vert = &gui_context.main_vert_idx.vert;

    V4 border_color = v4f(0.5f, 0.0f, 0.033f, g_translucentcy);
    add_border_s_d1(vert->data, &win->num_indices, border_color, top_left,
                    v2f(h_size.x, v_size.y), BORDER_THICKNESS);

    V3 graph_pos = v3f(top_left.x + BORDER_THICKNESS, top_left.y + BORDER_THICKNESS,
                       top_left.z);
    V2 graph_size =
        v2f(h_size.x - (BORDER_THICKNESS * 2), v_size.y - (BORDER_THICKNESS * 2));

    u32 rect_index = RECT_INDEX;
    const b8 graph_clicked = rect_index == index_clicked;
    const b8 graph_hover = rect_index == index_hover;

    if (graph_clicked)
    {
        graph_stop = graph_stop ? 0 : 1;
    }

    array_push(gui_context.rects,
              quad_d1(vert->data, &win->num_indices, graph_pos, graph_size,
                      v4f(0.005f, 0.005f, 0.005f, g_translucentcy)));
    array_back(gui_context.rects)->id = win_idx;

    graph_scissor.offset.x = (i32)clampf32_low(graph_pos.x, 0.0f);
    graph_scissor.offset.y = (i32)clampf32_low(graph_pos.y, 0.0f);
    graph_scissor.extent.width = (u32)clampf32_low(graph_size.x, 0.0f);
    graph_scissor.extent.height = (u32)clampf32_low(graph_size.y, 0.0f);

    Vertex_Buffer* graph_vert = &gui_context.graph_vert_idx.vert;

    if (y_min >= y_max)
    {
        SY_ERROR("Passing y_min that is grater or equal to y_max, in add_graph()");
    }
    graph_sec += dt;

    static V3 sample_pos = { 0 };

    static const f32 x_advance_per_sec = 20.0f;

    static char buffer[10] = { 0 };

    sample_pos = v3f(top_left.x + h_size.x - 5.0f, sample_pos.y, top_left.z);

    f32 mouse_x = gui_context.mouse_pos.x;
    f32 y_value_under_mouse = 0.0f;
    V3 interperlated_pos = v3f(mouse_x, top_left.y, top_left.z);
    for (u32 i = 0; i < samples_GUI; i++)
    {
        graph_vert->data[i].pos.x =
            sample_pos.x -
            ((x_advance_per_sec / sample_rate) * (samples_GUI - 1 - i));
        graph_vert->data[i].pos.y = top_left.y + v_size.y - y_values_pixels[i];
        graph_vert->data[i].pos.z = sample_pos.z;

        if (graph_hover && i > 0)
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
                samples_GUI = flush_graph();
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
    win->num_indices += text_2D(
        gui_context.font, 1.0f, buffer, (u32)strlen(buffer),
        v3f(x_pos_num, graph_vert->data[samples_GUI - 1].pos.y - 8.0f, sample_pos.z),
        font_color, 1.0f, NULL, NULL, vert->data);

    win->num_indices +=
        text_2D(gui_context.font, 1.0f, buffer_max, (u32)strlen(buffer_max),
                v3f(x_pos_num, top_left.y - 3.0f, sample_pos.z), font_color, 1.0f,
                NULL, NULL, vert->data);

    win->num_indices +=
        text_2D(gui_context.font, 1.0f, buffer_min, (u32)strlen(buffer_min),
                v3f(x_pos_num, top_left.y + v_size.y - 13.0f, sample_pos.z),
                font_color, 1.0f, NULL, NULL, vert->data);

    if (graph_hover && !ui_hold)
    {
        char buffer_value_under_mouse[10] = { 0 };
        f32_to_str(buffer_value_under_mouse, 7, y_value_under_mouse);
        win->num_indices +=
            text_2D(gui_context.font, 1.0f, buffer_value_under_mouse,
                    (u32)strlen(buffer_value_under_mouse),
                    v3f(mouse_x + 5.0f, top_left.y + 10.0f, sample_pos.z),
                    font_color, 1.0f, NULL, NULL, vert->data);

        f32 small_square_size = 10.0f;
        interperlated_pos.x -= small_square_size * 0.5f;
        interperlated_pos.y -= small_square_size * 0.5f;
        interperlated_pos.z = sample_pos.z;
        add_border_s_d0(vert->data, &win->num_indices, border_color,
                        interperlated_pos, v2i(small_square_size));

        quad_d1(vert->data, &win->num_indices,
                v3f(mouse_x, top_left.y, sample_pos.z), v_size, border_color);
    }

    win->g.y += v_size.y / 35.0f;
    set_bit(win->flags, WIN_GRAPH);
    win->last_button_width = 340.0f;
    update_misc();
    end_gridd();

    begin_gridd(2, 1);
    {
        char buffer_max_value[20] = "Max: ";
        char buffer_min_value[20] = "|  Min: ";

        f32_to_str_offset(buffer_max_value, 5, 7, max_value);
        f32_to_str_offset(buffer_min_value, 8, 7, min_value);
        add_text(buffer_max_value);
        add_text(buffer_min_value);
    }
    end_gridd();
}

b8 g_open[10] = { 0 };

static b8 showcase_entity(Dynamic_Entity_2D* e, Ui_Window* win, char* name)
{
    Vertex_Buffer* vert = &gui_context.main_vert_idx.vert;

    u32 drop_idx = g_entity_open_idx++;

    win->offset.y = win->start.y + ((win->g.y * 30.0f));

    Hover_Clicked hc = get_hover_clicked(RECT_INDEX);

    if (hc.clicked)
    {
        b_switch(g_open[drop_idx]);
    }

    V4 color = hand_hover(v4f(0.1f, 0.1f, 0.1f, g_translucentcy), hc.hover);

    V2 size = v2f(200.0f, 20.0f);
    u32 name_len = (u32)strlen(name);
    f32 name_width = calculate_text_advance(name, name_len) + PADDING_IN;
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

    array_push(gui_context.rects,
              quad_s_gradiant_d1(vert->data, &win->num_indices,
                                 v3f(win->offset.x, win->offset.y, 0.0f), size,
                                 color));
    array_back(gui_context.rects)->id = win_idx;

    if (name && *name)
    {
        win->num_indices += text_2D(
            gui_context.font, 1.0f, name, name_len,
            v3f(win->offset.x + (PADDING_IN * 0.61f), win->offset.y + 2.0f, 0.0f),
            font_color, 1.0f, NULL, NULL, vert->data);
    }
    win->g.y++;

    win->last_button_width = size.x;
    set_biggest_wide(win);

    return g_open[drop_idx];
}

void edit_show_entity(Dynamic_Entity_2D* e, char* name)
{
    Ui_Window* win = &ui_wins[win_idx];
    Vertex_Buffer* vert = &gui_context.main_vert_idx.vert;
    if (showcase_entity(e, win, name))
    {
        char buffer[50] = { 0 };
        val_to_str(buffer, "Pos: (x:%.2f, y:%.2f)", e->movement->pos.x,
                   e->movement->pos.y);
        win->offset.y = win->start.y + ((win->g.y * 30.0f));
        u32 buffer_len = (u32)strlen(buffer);

        win->num_indices +=
            text_2D(gui_context.font, 1.0f, buffer, buffer_len,
                    v3f(win->offset.x + 2.0f, win->offset.y + 2.0f, 0.0f),
                    font_color, 1.0f, NULL, NULL, vert->data);

        win->g.y++;

        begin_gridd(4, 1);
        {
            add_text("x:");
            add_input_float(&e->movement->pos.x, 0.0f, 5000.0f, 100.0f);
            add_text("y:");
            add_input_float(&e->movement->pos.y, 0.0f, 5000.0f, 100.0f);
        }
        end_gridd();

        val_to_str(buffer, "Vel: (x:%.2f, y:%.2f)", e->movement->vel.x,
                   e->movement->vel.y);
        win->offset.y = win->start.y + ((win->g.y * 30.0f));
        buffer_len = (u32)strlen(buffer);

        win->num_indices +=
            text_2D(gui_context.font, 1.0f, buffer, buffer_len,
                    v3f(win->offset.x + 2.0f, win->offset.y + 2.0f, 0.0f),
                    font_color, 1.0f, NULL, NULL, vert->data);

        win->g.y += 1.5f;
    }
}

void show_entity(Dynamic_Entity_2D* e, char* name)
{
    Ui_Window* win = &ui_wins[win_idx];
    Vertex_Buffer* vert = &gui_context.main_vert_idx.vert;
    if (showcase_entity(e, win, name))
    {
        win->offset.y = win->start.y + ((win->g.y * 30.0f));

        char buffer[100] = { 0 };
        val_to_str(buffer, "Pos: (x:%.2f, y:%.2f)\nVel: (x:%.2f, y:%.2f)",
                   e->movement->pos.x, e->movement->pos.y, e->movement->vel.x,
                   e->movement->vel.y);

        u32 buffer_len = (u32)strlen(buffer);

        win->num_indices +=
            text_2D(gui_context.font, 1.0f, buffer, buffer_len,
                    v3f(win->offset.x + 2.0f, win->offset.y + 2.0f, 0.0f),
                    font_color, 1.0f, NULL, NULL, vert->data);

        win->g.y += 1.5f;
    }
}

void entity_watch_window(void)
{
    Ui_Window* win = &ui_wins[win_idx];
    Vertex_Buffer* vert = &gui_context.main_vert_idx.vert;

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

        V4 button_color = v4f(0.7f, 0.0f, 0.033f, g_translucentcy);
        quad_s_gradiant_d1(vert->data, &win->num_indices, pos,
                           v2f(button_width, 20.0f), button_color);

        win->num_indices += text_2D(
            gui_context.font, 1.0f, buffer, len,
            v3f(win->offset.x + (PADDING_IN * 0.61f), win->offset.y + 2.0f, 0.0f),
            font_color, 1.0f, NULL, NULL, vert->data);
    }
}

void gui_destroy(VkDevice device, u32 num_semaphores)
{
    save_file_binary();
    destroy_graphic_pipeline(device, num_semaphores,
                             &gui_context.triangle_list_pipeline);
    destroy_graphic_pipeline(device, num_semaphores,
                             &gui_context.line_strip_pipeline);

    destroy_buffer(device, gui_context.main_vert_idx.vert.buffer);
    destroy_buffer(device, gui_context.main_vert_idx.idx.buffer);

    destroy_buffer(device, gui_context.graph_vert_idx.vert.buffer);
    destroy_buffer(device, gui_context.graph_vert_idx.idx.buffer);

    for (u32 i = 0; i < array_size(gui_context.textures); i++)
    {
        destroy_texture(device, gui_context.textures[i]);
    }
}

b8 is_focus(void)
{
    return ui_hit || ui_hold || ui_input_active;
}

void sy_print_text(char* text)
{
    if (terminal_buffer_init)
    {
        Array_Head* head = array_head(gui_context.terminal_buffer);
        char* temp_text = text;
        for (; *temp_text != '\0'; temp_text++)
        {
#if 1
            if (*temp_text == '\n')
            {
                new_lines++;
            }
#endif
            gui_context.terminal_buffer[head->size++] = *temp_text;
            if (head->size >= head->capacity)
            {
                flush_terminal();
            }
        }
    }
}


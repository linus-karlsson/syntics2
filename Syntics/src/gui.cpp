#include "gui.h"
#include "defines.h"
#include "vulkan_types.h"
#include "buffers.h"
#include "event_system.h"
#include "font.h"
#include "region_alloc.h"
#include "swap_chain.h"
#include "camera.h"
#include "collision.h"
#include "file_reading.h"
#include "ansi_keycodes.h"
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

#define MAX_SPACE 10000
#define RECTS_START 2
#define BUTTON_SIZE_MULTI 8.3f
#define Y_START_SHADOW ui_wins[win_idx].Y_START + 2.0f
#define X_START 11.0f
#define Y_START 25.0f

// TODO: Try to remove all bools in structs. No rush

struct Sy_Terminal_Attrib
{
    Sy_Terminal_Attrib();
    Vec2 dimensions;
    VkRect2D scissor;
    uint32 index_offset;
    uint32 num_indices;
    uint32 presist_offset_x;
    uint32 presist_offset_y;

    bool auto_scroll;
    bool presist_hold;
};
Sy_Terminal_Attrib::Sy_Terminal_Attrib()
{
    SET_0(scissor);
    index_offset = 0;
    num_indices = 0;
    presist_offset_x = 0;
    presist_offset_y = 0;

    auto_scroll = true;
    presist_hold = false;
}

template <size_t N>
struct Sy_Input
{
    Sy_Input();
    uint32 curr_index;
    uint32 buffer_size;
    uint32 frames_moved;

    float max;
    float min;
    float time;

    char text[N] = {};
    char last_text[N] = {};

    bool presist_clicked;
    bool presist_hold;
    bool highlight_on;
};
template <size_t N>
Sy_Input<N>::Sy_Input()
{
    curr_index = 0;
    buffer_size = 0;
    frames_moved = 0;

    max = 0;
    min = 0;
    time = 0;

    ARR_0(text);
    ARR_0(last_text);

    presist_clicked = false;
    presist_hold = false;
    highlight_on = false;
}

struct Sy_Gridd
{
    Sy_Gridd();
    float dimensions[2];
};
Sy_Gridd::Sy_Gridd()
{
    ARR_0(dimensions);
}

struct Sy_Ui_Window
{
    Sy_Ui_Window();

    Vec2 dimensions;
    Sy_Input<15> input_floats[10];
    Sy_Input<100> input_texts[10];

    VkRect2D scissor;
    Sy_Gridd gridd;

    uint32 input_f32_index;
    uint32 input_text_index;
    uint32 title_len;
    uint32 index_offset;
    uint32 num_indices;
    uint32 extra_hight;
    uint32 highest_high;

    float g_x;
    float g_y;
    float biggest_wide;
    float x_start;
    float y_start;
    float last_button_width;
    float x_offset;
    float y_offset;
    float presist_offset_x;
    float presist_offset_y;
    Vec2 size_cache;

    // TODO: like many other things are temp solutions
    bool retracted;
    bool first;
    bool gridd_start;
    bool presist_hold;
    bool dyn_resize;
    bool resize_hold;
    bool term;
    bool docked;
};
Sy_Ui_Window::Sy_Ui_Window()
{
    SET_0(scissor);
    input_f32_index = 0;
    input_text_index = 0;
    title_len = 0;
    index_offset = 0;
    num_indices = 0;
    extra_hight = 0;

    g_x = 0;
    g_y = 0;
    highest_high = 0;
    biggest_wide = 0;

    x_start = X_START;
    y_start = Y_START;

    last_button_width = 0.0f;
    x_offset = x_start;
    y_offset = 0.0f;

    presist_offset_x = 0.0f;
    presist_offset_y = 0.0f;

    retracted = false;
    first = true;
    gridd_start = false;
    presist_hold = false;
    dyn_resize = true;
    resize_hold = false;
    term = false;
    docked = false;
}

struct Sy_GUI
{
    Sy_GUI();

    Graphic_Pipline g_pipline;

    // TODO: Better setup
    VkRect2D scissor_whole_screen;

    const Swap_Chain_attrib* swap_chain;
    VkDevice device;
    Region_Alloc* region;

    Events* mouse_evt;
    Events* wheel_evt;
    Events* key_evt;

    Font font;
    Font font_ttf;
    Texture* textures;
    Rect* rects;

    Vec2 dimensions;
    Vec2 mouse_pos;

    Camera cam;

    char* terminal_buffer;
};
Sy_GUI::Sy_GUI()
{
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

static float g_translucentcy = 1.0f;

#define INDICES_PER_RECT 6
#define IDX_OFFSET (num_ui_rects * INDICES_PER_RECT)
#define VERTEX_PER_RECT 4

#define TOTAL_NUM_WINS 3
static Sy_GUI gui_context;
static Sy_Ui_Window ui_wins[TOTAL_NUM_WINS];
static Sy_Terminal_Attrib term;

static uint32 win_idx = 0;
static uint32 num_wins = 0;
static uint32 num_wins_frame = 0;
static uint32 rect_index = RECTS_START;
static uint32 index_hover = 0;
static uint32 index_clicked = 0;
static uint32 num_ui_rects = 0;
static uint32 win_hold_idx = 0;
static uint32 win_dock_hit_idx = 0;
static uint32 blue_rects_index_offset = 0;
static uint32 win_idx_resize_hover = 0;
static uint32 resize_idx = 0;
static uint32 extra_term = 0;

static bool ui_hit = false;
static bool ui_hold = false;
static bool ui_input_active = false;
static bool top_bar_presist_hold = false;
static bool is_holding = false;
static bool dock_hit[TOTAL_HIT] = {};
static bool recreate = false;
static bool terminal_buffer_init = false;

static float presist_offset_x = 0.0f;
static float presist_offset_y = 0.0f;
static float dt = 0;

static Rect blue_rects[TOTAL_HIT] = {};
static Rect dock_resized_rect = {};

static Vec4 font_color = Vec4(1.0f);

#define DEFAULT_TEXURE 0
#define TEXT_TEXURE 1
#define BUTTON_TEXURE 2

#define HEADER_HEIGHT 30

#define RECTS_PER_WINDOW 1000
#define INDICES_PER_WINDOW RECTS_PER_WINDOW * 6
#define VERTICES_PER_WINDOW RECTS_PER_WINDOW * 4
#define TERM_BUFFER_SIZE RECTS_PER_WINDOW - 10

void gui_init(Region_Alloc* region, VkDevice device,
              VkPhysicalDevice physical_device, VkCommandPool command_pool,
              VkQueue graphic_queue, const Swap_Chain_attrib& swap_chain,
              uint32 num_semaphores)
{
    subscribe(&gui_context.key_evt, EVT_KEY);
    subscribe(&gui_context.mouse_evt, EVT_MOUSE);
    subscribe(&gui_context.wheel_evt, EVT_WHEEL);

    gui_context.region = region;
    gui_context.textures = dyn_arrayP(region, 3, Texture);
    if (!terminal_buffer_init)
    {
        gui_context.terminal_buffer = dyn_arrayP(region, TERM_BUFFER_SIZE, char);
        terminal_buffer_init = true;
    }

    // Default tex: 4 bytes big. 1x1 pixel white image
    create_texture(device, physical_device, command_pool, graphic_queue, false,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/default.png",
                   &gui_context.textures[0]);
    get_head(gui_context.textures)->size++;

    create_texture(device, physical_device, command_pool, graphic_queue, false,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/ArialWhiteSmall.png",
                   &gui_context.textures[1]);
    get_head(gui_context.textures)->size++;

    create_texture(device, physical_device, command_pool, graphic_queue, false,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/button.png",
                   &gui_context.textures[2]);
    get_head(gui_context.textures)->size++;

#if 0
    ui_state.font_ttf =
        load_ftt_file(region, device, physical_device, command_pool, graphic_queue,
                      &ui_state.textures, "Syntics/res/Arial.ttf", 20.0f);
#endif

    gui_context.g_pipline.dynamic = true;
    create_graphics_pipeline(region, device, swap_chain.color_format,
                             swap_chain.render_pass, swap_chain.sample_count,
                             "Syntics/res/gui.vert.spv", "Syntics/res/gui.frag.spv",
                             swap_chain.extent_2D.width, swap_chain.extent_2D.height,
                             VK_CULL_MODE_BACK_BIT, size_arr(gui_context.textures),
                             &gui_context.scissor_whole_screen,
                             &gui_context.g_pipline);

    gui_context.device = device;
    gui_context.swap_chain = &swap_chain;

    gui_context.scissor_whole_screen.extent.width = swap_chain.extent_2D.width;
    gui_context.scissor_whole_screen.extent.height = swap_chain.extent_2D.height;

    gui_context.font = load_font_file(region, "Syntics/res/ArialWhiteSmall.fnt");
    gui_context.font.tex_index = 1.0f;

    uint32 num_ui_rects = 1000;
    gui_context.rects = dyn_arrayP(region, num_ui_rects, Rect);
    num_ui_rects = 0;

    init_graphics_pipeline(region, device, physical_device, command_pool,
                           graphic_queue, MAX_SPACE, num_semaphores,
                           gui_context.textures, gui_context.g_pipline);

    gui_context.g_pipline.idx_buffer.data =
        dyn_arrayP(region, MAX_SPACE * INDICES_PER_RECT, uint32);
    generate_indices(&gui_context.g_pipline.idx_buffer.data, 0, MAX_SPACE);
    gui_context.g_pipline.idx_buffer.size_bytes =
        capacity_arr(gui_context.g_pipline.idx_buffer.data) * sizeof(uint32);
    create_index_buffer(device, physical_device, command_pool, graphic_queue,
                        &gui_context.g_pipline.idx_buffer);

    region_pop(region, capacity_arr(gui_context.g_pipline.idx_buffer.data), uint32,
               PERM_ARRAY);
    gui_context.g_pipline.idx_buffer.data = NULL;

    gui_context.cam.position = v3f(0.0f, 0.0f, 0.0f);
    gui_context.cam.orientation = v3f(0.0f, 0.0f, 0.0f);
    gui_context.cam.mvp.model = mat4i(1.0f);
    gui_context.cam.mvp.view = mat4i(1.0f);
}

void gui_terminal_init(Region_Alloc* region)
{
    if (!terminal_buffer_init)
    {
        gui_context.terminal_buffer = dyn_arrayP(region, TERM_BUFFER_SIZE, char);
        terminal_buffer_init = true;
    }
}

static void gui_draw(VkCommandBuffer command_buffer, uint32 semaphore_idx,
                     const VkRect2D& scissor, uint32 index_offset,
                     uint32 num_indices)
{
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);
    bind_and_draw_graphics_pipline(
        command_buffer, gui_context.g_pipline.descriptors.desc_sets[semaphore_idx],
        index_offset, num_indices, gui_context.g_pipline);
}

void gui_render(VkCommandBuffer command_buffer, uint32 semaphore_idx)
{
    for (uint32 i = 0; i < win_idx; i++)
    {
        const Sy_Ui_Window* win = &ui_wins[i];
        gui_draw(command_buffer, semaphore_idx, win->scissor, win->index_offset,
                 win->num_indices);
        if (win->term && !win->retracted)
        {
            gui_draw(command_buffer, semaphore_idx, term.scissor, term.index_offset,
                     term.num_indices);
        }
    }

    if (blue_rects_index_offset)
    {
        gui_draw(command_buffer, semaphore_idx, gui_context.scissor_whole_screen,
                 blue_rects_index_offset, IDX_OFFSET);
    }
}

void gui_recreate(Region_Alloc* region)
{
    gui_context.scissor_whole_screen.extent = gui_context.swap_chain->extent_2D;
    recreate_graphic_pipline(region, gui_context.device, *gui_context.swap_chain,
                             "Syntics/res/gui.vert.spv", "Syntics/res/gui.frag.spv",
                             gui_context.g_pipline, size_arr(gui_context.textures),
                             &gui_context.scissor_whole_screen);
}

void gui_update_begin(Region_Alloc* region, const Vec2& dimensions,
                      uint32 semaphore_idx, float delta, float translucentcy)
{
    g_translucentcy = translucentcy;
    dt = delta;
    // Because vulkan is flipped this results in the oposite for y axis :|
    gui_context.cam.mvp.proj = ortho(0, 0, dimensions.x, dimensions.y, -1.0f, 1.0f);
    update_uniform_buffers(gui_context.device,
                           gui_context.g_pipline.uniform_buffers[semaphore_idx],
                           &gui_context.cam.mvp, sizeof(gui_context.cam.mvp));

    gui_context.dimensions = dimensions;
    gui_context.mouse_pos =
        Vec2((float)gui_context.mouse_evt->mouse_evt.move_evt.pos_x,
             (float)gui_context.mouse_evt->mouse_evt.move_evt.pos_y);

    index_hover = 0;
    index_clicked = 0;
    static bool first_clicked = true;
    const bool button_clicked = is_any_button_clicked(first_clicked);
    const uint8 action = gui_context.mouse_evt->mouse_evt.button_evt.action;
    static bool should_update = true;

    ui_input_active = false;

    if (should_update)
    {
        for (int i = size_arr(gui_context.rects) - 1; i >= 0; i--)
        {
            ui_hit = point_in_rect(gui_context.mouse_pos, gui_context.rects[i]);
            if (ui_hit)
            {
                index_hover = i + RECTS_START;

                if (button_clicked)
                {
                    index_clicked = i + RECTS_START;
                    ui_hold = true;
                }
                break;
            }
        }
    }
    if (!ui_hit && action)
    {
        should_update = false;
        index_clicked = 1;
    }
    else
    {
        should_update = true;
    }
    if (!gui_focus())
    {
        change_cursor(SYNT_NORMAL_CURSOR);
    }
    if (!action)
    {
        ui_hold = false;
    }
    for (uint32 i = 0; i < num_wins; i++)
    {
        ui_wins[i].highest_high = ui_wins[i].g_y;
        ui_wins[i].g_x = 0;
        ui_wins[i].g_y = 0;
        ui_wins[i].gridd.dimensions[0] = 0;
        ui_wins[i].gridd.dimensions[1] = 0;
        ui_wins[i].input_f32_index = 0;
        ui_wins[i].input_text_index = 0;
    }

    rect_index = RECTS_START;
    num_ui_rects = 0;

    get_head(gui_context.rects)->size = 0;

    get_head(gui_context.g_pipline.vert_buffer.data)->size = 0;

    extra_term = 0;
    win_idx = 0;
    win_hold_idx = 0;
}

static void set_dock_blue(uint32 side_hit, const Vec2& pos, const Vec2& size,
                          const Vec2& docked_pos, const Vec2& docked_size)
{
    if (!dock_hit[side_hit])
    {
        blue_rects[side_hit] =
            quad(&gui_context.g_pipline.vert_buffer.data, &num_ui_rects,
                 Vec3(pos.x, pos.y, -0.05f), size, Vec4(0.1f, 0.1f, 1.0f, 0.5f));
    }
    else
    {
        dock_resized_rect =
            quad(&gui_context.g_pipline.vert_buffer.data, &num_ui_rects,
                 Vec3(docked_pos.x, docked_pos.y, -0.05f), docked_size,
                 Vec4(0.1f, 0.1f, 1.0f, 0.5f));
        dock_resized_rect.id = side_hit;
    }
}

void gui_update_end()
{
    if (top_bar_presist_hold)
    {
        blue_rects_index_offset = INDICES_PER_WINDOW * (win_idx + extra_term);
        const Sy_Ui_Window* win = &ui_wins[win_hold_idx - 1];
        const Vec2 blue_side_size = Vec2(60.0f, 100.0f);
        const Vec2 docked_side_pos =
            Vec2(win->dimensions.x, gui_context.dimensions.y);
        set_dock_blue(LEFT_SIDE_HIT,
                      Vec2(40.0f, (gui_context.dimensions.y * 0.5f) - 50.0f),
                      blue_side_size, Vec2(0.0f), docked_side_pos);
        set_dock_blue(RIGHT_SIDE_HIT,
                      Vec2(gui_context.dimensions.x - 100.0f,
                           (gui_context.dimensions.y * 0.5f) - 50.0f),
                      blue_side_size,
                      Vec2(gui_context.dimensions.x - win->dimensions.x, 0.0f),
                      docked_side_pos);
        set_dock_blue(BOTTOM_HIT,
                      Vec2((gui_context.dimensions.x * 0.5f) - 50.0f,
                           gui_context.dimensions.y - 100.0f),
                      Vec2(100.0f, 60.0f),
                      Vec2(0.0f, gui_context.dimensions.y - win->dimensions.y),
                      Vec2(gui_context.dimensions.x, win->dimensions.y));
    }
    else
    {
        blue_rects_index_offset = 0;
    }
    win_dock_hit_idx = 0;
    for (uint32 i = 0; i < TOTAL_HIT; i++)
    {
        if ((dock_hit[i] = point_in_rect(gui_context.mouse_pos, blue_rects[i])))
        {
            win_dock_hit_idx = win_hold_idx;
            break;
        }
    }

    map_copy_mem(gui_context.device,
                 &gui_context.g_pipline.vert_buffer.buffer_memory,
                 gui_context.g_pipline.vert_buffer.size_bytes,
                 gui_context.g_pipline.vert_buffer.data);

    gui_context.g_pipline.idx_buffer.curr_size = IDX_OFFSET;

    num_wins = num_wins_frame;
    num_wins_frame = 0;
}

static bool borders = true;

static void change_size(float* win_dim_to_change, float* pos_to_change,
                        float* presist_offset, float win_dim, float mouse_pos)
{
    float change = (*presist_offset - mouse_pos);
    if (win_dim < *win_dim_to_change)
    {
        *pos_to_change -= change;
    }
    *win_dim_to_change += change;
    *presist_offset = mouse_pos;
}

static void set_resice(Sy_Ui_Window* win, float* presist_offset, float mouse_pos,
                       uint32 resize_id)
{
    *presist_offset = mouse_pos;
    win->resize_hold = true;
    resize_idx = resize_id;
}

void back_bord_begin(const char* title, const Vec2& pos)
{
    Sy_Ui_Window* win = &ui_wins[win_idx];
    win->index_offset = INDICES_PER_WINDOW * (win_idx + extra_term);
    win->num_indices = 0;
    if (win->first)
    {
        win->title_len = strlen(title);
        win->x_start = pos.x + X_START;
        win->y_start = pos.y + Y_START;
        win->first = false;
    }

    const bool hover = rect_index == index_hover;

    const bool retract_button_pressed = rect_index + 1 == index_clicked;

    const bool top_bar_clicked = rect_index + 2 == index_clicked;
    const bool top_bar_hover = rect_index + 2 == index_hover;

    const bool rezise_right_clicked = rect_index + 3 == index_clicked;
    const bool rezise_right_hover = rect_index + 3 == index_hover;

    const bool rezise_left_clicked = rect_index + 4 == index_clicked;
    const bool rezise_left_hover = rect_index + 4 == index_hover;

    const bool rezise_top_clicked = rect_index + 5 == index_clicked;
    const bool rezise_top_hover = rect_index + 5 == index_hover;

    const bool rezise_buttom_clicked = rect_index + 6 == index_clicked;
    const bool rezise_buttom_hover = rect_index + 6 == index_hover;

    const bool rezise_both_left_clicked = rect_index + 7 == index_clicked;
    const bool rezise_both_left_hover = rect_index + 7 == index_hover;

    const float title_bar_size = 20.0f;

    if (top_bar_clicked)
    {
        if (win->docked)
        {
            win->x_start =
                (gui_context.mouse_pos.x - (win->size_cache.x * 0.5f)) + X_START;
            win->dimensions = win->size_cache;
            win->docked = false;
        }
        win->presist_offset_x = gui_context.mouse_pos.x - (win->x_start);
        win->presist_offset_y = gui_context.mouse_pos.y - (win->y_start);
        win->presist_hold = true;
    }
    else if (rezise_left_clicked)
    {
        set_resice(win, &win->presist_offset_x, gui_context.mouse_pos.x,
                   RESIZE_LEFT);
    }
    else if (rezise_right_clicked)
    {
        set_resice(win, &win->presist_offset_x,
                   gui_context.mouse_pos.x - win->dimensions.x, RESIZE_RIGHT);
    }
    else if (rezise_top_clicked)
    {
        set_resice(win, &win->presist_offset_y, gui_context.mouse_pos.y, RESIZE_TOP);
    }
    else if (rezise_buttom_clicked)
    {
        set_resice(win, &win->presist_offset_y,
                   gui_context.mouse_pos.y - win->dimensions.y, RESIZE_BUTTOM);
    }
    else if (rezise_both_left_clicked)
    {
        set_resice(win, &win->presist_offset_x,
                   gui_context.mouse_pos.x - win->dimensions.x, RESIZE_BOTH_RIGHT);
        set_resice(win, &win->presist_offset_y,
                   gui_context.mouse_pos.y - win->dimensions.y, RESIZE_BOTH_RIGHT);
    }
    if (win->presist_hold)
    {
        change_cursor(SYNT_MOVE_CURSOR);

        win->x_start = gui_context.mouse_pos.x - win->presist_offset_x;
        win->y_start = gui_context.mouse_pos.y - win->presist_offset_y;
        is_holding = true;
        top_bar_presist_hold = true;
        win_hold_idx = win_idx + 1;
        win->dyn_resize = false;

        recreate = true;
    }
    if (!ui_hold)
    {
        if (hover || top_bar_hover)
        {
            change_cursor(SYNT_NORMAL_CURSOR);
        }
        win->presist_hold = false;
        win->resize_hold = false;
        is_holding = false;
        top_bar_presist_hold = false;
        win->dyn_resize = true;
    }
    if (win_dock_hit_idx - 1 == win_idx)
    {
        if (!ui_hold)
        {
            win->x_start = dock_resized_rect.pos.x + X_START;
            win->y_start = dock_resized_rect.pos.y + Y_START;
            if (!win->docked)
            {
                win->size_cache = win->dimensions;
                win->docked = true;
            }
            win->dimensions.x = dock_resized_rect.size.x;
            win->dimensions.y = dock_resized_rect.size.y;
        }
    }

#define REZIZE_BAR_SIZE 10.0f

    // TODO: Not sure where to place these:
    win->x_start =
        clampf32(win->x_start, X_START,
                 (gui_context.dimensions.x) - (win->dimensions.x - X_START));

    win->y_start =
        clampf32(win->y_start, Y_START,
                 (gui_context.dimensions.y) - (win->dimensions.y - Y_START));
    // TODO

    float wide = 0;
    float high = 0;
    if (win->dyn_resize)
    {
        wide = win->biggest_wide + REZIZE_BAR_SIZE - (win->x_start - X_START);
        high = ((float)win->highest_high * 33.0f) + Y_START + win->extra_hight;
    }
    if (win->resize_hold)
    {
        is_holding = true;
        recreate = true;
        if (resize_idx == RESIZE_LEFT)
        {
            change_size(&win->dimensions.x, &win->x_start, &win->presist_offset_x,
                        wide, gui_context.mouse_pos.x);
        }
        else if (resize_idx == RESIZE_RIGHT)
        {
            win->dimensions.x = gui_context.mouse_pos.x - win->presist_offset_x;
        }
        else if (resize_idx == RESIZE_TOP)
        {
            change_size(&win->dimensions.y, &win->y_start, &win->presist_offset_y,
                        high, gui_context.mouse_pos.y);
        }
        else if (resize_idx == RESIZE_BUTTOM)
        {
            win->dimensions.y = gui_context.mouse_pos.y - win->presist_offset_y;
        }
        else if (resize_idx == RESIZE_BOTH_RIGHT)
        {
            win->dimensions.x = gui_context.mouse_pos.x - win->presist_offset_x;
            win->dimensions.y = gui_context.mouse_pos.y - win->presist_offset_y;
        }
    }
    if (wide > win->dimensions.x)
    {
        win->dimensions.x = wide;
        recreate = true;
    }
    if (high > win->dimensions.y)
    {
        win->dimensions.y = high;
        recreate = true;
    }
    if (retract_button_pressed)
    {
        win->retracted = win->retracted ? false : true;
    }
    if (win->retracted)
    {
        win->dimensions.y = title_bar_size;
    }

    // TODO: this is for fullscreen mode, still sucks ass
    win->dimensions =
        Vec2(clampf32(win->dimensions.x, 0.0f, gui_context.dimensions.x),
             clampf32(win->dimensions.y, 0.0f, gui_context.dimensions.y));

    // TODO: This needs to be cleaned up, kinda buggy
    if (!win->retracted)
    {
        if (rezise_right_hover || rezise_left_hover)
        {
            win_idx_resize_hover = win_idx + 1;
            change_cursor(SYNT_RESIZE_H_CURSOR);
        }
        else if (rezise_top_hover || rezise_buttom_hover)
        {
            change_cursor(SYNT_RESIZE_V_CURSOR);
        }
        else if (rezise_both_left_hover)
        {
            change_cursor(SYNT_RESIZE_NW_CURSOR);
        }
        else if ((win_idx_resize_hover - 1 == win_idx) && !ui_hold)
        {
            change_cursor(SYNT_NORMAL_CURSOR);
            win_idx_resize_hover = 0;
        }
    }

    Vertex_Buffer* vert = &gui_context.g_pipline.vert_buffer;

    Vec4 back_bord_color = Vec4(0.03f, 0.03f, 0.03f, g_translucentcy);
    Vec3 back_bord_pos =
        Vec3(win->x_start - X_START, win->y_start - Y_START, -0.12f);

    Rect back_r = quad(&vert->data, &win->num_indices, back_bord_pos,
                       win->dimensions, back_bord_color);
    back_r.id = rect_index++;
    synt_push(gui_context.rects, back_r);

    Rect retract_rect = quad(
        &vert->data, &win->num_indices,
        Vec3(back_bord_pos.x + 10.0f, back_bord_pos.y, -0.04f), Vec2(title_bar_size),
        Vec4(0.0f, 0.0f, 0.0f, g_translucentcy * 0.22f), DEFAULT_TEXURE);
    synt_push(gui_context.rects, retract_rect);
    rect_index++;

    // TODO: Maybe have a recreate in each window
    if (recreate)
    {
        win->scissor.offset.x =
            (uint32)clampf32(back_r.pos.x, 0.0f, gui_context.dimensions.x);
        int32_t diff_x = back_r.pos.x < 0.0f ? (int32_t)back_r.pos.x : 0;
        win->scissor.extent.width = (uint32)back_r.size.x + diff_x + 1;

        win->scissor.offset.y = (int32_t)clampf32_low(back_r.pos.y, 0.0f);
        win->scissor.extent.height = (uint32)back_r.size.y + 1;

        recreate = false;
    }

    Vec4 border_color = Vec4(0.5f, 0.0f, 0.033f, g_translucentcy);

    Vec2 border_H_size = Vec2(win->dimensions.x, BORDER_THICKNESS);
    Vec2 border_V_size = Vec2(BORDER_THICKNESS,
                              win->dimensions.y - title_bar_size - BORDER_THICKNESS);

    back_bord_pos.z += 0.01f;
    back_bord_pos.y += title_bar_size;

    quad_s(&vert->data, &win->num_indices, back_bord_pos, border_V_size,
           border_color, DEFAULT_TEXURE, 1.0f);

    back_bord_pos.x += border_H_size.x - BORDER_THICKNESS;

    quad_s(&vert->data, &win->num_indices, back_bord_pos, border_V_size,
           border_color, DEFAULT_TEXURE, 1.0f);

    back_bord_pos.x -= border_H_size.x - BORDER_THICKNESS;
    back_bord_pos.y += border_V_size.y;

    quad_s(&vert->data, &win->num_indices, back_bord_pos, border_H_size,
           border_color, DEFAULT_TEXURE, 1.0f);

    synt_push(gui_context.rects,
              quad_s(&vert->data, &win->num_indices,
                     { win->x_start - X_START, win->y_start - Y_START, -0.11f },
                     Vec2(win->dimensions.x, title_bar_size),
                     Vec4(0.8f, 0.0f, 0.03f, g_translucentcy)));
    synt_back(gui_context.rects)->pos.x += title_bar_size + 10.0f;
    synt_back(gui_context.rects)->id = rect_index++;

    if (!win->retracted)

    {
        INIT_0(Rect, resize_right);
        INIT_0(Rect, resize_left);
        INIT_0(Rect, resize_top);
        INIT_0(Rect, resize_bottom);
        INIT_0(Rect, resize_both_right);
        resize_right = {
            { (win->x_start - 18.0f) + win->dimensions.x, win->y_start - Y_START },
            { 8.0f, win->dimensions.y - 10.0f },
            { 0.0f },
            { 0.0f },
            { rect_index++ },
        };
        resize_left = {
            { (win->x_start - X_START), win->y_start - Y_START },
            { 8.0f, win->dimensions.y },
            { 0.0f },
            { 0.0f },
            { rect_index++ },
        };
        resize_top = {
            { (win->x_start - X_START), (win->y_start - 37.0f) },
            { win->dimensions.x, 8.0f },
            { 0.0f },
            { 0.0f },
            { rect_index++ },
        };
        resize_bottom = {
            { (win->x_start - X_START), (win->y_start - 32.0f) + win->dimensions.y },
            { win->dimensions.x - 10.0f, 8.0f },
            { 0.0f },
            { 0.0f },
            { rect_index++ },
        };
        resize_both_right = {
            { resize_right.pos.x, resize_bottom.pos.y },
            { 10.0f },
            { 0.0f },
            { 0.0f },
            { rect_index++ },
        };
        synt_push(gui_context.rects, resize_right);
        synt_push(gui_context.rects, resize_left);
        synt_push(gui_context.rects, resize_top);
        synt_push(gui_context.rects, resize_bottom);
        synt_push(gui_context.rects, resize_both_right);
    }

    if (title && *title)
    {
        win->num_indices +=
            text_2D(gui_context.font, title, strlen(title),
                    Vec3(win->x_start - X_START + (win->dimensions.x / 2.0f) -
                             ((win->title_len * BUTTON_SIZE_MULTI) / 2),
                         win->y_start - 22.0f, -0.1f),
                    font_color, 1.0f, NULL, NULL, &vert->data);
    }

    win->biggest_wide = 0;

    num_wins_frame++;
}

static void move_to_next_chunk(uint32* num_indices)
{
    assert(*num_indices < RECTS_PER_WINDOW);

    get_head(gui_context.g_pipline.vert_buffer.data)->size +=
        (RECTS_PER_WINDOW - *num_indices) * VERTEX_PER_RECT;

    *num_indices *= INDICES_PER_RECT;
}
void back_bord_end()
{
    Sy_Ui_Window* win = &ui_wins[win_idx];
    // TODO: neeeeds to be fixed but can't be bother
    if (!win->term)
    {
        move_to_next_chunk(&win->num_indices);
    }

    ++win_idx;

    static bool first = true;
    if (first)
    {
        recreate = true;
        first = false;
    }
    win->gridd.dimensions[0] = 0;
    win->gridd.dimensions[1] = 0;
}

void gridd_begin(uint32 x, uint32 y)
{
    if (!x) x = 1;
    if (!y) y = 1;
    Sy_Ui_Window* win = &ui_wins[win_idx];

    win->gridd.dimensions[0] = (float)x;
    win->gridd.dimensions[1] += (float)y;
    win->gridd_start = true;

    if (win->biggest_wide < x)
    {
        win->biggest_wide = x;
    }
    win->g_x = 0.0f;
    win->x_offset = win->x_start;
}

void gridd_end()
{
    Sy_Ui_Window* win = &ui_wins[win_idx];
    if (win->g_x != 0.0f)
    {
        ++win->g_y;
    }
    win->gridd_start = false;
}

static void update_misc()
{
    Sy_Ui_Window* win = &ui_wins[win_idx];
    if (++win->g_x == win->gridd.dimensions[0])
    {
        win->g_x = 0.0f;

        float wide = win->x_offset + win->last_button_width;
        if (wide > win->biggest_wide)
        {
            win->biggest_wide = wide;
        }
        win->x_offset = win->x_start;

        if (++win->g_y >= win->gridd.dimensions[1])
        {
            win->gridd_start = false;
            win->last_button_width = 0;
        }
    }
}

bool add_button(const char* text)
{
    Sy_Ui_Window* win = &ui_wins[win_idx];
    if (!win->gridd_start)
    {
        SY_ERROR("Gridd overflow or is not started");
        return 0;
    }
    if (win->retracted)
    {
        return false;
    }
    win->y_offset = win->y_start + ((win->g_y * 30.0f));

    const bool clicked = rect_index == index_clicked;
    const bool hover = rect_index == index_hover;

    Vec4 button_color = Vec4(0.5f, 0.0f, 0.033f, g_translucentcy);
    if (hover && !ui_hold)
    {
        button_color *= 1.8f;
        button_color.w = 1.0f;
        change_cursor(SYNT_HAND_CURSOR);
    }

#define PADDING_IN 12.0f

    float x_advance = 0;
    size_t len = strlen(text);
    for (size_t i = 0; i < len; i++)
    {
        Character curr_char = gui_context.font.characters[text[i]];
        x_advance += (float)curr_char.x_advance * 1.0f;
    }
    float button_width = x_advance + PADDING_IN;

    if (win->g_x != 0) win->x_offset += win->last_button_width + PADDING;
    synt_push(gui_context.rects,
              quad_sl(&gui_context.g_pipline.vert_buffer.data, &win->num_indices,
                      { win->x_offset, win->y_offset, -0.11f },
                      Vec2(button_width, 20.0f), button_color));

    synt_back(gui_context.rects)->id = rect_index++;

    if (text && *text)
    {
        win->num_indices += text_2D(
            gui_context.font, text, len,
            Vec3(win->x_offset + (PADDING_IN * 0.61f), win->y_offset + 2.0f, -0.1f),
            font_color, 1.0f, NULL, NULL, &gui_context.g_pipline.vert_buffer.data);
    }
    win->last_button_width = button_width;
    update_misc();

    return clicked;
}

static bool is_character_number(uint16 key)
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
            return true;
        }
        default:
        {
            return false;
        }
    }
}

static bool is_character_letter(uint16 key)
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
            return true;
        }
        default:
        {
            return false;
        }
    }
}

static float abs_f32(float in)
{
    return in < 0.0f ? in * -1.0f : 1.0f;
}

template <size_t N>
static bool input_focused(Sy_Input<N>* curr_input, bool clicked, bool allow_letters,
                          bool cache_on_leave)
{
    bool result = true;
    if (clicked || curr_input->presist_clicked)
    {
        curr_input->presist_clicked = true;

        curr_input->curr_index =
            curr_input->highlight_on ? 0 : curr_input->buffer_size;

        Events* key_evt = gui_context.key_evt;
        if (key_evt->activated && key_evt->key_evt.action)
        {
            curr_input->highlight_on = false;

            uint16 key = key_evt->key_evt.key;
            char letter;
            if (key == SYNT_KEY_ENTER)
            {
                curr_input->presist_clicked = false;
                result = false;
                curr_input->buffer_size = curr_input->curr_index;
            }
            else if (key == SYNT_KEY_BACKSPACE)
            {
                curr_input
                    ->text[curr_input->curr_index != 0 ? --curr_input->curr_index
                                                       : 0] = '\0';
                curr_input->buffer_size = curr_input->curr_index;
            }
            else if (key != SYNT_KEY_CAPS)
            {
                bool is_letter = false;
                if (allow_letters)
                {
                    is_letter = is_character_letter(key);
                }
                bool is_number = is_character_number(key);
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
                        if (curr_input->curr_index < N - 1)
                        {
                            curr_input->text[curr_input->curr_index++] = letter;
                        }
                    }
                    curr_input->text[curr_input->curr_index] = '\0';
                }
                curr_input->buffer_size = curr_input->curr_index;
            }
        }
        if (!clicked && index_clicked)
        {
            if (!cache_on_leave)
            {
                memcpy(curr_input->text, curr_input->last_text,
                       sizeof(curr_input->text));
            }
            curr_input->presist_clicked = false;

            curr_input->highlight_on = false;
        }
    }
    ui_input_active |= curr_input->presist_clicked;
    return result;
}

template <size_t N>
static uint32 render_input(Sy_Input<N>* curr_input, Sy_Ui_Window* win,
                           const Vec4& input_color, const Vec4& text_color,
                           float min)
{
    win->y_offset = win->y_start + ((win->g_y * 30.0f));

    float x_advance = 0;
    size_t len = strlen(curr_input->text);
    for (size_t i = 0; i < len; i++)
    {
        Character curr_char = gui_context.font.characters[curr_input->text[i]];
        x_advance += (float)curr_char.x_advance * 1.0f;
    }

    float input_width = x_advance + 5.0f;

    if (input_width < min)
    {
        input_width = min;
    }
    if (win->last_button_width < min)
    {
        win->last_button_width = min;
    }
    if (win->g_x) win->x_offset += win->last_button_width + PADDING;

    synt_push(gui_context.rects,
              quad_s(&gui_context.g_pipline.vert_buffer.data, &win->num_indices,
                     { win->x_offset, win->y_offset, -0.11f },
                     Vec2(input_width, 20.0f), input_color));
    synt_back(gui_context.rects)->id = rect_index++;

    if (curr_input->highlight_on && len > 0)
    {
        quad(&gui_context.g_pipline.vert_buffer.data, &win->num_indices,
             { win->x_offset + 2.5f, win->y_offset + 2.0f, -0.105f },
             Vec2(x_advance, 16.0f), Vec4(0.0f, 0.0f, 1.0f, 0.7f));
    }
#if 1
    // Blinking cursor
    else if (curr_input->presist_clicked)
    {
        curr_input->time += dt;
        if (curr_input->time >= 0.4f || curr_input->highlight_on)
        {
            quad(&gui_context.g_pipline.vert_buffer.data, &win->num_indices,
                 { win->x_offset + x_advance + 1.0f, win->y_offset + 2.0f, -0.05f },
                 Vec2(2.0f, 16.0f), text_color);

            curr_input->time = curr_input->time >= 0.8f ? 0 : curr_input->time;
        }
    }
#else
    // Non blinking
    else if (curr_input->presist_clicked)
    {
        quad(&gui_context.g_pipline.vert_buffer.data, &win->num_indices,
             { win->x_offset + x_advance + 1.0f, win->y_offset + 2.0f, -0.05f },
             Vec2(2.0f, 16.0f), text_color);
    }

#endif

    win->num_indices +=
        text_2D(gui_context.font, curr_input->text, len,
                Vec3(win->x_offset + 3.0f, win->y_offset + 2.0f, -0.1f), text_color,
                1.0f, NULL, NULL, &gui_context.g_pipline.vert_buffer.data);

    win->last_button_width = input_width;

    return len;
}

bool add_input_float(float& input, float min, float max)
{
    Sy_Ui_Window* win = &ui_wins[win_idx];
    if (!win->gridd_start)
    {
        SY_ERROR("Gridd overflow or is not started\n");
        return 0;
    }
    if (win->retracted)
    {
        return 0;
    }
    const bool clicked = rect_index == index_clicked;
    const bool hover = rect_index == index_hover;

    Sy_Input<15>* curr_input = &win->input_floats[win->input_f32_index];

    curr_input->min = min;
    curr_input->max = max;

    if (curr_input->presist_hold || ((hover && ui_hold) && !is_holding))
    {
        const int16 mouse_x = gui_context.mouse_evt->mouse_evt.move_evt.pos_x;

        static int16 last_x = mouse_x;

        bool moved = false;
        if (!clicked)
        {
            float speed = (max - min) * 0.4f;
            if (last_x < mouse_x)
            {
                if (!curr_input->highlight_on)
                {
                    float multiplier = (float)(mouse_x - last_x);
                    input += speed * multiplier * dt;
                }
                moved = true;
            }
            else if (last_x > mouse_x)
            {
                if (!curr_input->highlight_on)
                {
                    float multiplier = (float)(last_x - mouse_x);
                    input -= speed * multiplier * dt;
                }
                moved = true;
            }
        }
        if (moved)
        {
            curr_input->frames_moved++;

            if (curr_input->frames_moved == 12)
            {
                curr_input->highlight_on = false;
                curr_input->curr_index = 0;
                curr_input->presist_clicked = false;
                curr_input->frames_moved = 0;
            }
            if (!curr_input->highlight_on)
            {
                input = clampf32(input, min, max);
                sprintf(curr_input->text, "%f", input);
            }
            memcpy(curr_input->last_text, curr_input->text,
                   sizeof(curr_input->last_text));
        }
        last_x = mouse_x;

        curr_input->presist_hold = true;
        is_holding = true;
        change_cursor(SYNT_RESIZE_H_CURSOR);
    }
    if (!ui_hold)
    {
        if (curr_input->presist_hold)
        {
            change_cursor(SYNT_NORMAL_CURSOR);
        }
        curr_input->presist_hold = false;
        is_holding = false;
        curr_input->frames_moved = 0;
    }
    if (clicked)
    {
        input = clampf32(input, min, max);
        sprintf(curr_input->text, "%f", input);
        curr_input->highlight_on = true;
    }
    if (!input_focused(curr_input, clicked, false, false))
    {
        input = (float)atof(curr_input->text);
        input = clampf32(input, min, max);
        sprintf(curr_input->text, "%f", input);

        memcpy(curr_input->last_text, curr_input->text,
               sizeof(curr_input->last_text));
    }
    Vec4 input_color = Vec4(0.0f, 0.244f, 1.0f, g_translucentcy);
    render_input(curr_input, win, input_color, font_color, 50.0f);
    win->input_f32_index++;
    update_misc();
    return clicked;
}

bool add_input_text(char** ptr_to_text, uint32* size)
{
    Sy_Ui_Window* win = &ui_wins[win_idx];
    bool result = false;
    if (win->retracted)
    {
        return result;
    }
    Sy_Input<100>* curr_input = &win->input_texts[win->input_text_index];
    curr_input->max = 100;

    const bool clicked = rect_index == index_clicked;
    const bool hover = rect_index == index_hover;

    if (clicked)
    {
        curr_input->highlight_on = curr_input->highlight_on ? false : true;
    }
    result = !input_focused(curr_input, clicked, true, true);

    Vec4 input_color = Vec4(1.0f, 1.0f, 1.0f, g_translucentcy);
    Vec4 text_color = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
    uint32 len = render_input(curr_input, win, input_color, text_color, 100.0f);

    if (ptr_to_text)
    {
        *ptr_to_text = curr_input->text;
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
    Sy_Ui_Window* win = &ui_wins[win_idx];
    if (win->retracted)
    {
        return;
    }
    win->y_offset = win->y_start + ((win->g_y * 30.0f));
    if (win->last_button_width < 50.0f)
    {
        win->last_button_width = 50.0f;
    }
    if (win->g_x) win->x_offset += win->last_button_width + 10.0f;
    float x_advance = 0;
    if (text && *text)
    {
#if 0
        win->num_indices += text_2D_ttf(ui_state.font_ttf, text,
                           Vec3(win->x_offset_button + 2.0f,
                                win->Y_START + 0.0f + (win->g_y * 30.0f), -0.1f),
                           1.0f, &ui_state.g_pipline.vert_buffer.data);
#endif
        uint32 len = strlen(text);
        win->num_indices += text_2D(
            gui_context.font, text, len,
            Vec3(win->x_offset + 2.0f, win->y_offset + 2.0f, -0.1f), font_color,
            1.0f, NULL, &x_advance, &gui_context.g_pipline.vert_buffer.data);
    }
    win->last_button_width = x_advance;
    update_misc();
}

static uint32 old_new_lines = 0;
static uint32 new_lines = 0;

static void flush_Buffer()
{
    uint32* size = &get_head(gui_context.terminal_buffer)->size;
    uint32 half_size = *size / 2;

    char* half_ptr = gui_context.terminal_buffer + half_size;
    memcpy(gui_context.terminal_buffer, half_ptr, half_size);
    // memset(half_ptr, 0, half_size);

    new_lines = 0;
    for_range(i, half_size)
    {
        if (gui_context.terminal_buffer[i] == '\n')
        {
            new_lines++;
        }
    }

    *size = half_size;
}

void print_text(char* text)
{
    bool flushed = false;
    if (terminal_buffer_init)
    {
        Array_Head* head = get_head(gui_context.terminal_buffer);
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
                flush_Buffer();
            }
        }
    }
}

void add_border(Sy_Ui_Window* win, Vertex_Buffer* vert, const Vec3& top_left,
                const Vec2& h_size, const Vec2& v_size, const Vec4& border_color)
{

    quad_s(&vert->data, &win->num_indices, top_left, h_size, border_color,
           DEFAULT_TEXURE, 1.0f);

    quad_s(&vert->data, &win->num_indices,
           Vec3(top_left.x, top_left.y + v_size.y, top_left.z), h_size, border_color,
           DEFAULT_TEXURE, 1.0f);

    quad_s(&vert->data, &win->num_indices, top_left, v_size, border_color,
           DEFAULT_TEXURE, 1.0f);

    quad_s(&vert->data, &win->num_indices,
           Vec3(top_left.x + h_size.x - BORDER_THICKNESS, top_left.y, top_left.z),
           v_size, border_color, DEFAULT_TEXURE, 1.0f);
}

// TODO: really strange bug with auto scroll when terrain gets updated every frame.
// It only happens in debug mode so not a big problem.
void add_terminal(float width, float height)
{
    Sy_Ui_Window* win = &ui_wins[win_idx];
    if (win->retracted)
    {
        move_to_next_chunk(&win->num_indices);
        gridd_end();
        return;
    }
    extra_term += 1;
    char* buffer = gui_context.terminal_buffer;
    Vertex_Buffer* vert = &gui_context.g_pipline.vert_buffer;

    win->gridd.dimensions[0] = 0;
    win->gridd.dimensions[1] = 0;

    gridd_begin(3, 1);

    if (add_button("Auto"))
    {
        term.auto_scroll = true;
    }
    static uint32 idx_ = 0;
    char temp[][6] = { "Stop", "Start" };
    if (add_button(temp[idx_]))
    {
        if (terminal_buffer_init)
        {
            synt_LOG_Term("Printing stopped\n");
            terminal_buffer_init = false;
        }
        else
        {
            terminal_buffer_init = true;
            synt_LOG_Term("Printing Starts...\n");
        }
        ++idx_ %= 2;
    }
    if (add_button("Flush"))
    {
        flush_Buffer();
    }

    float extra_padding = 8.0f;

#if 0
    Vec3 pos =
        Vec3(win->x_offset + extra_padding + BORDER_THICKNESS,
             win->y_start + 2.0f + BORDER_THICKNESS + (win->g_y * 30.0f), -0.1f);
#endif

    Vec3 top_left =
        Vec3(win->x_offset - BORDER_THICKNESS,
             win->y_start + 2.0f + (win->g_y * 30.0f) - BORDER_THICKNESS, -0.1f);

    float part_above_termnal = top_left.y + BORDER_THICKNESS + 5.0f + extra_padding -
                               (win->y_start - HEADER_HEIGHT);

    static bool first = true;
    if (first)
    {
        term.dimensions = Vec2(width, height);
        first = false;
        if (term.dimensions.x > win->dimensions.x)
        {
            win->dimensions.x = term.dimensions.x;
        }
    }
    else
    {
        // TODO: Bug win dimensions does not get set untill later frames. fucks up
        // the scissor
        term.dimensions.x = win->dimensions.x - 20.0f;
        term.dimensions.y = win->dimensions.y - part_above_termnal;
#if 0
        if (term.dimensions.y + part_above_termnal >= win->dimensions.y)
        {
            term.dimensions.y = win->dimensions.y - part_above_termnal;
        }
#endif
    }

    Vec2 term_H_size = Vec2(term.dimensions.x, BORDER_THICKNESS);
    Vec2 term_V_size =
        Vec2(BORDER_THICKNESS, term.dimensions.y + BORDER_THICKNESS + extra_padding);

    Vec3 term_pos = Vec3(top_left.x + BORDER_THICKNESS,
                         top_left.y + BORDER_THICKNESS, top_left.z - 0.001f);

    term.scissor.offset.x = (int32)clampf32_low(term_pos.x, 0.0f);
    term.scissor.offset.y = (int32)clampf32_low(term_pos.y, 0.0f);
    term.scissor.extent.width =
        (uint32)clampf32_low(term.dimensions.x - BORDER_THICKNESS, 0.0f);
    term.scissor.extent.height = (uint32)term.dimensions.y + extra_padding;

    // If term window should not be clipped to back bord
#if 0
    const bool clicked = rect_index == index_clicked;
    const bool hover = rect_index == index_hover;

    if (clicked)
    {
        term.presist_offset_y = ui_state.mouse_pos.y - term.dimensions.y;
    }
    if (hover)
    {
        change_cursor(SYNT_RESIZE_V_CURSOR);
    }

    if (term.presist_hold || ((hover && ui_hold) && !is_holding))
    {
        change_cursor(SYNT_RESIZE_V_CURSOR);

        term.presist_hold = true;
        is_holding = true;

        float new_dim = ui_state.mouse_pos.y - term.presist_offset_y;
        if (new_dim + part_above_termnal < win->dimensions.y)
        {
            term.dimensions.y = new_dim;
        }
    }
    if (!ui_hold)
    {
        term.presist_hold = false;
    }
#endif

    Vec4 border_color = Vec4(0.5f, 0.0f, 0.033f, g_translucentcy);
    add_border(win, vert, top_left, term_H_size, term_V_size, border_color);

    const bool terminal_clicked = rect_index == index_clicked;
    const bool terminal_hover = rect_index == index_hover;

    synt_push(gui_context.rects,
              quad(&vert->data, &win->num_indices, term_pos,
                   Vec2(term.scissor.extent.width, term_V_size.y - BORDER_THICKNESS),
                   Vec4(0.005f, 0.005f, 0.005f, g_translucentcy)));
    synt_back(gui_context.rects)->id = rect_index++;

    // TODO: Need to fix this more smoothly
    move_to_next_chunk(&win->num_indices);

    term.index_offset = INDICES_PER_WINDOW * (win_idx + extra_term);
    term.num_indices = 0;

    // Text moving upp

    float line_height = 0;
    float buffer_height = 0;
    static float buffer_diff = 0;

    if (terminal_hover)
    {
        change_cursor(SYNT_NORMAL_CURSOR);
        if (gui_context.wheel_evt->activated)
        {
            term.auto_scroll = false;
            buffer_diff += ((float)gui_context.wheel_evt->wheel_evt.z_delta * 0.3f);
        }
    }
    if (term.auto_scroll)
    {
        line_height = (float)gui_context.font.line_height * 1.0f;
        buffer_height = line_height * (float)new_lines;
        buffer_diff = term.dimensions.y - (buffer_height);
    }
    // new_lines = 0;

    uint32 buffer_size = size_arr(buffer);
    term_pos.x += extra_padding;
    term_pos.y += buffer_diff + extra_padding;
    term_pos.z += 0.001f;
    term.num_indices += text_2D(gui_context.font, buffer, buffer_size, term_pos,
                                font_color, 1.0f, NULL, NULL, &vert->data);

    move_to_next_chunk(&term.num_indices);

    win->last_button_width = width;
    win->extra_hight = height;
    update_misc();
    win->term = true;
}

void add_graph(float value, const char* y_title, float y_max, float y_min,
               float sample_rate, float dt)
{
    Sy_Ui_Window* win = &ui_wins[win_idx];
    if (!win->gridd_start)
    {
        SY_ERROR("Gridd overflow or is not started");
        return;
    }
    if (win->retracted)
    {
        return;
    }
    win->y_offset = win->y_start + ((win->g_y * 30.0f));

    Vec3 top_left = Vec3(win->x_offset - BORDER_THICKNESS,
                         win->y_offset - BORDER_THICKNESS, -0.1f);

    Vec2 h_size = Vec2(win->dimensions.x - 20.0f, BORDER_THICKNESS);
    Vec2 v_size = Vec2(BORDER_THICKNESS, 100.0f);

    win->y_offset += v_size.y;
}

void destroy_gui(VkDevice device, uint32 num_semaphores)
{
    destroy_graphic_pipeline(device, num_semaphores, gui_context.g_pipline);

    for (uint32 i = 0; i < size_arr(gui_context.textures); i++)
    {
        destroy_texture(device, gui_context.textures[i]);
    }
}

bool gui_focus()
{
    return ui_hit || ui_hold || ui_input_active;
}


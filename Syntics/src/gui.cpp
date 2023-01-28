#include "gui.h"
#include "vulkan_types.h"
#include "buffers.h"
#include "event_system.h"
#include "font.h"
#include "region_alloc.h"
#include "swap_chain.h"
#include "camera.h"
#include "collision.h"
#include "file_reading.h"
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

#define MAX_SPACE 10000
#define RECTS_START 2
#define BUTTON_SIZE_MULTI 8.3f
#define Y_START_SHADOW ui_wins[win_idx].Y_START + 2.0f
#define DOCKED_LEFT 1
#define DOCKED_RIGHT 2

// TODO: Try to remove all bools in structs. No rush

typedef struct Terminal_Attrib
{
    Vec2 dimensions;
    VkRect2D scissor;
    uint32 index_offset = 0;
    uint32 num_indices = 0;
    uint32 presist_offset_x;
    uint32 presist_offset_y;

    bool auto_scroll = true;
    bool presist_hold = false;
} Terminal_Attrib;

typedef struct Input_Float
{
    uint32 curr_index = 0;
    uint32 frames_moved = 0;

    float max = 0;
    float min = 0;

    char text[15] = {};
    char last_text[15] = {};

    bool presist_clicked = false;
    bool presist_hold = false;
    bool highlight_on = false;
} Input_Float;

typedef struct Gridd
{
    uint32 gridd_dimensions[2];
} Gridd;

typedef struct Ui_Window
{
    VkRect2D scissor;
    Input_Float input_floats[10];
    uint32 input_index = 0;
    uint32 gridd_dimensions[2];
    uint32 g_x = 0;
    uint32 g_y = 0;
    uint32 title_len = 0;
    uint32 index_offset = 0;
    uint32 num_indices = 0;
    uint32 extra_hight = 0;

    uint32 highest_high = 0;
    float biggest_wide = 0;

    float X_START = 11.0f;
    float Y_START = 30.0f;

    float last_button_width = 0;
    float x_offset_button = X_START;

    float extra_x_offset = 0;

    float presist_offset_x = 0;
    float presist_offset_y = 0;

    Vec2 dimensions;

    // TODO: like many other things are temp solution.
    bool first = true;
    bool gridd_start = false;
    bool presist_hold = false;

    bool dyn_resize = true;
    bool resize_hold = false;
    bool term = false;

} Ui_Window;

typedef struct Ui_State
{
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

} Ui_State;

#define TOTAL_NUM_WINS 3
static Ui_State ui_state;
static Ui_Window ui_wins[TOTAL_NUM_WINS];
static Terminal_Attrib term;
static uint32 win_idx = 0;
static uint32 num_wins = 0;
static uint32 num_wins_frame = 0;
static uint32 rect_index = RECTS_START;
static uint32 index_hover = 0;
static uint32 index_clicked = 0;
static bool ui_hit = false;
static bool ui_hold = false;
static uint32 num_ui_rects = 0;

static float presist_offset_x = 0.0f;
static float presist_offset_y = 0.0f;
static bool presist_hold = false;

static bool is_holding = false;

static uint32 blue_rects_index_offset = 0;

#define LEFT_SIDE_HIT 0
#define RIGHT_SIDE_HIT 1

#define LEFT_UPPER_HIT 2
#define LEFT_LOWER_HIT 3

#define RIGHT_UPPER_HIT 4
#define RIGHT_LOWER_HIT 5

#define TOTAL_HIT 2

#define REZIZE_LEFT 1
#define REZIZE_RIGHT 2
#define REZIZE_BUTTOM 3

#define INDICES_PER_RECT 6
#define IDX_OFFSET (num_ui_rects * INDICES_PER_RECT)

static Rect blue_rects[TOTAL_HIT] = {};
static bool dock_hit[TOTAL_HIT] = {};
static Rect dock_resized_rect = {};
static uint32 win_hold_idx = 0;
static uint32 win_dock_hit_idx = 0;

static uint32 win_idx_resize_hover = 0;
static uint32 resize_idx = 0;

static float dt = 0;

static bool recreate = false;

static const Vec4 SHADOW_COLOR = Vec4(0.0f, 0.0f, 0.0f, 0.7f);

#define DEFAULT_TEXURE 0
#define TEXT_TEXURE 1
#define BUTTON_TEXURE 2

#define HEADER_HEIGHT 30

void gui_init(Region_Alloc* region, VkDevice device,
              VkPhysicalDevice physical_device, VkCommandPool command_pool,
              VkQueue graphic_queue, const Swap_Chain_attrib& swap_chain,
              uint32 num_semaphores)
{
    subscribe(&ui_state.key_evt, EVT_KEY);
    subscribe(&ui_state.mouse_evt, EVT_MOUSE);
    subscribe(&ui_state.wheel_evt, EVT_WHEEL);

    ui_state.region = region;
    ui_state.textures = dyn_arrayP(region, 3, Texture);
    ui_state.terminal_buffer = dyn_arrayP(region, MAX_SPACE * 2, char);

    // Default tex: 4 bytes big. 1x1 pixel white image
    create_texture(device, physical_device, command_pool, graphic_queue, false,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/default.png",
                   &ui_state.textures[0]);
    get_head(ui_state.textures)->size++;

    create_texture(device, physical_device, command_pool, graphic_queue, false,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/Mono.png",
                   &ui_state.textures[1]);
    get_head(ui_state.textures)->size++;

    create_texture(device, physical_device, command_pool, graphic_queue, false,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/button.png",
                   &ui_state.textures[2]);
    get_head(ui_state.textures)->size++;

#if 0
    ui_state.font_ttf =
        load_ftt_file(region, device, physical_device, command_pool, graphic_queue,
                      &ui_state.textures, "Syntics/res/Arial.ttf", 20.0f);
#endif

    ui_state.g_pipline.dynamic = true;
    create_graphics_pipeline(region, device, swap_chain.color_format,
                             swap_chain.render_pass, swap_chain.sample_count,
                             "Syntics/res/gui.vert.spv", "Syntics/res/gui.frag.spv",
                             swap_chain.extent_2D.width, swap_chain.extent_2D.height,
                             VK_CULL_MODE_BACK_BIT, size_arr(ui_state.textures),
                             &ui_state.scissor_whole_screen, &ui_state.g_pipline);

    ui_state.device = device;
    ui_state.swap_chain = &swap_chain;

    ui_state.scissor_whole_screen.extent.width = swap_chain.extent_2D.width;
    ui_state.scissor_whole_screen.extent.height = swap_chain.extent_2D.height;

    ui_state.font = load_font_file(region, "Syntics/res/Mono.fnt");
    ui_state.font.tex_index = 1.0f;

    uint32 num_ui_rects = 10;
    ui_state.rects = dyn_arrayP(region, num_ui_rects * 3, Rect);

    init_graphics_pipeline(region, device, physical_device, command_pool,
                           graphic_queue, MAX_SPACE, num_semaphores,
                           ui_state.textures, ui_state.g_pipline);

    ui_state.g_pipline.idx_buffer.data =
        dyn_arrayP(region, MAX_SPACE * INDICES_PER_RECT, uint32);
    generate_indices(&ui_state.g_pipline.idx_buffer.data, MAX_SPACE);
    ui_state.g_pipline.idx_buffer.size_bytes =
        capacity_arr(ui_state.g_pipline.idx_buffer.data) * sizeof(uint32);
    create_index_buffer(device, physical_device, command_pool, graphic_queue,
                        &ui_state.g_pipline.idx_buffer);

    region_pop(region, capacity_arr(ui_state.g_pipline.idx_buffer.data), uint32,
               PERM_ARRAY);
    ui_state.g_pipline.idx_buffer.data = NULL;

    ui_state.cam.position = v3f(0.0f, 0.0f, 0.0f);
    ui_state.cam.orientation = v3f(0.0f, 0.0f, 0.0f);
    ui_state.cam.mvp.model = mat4i(1.0f);
    ui_state.cam.mvp.view = mat4i(1.0f);
}

static void gui_draw(VkCommandBuffer command_buffer, uint32 semaphore_idx,
                     VkRect2D& scissor, uint32 index_offset, uint32 num_indices)
{
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);
    bind_and_draw_graphics_pipline(
        command_buffer, ui_state.g_pipline.descriptors.desc_sets[semaphore_idx],
        index_offset, num_indices, ui_state.g_pipline);
}

void gui_render(VkCommandBuffer command_buffer, uint32 semaphore_idx)
{
    for (uint32 i = 0; i < win_idx; i++)
    {
        Ui_Window* win = &ui_wins[i];
        gui_draw(command_buffer, semaphore_idx, win->scissor, win->index_offset,
                 win->num_indices);
        if (win->term)
        {
            gui_draw(command_buffer, semaphore_idx, term.scissor, term.index_offset,
                     term.num_indices);
        }
    }

    if (blue_rects_index_offset)
    {
        gui_draw(command_buffer, semaphore_idx, ui_state.scissor_whole_screen,
                 blue_rects_index_offset, IDX_OFFSET - blue_rects_index_offset);
    }
}

void gui_recreate(Region_Alloc* region)
{
    ui_state.scissor_whole_screen.extent = ui_state.swap_chain->extent_2D;
    recreate_graphic_pipline(region, ui_state.device, *ui_state.swap_chain,
                             "Syntics/res/gui.vert.spv", "Syntics/res/gui.frag.spv",
                             ui_state.g_pipline, size_arr(ui_state.textures),
                             &ui_state.scissor_whole_screen);
}

void gui_update_begin(Region_Alloc* region, const Vec2& dimensions,
                      uint32 semaphore_idx, float delta)
{
    dt = delta;
    // Because vulkan is flipped this results in the oposite for y axis :|
    ui_state.cam.mvp.proj = ortho(0, 0, dimensions.x, dimensions.y, -1.0f, 1.0f);
    update_uniform_buffers(ui_state.device,
                           ui_state.g_pipline.uniform_buffers[semaphore_idx],
                           &ui_state.cam.mvp, sizeof(ui_state.cam.mvp));

    ui_state.dimensions = dimensions;
    ui_state.mouse_pos = Vec2((float)ui_state.mouse_evt->mouse_evt.move_evt.pos_x,
                              (float)ui_state.mouse_evt->mouse_evt.move_evt.pos_y);

    index_hover = 0;
    index_clicked = 0;
    static bool first_clicked = true;
    const bool button_clicked = is_any_button_clicked(first_clicked);
    const uint8 action = ui_state.mouse_evt->mouse_evt.button_evt.action;
    static bool should_update = true;

    if (should_update)
    {
        for (int i = size_arr(ui_state.rects) - 1; i >= 0; i--)
        {
            ui_hit = point_in_rect(ui_state.mouse_pos, ui_state.rects[i]);
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
        ui_wins[i].gridd_dimensions[0] = 0;
        ui_wins[i].gridd_dimensions[1] = 0;
        ui_wins[i].input_index = 0;
    }

    rect_index = RECTS_START;
    num_ui_rects = 0;

    get_head(ui_state.rects)->size = 0;

    get_head(ui_state.g_pipline.vert_buffer.data)->size = 0;

    win_idx = 0;
    win_hold_idx = 0;
}

static void set_dock_blue(Ui_Window* win, uint32 side_hit, float x_small_box,
                          float x_big_box)
{
    if (!dock_hit[side_hit])
    {
        blue_rects[side_hit] =
            quad(&ui_state.g_pipline.vert_buffer.data,
                 Vec3(x_small_box, (ui_state.dimensions.y * 0.5f) - 50.0f, -0.05f),
                 Vec2(60.0f, 100.0f), Vec4(0.1f, 0.1f, 1.0f, 0.5f));
    }
    else
    {
        dock_resized_rect =
            quad(&ui_state.g_pipline.vert_buffer.data, Vec3(x_big_box, 0.0f, -0.05f),
                 Vec2(win->dimensions.x, ui_state.dimensions.y),
                 Vec4(0.1f, 0.1f, 1.0f, 0.5f));
        dock_resized_rect.id = DOCKED_LEFT;
    }
    num_ui_rects++;
}

void gui_update_end()
{
    if (presist_hold)
    {
        blue_rects_index_offset = num_ui_rects * INDICES_PER_RECT;
        Ui_Window* win = &ui_wins[win_hold_idx - 1];
        set_dock_blue(win, LEFT_SIDE_HIT, 40.0f, 0.0f);
        set_dock_blue(win, RIGHT_SIDE_HIT, ui_state.dimensions.x - 100.0f,
                      ui_state.dimensions.x - win->dimensions.x);
    }
    else
    {
        blue_rects_index_offset = 0;
    }
    win_dock_hit_idx = 0;
    for (uint32 i = 0; i < 2; i++)
    {
        if ((dock_hit[i] = point_in_rect(ui_state.mouse_pos, blue_rects[i])))
        {
            win_dock_hit_idx = win_hold_idx;
            break;
        }
    }

    map_copy_mem(ui_state.device, &ui_state.g_pipline.vert_buffer.buffer_memory,
                 ui_state.g_pipline.vert_buffer.size_bytes,
                 ui_state.g_pipline.vert_buffer.data);

    ui_state.g_pipline.idx_buffer.curr_size = (num_ui_rects * INDICES_PER_RECT);

    num_wins = num_wins_frame;
    num_wins_frame = 0;
}

void back_bord_begin(const char* title, const Vec2& pos)
{
    Ui_Window* win = &ui_wins[win_idx];
    win->index_offset = IDX_OFFSET;
    if (win->first)
    {
        win->title_len = strlen(title);
        win->X_START = pos.x + 11.0f;
        win->Y_START = pos.y + 25.0f;
        win->first = false;
    }

    const bool hover = rect_index == index_hover;

    const bool top_bar_clicked = rect_index + 1 == index_clicked;
    const bool top_bar_hover = rect_index + 1 == index_hover;

    const bool rezise_right_clicked = rect_index + 2 == index_clicked;
    const bool rezise_right_hover = rect_index + 2 == index_hover;

    const bool rezise_left_clicked = rect_index + 3 == index_clicked;
    const bool rezise_left_hover = rect_index + 3 == index_hover;

    const bool rezise_buttom_clicked = rect_index + 4 == index_clicked;
    const bool rezise_buttom_hover = rect_index + 4 == index_hover;

    if (top_bar_clicked)
    {
        win->dyn_resize = true;
        win->extra_x_offset = 0;
        win->presist_offset_x = ui_state.mouse_pos.x - (win->X_START);
        win->presist_offset_y = ui_state.mouse_pos.y - (win->Y_START);
    }
    else if (rezise_left_clicked)
    {
        win->presist_offset_x = ui_state.mouse_pos.x;
        win->resize_hold = true;
        resize_idx = REZIZE_LEFT;
    }
    else if (rezise_right_clicked)
    {
        win->presist_offset_x = ui_state.mouse_pos.x - win->dimensions.x;
        win->resize_hold = true;
        resize_idx = REZIZE_RIGHT;
    }
    else if (rezise_buttom_clicked)
    {
        win->presist_offset_y = ui_state.mouse_pos.y - win->dimensions.y;
        win->resize_hold = true;
        resize_idx = REZIZE_BUTTOM;
    }
    if (win->presist_hold || ((top_bar_hover && ui_hold) && !is_holding))
    {
        change_cursor(SYNT_MOVE_CURSOR);

        win->X_START = ui_state.mouse_pos.x - win->presist_offset_x;
        win->Y_START = ui_state.mouse_pos.y - win->presist_offset_y;
        win->presist_hold = true;
        is_holding = true;
        presist_hold = true;
        win_hold_idx = win_idx + 1;

        recreate = true;
    }
    if (!ui_hold)
    {
        if (hover || win->presist_hold)
        {
            change_cursor(SYNT_NORMAL_CURSOR);
        }
        win->presist_hold = false;
        win->resize_hold = false;
        is_holding = false;
        presist_hold = false;
    }
    if (win_dock_hit_idx - 1 == win_idx)
    {
        if (!ui_hold)
        {
            win->X_START = dock_resized_rect.pos.x + 11.0f;
            win->Y_START = dock_resized_rect.pos.y + 25.0f;
            win->dimensions.y = dock_resized_rect.size.y;
            win->dyn_resize = false;
        }
    }

#define REZIZE_BAR_SIZE 10.0f

    float wide = win->biggest_wide + REZIZE_BAR_SIZE - (win->X_START - 11.0f);
    if (win->resize_hold)
    {
        recreate = true;
        is_holding = true;
        if (resize_idx == REZIZE_LEFT)
        {
            float change = (win->presist_offset_x - ui_state.mouse_pos.x);
            if (wide < win->dimensions.x)
            {
                win->X_START -= change;
            }
            win->dimensions.x += change;
            win->presist_offset_x = ui_state.mouse_pos.x;
        }
        else if (resize_idx == REZIZE_RIGHT)
        {
            win->dimensions.x = ui_state.mouse_pos.x - win->presist_offset_x;
        }
        else if (resize_idx == REZIZE_BUTTOM)
        {
            win->dimensions.y = ui_state.mouse_pos.y - win->presist_offset_y;
        }
    }
    if (wide > win->dimensions.x)
    {
        win->dimensions.x = wide;
        recreate = true;
    }
    float diff =
        ((win->X_START - 11.0f) + win->dimensions.x) - (ui_state.dimensions.x);
    if (diff > 0.0f)
    {
        win->X_START -= diff;
    }

    float high = 0;
    if (win->dyn_resize)
    {
        high = ((float)win->highest_high * 33.0f) + win->Y_START + win->extra_hight;

        high -= win->Y_START - 25.0f;
    }
    if (high > win->dimensions.y)
    {
        win->dimensions.y = high;
        recreate = true;
    }

    if (rezise_right_hover || rezise_left_hover)
    {
        win_idx_resize_hover = win_idx + 1;
        change_cursor(SYNT_RESIZE_H_CURSOR);
    }
    else if ((win_idx_resize_hover - 1 == win_idx) && !ui_hold)
    {
        change_cursor(SYNT_NORMAL_CURSOR);
        win_idx_resize_hover = 0;
    }
    if (rezise_buttom_hover)
    {
        change_cursor(SYNT_RESIZE_V_CURSOR);
    }

    uint32 out = 0;

    synt_push(ui_state.rects,
              quad_s(&ui_state.g_pipline.vert_buffer.data,
                     { win->X_START - 11.0f, win->Y_START - 25.0f, -0.12f },
                     win->dimensions, Vec4(0.2f, 0.2f, 0.2f, 1.0f)));
    synt_back(ui_state.rects).id = rect_index++;

    out += 2;

    // TODO: Maybe have a recreate in each window
    if (recreate)
    {
        Rect rect = synt_back(ui_state.rects);

        win->scissor.offset.x =
            (uint32)clampf32(rect.pos.x, 0.0f, ui_state.dimensions.x);
        int32_t diff_x = rect.pos.x < 0.0f ? (int32_t)rect.pos.x : 0;
        win->scissor.extent.width = (uint32)rect.size.x + diff_x + 1;

        win->scissor.offset.y = (int32_t)rect.pos.y;
        win->scissor.extent.height = (uint32)rect.size.y + 1;

        recreate = false;
    }

    synt_push(ui_state.rects,
              quad_s(&ui_state.g_pipline.vert_buffer.data,
                     { win->X_START - 11.0f, win->Y_START - 25.0f, -0.11f },
                     Vec2(win->dimensions.x, 20.0f), Vec4(1.0f, 0.0f, 0.03f, 1.0f)));
    synt_back(ui_state.rects).id = rect_index++;
    out += 2;

    synt_push(ui_state.rects,
              quad(&ui_state.g_pipline.vert_buffer.data,
                   { (win->X_START - 18.0f) + win->dimensions.x,
                     win->Y_START - 25.0f, -0.14f },
                   Vec2(8.0f, win->dimensions.y), Vec4(0.0f, 0.0f, 0.0f, 0.0f)));
    synt_back(ui_state.rects).id = rect_index++;
    out++;

    synt_push(ui_state.rects,
              quad(&ui_state.g_pipline.vert_buffer.data,
                   { (win->X_START - 11.0f), win->Y_START - 25.0f, -0.14f },
                   Vec2(8.0f, win->dimensions.y), Vec4(0.0f, 0.0f, 0.0f, 0.0f)));
    synt_back(ui_state.rects).id = rect_index++;
    out++;

    synt_push(ui_state.rects,
              quad(&ui_state.g_pipline.vert_buffer.data,
                   { (win->X_START - 11.0f),
                     (win->Y_START - 32.0f) + win->dimensions.y, -0.14f },
                   Vec2(win->dimensions.x, 8.0f), Vec4(0.0f, 0.0f, 0.0f, 0.0f)));
    synt_back(ui_state.rects).id = rect_index++;
    out++;

    if (title && *title)
    {
        out += text_2D(ui_state.font, title, strlen(title),
                       Vec3(win->X_START - 11.0f + (win->dimensions.x / 2.0f) -
                                ((win->title_len * BUTTON_SIZE_MULTI) / 2),
                            win->Y_START - 22.0f, -0.1f),
                       0.4f, &ui_state.g_pipline.vert_buffer.data);
    }

    num_ui_rects += out;

    win->biggest_wide = 0;

    num_wins_frame++;
}

void back_bord_end()
{
    // TODO: neeeeds to be fixed but can't be bother
    if (!ui_wins[win_idx].term)
    {
        ui_wins[win_idx].num_indices = IDX_OFFSET - ui_wins[win_idx].index_offset;
    }
    else
    {
    }

    ++win_idx;

    static bool first = true;
    if (first)
    {
        recreate = true;
        first = false;
    }
}

void gridd_begin(uint32 x, uint32 y)
{
    if (!x) x = 1;
    if (!y) y = 1;

    ui_wins[win_idx].gridd_dimensions[0] = x;
    ui_wins[win_idx].gridd_dimensions[1] += y;
    ui_wins[win_idx].gridd_start = true;

    if (ui_wins[win_idx].biggest_wide < x)
    {
        ui_wins[win_idx].biggest_wide = x;
    }

    ui_wins[win_idx].g_x = 0;
}

void gridd_end()
{
    ui_wins[win_idx].gridd_start = false;
}

static void update_misc()
{
    Ui_Window* win = &ui_wins[win_idx];
    if (++win->g_x == win->gridd_dimensions[0])
    {
        win->g_x = 0;

        float wide = win->x_offset_button + win->last_button_width;
        if (wide > win->biggest_wide)
        {
            win->biggest_wide = wide;
        }
        win->x_offset_button = win->X_START;

        if (++win->g_y >= win->gridd_dimensions[1])
        {
            win->gridd_start = false;
            win->last_button_width = 0;
        }
    }
}

bool add_button(const char* text)
{
    Ui_Window* win = &ui_wins[win_idx];
    if (!win->gridd_start)
    {
        SY_ERROR("Gridd overflow or is not started");
        return 0;
    }

    const bool clicked = rect_index == index_clicked;
    const bool hover = rect_index == index_hover;

    Vec4 button_color = Vec4(0.5f, 0.5f, 0.5f, 1.0f);
    if (hover && !ui_hold)
    {
        button_color = Vec4(0.7f, 0.7f, 0.7f, 1.0f);
        change_cursor(SYNT_HAND_CURSOR);
    }

#define MIN_BUTTON_SIZE 50.0f

    float button_width = (float)strlen(text) * BUTTON_SIZE_MULTI;
    if (button_width < MIN_BUTTON_SIZE)
    {
        button_width = MIN_BUTTON_SIZE;
    }
    if (win->g_x != 0) win->x_offset_button += win->last_button_width + 10.0f;

    synt_push(ui_state.rects,
              quad_s(&ui_state.g_pipline.vert_buffer.data,
                     { win->extra_x_offset + win->x_offset_button,
                       win->Y_START + (win->g_y * 30.0f), -0.11f },
                     Vec2(button_width, 20.0f), button_color, BUTTON_TEXURE));

    uint32 out = 2;

    synt_back(ui_state.rects).id = rect_index++;

    if (text && *text)
    {
        out += text_2D(ui_state.font, text, strlen(text),
                       Vec3(win->extra_x_offset + win->x_offset_button + 2.0f,
                            win->Y_START + 2.0f + (win->g_y * 30.0f), -0.1f),
                       0.4f, &ui_state.g_pipline.vert_buffer.data);
    }
    win->last_button_width = button_width;
    update_misc();
    num_ui_rects += out;
    return clicked;
}

static bool is_letter_number(uint16 key)
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

bool add_input_float(float& input, float min, float max)
{
    if (!ui_wins[win_idx].gridd_start)
    {
        SY_ERROR("Gridd overflow or is not started\n");
        return 0;
    }

    const bool clicked = rect_index == index_clicked;
    const bool hover = rect_index == index_hover;

    Input_Float* curr_input =
        &ui_wins[win_idx].input_floats[ui_wins[win_idx].input_index];

    curr_input->min = min;
    curr_input->max = max;

    if (curr_input->presist_hold || ((hover && ui_hold) && !is_holding))
    {
        const int16 mouse_x = ui_state.mouse_evt->mouse_evt.move_evt.pos_x;

        static int16 last_x = mouse_x;

        bool moved = false;
        if (!clicked)
        {
            if (last_x < mouse_x)
            {
                if (!curr_input->highlight_on)
                {
                    float multiplier = (float)(mouse_x - last_x);
                    input += 0.01f * multiplier;
                }
                moved = true;
            }
            else if (last_x > mouse_x)
            {
                if (!curr_input->highlight_on)
                {
                    float multiplier = (float)(last_x - mouse_x);
                    input -= 0.01f * multiplier;
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
    if (clicked || curr_input->presist_clicked)
    {
        curr_input->presist_clicked = true;

        Events* key_evt = ui_state.key_evt;
        if (key_evt->activated && key_evt->key_evt.action)
        {
            curr_input->highlight_on = false;

            uint16 key = key_evt->key_evt.key;
            char letter;
            if (key == SYNT_KEY_ENTER)
            {
                curr_input->curr_index = 0;
                curr_input->presist_clicked = false;

                input = (float)atof(curr_input->text);
                input = clampf32(input, min, max);
                sprintf(curr_input->text, "%f", input);

                memcpy(curr_input->last_text, curr_input->text,
                       sizeof(curr_input->last_text));
            }
            else if (key == SYNT_KEY_BACKSPACE)
            {
                curr_input
                    ->text[curr_input->curr_index != 0 ? --curr_input->curr_index
                                                       : 0] = '\0';
            }
            else
            {
                if (is_letter_number(key))
                {
                    letter = (char)code_to_ascii(key);
                    if (curr_input->curr_index < 14)
                    {
                        curr_input->text[curr_input->curr_index++] = letter;
                        curr_input->text[curr_input->curr_index] = '\0';
                    }
                }
            }
        }
        if (!clicked && index_clicked)
        {
            memcpy(curr_input->text, curr_input->last_text,
                   sizeof(curr_input->text));
            curr_input->curr_index = 0;
            curr_input->presist_clicked = false;

            curr_input->highlight_on = false;
        }
    }
    float wide = strlen(curr_input->text) * BUTTON_SIZE_MULTI;
    Ui_Window* win = &ui_wins[win_idx];
    if (wide < 50.0f)
    {
        wide = 50.0f;
    }
    if (win->last_button_width < 50.0f)
    {
        win->last_button_width = 50.0f;
    }
    if (win->g_x) win->x_offset_button += win->last_button_width + 10.0f;

    uint32 out = 0;

    synt_push(ui_state.rects,
              quad_s(&ui_state.g_pipline.vert_buffer.data,
                     { win->extra_x_offset + win->x_offset_button,
                       win->Y_START + (win->g_y * 30.0f), -0.11f },
                     Vec2(wide, 20.0f), Vec4(0.8f, 0.8f, 0.8f, 1.0f)));

    out += 2;

    if (curr_input->highlight_on)
    {
        quad(&ui_state.g_pipline.vert_buffer.data,
             { win->extra_x_offset + win->x_offset_button + 2.5f,
               win->Y_START + (win->g_y * 30.0f) + 2.0f, -0.105f },
             Vec2(wide - 5.0f, 16.0f), Vec4(0.0f, 0.0f, 1.0f, 0.7f));
        out++;
    }

    synt_back(ui_state.rects).id = rect_index++;

    out += text_2D(ui_state.font, curr_input->text, strlen(curr_input->text),
                   Vec3(win->extra_x_offset + win->x_offset_button + 3.0f,
                        win->Y_START + 2.0f + (win->g_y * 30.0f), -0.1f),
                   0.4f, &ui_state.g_pipline.vert_buffer.data);

    win->last_button_width = wide;
    num_ui_rects += out;
    win->input_index++;
    update_misc();

    return clicked;
}

// TODO: support for new line in text.
void add_text(const char* text)
{
    uint32 out = 0;
    Ui_Window* win = &ui_wins[win_idx];
    if (win->last_button_width < 50.0f)
    {
        win->last_button_width = 50.0f;
    }
    if (win->g_x) win->x_offset_button += win->last_button_width + 10.0f;
    if (text && *text)
    {
#if 0
        out += text_2D_ttf(ui_state.font_ttf, text,
                           Vec3(win->x_offset_button + 2.0f,
                                win->Y_START + 0.0f + (win->g_y * 30.0f), -0.1f),
                           1.0f, &ui_state.g_pipline.vert_buffer.data);
#endif
        out += text_2D(ui_state.font, text, strlen(text),
                       Vec3(win->x_offset_button + 2.0f,
                            win->Y_START + 2.0f + (win->g_y * 30.0f), -0.1f),
                       0.4f, &ui_state.g_pipline.vert_buffer.data);
    }
    win->last_button_width = 10.0f;
    num_ui_rects += out;
    update_misc();
}

static uint32_t new_lines = 0;

void print_text(char* text)
{
    char* temp_text = text;
    for (; *temp_text != '\0'; temp_text++)
    {
        if (*temp_text == '\n')
        {
            new_lines++;
        }
        synt_push(ui_state.terminal_buffer, *temp_text);
    }
}

#define BORDER_THICKNESS 5.0f

void add_terminal(float width, float height)
{
    static uint32 top_left_index = 0;
    static uint32 bottom_right_index = 0;

    char* buffer = ui_state.terminal_buffer;
    Ui_Window* win = &ui_wins[win_idx];
    Vertex_Buffer* vert = &ui_state.g_pipline.vert_buffer;

    if (add_button("Auto scroll"))
    {
        term.auto_scroll = true;
    }

    float extra_padding = 8.0f;

    Vec3 pos =
        Vec3(win->x_offset_button + extra_padding + BORDER_THICKNESS,
             win->Y_START + 2.0f + BORDER_THICKNESS + (win->g_y * 30.0f), -0.1f);

    Vec3 top_left = Vec3(pos.x - BORDER_THICKNESS - extra_padding,
                         pos.y - BORDER_THICKNESS - extra_padding, pos.z);

    Vec3 sides_pos = Vec3(top_left.x, top_left.y + BORDER_THICKNESS, pos.z);

    static bool first = true;
    if (first)
    {
        term.dimensions = Vec2(width, height);
        first = false;
    }
    else
    {
        term.dimensions.x = win->dimensions.x - 20.0f;
    }

    uint32 buffer_size = size_arr(buffer);

    Vec2 term_H_size = Vec2(term.dimensions.x, BORDER_THICKNESS);
    Vec2 term_V_size =
        Vec2(BORDER_THICKNESS, term.dimensions.y + BORDER_THICKNESS + extra_padding);

    term.scissor.offset.x = pos.x - extra_padding;
    term.scissor.offset.y = sides_pos.y;
    term.scissor.extent.width = (uint32)(term.dimensions.x - BORDER_THICKNESS);
    term.scissor.extent.height = (uint32)term.dimensions.y + extra_padding;

    quad_s(&vert->data, top_left, term_H_size, Vec4(0.8f, 0.0f, 0.03f, 1.0f),
           DEFAULT_TEXURE, 1.0f);
    num_ui_rects += 2;

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
        if (new_dim + (sides_pos.y + BORDER_THICKNESS + extra_padding -
                       (win->Y_START - HEADER_HEIGHT)) <
            win->dimensions.y)
        {
            term.dimensions.y = new_dim;
        }
    }
    if (!ui_hold)
    {
        term.presist_hold = false;
    }

    synt_push(
        ui_state.rects,
        quad_s(&vert->data, Vec3(top_left.x, pos.y + term.dimensions.y, top_left.z),
               term_H_size, Vec4(0.8f, 0.0f, 0.03f, 1.0f), DEFAULT_TEXURE, 1.0f));
    synt_back(ui_state.rects).id = rect_index++;
    num_ui_rects += 2;

    quad_s(&vert->data, sides_pos, term_V_size, Vec4(0.8f, 0.0f, 0.03f, 1.0f),
           DEFAULT_TEXURE, 1.0f);
    num_ui_rects += 2;

    quad_s(&vert->data,
           Vec3(sides_pos.x + term.dimensions.x - BORDER_THICKNESS, sides_pos.y,
                sides_pos.z),
           term_V_size, Vec4(0.8f, 0.0f, 0.03f, 1.0f), DEFAULT_TEXURE, 1.0f);
    num_ui_rects += 2;

    // TODO: Need to fix this more smoothly
    win->num_indices = (num_ui_rects * INDICES_PER_RECT) - win->index_offset;

    term.index_offset = IDX_OFFSET;

    const bool terminal_clicked = rect_index == index_clicked;
    const bool terminal_hover = rect_index == index_hover;

    synt_push(ui_state.rects,
              quad(&vert->data,
                   Vec3(term.scissor.offset.x, sides_pos.y, pos.z - 0.001f),
                   Vec2(term.scissor.extent.width, term_V_size.y - BORDER_THICKNESS),
                   Vec4(1.0f)));
    synt_back(ui_state.rects).id = rect_index++;
    num_ui_rects++;

    // Text moving upp

    static float line_height = 0;
    static float buffer_height = 0;
    static float buffer_diff = 0;

    if (terminal_hover)
    {
        change_cursor(SYNT_NORMAL_CURSOR);
        if (ui_state.wheel_evt->activated)
        {
            term.auto_scroll = false;
            buffer_diff += ((float)ui_state.wheel_evt->wheel_evt.z_delta * 0.3f);
        }
    }
    if (term.auto_scroll)
    {
        line_height = (float)ui_state.font.line_height * 0.4f;
        buffer_height = line_height * (float)new_lines;
        buffer_diff = term.dimensions.y - (line_height + buffer_height);
    }
    num_ui_rects +=
        text_2D(ui_state.font, buffer, buffer_size,
                Vec3(pos.x, pos.y + buffer_diff, pos.z), 0.4f, &vert->data);

    term.num_indices = IDX_OFFSET - term.index_offset;
    win->last_button_width = width;
    win->extra_hight = height;
    update_misc();
    win->term = true;
}

void destroy_gui(VkDevice device, uint32 num_semaphores)
{
    destroy_graphic_pipeline(device, num_semaphores, ui_state.g_pipline);

    for (uint32 i = 0; i < size_arr(ui_state.textures); i++)
    {
        destroy_texture(device, ui_state.textures[i]);
    }
}

bool gui_focus()
{
    return ui_hit || ui_hold;
}


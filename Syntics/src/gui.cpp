#include "gui.h"
#include "vulkan_types.h"
#include "buffers.h"
#include "event_system.h"
#include "font.h"
#include "region_alloc.h"
#include "swap_chain.h"
#include "camera.h"
#include "collision.h"
#include <stdlib.h>
#include <string.h>

#define MAX_SPACE 10000
#define RECTS_START 2
#define BUTTON_SIZE_MULTI 8.3f
#define Y_START_SHADOW ui_wins[win_idx].Y_START + 2.0f
#define DOCKED_LEFT 1
#define DOCKED_RIGHT 2

namespace synt {

typedef struct Input_Float
{
    uint32 curr_index    = 0;
    uint32 frames_moved  = 0;
    float max            = 0;
    float min            = 0;
    char text[15]        = {};
    char last_text[15]   = {};
    bool presist_clicked = false;
    bool presist_hold    = false;
    bool highlight_on    = false;
    bool dot_used        = false;
} Input_Float;

typedef struct Ui_Window
{
    Input_Float input_floats[10];
    uint32 input_index = 0;
    bool gridd_start   = false;
    uint32 gridd_dimensions[2];
    uint32 g_x = 0;
    uint32 g_y = 0;

    float biggest_wide     = 0;
    float latest_wide      = 0;
    float biggest_x_offset = 0;
    uint32 highest_high    = 0;

    float X_START = 11.0f;
    float Y_START = 30.0f;

    float last_button_wide = 0;
    float x_offset_button  = X_START;

    float extra_x_offset = 0;

    float presist_offset_x = 0;
    float presist_offset_y = 0;
    bool presist_hold      = false;

    uint32 title_len = 0;

    uint32 resize_idx = 0;

    Vec2 dimensions;

    // TODO: like many other things are temp solution.
    bool first = true;

    bool dyn_resize  = true;
    bool resize_hold = false;

} Ui_Window;

typedef struct Ui_State
{
    Graphic_Pipline g_pipline;

    Events* mouse_evt;
    Events* key_evt;

    Font font;
    Texture* textures;
    Rect* rects;

    Vec2 dimensions;
    Vec2 mouse_pos;

    Camera cam;

} Ui_State;

static Ui_State ui_state;
static Ui_Window ui_wins[3];
static uint32 win_idx        = 0;
static uint32 num_wins       = 0;
static uint32 num_wins_frame = 0;
static uint32 rect_index     = RECTS_START;
static uint32 index_hover    = 0;
static uint32 index_clicked  = 0;
static bool ui_hit           = false;
static bool ui_hold          = false;
static uint32 num_ui_rects   = 0;

static float presist_offset_x = 0.0f;
static float presist_offset_y = 0.0f;
static bool presist_hold      = false;

static bool is_holding = false;

static Rect blue_rects[2]      = {};
static bool dock_hit[2]        = {};
static Rect dock_resized_rect  = {};
static uint32 win_hold_idx     = 0;
static uint32 win_dock_hit_idx = 0;

static uint32 win_idx_resize_hover = 0;

static void generate_indices(uint32** data, uint32 num_indices)
{
    for (uint32 i = 0; i < num_indices; i++)
    {
        synt_push((*data), 0 + (4 * i));
        synt_push((*data), 1 + (4 * i));
        synt_push((*data), 2 + (4 * i));
        synt_push((*data), 2 + (4 * i));
        synt_push((*data), 3 + (4 * i));
        synt_push((*data), 0 + (4 * i));
    }
}

void gui_init(Region_Alloc* region, VkDevice device,
              VkPhysicalDevice physical_device, VkCommandPool command_pool,
              VkQueue graphic_queue, const Swap_Chain_attrib& swap_chain,
              uint32 num_semaphores)
{
    subscribe(&ui_state.key_evt, EVT_KEY);
    subscribe(&ui_state.mouse_evt, EVT_MOUSE);

    ui_state.textures = dyn_arrayP((*region), 3, Texture);

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

    create_graphics_pipeline(region, device, swap_chain.color_format,
                             swap_chain.render_pass, swap_chain.sample_count,
                             "Syntics/res/gui.vert.spv", "Syntics/res/gui.frag.spv",
                             swap_chain.extent_2D.width, swap_chain.extent_2D.height,
                             VK_CULL_MODE_NONE, size_arr(ui_state.textures),
                             &ui_state.g_pipline);

    ui_state.font           = load_font_file("Syntics/res/Mono.fnt");
    ui_state.font.tex_index = 1.0f;

    uint32 num_ui_rects = 10;
    ui_state.rects      = dyn_arrayP((*region), num_ui_rects * 3, Rect);

    ui_state.g_pipline.vert_buffer.data = NULL;

    ui_state.g_pipline.vert_buffer.size_bytes =
        ((num_ui_rects + MAX_SPACE) * 4) * sizeof(Vertex);
    create_vertex_buffer(device, physical_device, command_pool, graphic_queue,
                         &ui_state.g_pipline.vert_buffer);

    ui_state.g_pipline.idx_buffer.data =
        dyn_arrayP((*region), (num_ui_rects + MAX_SPACE) * 6, uint32);
    generate_indices(&ui_state.g_pipline.idx_buffer.data, num_ui_rects + MAX_SPACE);
    ui_state.g_pipline.idx_buffer.size_bytes =
        capacity_arr(ui_state.g_pipline.idx_buffer.data) * sizeof(uint32);
    create_index_buffer(device, physical_device, command_pool, graphic_queue,
                        &ui_state.g_pipline.idx_buffer);

    region_pop((*region), capacity_arr(ui_state.g_pipline.idx_buffer.data), uint32,
               PERM_ARRAY);
    ui_state.g_pipline.idx_buffer.data = NULL;

    ui_state.g_pipline.uniform_buffers =
        region_mallocP((*region), num_semaphores, Uniform_Buffer);
    ui_state.g_pipline.descriptors.desc_sets =
        region_mallocP((*region), num_semaphores, VkDescriptorSet);

    for (uint32 i = 0; i < num_semaphores; i++)
    {
        ui_state.g_pipline.uniform_buffers[i].size_bytes = (uint32)sizeof(MVP);

        create_uniform_buffer(device, physical_device,
                              &ui_state.g_pipline.uniform_buffers[i]);
    }
    create_descriptors(region, device, &ui_state.g_pipline.descriptors,
                       num_semaphores, ui_state.g_pipline.set_layout,
                       ui_state.textures, size_arr(ui_state.textures),
                       ui_state.g_pipline.uniform_buffers);

    ui_state.cam.position    = synt::v3f(0.0f, 0.0f, 0.0f);
    ui_state.cam.orientation = synt::v3f(0.0f, 0.0f, 0.0f);
    ui_state.cam.mvp.model   = mat4i(1.0f);
    ui_state.cam.mvp.view    = mat4i(1.0f);
}

void gui_render(VkCommandBuffer command_buffer, uint32 semaphore_idx)
{
    bind_and_draw_graphics_pipline(
        command_buffer, ui_state.g_pipline.descriptors.desc_sets[semaphore_idx],
        ui_state.g_pipline, true);
}

void gui_recreate(Region_Alloc* region, const Application_State& app_state)
{
    recreate_graphic_pipline(region, app_state, "Syntics/res/gui.vert.spv",
                             "Syntics/res/gui.frag.spv", ui_state.g_pipline,
                             size_arr(ui_state.textures));
}

void gui_update_begin(Region_Alloc* region, VkDevice device, const Vec2& dimensions,
                      uint32 semaphore_idx, float delta)
{
    // Because vulkan is flipped this results in the oposite for y axis :|
    ui_state.cam.mvp.proj = ortho(0, 0, dimensions.x, dimensions.y, -1.0f, 1.0f);
    update_uniform_buffers(device, ui_state.g_pipline.uniform_buffers[semaphore_idx],
                           &ui_state.cam.mvp, sizeof(ui_state.cam.mvp));

    ui_state.dimensions = dimensions;
    ui_state.mouse_pos  = Vec2((float)ui_state.mouse_evt->mouse_evt.move_evt.pos_x,
                               (float)ui_state.mouse_evt->mouse_evt.move_evt.pos_y);

    index_hover               = 0;
    index_clicked             = 0;
    static bool first_clicked = true;
    bool button_clicked       = is_any_button_clicked(first_clicked);
    uint8 action              = ui_state.mouse_evt->mouse_evt.button_evt.action;
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
                    ui_hold       = true;
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
        SYNT_CHANGE_CURSOR(SYNT_NORMAL_CURSOR);
    }
    if (!action)
    {
        ui_hold = false;
    }
    for (uint32 i = 0; i < num_wins; i++)
    {
        ui_wins[i].highest_high        = ui_wins[i].g_y;
        ui_wins[i].g_x                 = 0;
        ui_wins[i].g_y                 = 0;
        ui_wins[i].gridd_dimensions[0] = 0;
        ui_wins[i].gridd_dimensions[1] = 0;
        ui_wins[i].input_index         = 0;
    }

    rect_index   = RECTS_START;
    num_ui_rects = 0;

    get_head(ui_state.rects)->size = 0;

    ui_state.g_pipline.vert_buffer.data =
        dyn_arrayP((*region), (num_ui_rects + MAX_SPACE) * 4, Vertex);

    win_idx      = 0;
    win_hold_idx = 0;
}

void gui_update_end(Region_Alloc* region, VkDevice device)
{

    // TODO: Bug, gets bigger when docking on right
    if (presist_hold)
    {
        if (!dock_hit[0])
        {
            blue_rects[0] =
                quad(&ui_state.g_pipline.vert_buffer.data,
                     { 40.0f, (ui_state.dimensions.y / 2.0f) - 50.0f, -0.05f },
                     Vec2(60.0f, 100.0f), Vec4(0.1f, 0.1f, 1.0f, 0.5f), 0.0f);
        }
        else
        {
            Ui_Window* win = &ui_wins[win_hold_idx - 1];
            dock_resized_rect =
                quad(&ui_state.g_pipline.vert_buffer.data, { 0.0f, 0.0f, -0.05f },
                     Vec2(win->dimensions.x, ui_state.dimensions.y),
                     Vec4(0.1f, 0.1f, 1.0f, 0.5f), 0.0f);
            dock_resized_rect.id = DOCKED_LEFT;
        }
        num_ui_rects++;
        if (!dock_hit[1])
        {
            blue_rects[1] =
                quad(&ui_state.g_pipline.vert_buffer.data,
                     { ui_state.dimensions.x - 100.0f,
                       (ui_state.dimensions.y / 2.0f) - 50.0f, -0.05f },
                     Vec2(60.0f, 100.0f), Vec4(0.1f, 0.1f, 1.0f, 0.5f), 0.0f);
        }
        else
        {
            Ui_Window* win = &ui_wins[win_hold_idx - 1];
            dock_resized_rect =
                quad(&ui_state.g_pipline.vert_buffer.data,
                     { ui_state.dimensions.x - win->dimensions.x, 0.0f, -0.05f },
                     Vec2(win->dimensions.x, ui_state.dimensions.y),
                     Vec4(0.1f, 0.1f, 1.0f, 0.5f), 0.0f);
            dock_resized_rect.id = DOCKED_RIGHT;
        }
        num_ui_rects++;
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

    ui_state.g_pipline.vert_buffer.size_bytes =
        capacity_arr(ui_state.g_pipline.vert_buffer.data) * sizeof(Vertex);

    map_copy_mem(device, &ui_state.g_pipline.vert_buffer.buffer_memory,
                 ui_state.g_pipline.vert_buffer.size_bytes,
                 ui_state.g_pipline.vert_buffer.data);

    region_pop((*region), capacity_arr(ui_state.g_pipline.vert_buffer.data), Vertex,
               PERM_ARRAY);
    ui_state.g_pipline.vert_buffer.data = NULL;

    ui_state.g_pipline.idx_buffer.curr_size = (num_ui_rects * 6);

    num_wins       = num_wins_frame;
    num_wins_frame = 0;
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

void gridd_end() { ui_wins[win_idx].gridd_start = false; }

void back_bord_begin(const char* title, const Vec2& pos)
{
    Ui_Window* win = &ui_wins[win_idx];
    if (win->first)
    {
        win->title_len = strlen(title);
        win->X_START   = pos.x + 11.0f;
        win->Y_START   = pos.y + 25.0f;
        win->first     = false;
    }
    if (win->dyn_resize)
    {
        win->dimensions.y = ((float)win->highest_high * 33.0f) + win->Y_START;
        win->dimensions.y -= win->Y_START - 25.0f;
    }

    bool hover = rect_index == index_hover;
    // First rect is of no intresst. Hense "+ 1".
    bool top_bar_clicked = rect_index + 1 == index_clicked;
    bool top_bar_hover   = rect_index + 1 == index_hover;

    bool rezise_right_clicked = rect_index + 2 == index_clicked;
    bool rezise_right_hover   = rect_index + 2 == index_hover;

    bool rezise_left_clicked = rect_index + 3 == index_clicked;
    bool rezise_left_hover   = rect_index + 3 == index_hover;

    if (top_bar_clicked)
    {
        win->dyn_resize       = true;
        win->extra_x_offset   = 0;
        win->presist_offset_x = ui_state.mouse_pos.x - (win->X_START);
        win->presist_offset_y = ui_state.mouse_pos.y - (win->Y_START);
    }
    else if (rezise_left_clicked)
    {
        win->presist_offset_x = ui_state.mouse_pos.x;
        win->resize_hold      = true;
        win->resize_idx       = 1;
    }
    else if (rezise_right_clicked)
    {
        win->presist_offset_x = ui_state.mouse_pos.x - win->dimensions.x;
        win->resize_hold      = true;
        win->resize_idx       = 2;
    }
    if (win->presist_hold || ((top_bar_hover && ui_hold) && !is_holding))
    {
        SYNT_CHANGE_CURSOR(SYNT_MOVE_CURSOR);

        win->X_START      = ui_state.mouse_pos.x - win->presist_offset_x;
        win->Y_START      = ui_state.mouse_pos.y - win->presist_offset_y;
        win->presist_hold = true;
        is_holding        = true;
        presist_hold      = true;
        win_hold_idx      = win_idx + 1;

        win->dimensions.y = ((float)win->highest_high * 33.0f) + win->Y_START;

        win->dimensions.y -= win->Y_START - 25.0f;
    }
    if (!ui_hold)
    {
        if (hover || win->presist_hold)
        {
            SYNT_CHANGE_CURSOR(SYNT_NORMAL_CURSOR);
        }
        win->presist_hold = false;
        win->resize_hold  = false;
        is_holding        = false;
        presist_hold      = false;
    }
    if (win_dock_hit_idx - 1 == win_idx)
    {
        static bool first = true;
        if (!ui_hold)
        {
            // Things snaps to to right side. right side align
#if 0
            if (dock_resized_rect.id == DOCKED_RIGHT)
            {
                if (first)
                {
                    win->extra_x_offset =
                        win->dimensions.x -
                        ((win->biggest_x_offset + win->latest_wide + 10.0f) -
                         (win->X_START - 11.0f));

                    first = false;
                }
            }
#endif
            win->X_START      = dock_resized_rect.pos.x + 11.0f;
            win->Y_START      = dock_resized_rect.pos.y + 25.0f;
            win->dimensions.y = dock_resized_rect.size.y;
            win->dyn_resize   = false;
        }
#if 0 
        else
        {
            first = true;
        }
#endif
    }
    float wide;
    if (win->biggest_wide > (win->biggest_x_offset + win->latest_wide))
    {
        wide = win->biggest_wide + 10.0f;
    }
    else
    {
        wide = win->biggest_x_offset + win->latest_wide + 10.0f;
    }
    wide -= win->X_START - 11.0f;
    if (win->resize_hold)
    {
        if (win->resize_idx == 1)
        {
            float change = (win->presist_offset_x - ui_state.mouse_pos.x);
            if (wide < win->dimensions.x)
            {
                win->X_START -= change;
            }
            win->dimensions.x += change;
            win->presist_offset_x = ui_state.mouse_pos.x;
        }
        else if (win->resize_idx == 2)
        {
            win->dimensions.x = ui_state.mouse_pos.x - win->presist_offset_x;
        }
    }
    if (wide > win->dimensions.x && !is_holding)
    {
        win->dimensions.x = wide;
    }
    float diff =
        ((win->X_START - 11.0f) + win->dimensions.x) - (ui_state.dimensions.x);
    if (diff > 0.0f)
    {
        win->X_START -= diff;
    }
    else if (diff > -300.0f && !win->dyn_resize)
    {
        win->X_START -= diff;
    }
    if (rezise_right_hover || rezise_left_hover)
    {
        win_idx_resize_hover = win_idx + 1;
        SYNT_CHANGE_CURSOR(SYNT_RESIZE_CURSOR);
    }
    else if ((win_idx_resize_hover - 1 == win_idx) && !ui_hold)
    {
        SYNT_CHANGE_CURSOR(SYNT_NORMAL_CURSOR);
        win_idx_resize_hover = 0;
    }

    uint32 out = 0;

    quad(&ui_state.g_pipline.vert_buffer.data,
         { win->X_START - 9.0f, win->Y_START - 23.0f, -0.13f }, win->dimensions,
         Vec4(0.0f, 0.0f, 0.0f, 0.7f), 0.0f);
    out++;

    synt_push(ui_state.rects,
              quad(&ui_state.g_pipline.vert_buffer.data,
                   { win->X_START - 11.0f, win->Y_START - 25.0f, -0.12f },
                   win->dimensions, Vec4(0.2f, 0.2f, 0.2f, 1.0f), 0.0f));
    synt_back(ui_state.rects).id = rect_index++;
    out++;

    synt_push(ui_state.rects,
              quad(&ui_state.g_pipline.vert_buffer.data,
                   { win->X_START - 9.0f, win->Y_START - 23.0f, -0.111f },
                   Vec2(win->dimensions.x, 20.0f), Vec4(0.0f, 0.0f, 0.0f, 0.7f),
                   0.0f));
    synt_back(ui_state.rects).id = rect_index++;
    out++;

    quad(&ui_state.g_pipline.vert_buffer.data,
         { win->X_START - 11.0f, win->Y_START - 25.0f, -0.11f },
         Vec2(win->dimensions.x, 20.0f), Vec4(1.0f, 0.0f, 0.03f, 1.0f), 0.0f);
    out++;

    synt_push(ui_state.rects, quad(&ui_state.g_pipline.vert_buffer.data,
                                   { (win->X_START - 18.0f) + win->dimensions.x,
                                     win->Y_START - 25.0f, -0.14f },
                                   Vec2(8.0f, win->dimensions.y),
                                   Vec4(0.0f, 0.0f, 0.0f, 0.0f), 0.0f));
    synt_back(ui_state.rects).id = rect_index++;
    out++;

    synt_push(ui_state.rects,
              quad(&ui_state.g_pipline.vert_buffer.data,
                   { (win->X_START - 11.0f), win->Y_START - 25.0f, -0.14f },
                   Vec2(8.0f, win->dimensions.y), Vec4(0.0f, 0.0f, 0.0f, 0.0f),
                   0.0f));
    synt_back(ui_state.rects).id = rect_index++;
    out++;

    if (title && *title)
    {
        out += text_2D(ui_state.font, title,
                       Vec3(win->X_START - 11.0f + (win->dimensions.x / 2.0f) -
                                ((win->title_len * BUTTON_SIZE_MULTI) / 2),
                            win->Y_START - 22.0f, -0.1f),
                       0.4f, &ui_state.g_pipline.vert_buffer.data);
    }

    num_ui_rects += out;

    win->biggest_x_offset = 0;
    win->biggest_wide     = 0;

    num_wins_frame++;
}

void back_bord_end() { win_idx++; }

static void update_misc()
{
    Ui_Window* win = &ui_wins[win_idx];
    if (++win->g_x == win->gridd_dimensions[0])
    {
        win->g_x = 0;
        if (win->x_offset_button > win->biggest_x_offset)
        {
            win->biggest_x_offset = win->x_offset_button;
            win->latest_wide      = win->last_button_wide;
        }
        // Dynamic resize on text
#if 0
        if ((win->x_offset_button + win->last_button_wide) >
             (win->biggest_x_offset + win->latest_wide))
        {
             win->biggest_x_offset = win->x_offset_button;
             win->latest_wide      = win->last_button_wide;
        }
        if (win->last_button_wide > win->biggest_wide)
        {
            win->biggest_wide = win->last_button_wide;
        }
#endif
        win->x_offset_button = win->X_START;
        if (++win->g_y == win->gridd_dimensions[1])
        {
            win->gridd_start      = false;
            win->last_button_wide = 0;
        }
    }
}

bool add_button(const char* text)
{
    Ui_Window* win = &ui_wins[win_idx];
    if (!win->gridd_start)
    {
        synt_LOG("Gridd overflow or is not started\n");
        return 0;
    }

    bool clicked = rect_index == index_clicked;
    bool hover   = rect_index == index_hover;

    Vec4 button_color = Vec4(0.5f, 0.5f, 0.5f, 1.0f);
    if (hover && !ui_hold)
    {
        button_color = Vec4(0.7f, 0.7f, 0.7f, 1.0f);
        SYNT_CHANGE_CURSOR(SYNT_HAND_CURSOR);
    }

    float wide = (float)strlen(text) * BUTTON_SIZE_MULTI;
    if (wide < 50.0f)
    {
        wide = 50.0f;
    }
    if (win->last_button_wide < 50.0f)
    {
        win->last_button_wide = 50.0f;
    }
    if (win->g_x) win->x_offset_button += win->last_button_wide + 10.0f;

    quad(&ui_state.g_pipline.vert_buffer.data,
         { win->extra_x_offset + win->x_offset_button + 2.0f,
           Y_START_SHADOW + (win->g_y * 30.0f), -0.111f },
         Vec2(wide, 20.0f), Vec4(0.0f, 0.0f, 0.0f, 0.7f), 2.0f);

    synt_push(ui_state.rects, quad(&ui_state.g_pipline.vert_buffer.data,
                                   { win->extra_x_offset + win->x_offset_button,
                                     win->Y_START + (win->g_y * 30.0f), -0.11f },
                                   Vec2(wide, 20.0f), button_color, 2.0f));

    uint32 out = 2;

    synt_back(ui_state.rects).id = rect_index++;

    if (text && *text)
    {
        out += text_2D(ui_state.font, text,
                       Vec3(win->extra_x_offset + win->x_offset_button + 2.0f,
                            win->Y_START + 2.0f + (win->g_y * 30.0f), -0.1f),
                       0.4f, &ui_state.g_pipline.vert_buffer.data);
    }
    win->last_button_wide = wide;
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
        synt_LOG("Gridd overflow or is not started\n");
        return 0;
    }

    bool clicked = rect_index == index_clicked;
    bool hover   = rect_index == index_hover;

    Input_Float* curr_input =
        &ui_wins[win_idx].input_floats[ui_wins[win_idx].input_index];

    curr_input->min = min;
    curr_input->max = max;

    if (curr_input->presist_hold || ((hover && ui_hold) && !is_holding))
    {
        int16 mouse_x = ui_state.mouse_evt->mouse_evt.move_evt.pos_x;

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
                curr_input->highlight_on    = false;
                curr_input->curr_index      = 0;
                curr_input->presist_clicked = false;
                curr_input->frames_moved    = 0;
            }
            if (!curr_input->highlight_on)
            {
                input = clampf32(input, min, max);
                gcvt(input, 8, curr_input->text);
            }
            memcpy(curr_input->last_text, curr_input->text,
                   sizeof(curr_input->last_text));
        }
        last_x = mouse_x;

        curr_input->presist_hold = true;
        is_holding               = true;
        SYNT_CHANGE_CURSOR(SYNT_RESIZE_CURSOR);
    }
    if (!ui_hold)
    {
        if (curr_input->presist_hold)
        {
            SYNT_CHANGE_CURSOR(SYNT_NORMAL_CURSOR);
        }
        curr_input->presist_hold = false;
        is_holding               = false;
        curr_input->frames_moved = 0;
    }
    if (clicked)
    {
        input = clampf32(input, min, max);
        gcvt(input, 8, curr_input->text);
        curr_input->highlight_on = true;
    }
    if (clicked || curr_input->presist_clicked)
    {
        static bool first_clicked   = true;
        curr_input->presist_clicked = true;
        if (is_any_key_clicked(first_clicked))
        {
            curr_input->highlight_on = false;

            uint16 key = ui_state.key_evt->key_evt.key;
            char letter;
            if (key == SYNT_KEY_ENTER)
            {
                curr_input->curr_index      = 0;
                curr_input->presist_clicked = false;

                input = (float)atof(curr_input->text);
                input = clampf32(input, min, max);
                gcvt(input, 8, curr_input->text);

                memcpy(curr_input->last_text, curr_input->text,
                       sizeof(curr_input->last_text));
            }
            else if (key == SYNT_KEY_BACKSPACE)
            {
                if (curr_input->curr_index != 0)
                {
                    curr_input->text[--curr_input->curr_index] = '\0';
                }
            }
            else
            {
                if (is_letter_number(key))
                {
                    letter = (char)code_to_ascii(key);
                    if (curr_input->curr_index < 14)
                    {
                        curr_input->text[curr_input->curr_index++] = letter;
                        curr_input->text[curr_input->curr_index]   = '\0';
                    }
                }
            }
        }
        if (!clicked && index_clicked)
        {
            memcpy(curr_input->text, curr_input->last_text,
                   sizeof(curr_input->text));
            curr_input->curr_index      = 0;
            curr_input->presist_clicked = false;

            curr_input->highlight_on = false;
        }
    }
    float wide     = strlen(curr_input->text) * BUTTON_SIZE_MULTI;
    Ui_Window* win = &ui_wins[win_idx];
    if (wide < 50.0f)
    {
        wide = 50.0f;
    }
    if (win->last_button_wide < 50.0f)
    {
        win->last_button_wide = 50.0f;
    }
    if (win->g_x) win->x_offset_button += win->last_button_wide + 10.0f;

    uint32 out = 0;

    quad(&ui_state.g_pipline.vert_buffer.data,
         { win->extra_x_offset + win->x_offset_button + 2.0f,
           Y_START_SHADOW + (win->g_y * 30.0f), -0.111f },
         Vec2(wide, 20.0f), Vec4(0.0f, 0.0f, 0.0f, 0.7f), 0.0f);

    out++;

    synt_push(ui_state.rects,
              quad(&ui_state.g_pipline.vert_buffer.data,
                   { win->extra_x_offset + win->x_offset_button,
                     win->Y_START + (win->g_y * 30.0f), -0.11f },
                   Vec2(wide, 20.0f), Vec4(0.8f, 0.8f, 0.8f, 1.0f), 0.0f));

    out++;

    if (curr_input->highlight_on)
    {
        quad(&ui_state.g_pipline.vert_buffer.data,
             { win->extra_x_offset + win->x_offset_button + 2.5f,
               win->Y_START + (win->g_y * 30.0f) + 2.0f, -0.105f },
             Vec2(wide - 5.0f, 16.0f), Vec4(0.0f, 0.0f, 1.0f, 0.7f), 0.0f);
        out++;
    }

    synt_back(ui_state.rects).id = rect_index++;

    out += text_2D(ui_state.font, curr_input->text,
                   Vec3(win->extra_x_offset + win->x_offset_button + 3.0f,
                        win->Y_START + 2.0f + (win->g_y * 30.0f), -0.1f),
                   0.4f, &ui_state.g_pipline.vert_buffer.data);

    win->last_button_wide = wide;
    num_ui_rects += out;
    win->input_index++;
    update_misc();

    return clicked;
}

// TODO: support for new line in text.
void add_text(const char* text)
{
    uint32 out     = 0;
    Ui_Window* win = &ui_wins[win_idx];
    if (win->last_button_wide < 50.0f)
    {
        win->last_button_wide = 50.0f;
    }
    if (win->g_x) win->x_offset_button += win->last_button_wide + 10.0f;
    if (text && *text)
    {
        out += text_2D(ui_state.font, text,
                       Vec3(win->x_offset_button + 2.0f,
                            win->Y_START + 2.0f + (win->g_y * 30.0f), -0.1f),
                       0.4f, &ui_state.g_pipline.vert_buffer.data);
    }
    float wide = (float)strlen(text) * BUTTON_SIZE_MULTI;
    if (wide < 50.0f)
    {
        wide = 50.0f;
    }
    float diff = wide - win->dimensions.x;
    if (diff > 0.0f)
    {
        uint32 num_to_remove = (uint32)(diff / BUTTON_SIZE_MULTI);
        out -= num_to_remove;
        get_head(ui_state.g_pipline.vert_buffer.data)->size -= num_to_remove * 4;
    }
    win->last_button_wide = wide;
    num_ui_rects += out;
    update_misc();
}

void destroy_gui(VkDevice device, uint32 num_semaphores)
{
    vkDestroyPipelineLayout(device, ui_state.g_pipline.layout, NULL);
    vkDestroyPipeline(device, ui_state.g_pipline.pipeline, NULL);
    vkDestroyDescriptorSetLayout(device, ui_state.g_pipline.set_layout, NULL);
    destroy_buffer(device, ui_state.g_pipline.vert_buffer.buffer,
                   ui_state.g_pipline.vert_buffer.buffer_memory);
    destroy_buffer(device, ui_state.g_pipline.idx_buffer.buffer,
                   ui_state.g_pipline.idx_buffer.buffer_memory);

    vkDestroyDescriptorPool(device, ui_state.g_pipline.descriptors.desc_pool, NULL);

    for (uint32 i = 0; i < num_semaphores; i++)
    {
        destroy_buffer(device, ui_state.g_pipline.uniform_buffers[i].buffer,
                       ui_state.g_pipline.uniform_buffers[i].buffer_memory);
    }
    for (uint32 i = 0; i < size_arr(ui_state.textures); i++)
    {
        destroy_texture(device, ui_state.textures[i]);
    }
}

bool gui_focus() { return ui_hit || ui_hold; }

} // namespace synt

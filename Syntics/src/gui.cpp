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
#define Y_START_SHADOW Y_START + 2.0f

namespace synt {

typedef struct Input_Float
{
    uint32 curr_index    = 0;
    char text[15]        = {};
    bool presist_clicked = false;
    bool presist_hold    = false;
    bool dot_used        = false;
} Input_Float;

typedef struct Ui_Window
{
    Input_Float input_floats[10];
    uint32 input_is_holding = false;
    uint32 input_index      = 0;
    bool gridd_start        = false;
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

} Ui_Window;

typedef struct Ui_State
{
    Graphic_Pipline g_pipline;

    Events* mouse_evt;
    Events* key_evt;

    Font font;
    Texture* textures;
    Rect* rects;

    Camera cam;

} Ui_State;

static Ui_State ui_state;
static Ui_Window ui_windows[2];
static uint32 rect_index    = RECTS_START;
static uint32 index_hover   = 0;
static uint32 index_clicked = 0;
static bool ui_hit          = false;
static bool ui_hold         = false;
static uint32 num_ui_rects  = 0;

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

    create_texture(device, physical_device, command_pool, graphic_queue, false,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/button.png",
                   &ui_state.textures[0]);
    get_head(ui_state.textures)->size++;

    create_texture(device, physical_device, command_pool, graphic_queue, false,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/Mono.png",
                   &ui_state.textures[1]);
    get_head(ui_state.textures)->size++;

    create_texture(device, physical_device, command_pool, graphic_queue, false,
                   VK_FORMAT_R8G8B8A8_SRGB,
                   "Syntics/res/white-color-solid-background-1920x1080.png",
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
    ui_state.rects      = dyn_arrayP((*region), num_ui_rects, Rect);

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
    ui_state.cam.mvp.model   = translate(mat4i(1.0f), ui_state.cam.position);
    ui_state.cam.mvp.view    = mat4i(1.0f);
}

void gui_render(VkCommandBuffer command_buffer, uint32 semaphore_idx)
{
    bind_and_draw_graphics_pipline(
        command_buffer, ui_state.g_pipline.descriptors.desc_sets[semaphore_idx],
        ui_state.g_pipline, true);
}

void gui_update_begin(Region_Alloc* region, VkDevice device, const Vec2& dimensions,
                      uint32 semaphore_idx, float delta)
{
    // Because vulkan is flipped this results in the oposite for y axis :|
    ui_state.cam.mvp.proj = ortho(0, 0, dimensions.x, dimensions.y, -1.0f, 1.0f);
    update_uniform_buffers(device, ui_state.g_pipline.uniform_buffers[semaphore_idx],
                           &ui_state.cam.mvp, sizeof(ui_state.cam.mvp));

    index_hover               = 0;
    index_clicked             = 0;
    static bool first_clicked = true;
    bool button_clicked       = is_any_button_clicked(first_clicked);

    for (int i = size_arr(ui_state.rects) - 1; i >= 0; i--)
    {
        ui_hit =
            point_in_rect(Vec2((float)ui_state.mouse_evt->mouse_evt.move_evt.pos_x,
                               (float)ui_state.mouse_evt->mouse_evt.move_evt.pos_y),
                          ui_state.rects[i]);
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
    if (!ui_hit && button_clicked)
    {
        index_clicked = 1;
    }
    if (!ui_state.mouse_evt->mouse_evt.button_evt.action)
    {
        ui_hold = false;
    }

    highest_high = g_y;
    g_x          = 0;
    g_y          = 0;
    rect_index   = RECTS_START;
    num_ui_rects = 0;

    gridd_dimensions[0] = 0;
    gridd_dimensions[1] = 0;

    get_head(ui_state.rects)->size = 0;

    ui_state.g_pipline.vert_buffer.data =
        dyn_arrayP((*region), (num_ui_rects + MAX_SPACE) * 4, Vertex);

    quad(&ui_state.g_pipline.vert_buffer.data, Vec3(0.0f, 0.0f, -0.9f),
         Vec2(dimensions.x, dimensions.y), Vec4(0.0f), 0.0f);

    num_ui_rects += 1;

    input_index = 0;
}

void gui_update_end(Region_Alloc* region, VkDevice device)
{
    ui_state.g_pipline.vert_buffer.size_bytes =
        capacity_arr(ui_state.g_pipline.vert_buffer.data) * sizeof(Vertex);

    map_copy_mem(device, &ui_state.g_pipline.vert_buffer.buffer_memory,
                 ui_state.g_pipline.vert_buffer.size_bytes,
                 ui_state.g_pipline.vert_buffer.data);

    region_pop((*region), capacity_arr(ui_state.g_pipline.vert_buffer.data), Vertex,
               PERM_ARRAY);
    ui_state.g_pipline.vert_buffer.data = NULL;

    ui_state.g_pipline.idx_buffer.curr_size = num_ui_rects * 6;
}

void gridd_begin(uint32 x, uint32 y)
{
    if (!x) x = 1;
    if (!y) y = 1;

    gridd_dimensions[0] = x;
    gridd_dimensions[1] += y;
    gridd_start = true;

    if (biggest_wide < x)
    {
        biggest_wide = x;
    }

    g_x = 0;
}

void gridd_end() { gridd_start = false; }

void add_back_bord(const char* title)
{
    float wide;
    if (biggest_wide > (biggest_x_offset + latest_wide))
    {
        wide = biggest_wide + 10.0f;
    }
    else
    {
        wide = biggest_x_offset + latest_wide + 10.0f;
    }
    float high = ((float)highest_high * 33.0f) + Y_START;

    wide -= X_START - 11.0f;
    high -= Y_START - 25.0f;

    bool clicked = rect_index == index_clicked;
    bool hover   = rect_index == index_hover;

    static float presist_offset_x = 0.0f;
    static float presist_offset_y = 0.0f;
    if (clicked)
    {
        float mouse_x = (float)ui_state.mouse_evt->mouse_evt.move_evt.pos_x;
        float mouse_y = (float)ui_state.mouse_evt->mouse_evt.move_evt.pos_y;

        presist_offset_x = mouse_x - (X_START);
        presist_offset_y = mouse_y - (Y_START);
    }
    static bool presist_hold = false;
    if (presist_hold || ((hover && ui_hold) && !input_is_holding))
    {
        float mouse_x = (float)ui_state.mouse_evt->mouse_evt.move_evt.pos_x;
        float mouse_y = (float)ui_state.mouse_evt->mouse_evt.move_evt.pos_y;

        X_START          = mouse_x - presist_offset_x;
        Y_START          = mouse_y - presist_offset_y;
        presist_hold     = true;
        input_is_holding = true;
    }
    if (!ui_hold)
    {
        presist_hold     = false;
        input_is_holding = false;
    }

    synt_push(ui_state.rects,
              quad(&ui_state.g_pipline.vert_buffer.data,
                   { X_START - 9.0f, 2.0f + Y_START - 25.0f, -0.13f },
                   Vec2(wide, high), Vec4(0.0f, 0.0f, 0.0f, 0.7f), 2.0f));

    quad(&ui_state.g_pipline.vert_buffer.data,
         { X_START - 11.0f, 0.0f + Y_START - 25.0f, -0.12f }, Vec2(wide, high),
         Vec4(0.2f, 0.2f, 0.2f, 1.0f), 2.0f);

    quad(&ui_state.g_pipline.vert_buffer.data,
         { X_START - 11.0f, 0.0f + Y_START - 25.0f, -0.11f }, Vec2(wide, 20.0f),
         Vec4(0.0f, 0.0f, 0.0f, 1.0f), 2.0f);

    synt_back(ui_state.rects).id = rect_index++;

    uint32 out = 3;

    num_ui_rects += out;

    biggest_x_offset = 0;
    biggest_wide     = 0;
}

static void update_misc()
{
    if (++g_x == gridd_dimensions[0])
    {
        g_x = 0;
        if (x_offset_button > biggest_x_offset)
        {
            biggest_x_offset = x_offset_button;
            latest_wide      = last_button_wide;
        }
        if (last_button_wide > biggest_wide)
        {
            biggest_wide = last_button_wide;
        }
        x_offset_button = X_START;
        if (++g_y == gridd_dimensions[1])
        {
            gridd_start      = false;
            last_button_wide = 0;
        }
    }
}

bool add_button(const char* text)
{
    if (!gridd_start)
    {
        synt_LOG("Gridd overflow or is not started\n");
        return 0;
    }

    bool clicked = rect_index == index_clicked;
    bool hover   = rect_index == index_hover;

    Vec4 button_color = Vec4(0.5f, 0.5f, 0.5f, 1.0f);
    if (hover)
    {
        button_color = Vec4(0.7f, 0.7f, 0.7f, 1.0f);
    }

    float wide = (float)strlen(text) * BUTTON_SIZE_MULTI;
    if (wide < 50.0f)
    {
        wide = 50.0f;
    }
    if (last_button_wide < 50.0f)
    {
        last_button_wide = 50.0f;
    }
    if (g_x) x_offset_button += last_button_wide + 10.0f;

    quad(&ui_state.g_pipline.vert_buffer.data,
         { x_offset_button + 2.0f, Y_START_SHADOW + (g_y * 30.0f), -0.111f },
         Vec2(wide, 20.0f), Vec4(0.0f, 0.0f, 0.0f, 0.7f), 0.0f);

    synt_push(ui_state.rects,
              quad(&ui_state.g_pipline.vert_buffer.data,
                   { x_offset_button, Y_START + (g_y * 30.0f), -0.11f },
                   Vec2(wide, 20.0f), button_color, 0.0f));

    uint32 out = 2;

    synt_back(ui_state.rects).id = rect_index++;

    if (text && *text)
    {
        out += text_2D(
            ui_state.font, text,
            Vec3(x_offset_button + 2.0f, Y_START + 2.0f + (g_y * 30.0f), -0.1f),
            0.4f, &ui_state.g_pipline.vert_buffer.data);
    }
    last_button_wide = wide;
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

bool add_input_float(float& input)
{
    if (!gridd_start)
    {
        synt_LOG("Gridd overflow or is not started\n");
        return 0;
    }

    bool clicked = rect_index == index_clicked;
    bool hover   = rect_index == index_hover;

    if (input_floats[input_index].presist_hold ||
        ((hover && ui_hold) && !input_is_holding))
    {
        int16 mouse_x = ui_state.mouse_evt->mouse_evt.move_evt.pos_x;

        static int16 last_x = mouse_x;

        if (!clicked)
        {
            if (last_x < mouse_x)
            {
                float multiplier = mouse_x - last_x;
                input += 0.01f * multiplier;
            }
            else if (last_x > mouse_x)
            {
                float multiplier = last_x - mouse_x;
                input -= 0.01f * multiplier;
            }
        }
        gcvt(input, 5, input_floats[input_index].text);

        last_x = mouse_x;

        input_floats[input_index].presist_hold = true;
        input_is_holding                       = true;
    }
    if (!ui_hold)
    {
        input_floats[input_index].presist_hold = false;
        input_is_holding                       = false;
    }
    if (clicked || input_floats[input_index].presist_clicked)
    {
        static bool first_clicked                 = true;
        input_floats[input_index].presist_clicked = true;
        if (is_any_key_clicked(first_clicked))
        {
            uint16 key = ui_state.key_evt->key_evt.key;
            char letter;
            if (key == SYNT_KEY_ENTER)
            {
                input_floats[input_index].curr_index      = 0;
                input_floats[input_index].presist_clicked = false;
                input = (float)atof(input_floats[input_index].text);
            }
            else if (key == SYNT_KEY_BACKSPACE)
            {
                if (input_floats[input_index].curr_index != 0)
                {
                    input_floats[input_index]
                        .text[--input_floats[input_index].curr_index] = '\0';
                }
            }
            else
            {
                if (is_letter_number(key))
                {
                    letter = (char)code_to_ascii(key);
                    assert(input_floats[input_index].curr_index < 14);

                    input_floats[input_index]
                        .text[input_floats[input_index].curr_index++] = letter;
                    input_floats[input_index]
                        .text[input_floats[input_index].curr_index] = '\0';
                }
            }
        }
        if (!clicked && index_clicked)
        {
            input_floats[input_index].presist_clicked = false;
        }
    }
    float wide = strlen(input_floats[input_index].text) * BUTTON_SIZE_MULTI;
    if (wide < 50.0f)
    {
        wide = 50.0f;
    }
    if (last_button_wide < 50.0f)
    {
        last_button_wide = 50.0f;
    }
    if (g_x) x_offset_button += last_button_wide + 10.0f;

    quad(&ui_state.g_pipline.vert_buffer.data,
         { x_offset_button + 2.0f, Y_START_SHADOW + (g_y * 30.0f), -0.111f },
         Vec2(wide, 20.0f), Vec4(0.0f, 0.0f, 0.0f, 0.7f), 0.0f);

    synt_push(ui_state.rects,
              quad(&ui_state.g_pipline.vert_buffer.data,
                   { x_offset_button, Y_START + (g_y * 30.0f), -0.11f },
                   Vec2(wide, 20.0f), Vec4(0.8f, 0.8f, 0.8f, 1.0f), 0.0f));

    uint32 out = 2;

    synt_back(ui_state.rects).id = rect_index++;

    out +=
        text_2D(ui_state.font, input_floats[input_index].text,
                Vec3(x_offset_button + 3.0f, Y_START + 2.0f + (g_y * 30.0f), -0.1f),
                0.4f, &ui_state.g_pipline.vert_buffer.data);

    last_button_wide = wide;
    num_ui_rects += out;
    input_index++;
    update_misc();
    return clicked;
}

// TODO: support for new line in text.
void add_text(const char* text)
{
    uint32 out = 0;
    if (last_button_wide < 50.0f)
    {
        last_button_wide = 50.0f;
    }
    if (g_x) x_offset_button += last_button_wide + 10.0f;
    if (text && *text)
    {
        out += text_2D(
            ui_state.font, text,
            Vec3(x_offset_button + 2.0f, Y_START + 2.0f + (g_y * 30.0f), -0.1f),
            0.4f, &ui_state.g_pipline.vert_buffer.data);
    }
    float wide = (float)strlen(text) * BUTTON_SIZE_MULTI;
    if (wide < 50.0f)
    {
        wide = 50.0f;
    }
    last_button_wide = wide;
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

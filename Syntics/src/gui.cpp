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

#define MAX_SPACE 10000
#define BUTTON_START 2

namespace synt {

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
static bool gridd_start = false;
static uint32 gridd_dimensions[2];
static uint32 g_x           = 0;
static uint32 g_y           = 0;
static float test           = 0.0;
static uint32 button_index  = BUTTON_START;
static uint32 index_hover   = 0;
static uint32 index_clicked = 0;
static bool ui_hit          = false;

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

static void update_vertex_index(Region_Alloc* region, VkDevice device,
                                const Vec2& dimensions)
{
    g_x                 = 0;
    g_y                 = 0;
    uint32 num_ui_rects = 1;
    button_index        = BUTTON_START;

    get_head(ui_state.rects)->size = 0;

    ui_state.g_pipline.vert_buffer.data =
        dyn_arrayP((*region), (num_ui_rects + 10000) * 4, Vertex);

    quad(&ui_state.g_pipline.vert_buffer.data, Vec3(0.0f, 0.0f, -0.9f),
         Vec2(dimensions.x, dimensions.y), Vec4(0.0f), 0.0f);

    gridd_begin(2, 2);
    {
        if (add_button("+", num_ui_rects)) synt_LOG("+\n");
        if (add_button("Click me!", num_ui_rects)) synt_LOG("Click me\n");
        if (add_button("dd", num_ui_rects)) synt_LOG("dd\n");
        if (add_button("Hllo", num_ui_rects)) synt_LOG("Hllo\n");
    }
    gridd_end();

    gridd_begin(1, 1);
    {
        add_input_float(test, num_ui_rects);
    }
    gridd_end();

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

void gui_init(Region_Alloc* region, VkDevice device,
              VkPhysicalDevice physical_device, VkCommandPool command_pool,
              VkQueue graphic_queue, const Swap_Chain_attrib& swap_chain,
              uint32 num_semaphores)
{
    subscribe(&ui_state.key_evt, EVT_KEY);
    subscribe(&ui_state.mouse_evt, EVT_MOUSE);

    create_graphics_pipeline(region, device, swap_chain.color_format,
                             swap_chain.render_pass, swap_chain.sample_count,
                             "Syntics/res/gui.vert.spv", "Syntics/res/gui.frag.spv",
                             swap_chain.extent_2D.width, swap_chain.extent_2D.height,
                             VK_CULL_MODE_NONE, &ui_state.g_pipline);

    ui_state.textures = dyn_arrayP((*region), 2, Texture);

    create_texture(device, physical_device, command_pool, graphic_queue, false,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/button.png",
                   &ui_state.textures[0]);
    get_head(ui_state.textures)->size++;

    create_texture(device, physical_device, command_pool, graphic_queue, false,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/Mono.png",
                   &ui_state.textures[1]);
    get_head(ui_state.textures)->size++;

    ui_state.font           = load_font_file("Syntics/res/Mono.fnt");
    ui_state.font.tex_index = 1.0f;

    uint32 num_ui_rects = 10;
    ui_state.rects      = dyn_arrayP((*region), num_ui_rects, Rect);

    ui_state.g_pipline.vert_buffer.data = NULL;

    ui_state.g_pipline.vert_buffer.size_bytes =
        ((num_ui_rects + MAX_SPACE) * 4) * sizeof(Vertex);
    create_vertex_buffer(device, physical_device, command_pool, graphic_queue,
                         &ui_state.g_pipline.vert_buffer);

    update_vertex_index(
        region, device,
        Vec2(swap_chain.extent_2D.width, swap_chain.extent_2D.height));

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

void gui_update(Region_Alloc* region, VkDevice device, const Vec2& dimensions,
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

    for (uint32 i = 0; i < size_arr(ui_state.rects); i++)
    {
        ui_hit =
            point_in_rect(Vec2((float)ui_state.mouse_evt->mouse_evt.move_evt.pos_x,
                               (float)ui_state.mouse_evt->mouse_evt.move_evt.pos_y),
                          ui_state.rects[i]);
        if (ui_hit)
        {
            index_hover = i + BUTTON_START;

            if (button_clicked)
            {
                index_clicked = i + BUTTON_START;
            }
            break;
        }
    }
    if (!ui_hit && button_clicked)
    {
        index_clicked = 1;
    }
    update_vertex_index(region, device, dimensions);
}

void gridd_begin(uint32 x, uint32 y)
{
    if (!x) x = 1;
    if (!y) y = 1;

    gridd_dimensions[0] = x;
    gridd_dimensions[1] = y;
    gridd_start         = true;

    g_x = 0;
}

void gridd_end() { gridd_start = false; }

bool add_button(const char* text, uint32& num_indices)
{
    if (!gridd_start)
    {
        synt_LOG("Gridd overflow or is not started\n");
        return 0;
    }

    bool clicked = button_index == index_clicked;
    bool hover   = button_index == index_hover;

    uint32 out        = 1;
    Vec4 button_color = Vec4(0.5f, 0.5f, 0.5f, 1.0f);
    if (hover)
    {
        button_color = Vec4(0.7f, 0.7f, 0.7f, 1.0f);
    }
    synt_push(ui_state.rects, quad(&ui_state.g_pipline.vert_buffer.data,
                                   { 10.0f + ((g_x % gridd_dimensions[0]) * 60.0f),
                                     10.0f + (g_y * 30.0f), -0.11f },
                                   Vec2(50.0f, 20.0f), button_color, 0.0f));

    synt_back(ui_state.rects).id = button_index++;

    if (text && *text)
    {
        out += text_2D(ui_state.font, text,
                       Vec3(13.0f + ((g_x % gridd_dimensions[0]) * 60.0f),
                            12.0f + (g_y * 30.0f), -0.1f),
                       0.4f, &ui_state.g_pipline.vert_buffer.data);
    }

    if (++g_x == gridd_dimensions[0])
    {
        g_x = 0;
        if (++g_y == gridd_dimensions[1])
        {
            gridd_dimensions[0] = 0;
            gridd_dimensions[1] = 0;
            gridd_start         = false;
        }
    }
    num_indices += out;
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
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}

bool add_input_float(float& input, uint32& num_indices)
{
    if (!gridd_start)
    {
        synt_LOG("Gridd overflow or is not started\n");
        return 0;
    }
    uint32 out = 1;
    synt_push(ui_state.rects,
              quad(&ui_state.g_pipline.vert_buffer.data,
                   { 10.0f + ((g_x % gridd_dimensions[0]) * 60.0f),
                     10.0f + (g_y * 30.0f), -0.11f },
                   Vec2(50.0f, 20.0f), Vec4(0.8f, 0.8f, 0.8f, 1.0f), 0.0f));

    bool clicked = button_index == index_clicked;
    bool hover   = button_index == index_hover;

    synt_back(ui_state.rects).id = button_index++;

    static uint32 curr_index    = 0;
    static char text[15]        = {};
    static bool presist_clicked = false;
    if (clicked || presist_clicked)
    {
        static bool first_clicked = true;
        presist_clicked           = true;
        if (is_any_key_clicked(first_clicked))
        {
            uint16 key = ui_state.key_evt->key_evt.key;
            char letter;
            if (key == SYNT_KEY_ENTER)
            {
                curr_index      = 0;
                presist_clicked = false;
            }
            else if (key == SYNT_KEY_BACKSPACE)
            {
                if (curr_index != 0)
                {
                    text[--curr_index] = '\0';
                }
            }
            else
            {
                if (is_letter_number(key))
                {
                    letter = (char)code_to_ascii(key);
                    assert(curr_index < 14);

                    text[curr_index++] = letter;
                    text[curr_index]   = '\0';

                    input = (float)atof(text);
                }
            }
        }
        if (!clicked && index_clicked)
        {
            presist_clicked = false;
        }
    }

    out += text_2D(ui_state.font, text,
                   Vec3(13.0f + ((g_x % gridd_dimensions[0]) * 60.0f),
                        12.0f + (g_y * 30.0f), -0.1f),
                   0.4f, &ui_state.g_pipline.vert_buffer.data);

    if (++g_x == gridd_dimensions[0])
    {
        g_x = 0;
        if (++g_y == gridd_dimensions[1])
        {
            gridd_dimensions[0] = 0;
            gridd_dimensions[1] = 0;
            gridd_start         = false;
        }
    }
    num_indices += out;
    return clicked;
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

bool gui_focus() { return ui_hit; }

} // namespace synt

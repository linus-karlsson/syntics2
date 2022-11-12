#include "gui.h"
#include "vulkan_types.h"
#include "buffers.h"
#include "event_system.h"
#include "font.h"
#include "region_alloc.h"
#include "swap_chain.h"
#include "camera.h"
#include "collision.h"

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
static uint32 g_x = 0;
static uint32 g_y = 0;

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

    create_graphics_pipeline(region, device, swap_chain.color_format,
                             swap_chain.render_pass, swap_chain.sample_count,
                             "Syntics/res/gui.vert.spv", "Syntics/res/gui.frag.spv",
                             swap_chain.extent_2D.width, swap_chain.extent_2D.height,
                             VK_CULL_MODE_NONE, &ui_state.g_pipline);

    ui_state.textures = dyn_arrayP((*region), 2, Texture);

    create_texture(device, physical_device, command_pool, graphic_queue,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/button.png",
                   &ui_state.textures[0]);

    get_head(ui_state.textures)->size++;

    create_texture(device, physical_device, command_pool, graphic_queue,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/ArialSmall.png",
                   &ui_state.textures[1]);

    get_head(ui_state.textures)->size++;

    ui_state.font           = load_font_file("Syntics/res/ArialSmall.fnt");
    ui_state.font.tex_index = 1.0f;

    uint32 num_ui_rects = 10;
    ui_state.rects      = dyn_arrayP((*region), num_ui_rects, Rect);

    ui_state.g_pipline.vert_buffer.data =
        dyn_arrayP((*region), (num_ui_rects + 10000) * 4, Vertex);

    gridd_begin(2, 2);
    {
        num_ui_rects += add_button("Hello");
        num_ui_rects += add_button("Hello");
        num_ui_rects += add_button("Hello");
        num_ui_rects += add_button("Hello");
    }
    gridd_end();

    ui_state.g_pipline.idx_buffer.curr_size = num_ui_rects * 6;

    region_pop((*region), capacity_arr(ui_state.g_pipline.vert_buffer.data), Vertex,
               PERM_ARRAY);

    ui_state.g_pipline.vert_buffer.data = NULL;

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
    ui_state.cam.orientation = synt::v3f(0.0f, 0.0f, -1.0f);
    ui_state.cam.mvp.model   = mat4i(1.0f);
    ui_state.cam.mvp.view    = synt::view(
           ui_state.cam.position, ui_state.cam.position + ui_state.cam.orientation,
           ui_state.cam.up);
}

void gui_render(VkCommandBuffer command_buffer, uint32 semaphore_idx)
{
    bind_and_draw_graphics_pipline(
        command_buffer, ui_state.g_pipline.descriptors.desc_sets[semaphore_idx],
        ui_state.g_pipline, true);
}

void gui_update(VkDevice device, const Vec2& dimensions, uint32 semaphore_idx,
                float delta)
{
    // Because vulkan is flipped this results in the oposite for y axis :|
    ui_state.cam.mvp.proj = ortho(0, 0, dimensions.x, dimensions.y, -1.0f, 1.0f);
    update_uniform_buffers(device, ui_state.g_pipline.uniform_buffers[semaphore_idx],
                           &ui_state.cam.mvp, sizeof(ui_state.cam.mvp));

    bool ui_hit = false;
    if (ui_state.mouse_evt->activated)
    {
#if 1
        for (uint32 i = 0; i < size_arr(ui_state.rects); i++)
        {
            ui_hit = ui_hit ||
                     point_in_rect(
                         Vec2((float)ui_state.mouse_evt->mouse_evt.move_evt.pos_x,
                              (float)ui_state.mouse_evt->mouse_evt.move_evt.pos_y),
                         ui_state.rects[i]);
        }
#endif
    }
}

void gridd_begin(uint32 x, uint32 y)
{
    if (!x) x = 1;
    if (!y) y = 1;

    gridd_dimensions[0] = x;
    gridd_dimensions[1] = y;
    gridd_start         = true;

    g_x = 0;
    g_y = 0;
}

void gridd_end() { gridd_start = false; }

uint32 add_button(const char* text)
{
    if (!gridd_start)
    {
        ERROR("Gridd overflow or is not started");
    }
    uint32 out = 1;
    synt_push(ui_state.rects,
              quad(&ui_state.g_pipline.vert_buffer.data,
                   { 10.0f + ((g_x % gridd_dimensions[0]) * 60.0f),
                     10.0f + (g_y * 30.0f), 0.1f },
                   Vec2(50.0f, 20.0f), Vec4(0.2f, 0.2f, 0.2f, 1.0f), 0.0f));

    if (text && *text)
    {
        out += text_2D(ui_state.font, text,
                       Vec3(10.0f + ((g_x % gridd_dimensions[0]) * 60.0f,
                                     10.0f + (g_y * 30.0f), 0.0f)),
                       1.0f, &ui_state.g_pipline.vert_buffer.data);
    }

    if (++g_x == gridd_dimensions[0])
    {
        if (++g_y == gridd_dimensions[1])
        {
            gridd_dimensions[0] = 0;
            gridd_dimensions[1] = 0;
            gridd_start         = false;
        }
    }
    return out;
}

} // namespace synt

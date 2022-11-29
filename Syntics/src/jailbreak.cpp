#include "jailbreak.h"
#include "font.h"
#include "region_alloc.h"
#include "buffers.h"
#include "swap_chain.h"
#include "event_system.h"

namespace synt {

#define MAX_SPACE_JAIL 1000

typedef struct Game_State
{
    Graphic_Pipline g_pipline;

    MVP mvp;
    Vec3 pos;
    Font font;
    Texture* textures;
    Rect* rects;

    uint32 num_game_rects;

} Game_State;

static Game_State game_state;

void jail_init(Region_Alloc* region, VkDevice device,
               VkPhysicalDevice physical_device, VkCommandPool command_pool,
               VkQueue graphic_queue, const Swap_Chain_attrib& swap_chain,
               uint32 num_semaphores)
{
    game_state.textures = dyn_arrayP((*region), 3, Texture);

    create_texture(device, physical_device, command_pool, graphic_queue, false,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/default.png",
                   &game_state.textures[0]);
    get_head(game_state.textures)->size++;

    create_texture(device, physical_device, command_pool, graphic_queue, false,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/Ubuntu-white.png",
                   &game_state.textures[1]);
    get_head(game_state.textures)->size++;

    create_texture(device, physical_device, command_pool, graphic_queue, false,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/button.png",
                   &game_state.textures[2]);
    get_head(game_state.textures)->size++;

    create_graphics_pipeline(region, device, swap_chain.color_format,
                             swap_chain.render_pass, swap_chain.sample_count,
                             "Syntics/res/gui.vert.spv", "Syntics/res/gui.frag.spv",
                             swap_chain.extent_2D.width, swap_chain.extent_2D.height,
                             VK_CULL_MODE_BACK_BIT, size_arr(game_state.textures),
                             &game_state.g_pipline);

    game_state.font           = load_font_file("Syntics/res/Ubuntu-white.fnt");
    game_state.font.tex_index = 1.0f;

    game_state.rects = dyn_arrayP((*region), 30, Rect);

    init_graphics_pipeline(region, device, physical_device, command_pool,
                           graphic_queue, MAX_SPACE_JAIL, num_semaphores,
                           game_state.textures, game_state.g_pipline);

    game_state.mvp.model = mat4i(1.0f);
    game_state.mvp.view  = mat4i(1.0f);
}

static void update_camera(float dt)
{
    static bool first_clicked = true;
    if (is_any_button_pressed())
    {
        int16 mouse_x, mouse_y;
        get_pos(mouse_x, mouse_y);

        static int16 last_x = mouse_x;
        static int16 last_y = mouse_y;

#if 0
        hide_cursor();

        uint16 width, height;
        get_window_size(&width, &height);

        const uint16 half_width  = width / 2;
        const uint16 half_height = height / 2;

        if (mouse_x >= width - 300 || mouse_x <= 300)
        {
            set_mouse_pos(half_width, mouse_y);
            mouse_x = half_width;
            last_x  = mouse_x;
        }
        if (mouse_y >= height - 200 || mouse_y <= 200)
        {
            set_mouse_pos(mouse_x, half_height);
            mouse_y = half_height;
            last_y  = mouse_y;
        }
#endif

        float movement_x = 0.0f;
        float movement_y = 0.0f;

        if (!first_clicked)
        {
            movement_x = (float)((mouse_x - last_x));
            movement_y = (float)((mouse_y - last_y));
        }
        else
            first_clicked = false;

        game_state.pos.x += movement_x;
        game_state.pos.y += movement_y;

        last_x = mouse_x;
        last_y = mouse_y;
    }
    else
    {
        show_cursor_last_pos();
        first_clicked = true;
    }
}

static uint32 FPS = 0;

static void update_gui(Region_Alloc* region, float dt)
{
    static char fps_buffer[10]   = "FPS: ";
    static char milli_buffer[20] = {};
    back_bord_begin("First thing", Vec2(10.0f, 10.0f));
    {
        gridd_begin(2, 3);
        {
            static uint8 one_two = 0;
            if (add_button("Color"))
            {
            }
            if (add_button("Points"))
            {
            }
            if (add_button("Fan"))
            {
            }
            if (add_button("Strip"))
            {
            }
            if (add_button("Lines"))
            {
            }
            if (add_button("Triangle"))
            {
            }
        }
        gridd_end();

        gridd_begin(1, 1);
        {
            add_text("Position (x, y, z) This is a test");
        }
        gridd_end();

        gridd_begin(3, 1);
        {
            add_input_float(game_state.pos.x, -100.0f, 100.0f);
            add_input_float(game_state.pos.y, -100.0f, 100.0f);
            add_input_float(game_state.pos.z, -100.0f, 100.0f);
        }
        gridd_end();

        static float sec = 0.1f;
        sec += dt;
        if (sec >= 0.1f)
        {
            sprintf(fps_buffer + 5, "%u", FPS);
            sprintf(milli_buffer, "%f", dt * 1000);
            sprintf(milli_buffer + strlen(milli_buffer), " ms");
            sec = 0.0f;
        }
        gridd_begin(2, 1);
        {
            add_text(fps_buffer);
            add_text(milli_buffer);
        }
        gridd_end();
    }
    back_bord_end();
}

void jail_update(Region_Alloc* region, VkDevice device, const Vec2& dimensions,
                 uint32 semaphore_idx, float dt)
{
    gui_update_begin(region, device, dimensions, semaphore_idx, dt);
    {
        update_gui(region, dt);
    }
    gui_update_end(region, device);

    game_state.num_game_rects = 0;

    if (!gui_focus())
    {
        update_camera(dt);
    }

    game_state.mvp.model = translate(mat4i(1.0f), game_state.pos);
    game_state.mvp.proj  = ortho(0, 0, dimensions.x, dimensions.y, -1.0f, 1.0f);
    update_uniform_buffers(device,
                           game_state.g_pipline.uniform_buffers[semaphore_idx],
                           &game_state.mvp, sizeof(game_state.mvp));

    get_head(game_state.rects)->size = 0;

    get_head(game_state.g_pipline.vert_buffer.data)->size = 0;

    // game start.

    quad(&game_state.g_pipline.vert_buffer.data, { 200.0f, 200.0f, -0.51f },
         Vec2(60.0f, 100.0f), Vec4(0.1f, 0.1f, 1.0f, 0.5f), 0.0f);

    game_state.num_game_rects++;

    game_state.num_game_rects +=
        text_2D(game_state.font, "Hello", Vec3(200.0f, 200.0f, -0.5f), 1.0f,
                &game_state.g_pipline.vert_buffer.data);

    // game end.
    // game_state.g_pipline.vert_buffer.size_bytes = (game_state.num_game_rects * 4);

    map_copy_mem(device, &game_state.g_pipline.vert_buffer.buffer_memory,
                 game_state.g_pipline.vert_buffer.size_bytes,
                 game_state.g_pipline.vert_buffer.data);

    game_state.g_pipline.idx_buffer.curr_size = (game_state.num_game_rects * 6);
}

void jail_recreate(Region_Alloc* region, const Application_State& app_state)
{
    recreate_graphic_pipline(region, app_state, "Syntics/res/gui.vert.spv",
                             "Syntics/res/gui.frag.spv", game_state.g_pipline,
                             size_arr(game_state.textures));
}

void jail_render(VkCommandBuffer command_buffer, uint32 semaphore_idx)
{
    bind_and_draw_graphics_pipline(
        command_buffer, game_state.g_pipline.descriptors.desc_sets[semaphore_idx],
        game_state.g_pipline);
}

void jail_destroy(VkDevice device, uint32 num_semaphores)
{
    destroy_graphic_pipeline(device, num_semaphores, game_state.g_pipline);

    for (uint32 i = 0; i < size_arr(game_state.textures); i++)
    {
        destroy_texture(device, game_state.textures[i]);
    }
}

} // namespace synt

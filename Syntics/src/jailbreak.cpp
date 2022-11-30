#include "jailbreak.h"
#include "font.h"
#include "region_alloc.h"
#include "buffers.h"
#include "swap_chain.h"
#include "event_system.h"
#include "simple-particle.h"
#include "linux_platform.h"
#include "random.h"
#include "collision.h"
#include <math.h>

namespace synt {

#define MAX_SPACE_JAIL 2000

typedef struct Thing
{
    Vec3 pos;
    Vec2 vel;
    Rect rect;
} Thing;

typedef struct Game_State
{
    Graphic_Pipline g_pipline;

    MVP mvp;
    Vec3 pos;
    Font font;
    Texture* textures;
    Rect* rects;

    uint32 num_game_rects;

    Particles particles;

    Thing ball;
    Thing player;

} Game_State;

static Game_State game_state;

static uint32 dead_rect[130] = { 0 };

void jail_init(Region_Alloc* region, VkDevice device,
               VkPhysicalDevice physical_device, VkCommandPool command_pool,
               VkQueue graphic_queue, const Swap_Chain_attrib& swap_chain,
               uint32 num_semaphores)
{
    game_state.textures = dyn_arrayP((*region), 3, Texture);

    create_texture(device, physical_device, command_pool, graphic_queue, false,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/button.png",
                   &game_state.textures[0]);
    get_head(game_state.textures)->size++;

    create_texture(device, physical_device, command_pool, graphic_queue, false,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/Ubuntu-white.png",
                   &game_state.textures[1]);
    get_head(game_state.textures)->size++;

    create_texture(device, physical_device, command_pool, graphic_queue, false,
                   VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/Circle.png",
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

    game_state.rects = dyn_arrayP((*region), 130, Rect);

    init_graphics_pipeline(region, device, physical_device, command_pool,
                           graphic_queue, MAX_SPACE_JAIL, num_semaphores,
                           game_state.textures, game_state.g_pipline);

    game_state.mvp.model = mat4i(1.0f);
    game_state.mvp.view  = mat4i(1.0f);

    init_particles(region, game_state.particles, 1000);

    game_state.ball.pos = Vec3((swap_chain.extent_2D.width / 2) - 10.0f,
                               swap_chain.extent_2D.height - 121.0f, -0.51f);

    game_state.player.pos = Vec3((swap_chain.extent_2D.width / 2) - 75.0f,
                                 swap_chain.extent_2D.height - 100.0f, -0.51f);
}

static float x_start = 0.0f;
static float y_start = 0.0f;

static void update_camera(float dt)
{
    static bool first_clicked = true;
    if (is_any_button_pressed())
    {
        int16 mouse_x, mouse_y;
        get_pos(mouse_x, mouse_y);

        Particle_Attrib particle;
        particle.position.x = mouse_x - 5.0f;
        particle.position.y = mouse_y - 5.0f;
        emit_particle(game_state.particles, particle, Vec2(100.0f, 100.0f),
                      Vec2(0.5f), 5.0f);

#if 0
        static int16 last_x = mouse_x;
        static int16 last_y = mouse_y;
#endif

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

#if 0
        float movement_x = 0.0f;
        float movement_y = 0.0f;

        if (!first_clicked)
        {
            movement_x = (float)((mouse_x - last_x));
            movement_y = (float)((mouse_y - last_y));
        }
        else
            first_clicked = false;

        x_start += movement_x;
        y_start += movement_y;

        last_x = mouse_x;
        last_y = mouse_y;
#endif
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
        gridd_begin(3, 1);
        {
            add_input_float(y_start, -100.0f, 100.0f);
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

static void animate_background(Vec2 dimensions, float dt)
{

    Particle_Attrib particle;
    particle.position.x = rand_f32(0.0f, dimensions.x);
    particle.position.y = -30.0f;
    particle.color =
        Vec4(rand_f32(0.0, 1.0f), rand_f32(0.0, 1.0f), rand_f32(0.0, 1.0f), 1.0f);
    emit_particle(game_state.particles, particle, Vec2(0.0f, 80.0f), Vec2(0.0f),
                  30.0f);
}

static void update_player_pos(float dt)
{

    bool none = true;
    if (is_key_pressed(SYNT_A_PRESSED))
    {
        game_state.player.vel.x = -300.0f;
        none                    = false;
    }
    if (is_key_pressed(SYNT_D_PRESSED))
    {
        game_state.player.vel.x = 300.0f;
        none                    = false;
    }
    if (none)
    {
        game_state.player.vel.x = 0.0f;
    }
    game_state.player.pos.x += game_state.player.vel.x * dt;
}

#define for_range(i, n) for (uint32 i = 0; i < n; i++)

void jail_update(Region_Alloc* region, VkDevice device, const Vec2& dimensions,
                 uint32 semaphore_idx, float dt)
{
    static double sec        = 0.0f;
    static double start      = 0;
    static uint32 frames     = 0;
    static bool first_frame  = true;
    static bool game_started = false;

    sec += dt;

    if (frames == 0) start = get_time();
    if (frames++ >= 50)
    {
        animate_background(dimensions, dt);

        double end  = get_time();
        double time = end - start;

        FPS    = (uint32)(50 / time);
        frames = 0;
    }

    if (is_key_pressed(SYNT_Q_PRESSED))
    {
        x_start = 0;
        y_start = 0;
    }

    game_state.mvp.model = translate(mat4i(1.0f), game_state.pos);
    game_state.mvp.proj  = ortho(0, 0, dimensions.x, dimensions.y, -1.0f, 1.0f);
    update_uniform_buffers(device,
                           game_state.g_pipline.uniform_buffers[semaphore_idx],
                           &game_state.mvp, sizeof(game_state.mvp));

    get_head(game_state.rects)->size = 0;

    get_head(game_state.g_pipline.vert_buffer.data)->size = 0;

    game_state.num_game_rects = 0;

    // game start.

    quad(&game_state.g_pipline.vert_buffer.data, { -200.0f, -10.0f, -1.1f },
         dimensions * 1.5f, Vec4(0.0f, 0.0f, 0.0f, 1.0f), 0.0f);
    game_state.num_game_rects++;

    // gui_update_begin(region, device, dimensions, semaphore_idx, dt);
    // {
    //     update_gui(region, dt);
    // }
    // gui_update_end(region, device);

    game_state.num_game_rects += update_particles(
        game_state.particles, &game_state.g_pipline.vert_buffer.data, dt);

    // if (!gui_focus())
    // {
    //     update_camera(dt);
    // }

    uint32 width = (100.0f * 10.0f) + (10.0f * 9);

    float x_offset = 0.0f;
    float y_offset = 0.0f;
    x_start        = (dimensions.x / 2.0f) - (width / 2.0f);
    y_start        = 100.0f;
    Vec2 size(100.0f, 20.0f);
    uint32 count = 0;
    for_range(i, 7)
    {
        for_range(j, 10)
        {
            if (dead_rect[count++] == 0)
            {
                synt_push(
                    game_state.rects,
                    quad(&game_state.g_pipline.vert_buffer.data,
                         { x_start + x_offset, y_start + y_offset, -0.51f }, size,
                         Vec4(1.0f - i / 6.0f, 0.0f + i / 6.0f, 0.0f, 1.0f), 0.0f));
                game_state.num_game_rects++;
            }
            else
            {
                get_head(game_state.rects)->size++;
            }
            x_offset += size.x + 10.0f;
        }
        x_offset = 0.0f;
        y_offset += size.y + 10.0f;
    }
    for_range(i, 6)
    {
        for_range(j, 10)
        {
            if (dead_rect[count++] == 0)
            {
                synt_push(
                    game_state.rects,
                    quad(&game_state.g_pipline.vert_buffer.data,
                         { x_start + x_offset, y_start + y_offset, -0.51f }, size,
                         Vec4(0.0f, 1.0f - i / 5.0f, 0.0f + i / 5.0f, 1.0f), 0.0f));
                game_state.num_game_rects++;
            }
            else
            {
                get_head(game_state.rects)->size++;
            }
            x_offset += size.x + 10.0f;
        }
        x_offset = 0.0f;
        y_offset += size.y + 10.0f;
    }

    if (game_started)
    {
        game_state.ball.pos.x += game_state.ball.vel.x * dt;
        game_state.ball.pos.y += game_state.ball.vel.y * dt;
    }
    else
    {
        game_state.ball.pos.x = game_state.player.pos.x;
    }

    if (is_key_pressed(SYNT_SPACE_PRESSED))
    {
        if (!game_started)
        {
            game_state.ball.vel.y = -200.0f;
            game_state.ball.vel.x = game_state.player.vel.x;
        }
        game_started = true;
    }

    game_state.player.rect =
        quad(&game_state.g_pipline.vert_buffer.data, game_state.player.pos,
             Vec2(150.0f, 30.0f), Vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.0f);
    game_state.num_game_rects++;

    game_state.ball.rect =
        quad(&game_state.g_pipline.vert_buffer.data, game_state.ball.pos,
             Vec2(20.0f, 20.0f), Vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.0f);
    game_state.num_game_rects++;

    for_range(i, size_arr(game_state.rects))
    {
        if (dead_rect[i] == 0)
        {
            if (rect_in_rect(game_state.ball.rect, game_state.rects[i]))
            {
                Particle_Attrib particle;
                for_range(j, 10)
                {
                    particle.position.x = game_state.rects[i].pos.x + (j * 10.0f);
                    particle.position.y = game_state.rects[i].pos.y + (j);
                    particle.color      = game_state.rects[i].color;
                    emit_particle(game_state.particles, particle,
                                  Vec2(100.0f, 100.0f), Vec2(0.5f), 5.0f);
                }
                game_state.ball.vel.y = 200.0f;
                dead_rect[i]          = 1;
            }
        }
    }

    update_player_pos(dt);

    if (rect_in_rect(game_state.ball.rect, game_state.player.rect))
    {
        game_state.ball.vel.y = -200.0f;
        game_state.ball.vel.x = game_state.player.vel.x;
    }
    if (game_state.ball.pos.x <= 0.0f || game_state.ball.pos.x >= dimensions.x)
    {
        game_state.ball.vel.x *= -1.0f;
    }
    if (game_state.ball.pos.y <= 0.0f)
    {
        game_state.ball.vel.y *= -1.0f;
    }

    // num_game_rects += text_2D(game_state.font, "Hello",
    //                           Vec3(x_start + 100.0f, y_start + 100.0f,
    //                           -0.5f), 1.0f,
    //                           &game_state.g_pipline.vert_buffer.data);

    // game end.
    // game_state.g_pipline.vert_buffer.size_bytes = (num_game_rects * 4);

    map_copy_mem(device, &game_state.g_pipline.vert_buffer.buffer_memory,
                 game_state.g_pipline.vert_buffer.size_bytes,
                 game_state.g_pipline.vert_buffer.data);

    game_state.g_pipline.idx_buffer.curr_size = (game_state.num_game_rects * 6);

    first_frame = false;
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

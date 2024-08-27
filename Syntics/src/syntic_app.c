#ifndef SY_UNIT_BUILD
#include "vulkan_types.h"
#include "syntics_vulkan.h"
#include "frame_data.h"
#include "syntics_platform.h"
#include "application.h"
#include "thread_queue.h"
#include "event_system.h"
#include "ui.h"
#endif

#define PRINT_REGION
//
//

#if 0
#define big_to_little(s0, s1, s2, s3)                                                              \
    (((u32)s0 & 0xFF) | (((u32)s1 << 8) & 0xFF00) | (((u32)s2 << 16) & 0xFF0000) |                 \
     (((u32)s3 << 24) & 0xFF000000))

enum Header_Type
{
    WAV_RIFF = big_to_little('R', 'I', 'F', 'F'),
    WAV_WAVE = big_to_little('W', 'A', 'V', 'E'),
    WAV_FMT = big_to_little('f', 'm', 't', ' '),
    WAV_DATA = big_to_little('d', 'a', 't', 'a')
};

#endif

/*
void game_logic(void* data)
{
    Game_Logic* logic = (Game_Logic*)data;

    // printf("Game Frame:   %u | Time: %lf\n", logic->frame->id,
    // platform_get_time());

    gui_update_begin(logic->gui_ctx, logic->frame->dimensions,
                     logic->frame->semaphore_idx, logic->frame->dt);

    game_update(logic->game_state, logic->gui_ctx, logic->app_state,
                logic->frame, logic->frame->dimensions, logic->frame->dt);

    gui_update_end(logic->gui_ctx, logic->gui, logic->frame->copy_tasks,
                   logic->frame->render_tasks, &logic->frame->frame_region);

    platform_semaphore_increment(&logic->frame->render_counter);
}

void render_logic(void* data)
{
    Render_Logic* logic = (Render_Logic*)data;

    platform_semaphore_wait_and_decrement(&logic->frame->render_counter);

    // printf("Render Frame: %u | Time: %lf\n", logic->frame->id,
    // platform_get_time());

    vulkan_frame_begin(logic->render_state, logic->app_state);

    vulkan_frame_render(logic->render_state, logic->app_state,
                        logic->frame->copy_tasks, logic->frame->render_tasks,
                        logic->frame->dt);
}
*/

void run_app(void)
{
    Application_State* app_state = NULL;
    Render_State* render_state = NULL;

    application_init(MEGABYTE(10), MEGABYTE(200), 1400, 980, 40, false, 20, 1, &render_state,
                     &app_state);

    ui_context_create(app_state->device, app_state->phy_device, app_state->com_pool,
                      vulkan_graphic_queue_get(render_state), &app_state->swap_chain,
                      app_state->platform, app_state->num_semaphores);

    Game_State* game_state = region_calloc_struct(&app_state->region, Game_State);
    game_init(&app_state->region, &app_state->thread_queue.task_queue, app_state->device,
              app_state->phy_device, app_state->com_pool, vulkan_graphic_queue_get(render_state),
              &app_state->swap_chain, render_state, app_state->num_semaphores, game_state);

    /*
    Semaphore_Counter game_logic_counter = { 0 };
    Game_Logic game_log = { 0 };
    game_log.app_state = app_state;
    game_log.gui_ctx = gui_ctx;
    game_log.game_state = game_state;

    Semaphore_Counter render_logic_counter = { 0 };

    Render_Logic render_log = { 0 };
    render_log.app_state = app_state;
    render_log.render_state = render_state;

    // #define main_multi
    */

#ifdef main_multi
#define MAX_FRAMES 3
#else
#define MAX_FRAMES 1
#endif
    Frame_Data* frame_datas = region_array_calloc(&app_state->region, MAX_FRAMES, Frame_Data);
    for (u32 i = 0; i < MAX_FRAMES; i++)
    {
        Frame_Data* frame = region_array_value_ptr(frame_datas, i);
        region_init(&frame->frame_region, MEGABYTE(2));

        frame->id = i;

        frame->game_pipeline_layout = game_state->pipeline_layout;
        frame->game_descriptor_set_layout = game_state->descriptor_set_layout;
        frame->game_uniform_buffers = game_state->uniform_buffers;
        frame->game_descriptors = &game_state->descriptors;

        frame->game_triangle_strip_pipeline = game_state->triangle_strip_pipeline;
        frame->game_triangle_list_pipeline = game_state->triangle_list_pipeline;
        frame->game_line_list_pipeline = game_state->line_list_pipeline;
        frame->game_grass_pipeline = game_state->grass_pipeline;

        frame->game_vert_idx_buffer = game_state->vert_idx_buffer;
        frame->game_road_vert_idx = game_state->road_vert_idx;
        frame->game_road_line_vert_idx = game_state->road_line_vert_idx;
        frame->game_aabb_rep = game_state->aabb_rep;

        frame->game_terrain_offsets = game_state->terrain_offsets;
        frame->game_dude_offsets = game_state->dude_offsets;
        frame->game_tree_offsets = game_state->tree_offsets;
        frame->game_sign_offsets = game_state->sign_offsets;
        frame->game_grass_offsets = game_state->grass_offsets;
        frame->game_particles_offsets = game_state->particles_offsets;

        frame->game_particles_staging_buffer = game_state->particles_staging_buffer;
        vulkan_staging_buffer_create(app_state->device, app_state->phy_device, NULL,
                                     frame->game_particles_staging_buffer.size_bytes,
                                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                     &frame->game_particles_staging_buffer);

        frame->render_counter = platform_semaphore_create(0, 1);
    }

    u32 frame_index = 0;
    Application_Frame app_frame = application_frame_create();
    f64 sec = 0;
    app_state->running = true;

    Render_Task* copy_tasks = NULL;
    Render_Task* render_tasks = NULL;

    U32_Array windows = { 0 };
    array_create(&windows, 20);
    for (u32 i = 0; i < 20; ++i)
    {
        array_push(&windows, ui_window_create());
        ui_window_set_size(windows.data[i], v2f(200.0f, 200.0f));
        ui_window_set_position(windows.data[i], v2f(200.0f, 200.0f));
    }

    while (app_state->running)
    {
        f64 start = platform_get_time();

        app_frame = application_begin_frame(app_frame);

        V2 dimensions = v2f((f32)app_state->swap_chain.extent_2D.width,
                            (f32)app_state->swap_chain.extent_2D.height);

        app_state->fps = app_frame.fps;

        sec += app_frame.delta_time;
        if (sec >= 2.0f)
        {
            region_stack_begin_scope(region_print_stack);
#ifdef PRINT_REGION
            // region_print(&app_state->region);
            // sy_print("Stack size: %llu\n", region_stack_size());
#endif

            sec = 0;
            region_stack_end_scope(region_print_stack);
        }

        Frame_Data* frame = region_array_value_ptr(frame_datas, frame_index);

        region_reset(&frame->frame_region);

        u32 semaphore_idx = vulkan_get_semaphore_idx(render_state);

        frame->semaphore_idx = semaphore_idx;
        frame->dt = (f32)app_frame.delta_time;
        frame->dimensions = dimensions;
        render_tasks = region_array(&frame->frame_region, 20, Render_Task);
        copy_tasks = region_array(&frame->frame_region, 20, Render_Task);

        b8 result = vulkan_frame_begin(render_state, app_state);

        AABB_2D dock_space = {
            .min = v2d(),
            .size = dimensions,
        };
        ui_context_begin(dimensions, &dock_space, app_frame.delta_time, true);
        {
            ui_window_begin(windows.data[0], "Test", false,
                            UI_WINDOW_TOP_BAR | UI_WINDOW_RESIZEABLE);
            {
            }
            ui_window_end(false);
            ui_window_begin(windows.data[1], "Test", true,
                            UI_WINDOW_TOP_BAR | UI_WINDOW_RESIZEABLE);
            {
            }
            ui_window_end(false);
            ui_window_begin(windows.data[2], "Test", true,
                            UI_WINDOW_TOP_BAR | UI_WINDOW_RESIZEABLE);
            {
            }
            ui_window_end(false);
            ui_window_begin(windows.data[3], "Test", true,
                            UI_WINDOW_TOP_BAR | UI_WINDOW_RESIZEABLE);
            {
            }
            ui_window_end(false);
        }
        ui_context_end(copy_tasks, render_tasks);

        const Ui_Window* window = ui_window_get(windows.data[0]);
        VkViewport game_viewport = { 0 };
        game_viewport.maxDepth = 1.0f;
        game_viewport.x = sy_clamp_low(window->position.x, 0.0f);
        game_viewport.y = sy_clamp_low(window->position.y + 22.0f, 0.0f);
        game_viewport.width = window->size.width;
        game_viewport.height = window->size.height - 22.0f;
        frame->game_viewport = game_viewport;
        game_update(game_state, app_state, frame, frame->dimensions, render_tasks, copy_tasks,
                    (f32)app_frame.delta_time);

        if (result)
        {
            vulkan_frame_render(render_state, app_state, copy_tasks, render_tasks,
                                (f32)app_frame.delta_time);
        }

        event_poll(app_state->platform);
        if (event_is_key_pressed(SYNT_KEY_R))
        {
            app_state->running = false;
            goto Quit;
        }

        // NOTE: Vulkan vsync is used instead
        f64 end = platform_get_time();
        app_frame.delta_time = end - start;
#if 1
        const u32 target_milli = 8;
        const u64 curr_milli = (u64)(app_frame.delta_time * 1000.0f);
        if (target_milli > curr_milli)
        {
            u64 milli_to_sleep = (u64)(target_milli - curr_milli);
            platform_sleep(milli_to_sleep);
            f64 end2 = platform_get_time();
            app_frame.delta_time = end2 - start;
        }
#endif
        app_frame.frame_count++;
        frame_index++;
        frame_index %= MAX_FRAMES;
    }
Quit:
    // semaphore_counter_wait(&game_logic_counter);
    // semaphore_counter_wait(&render_logic_counter);
    threads_destroy(&app_state->thread_queue);
    ui_context_destroy();
    // gui_binary_file_save(gui_ctx);
    //  game_destroy();
    //  gui_destroy();
    //  vulkan_destroy(&app_state);
    //  platform_shut_down(app_state.platform);
}

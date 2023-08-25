#ifndef SY_INCLUDES // only for clangd
#include "syntics.h"
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

void find_working_dir(Region_Alloc* region)
{
    char file[MAX_PATH];
    u32 len = executable_directory(file, MAX_PATH);
    char* token = NULL;
    i32 steps = -1;
    for (; len > 0; len--)
    {
        steps++;
        if (file[len - 1] == '\\' || file[len - 1] == '/')
        {
            token = file + len;
            char temp = token[steps];
            token[steps] = '\0';
            if (!strcmp(token, "syntics2"))
            {
                token[steps] = temp;
                len += steps + 1;
                break;
            }
            token[steps] = temp;
            steps = -1;
        }
    }
    assert(len > 1);
    WORKING_DIR = region_array(region, len + 1, char);
    memcpy(WORKING_DIR, file, len);
    array_val(WORKING_DIR, len) = '\0';
    WORKING_DIR_LEN = len;
}

void instance_init_threaded(void* data)
{
    Instance_State* state = (Instance_State*)data;
    instance_init(&state->instance);
}

void game_logic(void* data)
{
    Game_Logic* logic = (Game_Logic*)data;

    // printf("Game Frame:   %u | Time: %lf\n", logic->frame->id,
    // platform_get_time());

    gui_update_begin(logic->gui_ctx, logic->frame->dimensions, logic->frame->semaphore_idx,
                     logic->frame->dt);

    game_update(logic->game_state, logic->gui_ctx, logic->app_state, logic->frame,
                logic->frame->dimensions, logic->frame->semaphore_idx, logic->frame->dt);

    gui_update_end(logic->gui_ctx, logic->frame);

    semaphore_increment(&logic->frame->render_counter);
}

void render_logic(void* data)
{
    Render_Logic* logic = (Render_Logic*)data;

    semaphore_wait_and_decrement(&logic->frame->render_counter);

    // printf("Render Frame: %u | Time: %lf\n", logic->frame->id,
    // platform_get_time());

    frame_begin(logic->render_state, logic->app_state);

    frame_render(logic->render_state, logic->app_state, logic->frame, logic->frame->dt);
}

void run_app(void)
{
    set_seed();

    Region_Alloc region = { 0 };
    stack_init(MEGABYTE(10));
    region_init(&region, MEGABYTE(200));
    logging_init(&region);

    Application_State* app_state = region_calloc_struct(&region, Application_State);
    u16 app_width = 1480;
    u16 app_height = 1000;

    // NOTE: main thread should be working while the other do as well.
    thread_init(&region, 40, platform_core_count() - 1);

    Instance_State instance_state = { 0 };
    Semaphore_Counter counter = { 0 };
    Thread_Task task = thread_task(instance_init_threaded, &instance_state);
    thread_tasks_push(&task, 1, &counter);

    find_working_dir(&region);

    platform_init(&region, "Syntics Engine", &app_width, &app_height, true, &app_state->platform);

    event_init(&region, app_state->platform, 20, &app_state->running);

    // Need both platform window and instance to initialize vulkan
    semaphore_counter_wait_and_free(&counter);

    Render_State* render_state = NULL;
    vulkan_init(&region, &instance_state, app_state, &render_state, (u32)app_width,
                (u32)app_height);

    const u32 window_count = 5;
    Gui_Context* gui_ctx = region_calloc_struct(&region, Gui_Context);
    gui_init(&region, app_state->device, app_state->phy_device, app_state->com_pool,
             graphic_queue_get(render_state), &app_state->swap_chain, app_state->platform,
             app_state->num_semaphores, window_count, true, gui_ctx);

    Game_State* game_state = region_calloc_struct(&region, Game_State);
    game_state->win_handles = region_array_calloc(&region, 2, Window_Handle);
    for (u32 i = 0; i < 2; i++)
    {
        array_val(game_state->win_handles, i) = window_create(gui_ctx);
    }
    game_init(&region, app_state->device, app_state->phy_device, app_state->com_pool,
              graphic_queue_get(render_state), &app_state->swap_chain, app_state->platform,
              render_state, app_state->num_semaphores, game_state);

    Semaphore_Counter game_logic_counter = { 0 };
    Semaphore_Counter render_logic_counter = { 0 };

    Game_Logic game_log = { 0 };
    game_log.app_state = app_state;
    game_log.gui_ctx = gui_ctx;
    game_log.game_state = game_state;

    Render_Logic render_log = { 0 };
    render_log.app_state = app_state;
    render_log.render_state = render_state;

    // #define main_multi

#ifdef main_multi
#define MAX_FRAMES 3
#else
#define MAX_FRAMES 1
#endif
    Frame_Data* frame_datas = region_array_calloc(&region, MAX_FRAMES, Frame_Data);
    for (u32 i = 0; i < MAX_FRAMES; i++)
    {
        Frame_Data* frame = array_val_ptr(frame_datas, i);
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
        staging_buffer_create(app_state->device, app_state->phy_device, NULL,
                              frame->game_particles_staging_buffer.size_bytes,
                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              &frame->game_particles_staging_buffer);

        frame->render_counter = semaphore_create(0, 1);
    }
    gui_frames_init(app_state->device, app_state->phy_device, app_state->com_pool,
                    graphic_queue_get(render_state), frame_datas, MAX_FRAMES, window_count);

    u32 frame_index = 0;

    const u32 frames_to_count = 30;
    f64 delta_time = MILLISECONDS(16.0);
    f64 delta_time_per_frame = MILLISECONDS(16.0);
    f64 sec_for_delta_update = 0.0;
    const f64 sec_for_delta_update_duration = 0.5;
    f64 sec2 = 0.0;
    u32 frame_count = 0;
    app_state->running = true;

    u32 last_id = 0;

    while (app_state->running)
    {
        Frame_Data* frame = array_val_ptr(frame_datas, frame_index);
        f64 start = platform_get_time();

        sec2 += delta_time_per_frame;
        sec_for_delta_update += delta_time_per_frame;

        if (sec_for_delta_update >= sec_for_delta_update_duration)
        {
            const f64 time = sec_for_delta_update;

            delta_time = time / (f64)frame_count;
            app_state->fps = (u32)((f64)frame_count / time);

            frame_count = 0;
            sec_for_delta_update = 0.0;
        }
        if (sec2 >= 2.0f)
        {
            stack_begin_scope(region_print_stack);
#ifdef PRINT_REGION
            region_print(&region);
            sy_print("Stack size: %llu\n", stack_size());
#endif

            sec2 = 0;
            stack_end_scope(region_print_stack);
        }

        frame->frame_region.current_pos = 0;

        Render_State_Internal* state_internal = (Render_State_Internal*)render_state;
        u32 semaphore_idx = state_internal->semaphore_index;

        V2 dimensions = v2f((f32)app_state->swap_chain.extent_2D.width,
                            (f32)app_state->swap_chain.extent_2D.height);

        frame->dimensions = dimensions;
        frame->semaphore_idx = semaphore_idx;
        frame->dt = (f32)delta_time;
        frame->dimensions = dimensions;
        frame->render_tasks = region_array(&frame->frame_region, 20, Render_Task);
        frame->copy_tasks = region_array(&frame->frame_region, 20, Render_Task);

#ifdef main_multi
        semaphore_counter_wait(&game_logic_counter);

        game_log.frame = frame;
        Thread_Task game_logic_task = thread_task(game_logic, &game_log);
        thread_tasks_push(&game_logic_task, 1, &game_logic_counter);

        semaphore_counter_wait(&render_logic_counter);

        render_log.frame = frame;
        Thread_Task render_logic_task = thread_task(render_logic, &render_log);
        thread_tasks_push(&render_logic_task, 1, &render_logic_counter);

#else
        game_log.frame = frame;
        render_log.frame = frame;

        // NOTE: This is has to be here for now. Gui is copying to the staging
        // buffer. And the command to copy the staging buffer to local storage
        // needs to have finished before that happens.
        frame_begin(render_log.render_state, render_log.app_state);

        gui_update_begin(game_log.gui_ctx, game_log.frame->dimensions,
                         game_log.frame->semaphore_idx, game_log.frame->dt);

        game_update(game_log.game_state, game_log.gui_ctx, game_log.app_state, game_log.frame,
                    game_log.frame->dimensions, game_log.frame->semaphore_idx, game_log.frame->dt);

        gui_update_end(game_log.gui_ctx, game_log.frame);

        frame_render(render_log.render_state, render_log.app_state, render_log.frame,
                     render_log.frame->dt);
#endif

        event_poll(app_state->platform);
        if (is_key_pressed(SYNT_KEY_R) && !gui_is_focus())
        {
            app_state->running = false;
            goto Quit;
        }

        // if (frame_count == 8)
        // {
        //    exit(0);
        // }

        f64 end = platform_get_time();
        delta_time_per_frame = end - start;
        frame_count++;

        // NOTE: Vulkan vsync is used instead
#if 0
        const u32 target_milli = 8;
        const u64 curr_milli = (u64)(delta_time * 1000.0f);
        if (target_milli > curr_milli)
        {
            u64 milli_to_sleep = (u64)(target_milli - curr_milli);
            platform_sleep(milli_to_sleep);
            f64 end2 = platform_get_time();
            delta_time = end2 - start;
        }
#endif

        last_id = frame_index;
        frame_index++;
        frame_index %= MAX_FRAMES;
    }
Quit:
    semaphore_counter_wait(&game_logic_counter);
    semaphore_counter_wait(&render_logic_counter);
    threads_destroy();
    binary_file_save(gui_ctx);
    // game_destroy();
    // gui_destroy();
    // vulkan_destroy(&app_state);
    // platform_shut_down(app_state.platform);
}

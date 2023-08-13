
#include "region_alloc.h"
#define PRINT_REGION
//
//

#if 0
#define big_to_little(s0, s1, s2, s3)                                               \
    (((u32)s0 & 0xFF) | (((u32)s1 << 8) & 0xFF00) | (((u32)s2 << 16) & 0xFF0000) |  \
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

void run_app(void)
{
    set_seed();

    Application_State app_state = { 0 };
    u16 app_width = 1480;
    u16 app_height = 1000;

    Region_Alloc region = { 0 };
    stack_init(MEGABYTE(20));
    region_init(&region, MEGABYTE(200));
    logging_init(&region);

    // NOTE: main thread should be working while the other do as well. Should it be
    // minus 1?
    thread_init(&region, 40, platform_core_count());

    Instance_State instance_state = { 0 };
    Semaphore_Counter counter = { 0 };
    Thread_Task task = thread_task(instance_init_threaded, &instance_state);
    thread_tasks_push(&task, 1, &counter);

    find_working_dir(&region);

    platform_init(&region, "Syntics Engine", &app_width, &app_height, true,
                  &app_state.platform);
    event_init(&region, app_state.platform, 20, &app_state.running);

    // Need both platform window and instance to initialize vulkan
    semaphore_counter_wait_and_free(&counter);

    vulkan_init(&region, &instance_state, &app_state, (u32)app_width,
                (u32)app_height);

    const u32 window_count = 5;
    gui_init(&region, app_state.device, app_state.phy_device, app_state.com_pool,
             graphic_queue_get(app_state.render_state), &app_state.swap_chain,
             app_state.platform, app_state.num_semaphores, window_count, true,
             &app_state.gui_ctx);

    app_state.win_handles =
        region_array_calloc(&region, window_count, Window_Handle);
    for (u32 i = 0; i < window_count; i++)
    {
        array_val(app_state.win_handles, i) = window_create(&app_state.gui_ctx);
    }
    Game_State game_state = { 0 };
    game_init(&region, app_state.device, app_state.phy_device, app_state.com_pool,
              graphic_queue_get(app_state.render_state), &app_state.swap_chain,
              app_state.platform, app_state.render_state, app_state.num_semaphores,
              &game_state);

#define MAX_FRAMES 2
    Frame_Data frame_datas[MAX_FRAMES] = { 0 };
    for (u32 i = 0; i < MAX_FRAMES; i++)
    {
        Frame_Data* frame = frame_datas + i;
        region_init(&frame->frame_region, MEGABYTE(2));

        frame->game_triangle_strip_pipeline = &game_state.triangle_strip_pipeline;
        frame->game_triangle_list_pipeline = &game_state.triangle_list_pipeline;
        frame->game_line_list_pipeline = &game_state.line_list_pipeline;
        frame->game_grass_pipeline = &game_state.grass_pipeline;

        frame->game_vert_idx_buffer = game_state.vert_idx_buffer;
        frame->game_road_vert_idx = game_state.road_vert_idx;
        frame->game_road_line_vert_idx = game_state.road_line_vert_idx;
        frame->game_particles_vert_idx = game_state.particles_vert_idx;
        frame->game_aabb_rep = game_state.aabb_rep;

        frame->game_terrain_offsets = game_state.terrain_offsets;
        frame->game_dude_offsets = game_state.dude_offsets;
        frame->game_tree_offsets = game_state.tree_offsets;
        frame->game_sign_offsets = game_state.sign_offsets;
        frame->game_grass_offsets = game_state.grass_offsets;
    }
    gui_frames_init(app_state.device, app_state.phy_device, app_state.com_pool,
                    graphic_queue_get(app_state.render_state), frame_datas,
                    MAX_FRAMES, window_count);

    u32 frame_index = 0;

    const u32 frames_to_count = 30;
    f64 delta_time = MILLISECONDS(16.0);
    f64 delta_time_per_frame = MILLISECONDS(16.0);
    f64 sec_for_delta_update = 0.0;
    const f64 sec_for_delta_update_duration = 0.5;
    f64 sec2 = 0.0;
    u32 frames = 0;
    app_state.running = true;
    while (app_state.running)
    {
        Frame_Data* frame = frame_datas + frame_index;
        frame->frame_region.current_pos = 0;
        f64 start = platform_get_time();

        sec2 += delta_time_per_frame;
        sec_for_delta_update += delta_time_per_frame;

        if (sec_for_delta_update >= sec_for_delta_update_duration)
        {
            const f64 time = sec_for_delta_update;

            delta_time = time / (f64)frames;
            app_state.fps = (u32)((f64)frames / time);

            frames = 0;
            sec_for_delta_update = 0.0;
        }
        if (sec2 >= 4.0f)
        {
            stack_begin_scope(region_print_stack);
#ifdef PRINT_REGION
            region_print(&region);
            sy_print("Stack size: %llu\n", stack_size());
#endif

            sec2 = 0;
            stack_end_scope(region_print_stack);
        }

        Render_State_Internal* state_internal =
            (Render_State_Internal*)app_state.render_state;
        u32 semaphore_idx = state_internal->semaphore_index;

        V2 dimensions = v2f((f32)app_state.swap_chain.extent_2D.width,
                            (f32)app_state.swap_chain.extent_2D.height);

        frame->dimensions = dimensions;

        gui_update_begin(&app_state.gui_ctx, dimensions, semaphore_idx,
                         (f32)delta_time);

        game_update(&game_state, &app_state, app_state.render_state,
                    &frame_datas[frame_index], dimensions, semaphore_idx,
                    (f32)delta_time);

        gui_update_end(&app_state.gui_ctx, app_state.render_state,
                       &frame_datas[frame_index]);

        frame_render(app_state.render_state, &app_state, (f32)delta_time);

        event_poll(app_state.platform);
        if (is_key_pressed(SYNT_KEY_R) && !is_focus())
        {
            app_state.running = false;
            goto Quit;
        }

        f64 end = platform_get_time();
        delta_time_per_frame = end - start;
        frames++;

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

        frame_index++;
        frame_index %= MAX_FRAMES;
    }
Quit:
    threads_destroy();
    // game_destroy();
    // gui_destroy();
    // vulkan_destroy(&app_state);
    // platform_shut_down(app_state.platform);
}

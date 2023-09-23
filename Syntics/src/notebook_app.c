#ifndef SY_INCLUDES // only for clangd
#include "syntics.h"
#endif

#define PRINT_NOTE_REGION

void run_notebook_app(void)
{
    set_seed();

    Region_Alloc region = { 0 };
    stack_init(MEGABYTE(2));
    region_init(&region, MEGABYTE(50));
    logging_init(&region);

    Application_State* app_state =
        region_calloc_struct(&region, Application_State);
    u16 app_width = 1480;
    u16 app_height = 1000;

    // NOTE: main thread should be working while the other do as well.
    thread_init(&region, 40, platform_core_count() - 1);

    Instance_State instance_state = { 0 };
    instance_init(&instance_state.instance);

    find_working_dir(&region);

    platform_init(&region, "Syntics Engine", &app_width, &app_height, true,
                  &app_state->platform);

    event_init(&region, app_state->platform, 20, &app_state->running);

    Render_State* render_state = NULL;
    vulkan_init(&region, &instance_state, app_state, &render_state,
                (u32)app_width, (u32)app_height);

    const u32 window_count = 5;
    Gui_Context* gui_ctx = region_calloc_struct(&region, Gui_Context);
    gui_init(&region, app_state->device, app_state->phy_device,
             app_state->com_pool, graphic_queue_get(render_state),
             &app_state->swap_chain, app_state->platform,
             app_state->num_semaphores, window_count, true, gui_ctx);

    Notebook* notebook = region_calloc_struct(&region, Notebook);
    const u32 gui_windows = 2;
    notebook->win_handles =
        region_array_calloc(&region, gui_windows, Window_Handle);
    for (u32 i = 0; i < gui_windows; i++)
    {
        array_val(notebook->win_handles, i) = window_create(gui_ctx);
    }
    notebook_init(&region, app_state->device, app_state->phy_device,
                  app_state->com_pool, graphic_queue_get(render_state),
                  &app_state->swap_chain, app_state->platform, render_state,
                  app_state->num_semaphores, notebook);

    Gui_Frame gui_frame = { 0 };
    gui_frames_init(app_state->device, app_state->phy_device,
                    app_state->com_pool, graphic_queue_get(render_state),
                    &gui_frame, 1, window_count);

    const u32 frames_to_count = 30;
    f64 delta_time = MILLISECONDS(16.0);
    f64 delta_time_per_frame = MILLISECONDS(16.0);
    f64 sec_for_delta_update = 0.0;
    const f64 sec_for_delta_update_duration = 0.5;
    f64 sec2 = 0.0;
    u32 frame_count = 0;
    app_state->running = true;

    Region_Alloc frame_region = { 0 };
    region_init(&frame_region, MEGABYTE(2));
    while (app_state->running)
    {
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
#ifdef PRINT_NOTE_REGION
            region_print(&region);
            sy_print("Stack size: %llu\n", stack_size());
#endif

            sec2 = 0;
            stack_end_scope(region_print_stack);
        }

        u32 semaphore_idx = semaphore_idx_get(render_state);

        V2 dimensions = v2f((f32)app_state->swap_chain.extent_2D.width,
                            (f32)app_state->swap_chain.extent_2D.height);

        region_reset(&frame_region);

        Render_Task* copy_tasks =
            region_array_calloc(&frame_region, 1, Render_Task);
        Render_Task* render_tasks =
            region_array_calloc(&frame_region, 1, Render_Task);

        gui_frame.semaphore_idx = semaphore_idx;
        gui_frame.dt = (f32)delta_time;
        gui_frame.dimensions = dimensions;

        // NOTE: This is has to be here for now. Gui is copying to the staging
        // buffer. And the command to copy the staging buffer to local storage
        // needs to have finished before that happens.
        frame_begin(render_state, app_state);

        gui_update_begin(gui_ctx, dimensions, semaphore_idx, (f32)delta_time);

        notebook_update(notebook, gui_ctx, app_state, dimensions, semaphore_idx,
                        (f32)delta_time);

        gui_update_end(gui_ctx, &gui_frame, copy_tasks, render_tasks,
                       &frame_region);

        frame_render(render_state, app_state, copy_tasks, render_tasks,
                     (f32)delta_time);

        event_poll(app_state->platform);
        if (is_key_pressed(SYNT_KEY_R) && !gui_is_focus())
        {
            app_state->running = false;
            goto Quit;
        }

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
    }
Quit:
    threads_destroy();
    // vulkan_destroy(&app_state);
    // platform_shut_down(app_state.platform);
}

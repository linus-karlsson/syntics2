#ifndef SY_UNIT_BUILD
#endif

#define PRINT_NOTE_REGION

void run_notebook_app(void)
{
    set_seed();

    Application_State* app_state = NULL;
    Render_State* render_state = NULL;

    application_init(MEGABYTE(2), MEGABYTE(50), 1200, 800, 40, false, 20, 1,
                     &render_state, &app_state);

    const u32 window_count = 5;
    Gui_Context* gui_ctx =
        region_calloc_struct(&app_state->region, Gui_Context);
    gui_init(&app_state->region, app_state->device, app_state->phy_device,
             app_state->com_pool, graphic_queue_get(render_state),
             &app_state->swap_chain, app_state->platform,
             app_state->num_semaphores, window_count, true, gui_ctx);

    Notebook* notebook = region_calloc_struct(&app_state->region, Notebook);
    const u32 gui_windows = 1;
    notebook->win_handles =
        region_array_calloc(&app_state->region, gui_windows, Window_Handle);
    for (u32 i = 0; i < gui_windows; i++)
    {
        array_val(notebook->win_handles, i) = window_create(gui_ctx);
    }
    notebook_init(&app_state->region, app_state->device, app_state->phy_device,
                  app_state->com_pool, graphic_queue_get(render_state),
                  &app_state->swap_chain, app_state->platform, render_state,
                  app_state->num_semaphores, notebook);

    Gui_Frame gui_frame = { 0 };
    gui_frames_init(app_state->device, app_state->phy_device,
                    app_state->com_pool, graphic_queue_get(render_state),
                    &gui_frame, 1, window_count);

    Application_Frame app_frame = application_frame_create();
    Region_Alloc frame_region = { 0 };
    region_init(&frame_region, MEGABYTE(2));

    f64 sec = 0.0;
    app_state->running = true;
    while (app_state->running)
    {
        f64 start = platform_get_time();

        app_frame = application_begin_frame(app_frame);

        app_state->fps = app_frame.fps;

        sec += app_frame.delta_time_per_frame;
        if (sec >= 2.0)
        {
#ifdef PRINT_NOTE_REGION
            region_print(&app_state->region);
            sy_print("Stack size: %llu\n", stack_size());
#endif

            sec = 0;
        }

        u32 semaphore_idx = semaphore_idx_get(render_state);

        V2 dimensions = v2f((f32)app_state->swap_chain.extent_2D.width,
                            (f32)app_state->swap_chain.extent_2D.height);

        region_reset(&frame_region);

        Render_Task* copy_tasks =
            region_array_calloc(&frame_region, 10, Render_Task);
        Render_Task* render_tasks =
            region_array_calloc(&frame_region, 10, Render_Task);

        gui_frame.semaphore_idx = semaphore_idx;
        gui_frame.dt = (f32)app_frame.delta_time;
        gui_frame.dimensions = dimensions;

        // NOTE: This is has to be here for now. Gui is copying to the staging
        // buffer. And the command to copy the staging buffer to local storage
        // needs to have finished before that happens.
        frame_begin(render_state, app_state);

        gui_update_begin(gui_ctx, dimensions, semaphore_idx,
                         (f32)app_frame.delta_time);

        notebook_update(notebook, gui_ctx, app_state, copy_tasks, render_tasks,
                        dimensions, semaphore_idx, (f32)app_frame.delta_time);

        gui_update_end(gui_ctx, &gui_frame, copy_tasks, render_tasks,
                       &frame_region);

        frame_render(render_state, app_state, copy_tasks, render_tasks,
                     (f32)app_frame.delta_time);

        event_poll(app_state->platform);
        if (is_key_pressed(SYNT_KEY_R) && !gui_is_focus())
        {
            app_state->running = false;
            goto Quit;
        }
        f64 end = platform_get_time();
        app_frame.delta_time_per_frame = end - start;
        app_frame.frame_count++;

        // NOTE: Vulkan vsync is used instead
#if 0
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
    }
Quit:
    threads_destroy();
    gui_binary_file_save(gui_ctx);
    // vulkan_destroy(&app_state);
    // platform_shut_down(app_state.platform);
}


void application_init(u32 stack_size, u64 main_region_size, u16 app_width,
                      u16 app_height, u32 thread_pool_queue_size,
                      b8 full_screen, u32 event_count,
                      u32 vulkan_frames_in_flight, Render_State** render_state,
                      Application_State** app)
{
    Region_Alloc region = { 0 };
    stack_init(stack_size);
    region_init(&region, main_region_size);
    logging_init(&region);

    Application_State* app_state =
        region_calloc_struct(&region, Application_State);

    thread_init(&region, thread_pool_queue_size, platform_core_count() - 1);

    Instance_State instance_state = { 0 };
    instance_init(&instance_state.instance);

    find_working_dir(&region);

    platform_init(&region, "Syntics Engine", &app_width, &app_height,
                  full_screen, &app_state->platform);

    event_init(&region, app_state->platform, event_count, &app_state->running);

    app_state->num_semaphores = vulkan_frames_in_flight;
    vulkan_init(&region, &instance_state, app_state, render_state,
                (u32)app_width, (u32)app_height);

    app_state->region = region;
    *app = app_state;
}

typedef struct Application_Frame
{
    f64 delta_time;
    f64 delta_time_per_frame;
    f64 sec_for_delta_update;
    f64 sec_for_delta_update_duration;

    u32 frame_count;
    u32 frames_to_count;
    u32 fps;
} Application_Frame;

Application_Frame application_frame_create()
{
    Application_Frame frame = { 0 };
    frame.delta_time = MILLISECONDS(16.0);
    frame.delta_time_per_frame = MILLISECONDS(16.0);
    frame.sec_for_delta_update_duration = 0.5;
    frame.frames_to_count = 30;
    return frame;
}

Application_Frame application_begin_frame(Application_Frame app_frame)
{
    app_frame.sec_for_delta_update += app_frame.delta_time_per_frame;

    if (app_frame.sec_for_delta_update >=
        app_frame.sec_for_delta_update_duration)
    {
        const f64 time = app_frame.sec_for_delta_update;

        app_frame.fps = (u32)((f64)app_frame.frame_count / time);
        app_frame.delta_time = time / (f64)app_frame.frame_count;

        app_frame.frame_count = 0;
        app_frame.sec_for_delta_update = 0.0;
    }
    return app_frame;
}

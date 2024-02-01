#ifndef SY_UNIT_BUILD
#include "application.h"
#include "region_alloc.h"
#include "logging.h"
#include "thread_queue.h"
#include "platform.h"
#include "instance_device.h"
#include "event_system.h"
#include "vulkan_api.h"
#endif

void instance_init_threaded(void* data)
{
    Instance_State* instance_state = (Instance_State*)data;
    instance_init(&(instance_state->instance));
}

void syntics_application_init(u32 stack_size, u64 main_region_size, u16 app_width,
                      u16 app_height, u32 thread_pool_queue_size,
                      b8 full_screen, u32 event_count,
                      u32 vulkan_frames_in_flight, Render_State** render_state,
                      Application_State** app)
{
    Region_Alloc region = { 0 };
    syntics_region_stack_init(stack_size);
    syntics_region_init(&region, main_region_size);
    logging_init(&region);

    Application_State* app_state =
        syntics_region_calloc_struct(&region, Application_State);

    thread_init(&region, thread_pool_queue_size, syntics_platform_get_core_count() - 1,
                &app_state->thread_queue);

#define mult__
    Instance_State instance_state = { 0 };
#ifdef mult__
    Semaphore_Counter instance_counter = { 0 };
    Thread_Task instance_task =
        thread_task(instance_init_threaded, &instance_state);
    thread_tasks_push(&app_state->thread_queue.task_queue, &instance_task, 1,
                      &instance_counter);
#else
    instance_init(&instance_state.instance);
#endif

    syntics_find_working_dir(&region);

    syntics_platform_init(&region, "Syntics Engine", &app_width, &app_height,
                  full_screen, &app_state->platform);

    event_init(&region, app_state->platform, event_count, &app_state->running);

#ifdef mult__
    semaphore_counter_wait_and_free(&instance_counter);
#endif

    app_state->num_semaphores = vulkan_frames_in_flight;
    vulkan_init(&region, &instance_state, app_state, render_state,
                (u32)app_width, (u32)app_height);

    app_state->region = region;
    *app = app_state;
}

Application_Frame syntics_application_frame_create(void)
{
    Application_Frame frame = { 0 };
    frame.delta_time = MILLISECONDS(16.0);
    frame.delta_time_per_frame = MILLISECONDS(16.0);
    frame.sec_for_delta_update_duration = 0.5;
    frame.frames_to_count = 30;
    return frame;
}

Application_Frame syntics_application_begin_frame(Application_Frame app_frame)
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

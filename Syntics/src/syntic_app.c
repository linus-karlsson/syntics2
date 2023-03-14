#include "syntic_app.h"
#include "logging.h"
#include "syntics.h"
#include "random.h"
#include <math.h>

static Application_State app_state = { 0 };
const u32 WIDTH = 1280;
const u32 HEIGHT = 800;

void run_app()
{
    set_seed();

    Region_Alloc region = { 0 };
    init_region(&region, MEGABYTE(10));
    gui_terminal_init(&region);
    init_events(&region, 20);
    init_platform("Syntics Engine", (u16)WIDTH, (u16)HEIGHT);
    init_vulkan(&region, &app_state, WIDTH, HEIGHT);

    const u32 frames_to_count = 50;
    // const u32 target_milli = 10;

    f64 delta_time = 0.0, sec2 = 0.0;
    u32 frames = 0;
    f64 start2 = 0;
    app_state.running = true;
    while (app_state.running)
    {
        f64 start = get_time();

        sec2 += delta_time;

        if (frames == 0) start2 = get_time();
        if (frames++ >= frames_to_count)
        {
            f64 end2 = get_time();
            f64 time = end2 - start2;

            app_state.fps = (uint32)(frames_to_count / time);
            frames = 0;
        }
        if (sec2 >= 2.0f)
        {
            print_region(&region);
            sec2 = 0;
        }
        render(&region, &app_state, (f32)delta_time);

        poll_events();
        if (is_key_pressed(SYNT_KEY_R) && !gui_focus())
        {
            app_state.running = false;
        }

        f64 end = get_time();
        delta_time = end - start;
#if 0
        const u64 curr_milli = (u64)(delta_time * 1000.0f);
        if (target_milli > curr_milli)
        {
            u64 milli_to_sleep = (u64)(target_milli - curr_milli);
            platform_sleep(milli_to_sleep);
            f64 end2 = get_time();
            delta_time = end2 - start;
        }
#endif
    }

    destroy_vulkan();
    shut_down_platform();

    printf("Complete!\n");
}


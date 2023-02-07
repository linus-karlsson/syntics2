#include "syntic_app.h"
#include "syntics.h"
#include "random.h"
#include <math.h>

static Application_State app_state = {};
const uint32 WIDTH = 1280;
const uint32 HEIGHT = 800;

void run_app()
{
    set_seed();

    Region_Alloc region;
    init_region(&region, MEGABYTE(10));
    gui_terminal_init(&region);
    init_events(&region, 20);
    init_platform("Syntics Engine", true, WIDTH, HEIGHT);
    init_vulkan(&region, &app_state, WIDTH, HEIGHT);

    Events* evt;
    subscribe(&evt, EVT_KEY);

    const uint32 frames_to_count = 50;
    const uint64 target_milli = 10;

    double delta_time = 0.0f, sec = 0.0f, sec2 = 0.0f, dddt;
    uint32 frames = 0;
    double start2 = 0;
    app_state.running = true;
    while (app_state.running)
    {
        double start = get_time();

        sec += delta_time;
        sec2 += delta_time;

        if (frames == 0) start2 = get_time();
        if (frames++ >= frames_to_count)
        {
            double end2 = get_time();
            double time = end2 - start2;

            app_state.fps = (uint32)(frames_to_count / time);
            frames = 0;
        }
        if (sec2 >= 2.0f)
        {
            print_region(region);
            sec2 = 0;
        }
        render(&region, app_state, (float)delta_time);

        poll_events();
        if (is_key_pressed(SYNT_R_PRESSED) && !gui_focus())
        {
            app_state.running = false;
        }

        double end = get_time();
        delta_time = end - start;
#if 1
        const uint64 curr_milli = (uint64)(delta_time * 1000.0f);
        if (target_milli > curr_milli)
        {
            DWORD milli_to_sleep = (DWORD)(target_milli - curr_milli);
            platform_sleep(milli_to_sleep);
            double end2 = get_time();
            delta_time = end2 - start;
        }
#endif
    }

    // destroy_vulkan();
    shut_down_platform();

    synt_LOG("\nComplete!\n");
}


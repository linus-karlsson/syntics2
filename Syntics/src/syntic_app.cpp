#include "syntic_app.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "syntics.h"
#include "font.h"

namespace synt {

static void uint_to_string(char* buffer, uint32 len_buffer, uint32 i)
{
    const char* numbers = "0123456789";

    uint32 n = i;
    uint32 c = 0;
    do
    {
        c++;
    } while (n /= 10);

    assert(len_buffer >= c);

    n = c;
    do
    {
        (buffer)[--c] = numbers[i % 10];
    } while (i /= 10);

    (buffer)[n] = '\0';
}

static Application_State app_state = {};
const uint32 WIDTH                 = 1280;
const uint32 HEIGHT                = 800;

void run_app(int argc, char* argv[])
{
    if (argc > 1)
    {
        if (argv[1] == "this") return;
        set_log(false);
        set_log_alloc(false);
    }

    Region_Alloc region;
    init_region(&region, 4000000);
    init_events(&region, 5);
    init_platform("Syntics Engine", WIDTH, HEIGHT);
    init_vulkan(&region, &app_state, WIDTH, HEIGHT);

    Events* evt;
    subscribe(&evt, EVT_KEY);

    const uint32 frames_to_count = 50;
    const uint32 target_milli    = 8;

    print_region(region);
    double delta_time = 0.0f, sec = 0.0f, sec2 = 0.0f;
    uint32 fps = 0, frames = 0;
    double start2 = 0;
    bool running  = true;
    while (running)
    {
        double start = get_time();

        sec += delta_time;
        sec2 += delta_time;

        if (frames == 0) start2 = get_time();
        if (frames++ >= frames_to_count)
        {
            double end2 = get_time();
            double time = end2 - start2;

            fps    = (uint32)(frames_to_count / time);
            frames = 0;
        }
        if (sec2 >= 2.0f)
        {
            print_region(region);
            sec2 = 0;
        }
        if (sec >= 0.1f)
        {
            char title[50] = "| Syntics Engine | FPS: ";
            uint32 len     = (uint32)strlen(title);
            uint_to_string(title + len, 50 - len, fps);
            change_title(title, strlen(title));
            sec = 0;
        }

        render(&region, app_state, (float)delta_time);

        poll_events();
        if (is_key_pressed(SYNT_R_PRESSED)) running = false;

        double end = get_time();
        delta_time = end - start;
#if 1
        const uint64 curr_milli = (uint64)(delta_time * 1000.0f);
        if (target_milli > curr_milli)
        {
            linux_sleep(target_milli - curr_milli);

            delta_time = (target_milli - curr_milli) * 0.001f;
        }
#endif
    }

    destroy_vulkan();
    shut_down_platform();

    synt_LOG("\nComplete!\n");
}

} // namespace synt

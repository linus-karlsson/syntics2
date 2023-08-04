
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

    Region_Alloc* region = NULL;
    stack_init(MEGABYTE(20));
    region_init(&region, MEGABYTE(200));
    logging_init(region);

    thread_init(region, 20);

    Instance_State instance_state = { 0 };
    Semaphore* thread_handle =
        thread_task_push(instance_init_threaded, &instance_state);

    find_working_dir(region);

    platform_init(region, "Syntics Engine", &app_width, &app_height, true,
                  &app_state.platform);
    event_init(region, app_state.platform, 20, &app_state.running);

    // Need both platform window and instance to initialize vulkan
    semaphore_wait(thread_handle);

    vulkan_init(region, &instance_state, &app_state, (u32)app_width,
                (u32)app_height);

#if 0
    Wav_Header header = {};
    i32* samples;

    {
        stack_begin_scope();

        File_Attrib file = {};
        read_file(&file, get_stack(), "Syntics/res/sound/tale.wav", "rb");

        b32 done = false;
        while(!done)
        {
            switch (*(u32*)file.buffer)
            {
                case WAV_RIFF:
                {
                    file.buffer += sizeof(u32);
                    u32 file_size = *(u32*)file.buffer;
                    file.buffer += sizeof(u32);
                    file_size += 0;
                    break;
                }
                case WAV_WAVE:
                {
                    file.buffer += sizeof(u32);
                    break;
                }
                case WAV_FMT:
                {
                    file.buffer += sizeof(u32);
                    file.buffer += sizeof(u32);
                    header = *(Wav_Header*)file.buffer;
                    file.buffer += sizeof(Wav_Header);
                    break;
                }
                case WAV_DATA:
                {
                    file.buffer += sizeof(u32);
                    u32 data_size = *(u32*)file.buffer;
                    file.buffer += sizeof(u32);

                    assert(data_size % sizeof(u32) == 0);
                    samples = region_array(&region, data_size / sizeof(u32), i32);
                    memcpy(samples, file.buffer, data_size);

                    done = true;
                    break;
                }
            }
        }

        stack_end_scope();
    }
#endif

    const u32 frames_to_count = 50;

    f64 delta_time = 0.0, sec2 = 0.0;
    u32 frames = 0;
    f64 start2 = 0;
    app_state.running = true;

    while (app_state.running)
    {

        f64 start = platform_get_time();

        sec2 += delta_time;

        if (frames == 0) start2 = platform_get_time();
        if (frames++ >= frames_to_count)
        {
            f64 end2 = platform_get_time();
            f64 time = end2 - start2;

            app_state.fps = (uint32)(frames_to_count / time);
            frames = 0;
        }
        if (sec2 >= 4.0f)
        {
            stack_begin_scope(region_print_stack);
#ifdef PRINT_REGION
            region_print(region);
            sy_print("Stack size: %llu\n", stack_size());
#endif

            sec2 = 0;
            stack_end_scope(region_print_stack);
        }
        render(region, app_state.render_state, app_state.platform, &app_state,
               (f32)delta_time);

        event_poll(app_state.platform);
        if (is_key_pressed(SYNT_KEY_R) && !is_focus())
        {
            app_state.running = false;
            goto Quit;
        }

        f64 end = platform_get_time();
        delta_time = end - start;

        // Vulkan vsync is used instead
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
    vulkan_destroy(&app_state);
    platform_shut_down(app_state.platform);

    printf("Complete!\n");
}


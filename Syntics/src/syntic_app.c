#include "syntic_app.h"
#include "logging.h"
#include "syntics.h"
// #include <dsound.h>
#include <math.h>

static Application_State app_state = { 0 };
u16 WIDTH = 1480;
u16 HEIGHT = 1000;

#if 0
#define DIRECT_SOUND_CREATE(name)                                                   \
    HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND8* ppDS,                 \
                        LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(Direct_Sound_Create);
#endif

void run_app()
{

    Region_Alloc region = { 0 };
    init_region(&region, MEGABYTE(10));
    init_stack(MEGABYTE(1));
    gui_terminal_init(&region);
    init_events(&region, 20);
    init_platform("Syntics Engine", &WIDTH, &HEIGHT, true);
    init_vulkan(&region, &app_state, (u32)WIDTH, (u32)HEIGHT);

#if 0
    LPDIRECTSOUNDBUFFER secondary_buffer = NULL;
    DWORD secondary_buffer_size = 0;

    HMODULE dsound_lib = LoadLibraryA("dsound.dll");

    DWORD bytes_per_sample = sizeof(i16) * 2;

    if (dsound_lib)
    {
        Direct_Sound_Create* direct_sound_create =
            (Direct_Sound_Create*)GetProcAddress(dsound_lib, "DirectSoundCreate8");

        LPDIRECTSOUND8 direct_sound;
        if (direct_sound_create &&
            SUCCEEDED(direct_sound_create(0, &direct_sound, 0)))
        {
            WAVEFORMATEX wave_format = { 0 };
            wave_format.wFormatTag = WAVE_FORMAT_PCM;
            wave_format.nChannels = 2;
            wave_format.nSamplesPerSec = 48000;
            wave_format.wBitsPerSample = 16;
            wave_format.nBlockAlign =
                (wave_format.nChannels * wave_format.wBitsPerSample) / 8;
            wave_format.nAvgBytesPerSec =
                wave_format.nSamplesPerSec * wave_format.nBlockAlign;
            wave_format.cbSize = 0;
            if (SUCCEEDED(IDirectSound8_SetCooperativeLevel(direct_sound, get_win(),
                                                            DSSCL_PRIORITY)))
            {
                DSBUFFERDESC buffer_desc = { 0 };
                buffer_desc.dwSize = sizeof(buffer_desc);
                buffer_desc.dwFlags = DSBCAPS_PRIMARYBUFFER;

                LPDIRECTSOUNDBUFFER prime_buffer;
                if (SUCCEEDED(IDirectSound8_CreateSoundBuffer(
                        direct_sound, &buffer_desc, &prime_buffer, 0)))
                {
                    if (SUCCEEDED(IDirectSoundBuffer8_SetFormat(prime_buffer,
                                                                &wave_format)))
                    {
                    }
                }
            }
            DSBUFFERDESC buffer_desc = { 0 };
            buffer_desc.dwSize = sizeof(buffer_desc);
            buffer_desc.dwFlags = 0;
            buffer_desc.dwBufferBytes =
                wave_format.nSamplesPerSec * bytes_per_sample;
            buffer_desc.lpwfxFormat = &wave_format;
            if (SUCCEEDED(IDirectSound8_CreateSoundBuffer(direct_sound, &buffer_desc,
                                                          &secondary_buffer, 0)))
            {
            }
        }
        else
        {
            SY_ERROR("dssound");
        }
    }
    else
    {
        SY_ERROR("dssound");
    }

    ASSERT(secondary_buffer, "secondary_buffer");

    u32 sample_index = 0;
    int sample_per_sec = 48000;
    int tone_hz = 256;
    i16 tone_volume = 1000;
    int square_wave_period = sample_per_sec / tone_hz;
    int half_square_period = square_wave_period / 2;
    secondary_buffer_size = sample_per_sec * bytes_per_sample;
    b32 sound_is_playing = false;
#endif

    const u32 frames_to_count = 50;
    const u32 target_milli = 10;

    f64 delta_time = 0.0, sec2 = 0.0;
    u32 frames = 0;
    f64 start2 = 0;
    app_state.running = true;

    while (app_state.running)
    {
#if 0 
        DWORD play_cursor;
        DWORD write_cursor;
        if (SUCCEEDED(IDirectSoundBuffer8_GetCurrentPosition(
                secondary_buffer, &play_cursor, &write_cursor)))
        {
            DWORD bytes_to_lock =
                (sample_index * bytes_per_sample) % secondary_buffer_size;
            DWORD bytes_to_write;
            if (bytes_to_lock == play_cursor)
            {
                bytes_to_write = secondary_buffer_size;
            }
            else if (bytes_to_lock > play_cursor)
            {
                bytes_to_write =
                    (secondary_buffer_size - bytes_to_lock) + play_cursor;
            }
            else
            {
                bytes_to_write = play_cursor - bytes_to_lock;
            }

            void* region1;
            DWORD region1_size;
            void* region2;
            DWORD region2_size;

            if (SUCCEEDED(IDirectSoundBuffer8_Lock(
                    secondary_buffer, bytes_to_lock, bytes_to_write, &region1,
                    &region1_size, &region2, &region2_size, 0)))
            {
                ASSERT(region1_size % bytes_per_sample == 0, "region1_size");
                ASSERT(region2_size % bytes_per_sample == 0, "region2_size");

                DWORD region1_sample_count = region1_size / bytes_per_sample;
                i16* sample_out = (i16*)region1;
                for (DWORD i = 0; i < region1_sample_count; i++)
                {
                    int16 sample_value = ((sample_index++ / half_square_period) % 2)
                                             ? tone_volume
                                             : -tone_volume;
                    *sample_out++ = sample_value;
                    *sample_out++ = sample_value;
                }

                DWORD region2_sample_count = region2_size / bytes_per_sample;
                sample_out = (i16*)region2;
                for (DWORD i = 0; i < region2_sample_count; i++)
                {
                    int16 sample_value = ((sample_index++ / half_square_period) % 2)
                                             ? tone_volume
                                             : -tone_volume;
                    *sample_out++ = sample_value;
                    *sample_out++ = sample_value;
                }
                IDirectSoundBuffer8_Unlock(secondary_buffer, region1, region1_size,
                                           region2, region2_size);
            }
            if (!sound_is_playing)
            {
                IDirectSoundBuffer8_Play(secondary_buffer, 0, 0, DSBPLAY_LOOPING);
                sound_is_playing = true;
            }
        }
        else
        {
            SY_ERROR("IN WHILE SOUND");
        }
#endif

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
            synt_LOG_Term("Stack size: %u\n", get_stack()->currentPos);
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
#if 1
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

    // destroy_vulkan();
    // shut_down_platform();

    printf("Complete!\n");
}


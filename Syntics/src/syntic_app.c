
#define PRINT_REGION
//
//

static Application_State global_app_state = { 0 };
u16 APP_WIDTH = 1480;
u16 APP_HEIGHT = 1000;

#if 0
#if 1
#define DIRECT_SOUND_CREATE(name)                                                   \
    HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND8* ppDS,                 \
                        LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(Direct_Sound_Create);
#endif

#pragma pack(push, 1)
struct Wav_Header
{
    u16 wave_format_size;
    u16 channels;
    u32 sample_rate;
    u32 bytes_per_sec;
    u16 block_alignment;
    u16 bits_per_sample;
};
#pragma pack(pop)

typedef struct Thread_Sound_Attrib
{
    u32 id;
    DWORD play_flag;
    HANDLE start_semaphore;
    u32 sample_rate;
    u32 bits_per_sample;
    u32* data;
} Thread_Sound_Attrib;

LPDIRECTSOUNDBUFFER secondary_buffer = NULL;
DWORD secondary_buffer_size = 0;

#if 0
unsigned long play_sound_thread(void* data)
{
    Thread_Sound_Attrib* attrib = (Thread_Sound_Attrib*)data;
    u32 sample_index = 0;
    int sample_per_sec = 48000;
    int tone_hz = 256;
    i16 tone_volume = 500;
    int square_wave_period = sample_per_sec / tone_hz;
    int half_square_period = square_wave_period / 2;
    secondary_buffer_size = sample_per_sec * bytes_per_sample;
    b32 sound_is_playing = false;

    for (;;)
    {
        WaitForSingleObject(attrib->start_semaphore, INFINITE);
        sound_is_playing = false;
#if 0
        DWORD play_cursor;
        DWORD write_cursor;
        if (SUCCEEDED(IDirectSoundBuffer8_GetCurrentPosition(
                secondary_buffer, &play_cursor, &write_cursor)))
#endif
        {
#if 0
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
#else
            sample_index = 0;
            DWORD bytes_to_lock = 0;
            DWORD bytes_to_write = secondary_buffer_size;
#endif

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
                // DSBPLAY_LOOPING
                IDirectSoundBuffer8_Play(secondary_buffer, 0, 0, 0);
                sound_is_playing = true;
            }
        }
#if 0
        else
        {
            SY_ERROR("IN WHILE SOUND");
        }
#endif
    }
}
#endif

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

void run_app()
{
    set_seed();
    init_logging();

    Region_Alloc region = { 0 };
    init_region(&region, MEGABYTE(20));
    init_stack(MEGABYTE(70));
    init_terminal(&region);
    init_events(&region, 20);
    init_platform("Syntics Engine", &APP_WIDTH, &APP_HEIGHT, true);
    init_vulkan(&region, &global_app_state, (u32)APP_WIDTH, (u32)APP_HEIGHT);

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
                    samples = dyn_arrayP(&region, data_size / sizeof(u32), i32);
                    memcpy(samples, file.buffer, data_size);

                    done = true;
                    break;
                }
            }
        }

        stack_end_scope();
    }

    HMODULE dsound_lib = LoadLibraryA("dsound.dll");

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
            wave_format.nChannels = header.channels;
            wave_format.nSamplesPerSec = header.sample_rate;
            wave_format.wBitsPerSample = 32;
            wave_format.nBlockAlign = header.block_alignment;
            wave_format.nAvgBytesPerSec = header.bytes_per_sec;
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

#endif

    const u32 frames_to_count = 50;

    f64 delta_time = 0.0, sec2 = 0.0;
    u32 frames = 0;
    f64 start2 = 0;
    global_app_state.running = true;

#if 0
    HANDLE start_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
    Thread_Sound_Attrib th = {};
    th.sample_rate = header.sample_rate;
    th.bits_per_sample = 32;
    th.start_semaphore = start_semaphore;
    th.id = 0;
    thread_create(&th, play_sound_thread, 0, NULL);
#endif

    while (global_app_state.running)
    {

#if 0
        presist b8 clicked = true;
        if (is_key_clicked(&clicked, SYNT_KEY_Y))
        {
            ReleaseSemaphore(start_semaphore, 1, 0);
        }
#endif
        f64 start = get_time();

        sec2 += delta_time;

        if (frames == 0) start2 = get_time();
        if (frames++ >= frames_to_count)
        {
            f64 end2 = get_time();
            f64 time = end2 - start2;

            global_app_state.fps = (uint32)(frames_to_count / time);
            frames = 0;
        }
        if (sec2 >= 4.0f)
        {
            stack_begin_scope();
#ifdef PRINT_REGION
            print_region(&region);
            sy_print("Stack size: %llu\n", get_stack()->currentPos);
#endif

            sec2 = 0;
            stack_end_scope();
        }
        render(&region, &global_app_state, (f32)delta_time);

        poll_events();
        if (is_key_pressed(SYNT_KEY_R) && !is_focus())
        {
            global_app_state.running = false;
        }

        f64 end = get_time();
        delta_time = end - start;
#if 1
        const u32 target_milli = 10;
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


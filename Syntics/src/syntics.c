#define SY_UNIT_BUILD

#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
//#include <immintrin.h> // This takes a lot of time to compile linux: ~0.25 sec


#include <vulkan/vulkan.h> // comp time linux: ~0.02 sec

#ifdef LINUX

// NOTE: creating a smaller file for these does NOT increase compilation time
#include <xcb/xcb.h> 
#include <xcb/xcb_cursor.h>
#include <xcb/xfixes.h>
#include <vulkan/vulkan_xcb.h>

//////////////////////////////

// NOTE: creating a smaller file for these does NOT increase compilation time
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
///////////////////////

#define thread_return_value void*
#define File_Change_Handle void*
#define Thread_Handle pthread_t
#define Mutex pthread_mutex_t
#define Semaphore sem_t

#define MAX_PATH 260

#define sysprintf(...) snprintf(__VA_ARGS__)
#define syscanf(...) sscanf(__VA_ARGS__)
#define sy_gcvt(buffer, buffer_size, val, num_digits)                          \
    gcvt(val, num_digits, buffer);


#else
#if 0
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else

#include "win32/sy_windows.h"
#include <vulkan/vulkan_win32.h>

#define thread_return_value unsigned long
#define File_Change_Handle HANDLE
#define Thread_Handle HANDLE
#define Mutex HANDLE
#define Semaphore HANDLE

#define sysprintf(...) sprintf_s(__VA_ARGS__)
#define syscanf(...) sscanf_s(__VA_ARGS__)
#define sy_gcvt(...) _gcvt_s(__VA_ARGS__);

#endif
#endif

// Vendor
#include <stb/stb_image_min.h>
#include <stb/stb_truetype.h>

// NOTE: To here the compilation time on linux is ~0.148 without immintrin.h

#include "defines.h"
#include "math/syntics_math.h"
#include "vulkan_types.h"

#include "ansi_keycodes.h"
#include "event_system.h"
#include "region_alloc.h"
#include "lookup_table.h"
#include "entity.h"
#include "font.h"
#include "render_util.h"
#include "simple_particle.h"
#include "camera.h"
#include "logging.h"
#include "collision.h"
#include "gui.h"
#include "game.h"
#include "obj_load.h"
#include "notebook.h"

typedef enum Visible_Local
{
    VERTEX_INDEX_VISIBLE_VISIBLE,
    VERTEX_INDEX_VISIBLE_LOCAL,
    VERTEX_INDEX_LOCAL_VISIBLE,
    VERTEX_INDEX_LOCAL_LOCAL,
} Visible_Local;

#ifdef DEBUG
global const b8 VALIDATIONS_ENABLE = true;
#else
global const b8 VALIDATIONS_ENABLE = false;
#endif

global char* WORKING_DIR = NULL;
global u32 WORKING_DIR_LEN = 0;

#include "application.h"
#include "frame_data.h"
#include "region_alloc.h"
#include "file_reading.h"
#include "render.h"

#include "file_reading.c"
#include "noise.c"
#include "random.c"

#ifdef LINUX
#include "linux/linux_platform.c"
#else
#include "win32/win32_platform.c"
#endif

#include "region_alloc.c"

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

#include "logging.c"
#include "thread_queue.c"

#include "instance_device.c"
#include "buffers.c"
#include "event_system.c"
#include "lookup_table.c"
#include "entity.c"
#include "math/syntics_math.c"
#include "obj_load.c"
#include "collision.c"
#include "swap_chain.c"
#include "render_util.c"
#include "font.c"
#include "simple_particle.c"
#include "camera.c"
#include "render.c"
#include "gui.c"
#include "game.c"
//#include "test_bed.c"
#include "notebook.c"
#include "vulkan_api.c"
#include "application.c"
#include "notebook_app.c"
#include "syntic_app.c"
#include "main.c"

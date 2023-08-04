#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <immintrin.h>

#include <vulkan/vulkan.h>

#ifdef LINUX

#include <xcb/xcb.h>
#include <xcb/xfixes.h>
#include <xcb/xcb_cursor.h>
#include <vulkan/vulkan_xcb.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <unistd.h>

#define thread_return_value void*
#define File_Change_Handle void*
#define Thread_Handle pthread_t
#define Mutex pthread_mutex_t
#define Semaphore sem_t

#define MAX_PATH 260

#define sysprintf(...) snprintf(__VA_ARGS__)
#define syscanf(...) sscanf(__VA_ARGS__)
#define sy_gcvt(buffer, buffer_size, val, num_digits) gcvt(val, num_digits, buffer);

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

#define SY_INCLUDES

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

typedef struct File_Attrib
{
    u8* buffer;
    u32 current_pos;
    u32 size;
} File_Attrib;

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

#include "noise.c"
#include "random.c"
#include "region_alloc.c"

#ifdef LINUX
#include "linux/linux_platform.c"
#else
#include "win32/win32_platform.c"
#endif

#include "logging.c"
#include "file_reading.c"

#include "thread_queue.c"

#include "instance_device.c"
#include "buffers.c"
#include "event_system.c"
#include "lookup_table.c"
#include "entity.c"
#include "math/syntics_math.c"
#include "obj_load.c"
#include "collision.c"
#include "font.c"
#include "swap_chain.c"
#include "render_util.c"
#include "simple_particle.c"
#include "camera.c"
#include "render.c"
#include "gui.c"
#include "game.c"
#include "test_bed.c"
#include "vulkan_api.c"
#include "syntic_app.c"
#include "main.c"

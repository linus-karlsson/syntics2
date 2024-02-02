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
#endif
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
#include "platform.h"

#ifdef DEBUG
global const b8 VALIDATIONS_ENABLE = true;
#else
global const b8 VALIDATIONS_ENABLE = false;
#endif

#include "thread_queue.h"
#include "application.h"
#include "frame_data.h"
#include "region_alloc.h"
#include "file_reading.h"
#include "render.h"
#include "hash_table.h"

#include "hash.c"
#include "file_reading.c"
#include "noise.c"
#include "random.c"


#ifdef LINUX
#include "linux/linux_platform.c"
#else
#include "win32/win32_platform.c"
#endif

#include "region_alloc.c"

#include "logging.c"
#include "thread_queue.c"

#include "hash_table.c"
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
//#include "notebook.c"
#include "vulkan_api.c"
#include "application.c"
//#include "notebook_app.c"
#include "syntic_app.c"
#include "vulkan_types.c"
#include "main.c"

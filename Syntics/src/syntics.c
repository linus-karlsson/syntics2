#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#if 1
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <win32/sy_windows.h>
#endif

#include "defines.h"
#include "math/syntics_math.c"

#include "vulkan_types.h"
#include <vulkan/vulkan_win32.h>

#include "ansi_keycodes.h"
#include "event_system.h"
#include "entity.h"
#include "region_alloc.h"
#include "lookup_table.h"
#include "font.h"
#include "render_util.h"
#include "simple_particle.h"
#include "camera.h"
#include "logging.h"

// Vendor
#include "stb/stb_image.h"

typedef struct File_Attrib
{
    unsigned char* buffer;
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

#include "noise.c"
#include "random.c"
#include "logging.c"
#include "region_alloc.c"
#include "file_reading.c"
#include "win32/win32_platform.c"
#include "instance_device.c"
#include "buffers.c"
#include "event_system.c"
#include "lookup_table.c"
#include "entity.c"
#include "collision.c"
#include "font.c"
#include "swap_chain.c"
#include "render_util.c"
#include "camera.c"
#include "gui.c"
#include "render.c"
#include "vulkan_api.c"
#include "syntic_app.c"
#include "test_bed.c"
#include "main.c"


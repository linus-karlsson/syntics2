#pragma once

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <tchar.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "ansi_keycodes.h"
#include "defines.h"
#include "vulkan_types.h"
#include "stb/stb_image.h"
#include "event_system.h"
#include "entity.h"
#include "lookup_table.h"
#include "font.h"
#include "render_util.h"
#include "simple_particle.h"

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

#include "noise.cc"
#include "random.cc"
#include "logging.cc"
#include "region_alloc.cc"
#include "file_reading.cc"
#include "win32/win32_platform.cc"
#include "math/syntics_math.cc"
#include "instance_device.cc"
#include "buffers.cc"
#include "event_system.cc"
#include "lookup_table.cc"
#include "entity.cc"
#include "collision.cc"
#include "font.cc"
#include "swap_chain.cc"
#include "render_util.cc"
#include "simple_particle.cc"

/*
#include "vulkan_api.h"
#include "vulkan_types.h"
#include "event_system.h"
#ifdef LINUX
#include "linux/linux_platform.h"
#else
#include "win32/win32_platform.h"
#endif
#include "region_alloc.h"
#include "render.h"
#include "gui.h"
#include "random.h"
*/

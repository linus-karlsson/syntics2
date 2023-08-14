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

#include "syntics_app.h"

typedef struct Render_Task
{
    void (*draw_callback)(void* data, VkCommandBuffer command_buffer,
                          u32 semaphore_idx);
    void* data;
} Render_Task;

typedef struct Frame_Data
{
    Semaphore render_counter;

    Region_Alloc frame_region;

    Render_Task* render_tasks;

    u32 id;

    V2 dimensions;
    u32 semaphore_idx;
    f32 dt;

    VP game_cam_vp;
    Push_Constant* game_sign_constants;
    M4** game_dude_models;

    u32 game_aabb_count;
    u32 game_aabb_indices_count;

    Graphic_Pipeline* game_triangle_strip_pipeline;
    Graphic_Pipeline* game_triangle_list_pipeline;
    Graphic_Pipeline* game_line_list_pipeline;
    Graphic_Pipeline* game_grass_pipeline;

    Vertex_Index_Buffer game_vert_idx_buffer;

    Vertex_Index_Buffer game_road_vert_idx;
    Vertex_Index_Buffer game_road_line_vert_idx;
    Vertex_Index_Buffer game_particles_vert_idx;
    Vertex_Index_Buffer game_aabb_rep;

    Index_Offset_Render game_terrain_offsets;
    Index_Offset_Render game_dude_offsets;
    Index_Offset_Render game_tree_offsets;
    Index_Offset_Render game_sign_offsets;
    Index_Offset_Render game_grass_offsets;

    VP gui_cam_vp;

    Ui_Window_Render* gui_windows;
    Terminal_Render gui_terminal;
    u32 gui_blue_rects_index_offset;
    u32 gui_docking_display_quad_count;

    Graphic_Pipeline* gui_triangle_list_pipeline;
    Graphic_Pipeline* gui_line_strip_pipeline;

    Vertex_Index_Buffer gui_main_vert_idx;
    Vertex_Index_Buffer gui_terminal_vert_idx;
}Frame_Data;

typedef struct Game_Logic {

    Application_State* app_state;
    Gui_Context* gui_ctx;
    Game_State* game_state;
    Frame_Data* frame;
}Game_Logic;

typedef struct Render_Logic {

    Application_State* app_state;
    Render_State* render_state;
    Frame_Data* frame;
}Render_Logic;

void frame_data_create()
{

}

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
//#include "test_bed.c"
#include "vulkan_api.c"
#include "syntic_app.c"
#include "main.c"

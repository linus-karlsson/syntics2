#pragma once
#ifndef SY_UNIT_BUILD
#include "vulkan_internal_api.h"
#endif

typedef struct Application_State
{
    VkPhysicalDevice phy_device;
    VkDevice device;
    Queue_Family_Indices q_indices;
    VkSurfaceKHR surface;
    VkCommandPool com_pool;
    Swap_Chain_Attrib swap_chain;
    
    Image depth_img;
    Image color_img;

    Platform* platform;

    Region_Alloc region;

    u32 num_semaphores;
    u32 fps;

    b8 running;
} Application_State;

void application_init(u32 stack_size, u64 main_region_size, u16 app_width,
                      u16 app_height, u32 thread_pool_queue_size,
                      b8 full_screen, u32 event_count,
                      u32 vulkan_frames_in_flight, Render_State** render_state,
                      Application_State** app);

typedef struct Application_Frame
{
    f64 delta_time;
    f64 delta_time_per_frame;
    f64 sec_for_delta_update;
    f64 sec_for_delta_update_duration;

    u32 frame_count;
    u32 frames_to_count;
    u32 fps;
} Application_Frame;

Application_Frame application_frame_create();

Application_Frame application_begin_frame(Application_Frame app_frame);



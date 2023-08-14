#pragma once

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

    u32 num_semaphores;
    u32 fps;

    b8 running;
} Application_State;


#pragma once

#include "vulkan_internal_api.h"

#ifdef DEBUG
static const b8 VALIDATIONS_ENABLE = true;
#else
static const b8 VALIDATIONS_ENABLE = false;
#endif

void init_instance(Region_Alloc* region);

const VkInstance get_instance();
const VkDebugUtilsMessengerEXT get_debug_messenger();

VKAPI_ATTR VkBool32 VKAPI_CALL msg_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data, void* p_user_data);

VkDebugUtilsMessengerCreateInfoEXT config_debug_info();

void init_debug_messenger();

VkResult create_debug_utils_messenger_EXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* p_create_info,
    const VkAllocationCallbacks* p_allocator,
    VkDebugUtilsMessengerEXT* p_debug_messenger);

void destroy_debug_messenger(VkInstance instance,
                             VkDebugUtilsMessengerEXT debug_messenger,
                             const VkAllocationCallbacks* p_allocator);

Queue_Family_Indices get_queue_indices(Region_Alloc* region,
                                       VkPhysicalDevice physical_device,
                                       VkSurfaceKHR surface, b8* all_supported);

void pick_physical_device(Region_Alloc* region, VkInstance instance,
                          VkSurfaceKHR surface, VkPhysicalDevice* physical_device,
                          Queue_Family_Indices* q_indices);

void create_logical_device(VkPhysicalDevice physical_device,
                           Queue_Family_Indices q_indices, VkDevice* device);

#ifdef LINUX
#include "linux/linux_platform.h"
void create_surface(Linux_Platform xcb, VkSurfaceKHR* surface);
#else

// #include "win32/win32_platform.h"
typedef struct HWND__* HWND;

void create_surface(HWND win, VkSurfaceKHR* surface);

#endif

void destroy_instance();


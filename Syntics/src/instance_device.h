#pragma once

#include "vulkan_types.h"
#include "linux_platform.h"

namespace synt {

#ifdef DEBUG
static const bool VALIDATIONS_ENABLE = 1;
#else
static const bool VALIDATIONS_ENABLE = 0;
#endif

#define SIZE(array) (uint32)(sizeof(array) / sizeof(array[0]))

typedef struct Region_Alloc Region_Alloc;

void init_instance(Region_Alloc* region);

const VkInstance& get_instance();
const VkDebugUtilsMessengerEXT& get_debug_messenger();

VKAPI_ATTR VkBool32 VKAPI_CALL
msg_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
             VkDebugUtilsMessageTypeFlagsEXT message_type,
             const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
             void* p_user_data);

VkDebugUtilsMessengerCreateInfoEXT config_debug_info();

void init_debug_messenger();

VkResult create_debug_utils_messenger_EXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* p_create_info,
    const VkAllocationCallbacks* p_allocator,
    VkDebugUtilsMessengerEXT* p_debug_messenger);

void destroy_debug_messenger(VkInstance instance,
                             VkDebugUtilsMessengerEXT debug_messenger,
                             const VkAllocationCallbacks* p_allocator);

Queue_Family_Indices get_queue_indices(Region_Alloc* region,
                                       VkPhysicalDevice physical_device,
                                       VkSurfaceKHR surface,
                                       bool* all_supported);

void pick_physical_device(Region_Alloc* region, VkInstance instance,
                          VkSurfaceKHR surface,
                          VkPhysicalDevice* physical_device,
                          Queue_Family_Indices* q_indices);

void create_logical_device(VkPhysicalDevice physical_device,
                           Queue_Family_Indices q_indices, VkDevice* device);

void create_surface(Linux_Platform xcb, VkSurfaceKHR* surface);

void destroy_instance();

} // namespace synt

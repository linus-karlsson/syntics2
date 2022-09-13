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

void create_instance(Region_Alloc* region, VkInstance* instance);

Queue_Family_Indices get_queue_indices(Region_Alloc* region,
                                       VkPhysicalDevice physical_device,
                                       VkSurfaceKHR surface, bool* all_supported);

void pick_physical_device(Region_Alloc* region, VkInstance instance,
                          VkSurfaceKHR surface, VkPhysicalDevice* physical_device,
                          Queue_Family_Indices* q_indices);

void create_logical_device(VkPhysicalDevice physical_device,
                           Queue_Family_Indices q_indices, VkDevice* device);

void get_surface(VkInstance instance, Linux_Platform xcb, VkSurfaceKHR* surface);

} // namespace synt

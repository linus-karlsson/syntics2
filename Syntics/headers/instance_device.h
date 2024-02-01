#pragma once
#ifndef SY_UNIT_BUILD
#include "vulkan_internal_api.h"
#endif

b8 validation_enable();
void debug_messenger_init(Instance_State* state);
void debug_messenger_destroy(VkInstance instance,
                             VkDebugUtilsMessengerEXT debugMessenger,
                             const VkAllocationCallbacks* pAllocator);
void instance_init(VkInstance* instance);
Queue_Family_Indices queue_indices_get(VkPhysicalDevice physical_device,
                                       VkSurfaceKHR surface, b8* all_supported);
void physical_device_pick(Region_Alloc* region, VkInstance instance,
                          VkSurfaceKHR surface,
                          VkPhysicalDevice* physical_device,
                          Queue_Family_Indices* q_indices);
void logical_device_create(VkPhysicalDevice physical_device,
                           Queue_Family_Indices q_indices, VkDevice* device);
void surface_create(Platform* platform, VkInstance instance,
                    VkSurfaceKHR* surface);
void instance_destroy(Instance_State* state);

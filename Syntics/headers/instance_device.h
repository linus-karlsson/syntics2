#pragma once
#ifndef SY_UNIT_BUILD
#include "vulkan_internal_api.h"
#endif

b8   vulkan_enable_validation();
void vulkan_debug_messenger_init(Instance_State* state);
void vulkan_debug_messenger_destroy(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

void vulkan_instance_init(VkInstance* instance);
void vulkan_instance_destroy(Instance_State* state);

Queue_Family_Indices
     vulkan_queue_indices_get(VkPhysicalDevice physical_device, VkSurfaceKHR surface, b8* all_supported);
void vulkan_pick_physical_device(Region_Alloc* region, VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice* physical_device, Queue_Family_Indices* q_indices);
void vulkan_logical_device_create(VkPhysicalDevice physical_device, Queue_Family_Indices q_indices, VkDevice* device);
void vulkan_surface_create(Platform* platform, VkInstance instance, VkSurfaceKHR* surface);

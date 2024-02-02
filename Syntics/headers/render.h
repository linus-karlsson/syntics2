#pragma once
#ifndef SY_UNIT_BUILD
#include "vulkan_internal_api.h"
#endif

typedef void Render_State;
typedef void (*Update_Callback)(void* data, Region_Alloc* region, const Application_State* app_state, Render_State* render_state, V2 dimensions, u32 semaphore_idx, f32 dt);
typedef void (*Recreate_Callback)(void* data, const Application_State* app_state);
typedef void (*Recreate_Graphic_Pipeline_callback)(void* data, const Application_State* app_state);
typedef void (*Destroy_Callback)(void* data, VkDevice device, u32 num_semaphores);

u32   vulkan_get_semaphore_idx(Render_State* render_state);
void  vulkan_fence_and_semaphore_create(VkDevice device, VkFence* fence, VkSemaphore* image_semaphores, VkSemaphore* present_semaphores);

void  vulkan_render_state_init(Region_Alloc* region, VkDevice device, Queues queues, VkPhysicalDevice physical_device, VkCommandPool command_pool, const Queue_Family_Indices* q_indices, u32 num_semaphores, const Swap_Chain_Attrib* swap_chain, const Platform* platform, Render_State** render_state);
void  vulkan_render_state_destroy(VkDevice device, Render_State* render_state);

VkQueue 
      vulkan_graphic_queue_get(Render_State* render_state);

void  vulkan_subscribe_to_update_callback(Render_State* render_state, Update_Callback callback, void* data);
void  vulkan_subscribe_to_recreate_callback(Render_State* render_state, Recreate_Callback callback, void* data);
void  vulkan_subscribe_to_recreate_gp_callback(Render_State* render_state, Recreate_Graphic_Pipeline_callback callback, void* data);
void  vulkan_subscribe_to_destroy_callback(Render_State* render_state, Destroy_Callback callback, void* data);

void  vulkan_submit_and_present(VkQueue graphic_queue, VkQueue present_queue, VkSemaphore image_semaphore, VkSemaphore present_semaphore, VkFence fence, VkCommandBuffer* command_buffers, u32 command_buffer_count, VkSwapchainKHR swap_chain, u32 image_index);
void  vulkan_frame_begin(Render_State* render_state, Application_State* app_state);
void  vulkan_frame_render(Render_State* render_state, Application_State* app_state, Render_Task* copy_tasks, Render_Task* render_tasks, f32 dt);


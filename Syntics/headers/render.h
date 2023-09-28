#pragma once
#ifndef SY_UNIT_BUILD
#include "vulkan_internal_api.h"
#endif

typedef void Render_State;

u32 semaphore_idx_get(Render_State* render_state);
void fence_semaphore_create(VkDevice device, VkFence* fence,
                            VkSemaphore* image_semaphores,
                            VkSemaphore* present_semaphores);

void render_state_init(Region_Alloc* region, VkDevice device, Queues queues,
                       VkPhysicalDevice physical_device,
                       VkCommandPool command_pool,
                       const Queue_Family_Indices* q_indices,
                       u32 num_semaphores, const Swap_Chain_Attrib* swap_chain,
                       const Platform* platform, Render_State** render_state);

VkQueue graphic_queue_get(Render_State* render_state);

void subscribe_update_callback(
    Render_State* render_state,
    void (*update_callback_p)(void* data, Region_Alloc* region,
                              const Application_State* app_state,
                              Render_State* render_state, V2 dimensions,
                              u32 semaphore_idx, f32 dt),
    void* data);

void subscribe_recreate_callback(
    Render_State* render_state,
    void (*rc_callback)(void* data, const Application_State* app_state),
    void* data);

void subscribe_recreate_gp_callback(
    Render_State* render_state,
    void (*rc_gp_callback)(void* data, const Application_State* app_state),
    void* data);

void subscribe_destroy_callback(Render_State* render_state,
                                void (*destroy_callback)(void* data,
                                                         VkDevice device,
                                                         u32 num_semaphores),
                                void* data);

void submit_and_present(VkQueue graphic_queue, VkQueue present_queue,
                        VkSemaphore image_semaphore,
                        VkSemaphore present_semaphore, VkFence fence,
                        VkCommandBuffer* command_buffers,
                        u32 command_buffer_count, VkSwapchainKHR swap_chain,
                        u32 image_index);

void frame_begin(Render_State* render_state, Application_State* app_state);

void frame_render(Render_State* render_state, Application_State* app_state,
                  Render_Task* copy_tasks, Render_Task* render_tasks, f32 dt);

void render_state_destroy(VkDevice device, Render_State* render_state);

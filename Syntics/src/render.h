#pragma once

#include "vulkan_types.h"

typedef struct Region_Alloc Region_Alloc;

void init_render_state(Region_Alloc* region, VkDevice device, Queues queues,
                       VkPhysicalDevice physical_device, VkCommandPool command_pool,
                       const Queue_Family_Indices& q_indices, uint32 num_semaphores,
                       const Swap_Chain_attrib& swap_chain);

void create_fence_semaphore(VkDevice device, VkFence* fence,
                            VkSemaphore* image_semaphores,
                            VkSemaphore* present_semaphores);

void render(Region_Alloc* region, Application_State& app_state, float dt);

void submit_and_present(VkQueue graphic_queue, VkQueue present_queue,
                        VkSemaphore image_semaphore, VkSemaphore present_semaphore,
                        VkFence fence, VkCommandBuffer command_buffer,
                        VkSwapchainKHR swap_chain, uint32 image_index);

void destroy_render_state();


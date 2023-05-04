#pragma once

#include "vulkan_internal_api.h"

void init_game(Region_Alloc* region, VkDevice device,
               VkPhysicalDevice physical_device, VkCommandPool command_pool,
               VkQueue graphic_queue, const Swap_Chain_attrib* swap_chain,
               u32 num_semaphores);

// void render_testing_recreate(Region_Alloc* region,
//                              const Application_State& app_state);

void update_game(Region_Alloc* region, const Application_State* app_state,
                 VkDevice device, V2 dimensions, u32 semaphore_idx, f32 dt);

// void render_render_testing(VkCommandBuffer command_buffer, u32 semaphore_idx);

// void render_testing_destroy(VkDevice device, u32 num_semaphores);

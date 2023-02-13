#pragma once

#include "defines.h"
#include "vulkan_types.h"

typedef struct Region_Alloc Region_Alloc;

void init_platform_game(Region_Alloc* region, VkDevice device,
                        VkPhysicalDevice physical_device, VkCommandPool command_pool,
                        VkQueue graphic_queue, const Swap_Chain_attrib& swap_chain,
                        u32 num_semaphores);

void recreate_platform_game(Region_Alloc* region,
                            const Application_State& app_state);

void update_platform_game(Region_Alloc* region, VkDevice device,
                          const Vec2& dimensions, u32 semaphore_idx, f32 dt);

void render_platform_game(VkCommandBuffer command_buffer, u32 semaphore_idx);

void destroy_platform_game(VkDevice device, u32 num_semaphores);


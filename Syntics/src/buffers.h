#pragma once
#include "vulkan_types.h"

namespace synt {

void create_command_pool(VkDevice device, uint32 queue_fam_index,
                         VkCommandPool* command_pool);

void allocate_commandbuffer(VkDevice device, VkCommandPool command_pool,
                            VkCommandBuffer* command_buffer);

void record_commandbuffer(VkCommandBuffer command_buffer);

void create_fence_semaphore(VkDevice device, VkFence* fence, VkSemaphore* semaphore);
} // namespace synt

#pragma once
#include "vulkan_types.h"

namespace synt {

void create_vertex_buffer(VkDevice device,
                          const Queue_Family_Indices& queue_fam_index,
                          VkBuffer* vertex_buffer);

void create_index_buffer(VkDevice device,
                         const Queue_Family_Indices& queue_fam_index,
                         VkBuffer* index_buffer);

void create_command_pool(VkDevice device, uint32 queue_fam_index,
                         VkCommandPool* command_pool);

void allocate_commandbuffer(VkDevice device, VkCommandPool command_pool,
                            VkCommandBuffer* command_buffer);

void record_commandbuffer(VkCommandBuffer command_buffer);

void create_fence_semaphore(VkDevice device, VkFence* fence, VkSemaphore* semaphore);
} // namespace synt

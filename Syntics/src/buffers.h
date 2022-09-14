#pragma once
#include "vulkan_types.h"

namespace synt {

void create_vertex_buffer(VkDevice device, VkPhysicalDevice physical_device,
                          Vertex* vertices, uint32 num_vertices,
                          VkDeviceMemory* buffer_memory, VkBuffer* vertex_buffer);

void create_index_buffer(VkDevice device, VkPhysicalDevice physical_device,
                         uint32* indices, uint32 num_indices,
                         VkDeviceMemory* buffer_memory, VkBuffer* index_buffer);

void create_command_pool(VkDevice device, uint32 queue_fam_index,
                         VkCommandPool* command_pool);

void allocate_commandbuffer(VkDevice device, VkCommandPool command_pool,
                            VkCommandBuffer* command_buffer);

void record_commandbuffer(VkCommandBuffer command_buffer);

void create_fence_semaphore(VkDevice device, VkFence* fence, VkSemaphore* semaphore);
} // namespace synt

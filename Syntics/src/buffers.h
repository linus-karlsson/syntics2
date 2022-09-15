#pragma once
#include "vulkan_types.h"

namespace synt {

void create_vertex_buffer(VkDevice device, VkPhysicalDevice physical_device,
                          Vertex_Buffer* vertex_buffer);

void create_index_buffer(VkDevice device, VkPhysicalDevice physical_device,
                         Index_Buffer* index_buffer);

void create_command_pool(VkDevice device, uint32 queue_fam_index,
                         VkCommandPool* command_pool);

void allocate_commandbuffer(VkDevice device, VkCommandPool command_pool,
                            VkCommandBuffer* command_buffer);

void record_execute_commandbuffer(VkCommandBuffer command_buffer,
                                  VkFramebuffer framebuffer, VkExtent2D extent_2D,
                                  VkBuffer vertex_buffer, VkBuffer index_buffer,
                                  uint32 index_count,
                                  const Graphic_Pipline& graphic_pipline);

void create_fence_semaphore(VkDevice device, VkFence* fence,
                            VkSemaphore* image_semaphores,
                            VkSemaphore* present_semaphores);

void destroy_buffer(VkDevice device, VkBuffer buffer, VkDeviceMemory buffer_memory);

} // namespace synt

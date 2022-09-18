#pragma once
#include "vulkan_types.h"

namespace synt {

void create_vertex_buffer(VkDevice device, VkPhysicalDevice physical_device,
                          Vertex_Buffer* vertex_buffer);

void create_index_buffer(VkDevice device, VkPhysicalDevice physical_device,
                         Index_Buffer* index_buffer);

void create_uniform_buffer(VkDevice device, VkPhysicalDevice physical_device,
                           Uniform_Buffer* uniform_buffer);

void create_command_pool(VkDevice device, uint32 queue_fam_index,
                         VkCommandPool* command_pool);

void allocate_commandbuffer(VkDevice device, VkCommandPool command_pool,
                            VkCommandBuffer* command_buffer);

void create_descriptors(VkDevice device, Descriptors* desciptors, uint32 desc_count,
                        VkDescriptorSetLayout desc_layout,
                        Uniform_Buffer* uniform_buffers);

void create_image(uint32_t width, uint32_t height, VkDevice device,
                  VkPhysicalDevice physical_device, VkFormat format, VkImageTiling tiling,
                  VkImageUsageFlags usage, VkMemoryPropertyFlags wanted_mem_props,
                  VkImage* image, VkDeviceMemory* image_mem, uint32_t mip_map_lvl,
                  VkSampleCountFlagBits num_samples);

void create_texture(VkDevice device, VkPhysicalDevice physical_device,
                    const char* tex_path, Texture* texture);

void record_execute_commandbuffer(VkCommandBuffer command_buffer,
                                  VkFramebuffer framebuffer, VkExtent2D extent_2D,
                                  VkBuffer vertex_buffer, VkBuffer index_buffer,
                                  uint32 index_count, VkDescriptorSet desc_set,
                                  const Graphic_Pipline& graphic_pipline);

void create_fence_semaphore(VkDevice device, VkFence* fence,
                            VkSemaphore* image_semaphores,
                            VkSemaphore* present_semaphores);

void destroy_buffer(VkDevice device, VkBuffer buffer, VkDeviceMemory buffer_memory);

} // namespace synt

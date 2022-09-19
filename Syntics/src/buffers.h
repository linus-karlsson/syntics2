#pragma once
#include "vulkan_types.h"

namespace synt {

VkCommandBuffer begin_command_buffer(VkDevice device, VkCommandPool command_pool);

void end_command_buffer(VkDevice device, VkCommandPool command_pool,
                        VkCommandBuffer command_buff, VkQueue graphics_queue);

void copy_buffer(VkDevice device, VkCommandPool command_pool, VkBuffer src_buffer,
                 VkBuffer dst_buffer, VkQueue graphics_queue, VkDeviceSize size_bytes);

void create_vertex_buffer(VkDevice device, VkPhysicalDevice physical_device,
                          VkCommandPool command_pool, VkQueue graphics_queue,
                          Vertex_Buffer* vertex_buffer);

void create_index_buffer(VkDevice device, VkPhysicalDevice physical_device,
                         VkCommandPool command_pool, VkQueue graphics_queue,
                         Index_Buffer* index_buffer);

void create_uniform_buffer(VkDevice device, VkPhysicalDevice physical_device,
                           Uniform_Buffer* uniform_buffer);

void create_command_pool(VkDevice device, uint32 queue_fam_index,
                         VkCommandPool* command_pool);

void allocate_commandbuffer(VkDevice device, VkCommandPool command_pool,
                            VkCommandBuffer* command_buffer);

void update_descritors(VkDevice device, Descriptors* desciptors, uint32 desc_count,
                       const Texture& texture, Uniform_Buffer* uniform_buffers);

void create_descriptors(VkDevice device, Descriptors* desciptors, uint32 desc_count,
                        VkDescriptorSetLayout desc_layout, const Texture& texture,
                        Uniform_Buffer* uniform_buffers);

void create_image(uint32_t width, uint32_t height, VkDevice device,
                  VkPhysicalDevice physical_device, VkFormat format, VkImageTiling tiling,
                  VkImageUsageFlags usage, VkMemoryPropertyFlags wanted_mem_props,
                  VkImage* image, VkDeviceMemory* image_mem, uint32_t mip_map_lvl,
                  VkSampleCountFlagBits num_samples);

void create_sampler(VkDevice device, Texture* textue);

void copy_buffer_image(VkDevice device, VkCommandPool command_pool, uint32 width,
                       uint32 height, VkBuffer src_buffer, VkImage dst_image,
                       VkQueue graphics_queue, VkDeviceSize size_bytes);

void set_texture_data(VkDevice device, VkPhysicalDevice physical_device, void* data,
                      VkCommandPool command_pool, VkQueue graphics_queue,
                      Texture* texture, VkDeviceSize size_bytes);

void create_texture(VkDevice device, VkPhysicalDevice physical_device,
                    VkCommandPool command_pool, VkQueue graphics_queue,
                    const char* tex_path, Texture* texture);

void create_texture(VkDevice device, VkPhysicalDevice physical_device, uint32 width,
                    uint32 height, VkCommandPool command_pool, VkQueue graphics_queue,
                    Texture* texture);

void ray_casting_ex(VkDevice device, VkPhysicalDevice physical_device, const Vec3& ray_o,
                    const Vec3& ray_dir, VkCommandPool command_pool,
                    VkQueue graphics_queue, Texture* texture);

void create_depth_image(VkDevice device, VkPhysicalDevice physical_device,
                        VkExtent2D extent_2D, Image* depth_image);

uint32_t rand_rgb(uint32_t upper, uint32_t under);

void record_execute_commandbuffer(VkCommandBuffer command_buffer,
                                  VkFramebuffer framebuffer, VkExtent2D extent_2D,
                                  VkBuffer vertex_buffer, VkBuffer index_buffer,
                                  uint32 index_count, VkDescriptorSet desc_set,
                                  const Graphic_Pipline& graphic_pipline);

void create_fence_semaphore(VkDevice device, VkFence* fence,
                            VkSemaphore* image_semaphores,
                            VkSemaphore* present_semaphores);

void destroy_buffer(VkDevice device, VkBuffer buffer, VkDeviceMemory buffer_memory);

void destroy_texture(VkDevice device, Texture& texture);

void destroy_image(VkDevice device, Image& image);

} // namespace synt

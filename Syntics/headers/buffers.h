#pragma once
#ifndef SY_UNIT_BUILD
#include "vulkan_internal_api.h"
#endif

#define sy_RGB(v) ((v) / 255.0f)

void buffer_destroy(VkDevice device, Buffer buffer);
void texture_destroy(VkDevice device, Texture texture);
void image_destroy(VkDevice device, Image image);

void buffers_update(VkDevice device, Buffer* buffer, void* data,
                    size_t size_bytes);

void commandbuffers_allocate(VkDevice device,
                                      VkCommandPool command_pool,
                                      VkCommandBufferLevel level,
                                      u32 command_buffer_count,
                                      VkCommandBuffer* command_buffer);

VkCommandBuffer command_buffer_begin(VkDevice device,
                                     VkCommandPool command_pool,
                                     VkCommandBufferLevel level);
void command_buffer_end(VkDevice device, VkCommandPool command_pool,
                        VkCommandBuffer command_buff, VkQueue graphics_queue);

void buffer_copy(VkDevice device, VkCommandPool command_pool,
                 VkBuffer src_buffer, VkBuffer dst_buffer,
                 VkQueue graphics_queue, VkDeviceSize size_bytes);

void create_alloc_bind(VkDevice device, VkPhysicalDevice physical_device,
                       VkMemoryPropertyFlags wanted_mem_props,
                       VkBufferUsageFlags usage_flags, VkBuffer* buffer,
                       VkDeviceMemory* buffer_memory, VkDeviceSize data_size);

void staging_buffer_create(VkDevice device, VkPhysicalDevice physical_device,
                           void* data, VkDeviceSize size_bytes,
                           VkBufferUsageFlags usage_flags, Buffer* buffer);

void staging_buffer_to_local(VkDevice device, VkPhysicalDevice physical_device,
                             VkCommandPool command_pool, VkQueue graphics_queue,
                             VkBufferUsageFlags vertex_or_index, void* data,
                             VkBuffer* buffer, VkDeviceMemory* buffer_memory,
                             VkDeviceSize size_bytes);

void vertex_buffer_create_visible(VkDevice device,
                                  VkPhysicalDevice physical_device,
                                  Vertex_Buffer* vertex_buffer);
void vertex_buffer_create_local(VkDevice device,
                                VkPhysicalDevice physical_device,
                                VkCommandPool command_pool,
                                VkQueue graphics_queue,
                                Vertex_Buffer* vertex_buffer);

void index_buffer_create_visible(VkDevice device,
                                 VkPhysicalDevice physical_device,
                                 Index_Buffer* index_buffer);
void index_buffer_create_local(VkDevice device,
                               VkPhysicalDevice physical_device,
                               VkCommandPool command_pool,
                               VkQueue graphics_queue,
                               Index_Buffer* index_buffer);

void vertex_index_buffer_create_default(VkDevice device,
                                        VkPhysicalDevice physical_device,
                                        VkCommandPool command_pool,
                                        VkQueue graphics_queue,
                                        Visible_Local visible_local,
                                        Vertex_Buffer* vertex_buffer,
                                        Index_Buffer* index_buffer);

void vertex_index_buffer_create_default1(
    VkDevice device, VkPhysicalDevice physical_device,
    VkCommandPool command_pool, VkQueue graphics_queue,
    Visible_Local visible_local, Vertex_Index_Buffer* vertex_index_buffer);

void uniform_buffer_create(VkDevice device, VkPhysicalDevice physical_device,
                           Buffer* uniform_buffer);

void command_pool_create(VkDevice device, u32 queue_fam_index,
                         VkCommandPool* command_pool);

void update_descritors(Region_Alloc* region, VkDevice device,
                       Descriptors* desciptors, u32 desc_count,
                       const Texture* textures, u32 num_textures,
                       Buffer* uniform_buffers);

void descriptors_create(Region_Alloc* region, VkDevice device,
                        Descriptors* desciptors, u32 desc_count,
                        VkDescriptorSetLayout desc_layout,
                        const Texture* texture, u32 num_textures,
                        Buffer* uniform_buffers);

void image_create(u32 width, u32 height, VkDevice device,
                  VkPhysicalDevice physical_device, VkFormat format,
                  VkImageTiling tiling, VkImageUsageFlags usage,
                  VkMemoryPropertyFlags wanted_mem_props,
                  VkSampleCountFlagBits num_samples, u32 mip_map_lvl,
                  VkImage* image, VkDeviceMemory* image_mem);

void image_view_create(VkDevice device, VkImage image,
                       VkImageViewType image_view_type, VkFormat image_format,
                       VkImageAspectFlags aspect_mask, u32 mip_map_lvl,
                       VkImageView* image_view);

void sampler_create(VkDevice device, Texture* textue);

void buffer_image_copy(VkDevice device, VkCommandPool command_pool, u32 width,
                       u32 height, u32 mip_map_lvl, VkBuffer src_buffer,
                       VkImage dst_image, VkQueue graphics_queue,
                       VkDeviceSize size_bytes);

void mipmap_enable(VkDevice device, VkCommandPool command_pool,
                   VkQueue graphics_queue, VkImage image,
                   const Texture* texture);

void frame_buffer_create(VkDevice device, VkRenderPass render_pass,
                         VkExtent2D extent_2D, VkImageView img_view,
                         VkImageView depth_view, VkImageView color_view,
                         VkFramebuffer* framebuffer);

void texture_data_set(VkDevice device, VkPhysicalDevice physical_device,
                      void* data, VkCommandPool command_pool,
                      VkQueue graphics_queue, Texture* texture,
                      VkDeviceSize size_bytes);

void image_change_layout(VkDevice device, VkCommandPool command_pool,
                         VkQueue graphic_queue, VkImage image, VkFormat format,
                         VkImageLayout old_layout, VkImageLayout new_layout);

void texture_path_create(VkDevice device, VkPhysicalDevice physical_device,
                         VkCommandPool command_pool, VkQueue graphics_queue,
                         b8 mip_map, VkFormat image_format,
                         const char* tex_path, Texture* texture);

u32 textures_path_create(VkDevice device, VkPhysicalDevice physical_device,
                         VkCommandPool command_pool, VkQueue graphics_queue,
                         b8 mip_map, u32 num_textures, const char** tex_paths,
                         Texture* textures);

void texture_buffer_create(VkDevice device, VkPhysicalDevice physical_device,
                           VkCommandPool command_pool, VkQueue graphics_queue,
                           VkFormat image_format, unsigned char* tex_buffer,
                           Texture* texture);

void texture_create(VkDevice device, VkPhysicalDevice physical_device,
                    u32 width, u32 height, VkCommandPool command_pool,
                    VkQueue graphics_queue, Texture* texture);

void depth_image_create(VkDevice device, VkPhysicalDevice physical_device,
                        const VkExtent2D* extent_2D,
                        VkSampleCountFlagBits sample_count, Image* depth_image);

void render_pass_begin(VkCommandBuffer command_buffer, VkRenderPass render_pass,
                       VkFramebuffer framebuffer, const VkExtent2D* extent_2D);

void render_pass_end(VkCommandBuffer command_buffer);

void push_constant(VkCommandBuffer command_buffer, VkPipelineLayout layout,
                   void* data, u32 size);

void draw(VkCommandBuffer command_buffer, u32 offset, u32 count);

void vertex_index_buffer_bind(VkCommandBuffer command_buffer,
                              const Vertex_Buffer* vert_buffer,
                              const Index_Buffer* index_buffer);

void vertex_index_buffer1_bind(VkCommandBuffer command_buffer,
                               const Vertex_Index_Buffer* buffer);

void data_buffer_copy(Buffer* buffer, void* data, size_t size_bytes);

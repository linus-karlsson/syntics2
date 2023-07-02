#pragma once
#include "vulkan_internal_api.h"

typedef enum Visible_Local
{
    VERTEX_INDEX_VISIBLE_VISIBLE,
    VERTEX_INDEX_VISIBLE_LOCAL,
    VERTEX_INDEX_LOCAL_VISIBLE,
    VERTEX_INDEX_LOCAL_LOCAL,
} Visible_Local;

VkCommandBuffer begin_command_buffer(VkDevice device, VkCommandPool command_pool,
                                     VkCommandBufferLevel level);

void end_command_buffer(VkDevice device, VkCommandPool command_pool,
                        VkCommandBuffer command_buff, VkQueue graphics_queue);

void copy_buffer(VkDevice device, VkCommandPool command_pool, VkBuffer src_buffer,
                 VkBuffer dst_buffer, VkQueue graphics_queue,
                 VkDeviceSize size_bytes);

void map_copy_mem(VkDevice device, Buffer* buffer, void* data);

void map_copy_unmap_mem(VkDevice device, Buffer* buffer, void* data);

void create_vertex_index_buffer_default(
    VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool,
    VkQueue graphics_queue, Visible_Local visible_local,
    Vertex_Buffer* vertex_buffer, Index_Buffer* index_buffer);

void create_vertex_index_buffer_default(VkDevice device,
                                        VkPhysicalDevice physical_device,
                                        VkCommandPool command_pool,
                                        VkQueue graphics_queue,
                                        Visible_Local visible_local,
                                        Vertex_Index_Buffer* vertex_index_buffer);

void create_vertex_buffer_visible(VkDevice device, VkPhysicalDevice physical_device,
                                  Vertex_Buffer* vertex_buffer);

void create_vertex_buffer_test(VkDevice device, VkPhysicalDevice physical_device,
                               Vertex_Buffer* vertex_buffer);

void create_vertex_buffer_local(VkDevice device, VkPhysicalDevice physical_device,
                                VkCommandPool command_pool, VkQueue graphics_queue,
                                Vertex_Buffer* vertex_buffer);

void create_index_buffer_test(VkDevice device, VkPhysicalDevice physical_device,
                              Index_Buffer* index_buffer);

void create_index_buffer_visible(VkDevice device, VkPhysicalDevice physical_device,
                                 Index_Buffer* index_buffer);

void create_index_buffer_local(VkDevice device, VkPhysicalDevice physical_device,
                               VkCommandPool command_pool, VkQueue graphics_queue,
                               Index_Buffer* index_buffer);

void create_uniform_buffer(VkDevice device, VkPhysicalDevice physical_device,
                           Uniform_Buffer* uniform_buffer);

void create_uniform_buffer_test(VkDevice device, VkPhysicalDevice physical_device,
                                Uniform_Buffer* uniform_buffer, void** data);

void create_command_pool(VkDevice device, u32 queue_fam_index,
                         VkCommandPool* command_pool);

void allocate_commandbuffers(VkDevice device, VkCommandPool command_pool,
                             VkCommandBufferLevel level, u32 command_buffer_count,
                             VkCommandBuffer* command_buffer);

void update_descritors(Region_Alloc* region, VkDevice device,
                       Descriptors* desciptors, u32 desc_count,
                       const Texture* textures, u32 num_textures,
                       Uniform_Buffer* uniform_buffers);

void create_descriptors(Region_Alloc* region, VkDevice device,
                        Descriptors* desciptors, u32 desc_count,
                        VkDescriptorSetLayout desc_layout, const Texture* texture,
                        u32 num_textures, Uniform_Buffer* uniform_buffers);

void create_image(u32 width, u32 height, VkDevice device,
                  VkPhysicalDevice physical_device, VkFormat format,
                  VkImageTiling tiling, VkImageUsageFlags usage,
                  VkMemoryPropertyFlags wanted_mem_props,
                  VkSampleCountFlagBits num_samples, u32 mip_map_lvl, VkImage* image,
                  VkDeviceMemory* image_mem);

void create_sampler(VkDevice device, Texture* textue);

void copy_buffer_image(VkDevice device, VkCommandPool command_pool, u32 width,
                       u32 height, u32 mip_map_lvl, VkBuffer src_buffer,
                       VkImage dst_image, VkQueue graphics_queue,
                       VkDeviceSize size_bytes);

void enable_bitmap(VkDevice device, VkCommandPool command_pool,
                   VkQueue graphics_queue, VkImage image, const Texture* texture);

void set_texture_data(VkDevice device, VkPhysicalDevice physical_device, void* data,
                      VkCommandPool command_pool, VkQueue graphics_queue,
                      Texture* texture, VkDeviceSize size_bytes);

u32 create_textures_path(VkDevice device, VkPhysicalDevice physical_device,
                         VkCommandPool command_pool, VkQueue graphics_queue,
                         b8 mip_map, u32 num_textures, const char** tex_paths,
                         Texture* textures);

void create_texture_path(VkDevice device, VkPhysicalDevice physical_device,
                         VkCommandPool command_pool, VkQueue graphics_queue,
                         b8 mip_map, VkFormat image_format, const char* tex_path,
                         Texture* texture);

void create_texture(VkDevice device, VkPhysicalDevice physical_device, u32 width,
                    u32 height, VkCommandPool command_pool, VkQueue graphics_queue,
                    Texture* texture);

void create_texture_buffer(VkDevice device, VkPhysicalDevice physical_device,
                           VkCommandPool command_pool, VkQueue graphics_queue,
                           VkFormat image_format, Texture* texture,
                           unsigned char* tex_buffer);

void ray_casting_ex(VkDevice device, VkPhysicalDevice physical_device,
                    const Camera_2D* camera, VkCommandPool command_pool,
                    VkQueue graphics_queue, Texture* texture);

void create_depth_image(VkDevice device, VkPhysicalDevice physical_device,
                        const VkExtent2D* extent_2D,
                        VkSampleCountFlagBits sample_count, Image* depth_image);

uint32_t rand_rgb(uint32_t upper, uint32_t under);

void begin_render_pass(VkCommandBuffer command_buffer, VkRenderPass render_pass,
                       VkFramebuffer framebuffer, const VkExtent2D* extent_2D);

void end_render_pass(VkCommandBuffer command_buffer);

void bind_graphics_pipline(VkCommandBuffer command_buffer,
                                  const Graphic_Pipeline& graphic_pipline,
                                  u32 semaphore_idx);

void push_model(VkCommandBuffer command_buffer, VkPipelineLayout layout,
                       const M4& model);

void bind_vertex_index_buffer(VkCommandBuffer command_buffer,
                                     const Vertex_Buffer& vert_buffer,
                                     const Index_Buffer& index_buffer);

void bind_vertex_index_buffer(VkCommandBuffer command_buffer,
                                     const Vertex_Index_Buffer& buffer);

void bind_and_draw_graphics_pipline(
    VkCommandBuffer command_buffer, VkDescriptorSet desc_set, u32 index_offset,
    u32 index_count, const Vertex_Buffer& vertex_buffer,
    const Index_Buffer& index_buffer, const Graphic_Pipeline& graphic_pipline,
    const VkViewport& view_port, const VkRect2D* scissor);

void bind_and_draw_graphics_pipline(VkCommandBuffer command_buffer,
                                    VkDescriptorSet desc_set, u32 index_offset,
                                    u32 index_count,
                                    const Graphic_Pipeline& graphic_pipline,
                                    const VkViewport& view_port,
                                    const VkRect2D* scissor);

void create_fence_semaphore(VkDevice device, VkFence* fence,
                            VkSemaphore* image_semaphores,
                            VkSemaphore* present_semaphores);

void destroy_buffer(VkDevice device, Buffer buffer);

void destroy_texture(VkDevice device, Texture texture);

void destroy_image(VkDevice device, Image image);

void copy_data_buffer(Buffer* buffer, void* data, size_t size_bytes);

void update_buffers(VkDevice device, Buffer* buffer, void* data, size_t size_bytes);

u32 float_rgba(V4 color);


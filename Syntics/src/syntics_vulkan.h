#pragma once

#ifndef SY_UNIT_BUILD
#include "vulkan_internal_api.h"
#endif

typedef void Render_State;
typedef void (*Update_Callback)(void* data, Region_Alloc* region, const Application_State* app_state, Render_State* render_state, V2 dimensions, u32 semaphore_idx, f32 dt);
typedef void (*Recreate_Callback)(void* data, const Application_State* app_state);
typedef void (*Recreate_Graphic_Pipeline_callback)(void* data, const Application_State* app_state);
typedef void (*Destroy_Callback)(void* data, VkDevice device, u32 num_semaphores);

u32  u32_clamp(u32 value, u32 min, u32 max);

b8   vulkan_enable_validation(void);
void vulkan_debug_messenger_init(Instance_State* state);
void vulkan_debug_messenger_destroy(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

void vulkan_instance_init(VkInstance* instance);
void vulkan_instance_destroy(Instance_State* state);

Queue_Family_Indices
     vulkan_queue_indices_get(VkPhysicalDevice physical_device, VkSurfaceKHR surface, b8* all_supported);
void vulkan_pick_physical_device(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice* physical_device, Queue_Family_Indices* q_indices);
void vulkan_logical_device_create(VkPhysicalDevice physical_device, Queue_Family_Indices q_indices, VkDevice* device);
void vulkan_surface_create(Platform* platform, VkInstance instance, VkSurfaceKHR* surface);

void vulkan_swapchain_create(VkPhysicalDevice physical_device, VkDevice device, VkSurfaceKHR surface, u32 width, u32 height, Queue_Family_Indices indices, VkSwapchainKHR old_swap_chain, b8 vsync, Swap_Chain_Attrib* swap_chain);
void vulkan_swapchain_get_images(Region_Alloc* region, VkDevice device, Swap_Chain_Attrib* swap_chain);
void vulkan_swapchain_recreate(Application_State* app_state, u32 width, u32 height);

void vulkan_graphic_pipeline_create(VkDevice device, VkRenderPass render_pass, u32 subpass, VkSampleCountFlagBits sample_count, VkPipelineLayout pipeline_layout, const Vertex_Info* vertex_info, Graphic_Pipeline_Attrib* graphic_info, const char* vert_path, const char* frag_path, VkPipeline* graphic_pipline);
void vulkan_graphic_pipeline_create_deluxe(VkDevice device, VkPipelineLayout pipeline_layout, Graphic_Pipeline_Attrib* graphic_info, const char* vert_path, const char* frag_path, const Swap_Chain_Attrib* swap_chain, VkPipeline* graphic_pipline);
void vulkan_graphic_pipeline_create_deluxe_2d(VkDevice device, VkPipelineLayout pipeline_layout, Graphic_Pipeline_Attrib* graphic_info, const char* vert_path, const char* frag_path, VkRenderPass render_pass, VkSampleCountFlagBits sample_count, VkPipeline* graphic_pipline);
void vulkan_graphic_pipline_recreate(VkDevice device, VkPipelineLayout pipeline_layout, Graphic_Pipeline_Attrib* graphic_info, const char* vert_path, const char* frag_path, const Swap_Chain_Attrib* swap_chain, VkPipeline* graphic_pipline);
void vulkan_graphic_pipeline_layout_create(VkDevice device, VkDescriptorSetLayout set_layout, VkPipelineLayout* layout);

void vulkan_render_pass_create(VkDevice device, VkFormat color_format, VkSampleCountFlagBits sample_count, VkRenderPass* render_pass);
void vulkan_descriptor_set_layout_create(VkDevice device, u32 num_textures, VkDescriptorSetLayout* layout);
void vulkan_uniforms_descriptors_init(Region_Alloc* region, VkDevice device, VkPhysicalDevice physical_device, Buffer** uniform_buffers, Descriptors* descriptors, VkDescriptorSetLayout set_layout, u32 num_semaphores, const Texture* textures, u32 num_textures);
void vulkan_enable_multisample(const Swap_Chain_Attrib* swap_chain, VkDevice device, VkPhysicalDevice physical_device, Image* color_image);

void vulkan_buffer_create_alloc_bind(VkDevice device, VkPhysicalDevice physical_device, VkMemoryPropertyFlags wanted_mem_props, VkBufferUsageFlags usage_flags, VkBuffer* buffer, VkDeviceMemory* buffer_memory, VkDeviceSize data_size);
void vulkan_buffer_update(VkDevice device, Buffer* buffer, const void* data, size_t size_bytes);
void vulkan_buffer_copy(VkDevice device, VkCommandPool command_pool, VkBuffer src_buffer, VkBuffer dst_buffer, VkQueue graphics_queue, VkDeviceSize size_bytes);
void vulkan_buffer_copy_data(Buffer* buffer, const void* data, size_t size_bytes);
void vulkan_buffer_destroy(VkDevice device, Buffer buffer);

VkCommandBuffer 
     vulkan_command_buffer_begin(VkDevice device, VkCommandPool command_pool, VkCommandBufferLevel level);
void vulkan_command_buffer_end(VkDevice device, VkCommandPool command_pool, VkCommandBuffer command_buff, VkQueue graphics_queue);
void vulkan_command_buffers_allocate(VkDevice device, VkCommandPool command_pool, VkCommandBufferLevel level, u32 command_buffer_count, VkCommandBuffer* command_buffer);

void vulkan_staging_buffer_create(VkDevice device, VkPhysicalDevice physical_device, void* data, VkDeviceSize size_bytes, VkBufferUsageFlags usage_flags, Buffer* buffer);
void vulkan_staging_buffer_to_local(VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool, VkQueue graphics_queue, VkBufferUsageFlags vertex_or_index, void* data, VkBuffer* buffer, VkDeviceMemory* buffer_memory, VkDeviceSize size_bytes);

void vulkan_vertex_index_buffer_create_default(VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool, VkQueue graphics_queue, Visible_Local visible_local, Vertex_Buffer* vertex_buffer, Index_Buffer* index_buffer);
void vulkan_vertex_index_buffer_create_default1(VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool, VkQueue graphics_queue, Visible_Local visible_local, Vertex_Index_Buffer* vertex_index_buffer);
void vulkan_vertex_index_buffer_bind(VkCommandBuffer command_buffer, const Buffer* vert_buffer, const Index_Buffer* index_buffer);
void vulkan_vertex_index_buffer_bind1(VkCommandBuffer command_buffer, const Vertex_Index_Buffer* buffer);

void vulkan_vertex_buffer_create_visible(VkDevice device, VkPhysicalDevice physical_device, Buffer* vertex_buffer, void* data);
void vulkan_vertex_buffer_create_local(VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool, VkQueue graphics_queue, Buffer* vertex_buffer, void* data);

void vulkan_index_buffer_create_visible(VkDevice device, VkPhysicalDevice physical_device, Index_Buffer* index_buffer);
void vulkan_index_buffer_create_local(VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool, VkQueue graphics_queue, Index_Buffer* index_buffer);

void vulkan_uniform_buffer_create(VkDevice device, VkPhysicalDevice physical_device, Buffer* uniform_buffer);
void vulkan_command_pool_create(VkDevice device, u32 queue_fam_index, VkCommandPool* command_pool);

void vulkan_descriptors_create(VkDevice device, Descriptors* desciptors, u32 desc_count, VkDescriptorSetLayout desc_layout, const Texture* texture, u32 num_textures, Buffer* uniform_buffers);
void vulkan_descriptors_update(VkDevice device, Descriptors* desciptors, u32 desc_count, const Texture* textures, u32 num_textures, Buffer* uniform_buffers);

void vulkan_image_create(u32 width, u32 height, VkDevice device, VkPhysicalDevice physical_device, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags wanted_mem_props, VkSampleCountFlagBits num_samples, u32 mip_map_lvl, VkImage* image, VkDeviceMemory* image_mem);
void vulkan_image_view_create(VkDevice device, VkImage image, VkImageViewType image_view_type, VkFormat image_format, VkImageAspectFlags aspect_mask, u32 mip_map_lvl, VkImageView* image_view);
void vulkan_image_change_layout(VkDevice device, VkCommandPool command_pool, VkQueue graphic_queue, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);
void vulkan_image_destroy(VkDevice device, Image image);

void vulkan_frame_buffer_create(VkDevice device, VkRenderPass render_pass, VkExtent2D extent_2D, VkImageView* views, const u32 view_count, VkFramebuffer* framebuffer);

void vulkan_texture_create(VkDevice device, VkPhysicalDevice physical_device, u32 width, u32 height, VkCommandPool command_pool, VkQueue graphics_queue, VkFilter filter, Texture* texture);
void vulkan_texture_path_create(VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool, VkQueue graphics_queue, b8 mip_map, VkFormat image_format, VkFilter filter, const char* tex_path, Texture* texture);
u32  vulkan_texture_multiple_path_create(VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool, VkQueue graphics_queue, VkFilter filter, b8 mip_map, u32 num_textures, const char** tex_paths, Texture* textures);
void vulkan_texture_buffer_create(VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool, VkQueue graphics_queue, VkFormat image_format, VkFilter filter, unsigned char* tex_buffer, Texture* texture);
void vulkan_texture_set_data(VkDevice device, VkPhysicalDevice physical_device, void* data, VkCommandPool command_pool, VkQueue graphics_queue, Texture* texture, VkDeviceSize size_bytes);
void vulkan_texture_destroy(VkDevice device, Texture texture);

void vulkan_depth_image_create(VkDevice device, VkPhysicalDevice physical_device, const VkExtent2D* extent_2D, VkSampleCountFlagBits sample_count, Image* depth_image);

void vulkan_push_constant(VkCommandBuffer command_buffer, VkPipelineLayout layout, const void* data, u32 size);
void vulkan_draw(VkCommandBuffer command_buffer, u32 offset, u32 count);

u32  vulkan_get_semaphore_idx(Render_State* render_state);
void vulkan_fence_and_semaphore_create(VkDevice device, VkFence* fence, VkSemaphore* image_semaphores, VkSemaphore* present_semaphores);

void vulkan_render_state_init(Region_Alloc* region, VkDevice device, Queues queues, VkPhysicalDevice physical_device, VkCommandPool command_pool, const Queue_Family_Indices* q_indices, u32 num_semaphores, const Swap_Chain_Attrib* swap_chain, const Platform* platform, Render_State** render_state);
void vulkan_render_state_destroy(VkDevice device, Render_State* render_state);

void vulkan_render_pass_begin(VkCommandBuffer command_buffer, VkRenderPass render_pass, VkFramebuffer framebuffer, const VkExtent2D* extent_2D);
void vulkan_render_pass_end(VkCommandBuffer command_buffer);

VkQueue 
     vulkan_graphic_queue_get(Render_State* render_state);

void vulkan_subscribe_to_update_callback(Render_State* render_state, Update_Callback callback, void* data);
void vulkan_subscribe_to_recreate_callback(Render_State* render_state, Recreate_Callback callback, void* data);
void vulkan_subscribe_to_recreate_gp_callback(Render_State* render_state, Recreate_Graphic_Pipeline_callback callback, void* data);
void vulkan_subscribe_to_destroy_callback(Render_State* render_state, Destroy_Callback callback, void* data);

VkResult 
     vulkan_submit_and_present(VkQueue graphic_queue, VkQueue present_queue, VkSemaphore image_semaphore, VkSemaphore present_semaphore, VkFence fence, VkCommandBuffer* command_buffers, u32 command_buffer_count, VkSwapchainKHR swap_chain, u32 image_index);
b8   vulkan_frame_begin(Render_State* render_state, Application_State* app_state);
void vulkan_frame_render(Render_State* render_state, Application_State* app_state, Render_Task* copy_tasks, Render_Task* render_tasks_3d, Render_Task* render_tasks_2d, f32 dt);


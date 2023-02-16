#pragma once
#include "vulkan_internal_api.h"

typedef struct Region_Alloc Region_Alloc;

void create_swapchain(Region_Alloc* region, VkPhysicalDevice physical_device,
                      VkDevice device, VkSurfaceKHR surface, u32 width, u32 height,
                      Queue_Family_Indices indices, Swap_Chain_attrib* swap_chain);

void create_render_pass(VkDevice device, VkFormat color_format,
                        VkSampleCountFlagBits sample_count,
                        VkRenderPass* render_pass);

void get_swapchain_images(Region_Alloc* region, VkDevice device,
                          Swap_Chain_attrib* swap_chain);

void create_image_view(VkDevice device, VkImage image,
                       VkImageViewType image_view_type, VkFormat image_format,
                       VkImageAspectFlags aspect_mask, u32 mip_map_lvl,
                       VkImageView* image_view);

void create_frame_buffer(VkDevice device, VkRenderPass render_pass,
                         VkExtent2D extent_2D, VkImageView img_view,
                         VkImageView depth_view, VkImageView color_view,
                         VkFramebuffer* framebuffer);

void create_graphics_pipeline(Region_Alloc* region, VkDevice device, VkFormat format,
                              VkRenderPass render_pass,
                              VkSampleCountFlagBits sample_count,
                              const char* vert_path, const char* frag_path,
                              u32 width, u32 height, VkCullModeFlags cull_mode,
                              u32 num_textures, const VkRect2D* sciss,
                              Graphic_Pipline* graphic_pipline);

void init_graphics_pipeline(Region_Alloc* region, VkDevice device,
                            VkPhysicalDevice physical_device,
                            VkCommandPool command_pool, VkQueue graphic_queue,
                            u32 max_space, u32 num_semaphores,
                            const Texture* textures, u32 num_textures,
                            Graphic_Pipline& gp);

void enable_multisample(const Swap_Chain_attrib& swap_chain, VkDevice device,
                        VkPhysicalDevice physical_device, Image* color_image);

void recreate_graphic_pipline(Region_Alloc* region, VkDevice device,
                              const Swap_Chain_attrib& swap_chain,
                              const char* vert_file, const char* frag_file,
                              Graphic_Pipline& graphic_pipline, u32 num_textures,
                              const VkRect2D* scissor);

void recreate_graphic_pipline(Region_Alloc* region,
                              const Application_State& app_state,
                              const char* vert_file, const char* frag_file,
                              Graphic_Pipline& graphic_pipline, u32 num_textures,
                              const VkRect2D* scissor);

void recreate_swapchain(Region_Alloc* region, Application_State* app_state,
                        u32 width, u32 height, u32 num_textures);

void destroy_graphic_pipeline(VkDevice device, u32 num_semaphores,
                              Graphic_Pipline& gp);

void generate_indices(uint32** data, u32 offset, u32 num_indices);

void spirv_init();

void spirv_finilize();


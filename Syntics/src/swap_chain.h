#pragma once
#include "vulkan_internal_api.h"

typedef enum Visible_Local
{
    VERTEX_INDEX_VISIBLE_VISIBLE,
    VERTEX_INDEX_VISIBLE_LOCAL,
    VERTEX_INDEX_LOCAL_VISIBLE,
    VERTEX_INDEX_LOCAL_LOCAL,
} Visible_Local;

void create_swapchain(VkPhysicalDevice physical_device, VkDevice device,
                      VkSurfaceKHR surface, u32 width, u32 height,
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

void create_graphics_pipeline(VkDevice device, VkRenderPass render_pass,
                              VkSampleCountFlagBits sample_count,
                              const char* vert_path, const char* frag_path,
                              u32 width, u32 height, u32 num_textures,
                              const VkRect2D* sciss,
                              Graphic_Pipeline* graphic_pipline);

void create_graphics_pipeline_deluxe(
    Region_Alloc* region, VkDevice device, VkPhysicalDevice phy_device,
    VkCommandPool command_pool, VkQueue graphics_queue, u32 num_semaphores,
    const Swap_Chain_attrib* swap_chain, VkExtent2D extent_2D, u32 num_textures,
    const VkRect2D* sciss, Visible_Local visible_local,
    Graphic_Pipeline* graphic_pipline);

void init_gp(Region_Alloc* region, VkDevice device, VkPhysicalDevice physical_device,
             u32 num_semaphores, const Texture* textures, u32 num_textures,
             Graphic_Pipeline* gp);

void init_graphics_pipeline(Region_Alloc* region, VkDevice device,
                            VkPhysicalDevice physical_device, u32 max_space,
                            u32 num_semaphores, const Texture* textures,
                            u32 num_textures, Graphic_Pipeline* gp);

void init_graphics_pipeline_test(Region_Alloc* region, VkDevice device,
                                 VkPhysicalDevice physical_device, u32 max_space,
                                 u32 num_semaphores, const Texture* textures,
                                 u32 num_textures, Graphic_Pipeline* gp);

void enable_multisample(const Swap_Chain_attrib* swap_chain, VkDevice device,
                        VkPhysicalDevice physical_device, Image* color_image);

void recreate_graphic_pipline_sw(Region_Alloc* region, VkDevice device,
                                 const Swap_Chain_attrib* swap_chain,
                                 const char* vert_file, const char* frag_file,
                                 Graphic_Pipeline* graphic_pipline, u32 num_textures,
                                 const VkRect2D* scissor);

void recreate_graphic_pipline_ap(Region_Alloc* region,
                                 const Application_State* app_state,
                                 const char* vert_file, const char* frag_file,
                                 Graphic_Pipeline* graphic_pipline, u32 num_textures,
                                 const VkRect2D* scissor);

void recreate_swapchain(Region_Alloc* region, Application_State* app_state,
                        u32 width, u32 height, u32 num_textures);

void destroy_graphic_pipeline(VkDevice device, u32 num_semaphores,
                              Graphic_Pipeline* gp);

void spirv_init();

void spirv_finilize();


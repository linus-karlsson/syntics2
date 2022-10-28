#pragma once
#include "vulkan_types.h"

namespace synt {

typedef struct Region_Alloc Region_Alloc;

void create_swapchain(Region_Alloc* region, VkPhysicalDevice physical_device,
                      VkDevice device, VkSurfaceKHR surface, uint32 width,
                      uint32 height, Queue_Family_Indices indices,
                      Swap_Chain_attrib* swap_chain);

void create_render_pass(VkDevice device, VkFormat color_format,
                        VkSampleCountFlagBits sample_count,
                        VkRenderPass* render_pass);

void get_swapchain_images(Region_Alloc* region, VkDevice device,
                          Swap_Chain_attrib* swap_chain);

void create_image_view(VkDevice device, VkImage image,
                       VkImageViewType image_view_type, VkFormat image_format,
                       VkImageAspectFlags aspect_mask, uint32 mip_map_lvl,
                       VkImageView* image_view);

void create_frame_buffer(VkDevice device, VkRenderPass render_pass,
                         VkExtent2D extent_2D, VkImageView img_view,
                         VkImageView depth_view, VkImageView color_view,
                         VkFramebuffer* framebuffer);

void create_graphics_pipeline(Region_Alloc* region, VkDevice device,
                              VkFormat format, VkRenderPass render_pass,
                              VkSampleCountFlagBits sample_count,
                              const char* vert_path, const char* frag_path,
                              uint32 width, uint32 height,
                              Graphic_Pipline* graphic_pipline);

void enable_multisample(const Swap_Chain_attrib& swap_chain, VkDevice device,
                        VkPhysicalDevice physical_device, Image* color_image);

void recreate_swapchain(Region_Alloc* region, Application_State* app_state,
                        Graphic_Pipline** graphic_piplines, uint32 width,
                        uint32 height);

} // namespace synt

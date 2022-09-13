#pragma once
#include "vulkan_types.h"

namespace synt {

    typedef struct Region_Alloc Region_Alloc;

    void create_swapchain(Region_Alloc* region, VkPhysicalDevice physical_device,
                          VkDevice device, VkSurfaceKHR surface, uint32 width,
                          uint32 height, Queue_Family_Indices indices,
                          Swap_Chain_attrib* swap_chain);

    void create_render_pass(VkDevice device, VkFormat color_format,
                            VkRenderPass* render_pass);

    void get_swapchain_images(Region_Alloc* region, VkDevice device,
                              Swap_Chain_attrib* swap_chain);

    void create_image_view(VkDevice device, VkImage image,
                           VkImageViewType image_view_type, VkFormat image_format,
                           VkImageAspectFlags aspect_mask, VkImageView* image_view);

    void create_frame_buffer(VkDevice device, VkRenderPass render_pass,
                             VkExtent2D extent_2D, VkImageView img_view,
                             VkFramebuffer* framebuffer);

    void create_graphics_pipeline(Region_Alloc* region, VkDevice device,
                                  VkFormat format, const char* vert_path,
                                  const char* frag_path, uint32 width, uint32 height,
                                  Graphic_Pipline* graphic_pipline);

} // namespace synt

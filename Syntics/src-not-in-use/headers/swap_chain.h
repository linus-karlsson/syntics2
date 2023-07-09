#pragma once
#include "vulkan_internal_api.h"

void create_swapchain(VkPhysicalDevice physical_device, VkDevice device,
                      VkSurfaceKHR surface, u32 width, u32 height,
                      Queue_Family_Indices indices, VkSwapchainKHR old_swap_chain,
                      Swap_Chain_Attrib* swap_chain);

void create_render_pass(VkDevice device, VkFormat color_format,
                        VkSampleCountFlagBits sample_count,
                        VkRenderPass* render_pass);

void get_swapchain_images(Region_Alloc* region, VkDevice device,
                          Swap_Chain_Attrib* swap_chain);

void create_graphics_pipeline(VkDevice device, VkRenderPass render_pass,
                              VkSampleCountFlagBits sample_count,
                              const char* vert_path, const char* frag_path,
                              u32 width, u32 height, u32 num_textures,
                              const VkRect2D* sciss,
                              Graphic_Pipeline* graphic_pipline);

void create_graphics_pipeline_deluxe(Region_Alloc* region, VkDevice device,
                                     VkPhysicalDevice phy_device, u32 num_semaphores,
                                     const char* vert_path, const char* frag_path,
                                     const Swap_Chain_Attrib& swap_chain,
                                     const Texture* textures, u32 num_textures,
                                     Graphic_Pipeline* graphic_pipline);

void init_uniforms_descriptors(Region_Alloc* region, VkDevice device,
                               VkPhysicalDevice physical_device,
                               Uniform_Buffer** uniform_buffers,
                               Descriptors* descriptors,
                               VkDescriptorSetLayout set_layout, u32 num_semaphores,
                               const Texture* textures, u32 num_textures);

void init_graphics_pipeline(Region_Alloc* region, VkDevice device,
                            VkPhysicalDevice physical_device, u32 num_semaphores,
                            const Texture* textures, u32 num_textures,
                            Graphic_Pipeline* gp);

void enable_multisample(const Swap_Chain_Attrib* swap_chain, VkDevice device,
                        VkPhysicalDevice physical_device, Image* color_image);

void recreate_graphic_pipline_sw(VkDevice device,
                                 const Swap_Chain_Attrib* swap_chain,
                                 const char* vert_file, const char* frag_file,
                                 Graphic_Pipeline* graphic_pipline, u32 num_textures,
                                 const VkRect2D* scissor);

void recreate_graphic_pipline_ap(const Application_State* app_state,
                                 const char* vert_file, const char* frag_file,
                                 Graphic_Pipeline* graphic_pipline, u32 num_textures,
                                 const VkRect2D* scissor);

void recreate_swapchain(Region_Alloc* region, Application_State* app_state,
                        u32 width, u32 height);

void destroy_graphic_pipeline(VkDevice device, u32 num_semaphores,
                              Graphic_Pipeline* gp);

void spirv_init();

void spirv_finilize();


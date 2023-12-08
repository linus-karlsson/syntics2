#pragma once
#ifndef SY_UNIT_BUILD
#include "vulkan_internal_api.h"
#endif

u32 u32_clamp(u32 value, u32 min, u32 max);
void swapchain_create(VkPhysicalDevice physical_device, VkDevice device,
                      VkSurfaceKHR surface, u32 width, u32 height,
                      Queue_Family_Indices indices,
                      VkSwapchainKHR old_swap_chain, b8 vsync,
                      Swap_Chain_Attrib* swap_chain);
void render_pass_create(VkDevice device, VkFormat color_format,
                        VkSampleCountFlagBits sample_count,
                        VkRenderPass* render_pass);
void swapchain_images_get(Region_Alloc* region, VkDevice device,
                          Swap_Chain_Attrib* swap_chain);
void descriptor_set_layout_create(VkDevice device, u32 num_textures,
                                  VkDescriptorSetLayout* layout);
void pipeline_layout_create(VkDevice device, VkDescriptorSetLayout set_layout,
                            VkPipelineLayout* layout);
void graphics_pipeline_create(VkDevice device, VkRenderPass render_pass,
                              VkSampleCountFlagBits sample_count,
                              VkPipelineLayout pipeline_layout,
                              const Vertex_Info* vertex_info,
                              Graphic_Pipeline_Attrib* graphic_info,
                              const char* vert_path, const char* frag_path,
                              VkPipeline* graphic_pipline);
void uniforms_descriptors_init(Region_Alloc* region, VkDevice device,
                               VkPhysicalDevice physical_device,
                               Buffer** uniform_buffers,
                               Descriptors* descriptors,
                               VkDescriptorSetLayout set_layout,
                               u32 num_semaphores, const Texture* textures,
                               u32 num_textures);
void graphics_pipeline_create_deluxe(VkDevice device,
                                     VkPipelineLayout pipeline_layout,
                                     Graphic_Pipeline_Attrib* graphic_info,
                                     const char* vert_path,
                                     const char* frag_path,
                                     const Swap_Chain_Attrib* swap_chain,
                                     VkPipeline* graphic_pipline);
void multisample_enable(const Swap_Chain_Attrib* swap_chain, VkDevice device,
                        VkPhysicalDevice physical_device, Image* color_image);
void graphic_pipline_recreate(VkDevice device, VkPipelineLayout pipeline_layout,
                              Graphic_Pipeline_Attrib* graphic_info,
                              const char* vert_path, const char* frag_path,
                              const Swap_Chain_Attrib* swap_chain,
                              VkPipeline* graphic_pipline);
void swapchain_recreate(Application_State* app_state, u32 width, u32 height);

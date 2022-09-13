#pragma once
#include "defines.h"
#include <vulkan/vulkan.h>
#include <assert.h>

namespace synt {

#define VK_ASSERT(function)                                                         \
    ({                                                                              \
        VkResult res = function;                                                    \
        assert(res == VK_SUCCESS);                                                  \
    })

#define GRAPHICS_QUEUE_IDX 0

    typedef struct Queue_Family_Indices
    {
        uint32 indices[1];
        uint32 num_index_fam;
    } Queue_Family_Indices;

    typedef struct Graphic_Pipline
    {
        VkRenderPass render_pass;
        VkPipelineLayout layout;
        VkDescriptorSetLayout set_layout;
    } Graphic_Pipline;

    typedef struct Swap_Chain_attrib
    {
        Swap_Chain_attrib();

        VkSwapchainKHR swap_chain;
        VkExtent2D extent_2D;
        VkFormat color_format;
        VkImageView* img_views;
        VkImage* images;
        VkFramebuffer* framebuffers;
        uint32 num_images;
    } Swap_Chain_attrib;

} // namespace synt

#pragma once
#include "defines.h"
#include <vulkan/vulkan.h>
#include <assert.h>
#include "math/transforms.h"

namespace synt {

#define VK_ASSERT(function)                                                              \
    ({                                                                                   \
        VkResult res = function;                                                         \
        assert(res == VK_SUCCESS);                                                       \
    })

#define GRAPHICS_QUEUE_IDX 0

typedef struct Queue_Family_Indices
{
    uint32 indices[1];
    uint32 num_index_fam;
} Queue_Family_Indices;

typedef struct Graphic_Pipline
{
    VkPipeline pipeline;
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

    Graphic_Pipline graphic_pipline;
} Swap_Chain_attrib;

typedef struct Vertex
{
    Vec3 pos;
    Vec4 color;
} Vertex;

typedef struct Vertex_Buffer
{
    VkBuffer buffer;
    VkDeviceMemory buffer_memory;
    VkDeviceSize size_bytes;
    Vertex* data;
} Vertex_Buffer;

typedef struct Index_Buffer
{
    VkBuffer buffer;
    VkDeviceMemory buffer_memory;
    VkDeviceSize size_bytes;
    uint32* data;
} Index_Buffer;

typedef struct Application_State
{
    VkPhysicalDevice phy_device;
    VkDevice device;
    Queue_Family_Indices q_indices;
    VkSurfaceKHR surface;
    VkCommandPool com_pool;
    Swap_Chain_attrib swap_chain;

    Vertex_Buffer vert_buffer;
    Index_Buffer idx_buffer;

} Application_State;
} // namespace synt

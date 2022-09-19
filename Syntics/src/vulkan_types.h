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

typedef struct Queues
{
    VkQueue graphic_queue;
    VkQueue present_queue;
} Queues;

typedef struct Vertex
{
    Vec3 pos;
    Vec4 color;
    Vec2 tex_coords;
} Vertex;

typedef struct MVP
{
    Mat4f model;
    Mat4f view;
    Mat4f proj;
} MVP;

typedef struct Buffer
{
    VkBuffer buffer;
    VkDeviceMemory buffer_memory;
    VkDeviceSize size_bytes;
} Buffer;

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

typedef struct Uniform_Buffer
{
    VkBuffer buffer;
    VkDeviceMemory buffer_memory;
    VkDeviceSize size_bytes;
} Uniform_Buffer;

typedef struct Image
{
    VkImage image;
    VkDeviceMemory img_memory;
    VkImageView img_view;
} Image;

typedef struct Texture
{
    uint32_t width;
    uint32_t height;
    uint32_t mip_map_lvl;
    VkImage image;
    VkDeviceMemory img_memory;
    VkImageView img_view;
    VkSampler texture_sampler;
    VkDeviceSize size_bytes;
} Texture;

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

typedef struct Descriptors
{
    Descriptors();

    VkDescriptorPool desc_pool;
    uint32 desc_count;
    VkDescriptorSet* desc_sets;
} Descriptors;

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

    Texture texture;
    Image depth_img;

    uint32 num_semaphores;

} Application_State;
} // namespace synt

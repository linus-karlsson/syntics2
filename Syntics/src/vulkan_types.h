#pragma once
#include "defines.h"
#include <vulkan/vulkan.h>
#include <assert.h>
#include "math/transforms.h"
#include "file_reading.h"

#define VK_ASSERT(function)                                                         \
    do                                                                              \
    {                                                                               \
        VkResult res = function;                                                    \
        assert(res == VK_SUCCESS);                                                  \
    } while (0)

#define GRAPHICS_QUEUE_IDX 0

typedef struct Queue_Family_Indices
{
    u32 indices[1];
    u32 num_index_fam;
} Queue_Family_Indices;

typedef struct Queues
{
    VkQueue graphic_queue;
    VkQueue present_queue;
} Queues;

typedef struct Quad
{
    Vec3 corners[4];
} Quad;

typedef struct Vertex
{
    Vec4 pos;
    Vec4 color;
    Vec2 tex_coords;
    f32 tex_index;
} Vertex;

b8 operator==(const Vertex& f, const Vertex& s);

typedef struct MVP
{
    Mat4f model;
    Mat4f view;
    Mat4f proj;
    Vec3 light_pos;
} MVP;

b8 operator==(const MVP& f, const MVP& s);

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
    u32 curr_size;
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
    u32 width;
    u32 height;
    u32 mip_map_lvl;
    VkImage image;
    VkDeviceMemory img_memory;
    VkImageView img_view;
    VkSampler texture_sampler;
    VkDeviceSize size_bytes;
} Texture;

typedef struct Descriptors
{
    Descriptors();

    VkDescriptorPool desc_pool;
    u32 desc_count;
    VkDescriptorSet* desc_sets;
} Descriptors;

typedef struct Graphic_Pipline
{
    VkPipeline pipeline;
    VkPipelineLayout layout;
    VkDescriptorSetLayout set_layout;

    Vertex_Buffer vert_buffer;
    Index_Buffer idx_buffer;

    Uniform_Buffer* uniform_buffers;
    Descriptors descriptors;
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    u32 dynamic = false;
} Graphic_Pipline;

typedef struct Swap_Chain_attrib
{
    Swap_Chain_attrib();

    VkSwapchainKHR swap_chain;
    VkExtent2D extent_2D;
    VkFormat color_format;
    VkSampleCountFlagBits sample_count;
    VkImageView* img_views;
    VkImage* images;
    VkFramebuffer* framebuffers;
    u32 num_images;
    VkRenderPass render_pass;

} Swap_Chain_attrib;

typedef struct Application_State
{
    VkPhysicalDevice phy_device;
    VkDevice device;
    Queue_Family_Indices q_indices;
    VkSurfaceKHR surface;
    VkCommandPool com_pool;
    Swap_Chain_attrib swap_chain;

    Image depth_img;
    Image color_img;

    u32 num_semaphores;
    b8 running;

    u32 fps;

} Application_State;


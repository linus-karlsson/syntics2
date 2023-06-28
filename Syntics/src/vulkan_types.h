#pragma once
#include <vulkan/vulkan.h>
#include "math/transforms.h"

/* https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkResult.html
Success Codes
VK_SUCCESS Command successfully completed

VK_NOT_READY A fence or query has not yet completed

VK_TIMEOUT A wait operation has not completed in the specified time

VK_EVENT_SET An event is signaled

VK_EVENT_RESET An event is unsignaled

VK_INCOMPLETE A return array was too small for the result

VK_SUBOPTIMAL_KHR A swapchain no longer matches the surface properties exactly, but
can still be used to present to the surface successfully.

VK_THREAD_IDLE_KHR A deferred operation is not complete but there is currently no
work for this thread to do at the time of this call.

VK_THREAD_DONE_KHR A deferred operation is not complete but there is no work
remaining to assign to additional threads.

VK_OPERATION_DEFERRED_KHR A deferred operation was requested and at least some of the
work was deferred.

VK_OPERATION_NOT_DEFERRED_KHR A deferred operation was requested and no operations
were deferred.

VK_PIPELINE_COMPILE_REQUIRED A requested pipeline creation would have required
compilation, but the application requested compilation to not be performed.

Error codes
VK_ERROR_OUT_OF_HOST_MEMORY A host memory allocation has failed.

VK_ERROR_OUT_OF_DEVICE_MEMORY A device memory allocation has failed.

VK_ERROR_INITIALIZATION_FAILED Initialization of an object could not be completed for
implementation-specific reasons.

VK_ERROR_DEVICE_LOST The logical or physical device has been lost. See Lost Device

VK_ERROR_MEMORY_MAP_FAILED Mapping of a memory object has failed.

VK_ERROR_LAYER_NOT_PRESENT A requested layer is not present or could not be loaded.

VK_ERROR_EXTENSION_NOT_PRESENT A requested extension is not supported.

VK_ERROR_FEATURE_NOT_PRESENT A requested feature is not supported.

VK_ERROR_INCOMPATIBLE_DRIVER The requested version of Vulkan is not supported by the
driver or is otherwise incompatible for implementation-specific reasons.

VK_ERROR_TOO_MANY_OBJECTS Too many objects of the type have already been created.

VK_ERROR_FORMAT_NOT_SUPPORTED A requested format is not supported on this device.

VK_ERROR_FRAGMENTED_POOL A pool allocation has failed due to fragmentation of the
pool’s memory. This must only be returned if no attempt to allocate host or device
memory was made to accommodate the new allocation. This should be returned in
preference to VK_ERROR_OUT_OF_POOL_MEMORY, but only if the implementation is certain
that the pool allocation failure was due to fragmentation.

VK_ERROR_SURFACE_LOST_KHR A surface is no longer available.

VK_ERROR_NATIVE_WINDOW_IN_USE_KHR The requested window is already in use by Vulkan or
another API in a manner which prevents it from being used again.

VK_ERROR_OUT_OF_DATE_KHR A surface has changed in such a way that it is no longer
compatible with the swapchain, and further presentation requests using the swapchain
will fail. Applications must query the new surface properties and recreate their
swapchain if they wish to continue presenting to the surface.

VK_ERROR_INCOMPATIBLE_DISPLAY_KHR The display used by a swapchain does not use the
same presentable image layout, or is incompatible in a way that prevents sharing an
image.

VK_ERROR_INVALID_SHADER_NV One or more shaders failed to compile or link. More
details are reported back to the application via VK_EXT_debug_report if enabled.

VK_ERROR_OUT_OF_POOL_MEMORY A pool memory allocation has failed. This must only be
returned if no attempt to allocate host or device memory was made to accommodate the
new allocation. If the failure was definitely due to fragmentation of the pool,
VK_ERROR_FRAGMENTED_POOL should be returned instead.

VK_ERROR_INVALID_EXTERNAL_HANDLE An external handle is not a valid handle of the
specified type.

VK_ERROR_FRAGMENTATION A descriptor pool creation has failed due to fragmentation.

VK_ERROR_INVALID_DEVICE_ADDRESS_EXT A buffer creation failed because the requested
address is not available.

VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS A buffer creation or memory allocation failed
because the requested address is not available. A shader group handle assignment
failed because the requested shader group handle information is no longer valid.

VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT An operation on a swapchain created with
VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT failed as it did not have
exclusive full-screen access. This may occur due to implementation-dependent reasons,
outside of the application’s control.

VK_ERROR_COMPRESSION_EXHAUSTED_EXT An image creation failed because internal
resources required for compression are exhausted. This must only be returned when
fixed-rate compression is requested.

VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR The requested VkImageUsageFlags are not
supported.

VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR The requested video picture layout is
not supported.

VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR A video profile operation
specified via VkVideoProfileInfoKHR::videoCodecOperation is not supported.

VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR Format parameters in a requested
VkVideoProfileInfoKHR chain are not supported.

VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR Codec-specific parameters in a
requested VkVideoProfileInfoKHR chain are not supported.

VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR The specified video Std header version
is not supported.

VK_ERROR_UNKNOWN An unknown error has occurred; either the application has provided
invalid input, or an implementation failure has occurred. */

#define VK_ASSERT(function)                                                         \
    do                                                                              \
    {                                                                               \
        VkResult res = function;                                                    \
        ASSERT(res == VK_SUCCESS, "");                                              \
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

typedef struct Buffer
{
    VkBuffer buffer;
    VkDeviceMemory buffer_memory;
    VkDeviceSize size_bytes;
    void* transfer_data;
} Buffer;

typedef struct Vertex_Buffer
{
    Buffer buffer;
    Vertex* data;
} Vertex_Buffer;

typedef struct Index_Buffer
{
    Buffer buffer;
    u32 curr_size;
    uint32* data;
} Index_Buffer;

typedef struct Vertex_Index_Buffer
{
    Vertex_Buffer vert;
    Index_Buffer idx;
} Vertex_Index_Buffer;

typedef struct Uniform_Buffer
{
    Buffer buffer;
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
    VkDescriptorPool desc_pool;
    VkDescriptorSet* desc_sets;
    u32 desc_count;
} Descriptors;

typedef struct Graphic_Pipeline_Info
{

}Graphic_Pipeline_Info;

typedef struct Graphic_Pipeline
{
    VkPipeline pipeline;
    VkPipelineLayout layout;

    VkDescriptorSetLayout set_layout;

    Vertex_Buffer vert_buffer;
    Index_Buffer idx_buffer;

    Texture* textures;
    Uniform_Buffer* uniform_buffers;
    Descriptors descriptors;


    // TODO: move
    const char* frag_path;
    const char* vert_path;
    VkPrimitiveTopology topology;
    VkCullModeFlags cull_mode;
    VkPolygonMode poly_mode;
    f32 line_width;
    u32 dynamic;
    VkDynamicState dynamic_states[2];

} Graphic_Pipeline;

#define gp_default0()                                                               \
    gp_create(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_NONE,               \
              VK_POLYGON_MODE_FILL, 0)
#define gp_default1(topology)                                                       \
    gp_create(topology, VK_CULL_MODE_NONE, VK_POLYGON_MODE_FILL, 0)
#define gp_default2(topology, cull_mode)                                            \
    gp_create(topology, cull_mode, VK_POLYGON_MODE_FILL, 0)
Graphic_Pipeline gp_create(VkPrimitiveTopology topology, VkCullModeFlags cull_mode,
                           VkPolygonMode poly_mode, u32 dynamic);

typedef struct Swap_Chain_attrib
{
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


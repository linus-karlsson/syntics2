#include "linux_platform.h"
#include "event_system.h"
#include "region_alloc.h"
#include "math/transforms.h"
#include "logging.h"
#include "file_reading.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xcb.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef DEBUG
static const bool VALIDATIONS_ENABLE = 1;
#else
static const bool VALIDATIONS_ENABLE = 0;
#endif

#define SIZE(array) (uint32)(sizeof(array) / sizeof(array[0]))
#define VK_ASSERT(function)                                                         \
    ({                                                                              \
        VkResult res = function;                                                    \
        assert(res == VK_SUCCESS);                                                  \
    })

namespace synt {

    typedef struct Swap_Chain_attrib
    {
        Swap_Chain_attrib() : img_views(0), images(0), framebuffers(0), num_images(0)
        {
        }
        VkSwapchainKHR swap_chain;
        VkExtent2D extent_2D;
        VkFormat color_format;
        VkImageView* img_views;
        VkImage* images;
        VkFramebuffer* framebuffers;
        uint32 num_images;
    } Swap_Chain_attrib;

    static uint32 clamp_u32(uint32 value, uint32 min, uint32 max)
    {
        if (value > max)
            return max;
        else if (value < min)
            return min;
        return value;
    }

    void create_instance(Region_Alloc* region, VkInstance* instance)
    {
        uint32 version_supported = 0;
        VK_ASSERT(vkEnumerateInstanceVersion(&version_supported));
        synt_LOG("\nVulkan Version: %u.%u.%u.%u\n",
                 VK_API_VERSION_VARIANT(version_supported),
                 VK_API_VERSION_MAJOR(version_supported),
                 VK_API_VERSION_MINOR(version_supported),
                 VK_API_VERSION_PATCH(version_supported));

        VkApplicationInfo app_info  = {};
        app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName   = "Sandy";
        app_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        app_info.pEngineName        = "Syntics";
        app_info.engineVersion      = VK_MAKE_API_VERSION(0, 1, 0, 0);
        app_info.apiVersion         = VK_API_VERSION_1_3;

        uint32 extension_count    = 2;
        const char* extensions[3] = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_XCB_SURFACE_EXTENSION_NAME,
        };

        VkInstanceCreateInfo info = {};
        info.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.pApplicationInfo     = &app_info;

        if (VALIDATIONS_ENABLE)
        {
            const char* validations[]     = { "VK_LAYER_KHRONOS_validation" };
            info.enabledLayerCount        = 1;
            info.ppEnabledLayerNames      = validations;
            extensions[extension_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        }

        info.enabledExtensionCount   = extension_count;
        info.ppEnabledExtensionNames = extensions;

        synt_LOG("\nExtensions used: \n");
        for (uint32 i = 0; i < extension_count; i++)
            synt_LOG("\t%s\n", extensions[i]);
        synt_LOG("\n");

        *instance = VK_NULL_HANDLE;

        VK_ASSERT(vkCreateInstance(&info, NULL, instance));
    }

#define GRAPHICS_QUEUE_IDX 0

    typedef struct Queue_Family_Indices
    {
        uint32 indices[1];
        uint32 num_index_fam;
    } Queue_Family_Indices;

    Queue_Family_Indices get_queue_indices(Region_Alloc* region,
                                           VkPhysicalDevice physical_device,
                                           VkSurfaceKHR surface, bool* all_supported)
    {
        uint32 queue_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count,
                                                 NULL);

        Temp_Alloc<VkQueueFamilyProperties> queue_props(region, queue_count);

        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count,
                                                 queue_props.data);

        Queue_Family_Indices indices = {};
        bool graphic_supported       = false;
        bool presentation_supported  = false;
        for (uint32 i = 0; i < queue_count; i++)
        {
            if (queue_props.data[i].queueCount > 0 &&
                (queue_props.data[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) ==
                    VK_QUEUE_GRAPHICS_BIT)
            {
                indices.indices[GRAPHICS_QUEUE_IDX] = i;

                graphic_supported = true;
            }
            VkBool32 surface_support = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface,
                                                 &surface_support);
            if (surface_support && graphic_supported &&
                indices.indices[GRAPHICS_QUEUE_IDX] == i)
            {
                presentation_supported = true;
            }
        }
        *all_supported = graphic_supported && presentation_supported;

        bool dublicate        = false;
        indices.num_index_fam = 0;
        for (uint32 i = 0; i < SIZE(indices.indices); i++)
        {
            for (uint32 j = 0; j < i; j++)
            {
                if (indices.indices[i] == indices.indices[j])
                {
                    dublicate = true;
                }
            }
            if (!dublicate) indices.num_index_fam++;

            dublicate = false;
        }

        return indices;
    }

    void enumerate_py_devices(Region_Alloc* region, VkInstance instance,
                              VkSurfaceKHR surface,
                              VkPhysicalDevice* physical_device,
                              Queue_Family_Indices* q_indices)
    {
        uint32 device_count = 0;
        VK_ASSERT(vkEnumeratePhysicalDevices(instance, &device_count, NULL));

        Temp_Alloc<VkPhysicalDevice> physical_devices(region, device_count);

        VK_ASSERT(vkEnumeratePhysicalDevices(instance, &device_count,
                                             physical_devices.data));

        *physical_device = VK_NULL_HANDLE;

        Temp_Alloc<char*> buffer(region, device_count + 1);
        Temp_Alloc<VkPhysicalDeviceProperties> props(region, device_count);
        buffer.data[0]      = (char*)"\nAvailable Physical devices: ";
        bool supported      = false;
        uint32 device_index = 0;
        for (uint32 i = 0; i < device_count; i++)
        {
            vkGetPhysicalDeviceProperties(physical_devices.data[i], &props.data[i]);
            buffer.data[i + 1] = props.data[i].deviceName;
            if (!supported)
            {
                *q_indices = get_queue_indices(region, physical_devices.data[i],
                                               surface, &supported);
                if (supported)
                {
                    *physical_device = physical_devices.data[i];
                    device_index     = i;
                }
            }
        }
        synt_LOG("%s\n\t", buffer.data[0]);
        for (uint32 i = 0; i < device_count; i++)
            synt_LOG("%s\n\t", buffer.data[i + 1]);

        assert(physical_device);

        synt_LOG("\nDevice in use: \n");
        synt_LOG("\t%s\n\n", props.data[device_index].deviceName);
    }

    void logical_device(VkPhysicalDevice physical_device,
                        Queue_Family_Indices q_indices, VkDevice* device)
    {
        *device = VK_NULL_HANDLE;

        float queue_prio = 1.0f;
        VkDeviceQueueCreateInfo queue_infos[SIZE(q_indices.indices)];

        for (uint32 i = 0; i < q_indices.num_index_fam; i++)
        {
            VkDeviceQueueCreateInfo queue_info = {};
            queue_info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info.queueCount       = 1;
            queue_info.pQueuePriorities = &queue_prio;
            queue_info.queueFamilyIndex = q_indices.indices[i];

            queue_infos[i] = queue_info;
        }

        synt_LOG("\nNumber of queue indices: %u\n\n", q_indices.num_index_fam);

        const char* extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        VkDeviceCreateInfo device_info      = {};
        device_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_info.queueCreateInfoCount    = q_indices.num_index_fam;
        device_info.pQueueCreateInfos       = queue_infos;
        device_info.enabledExtensionCount   = SIZE(extensions);
        device_info.ppEnabledExtensionNames = extensions;

        VK_ASSERT(vkCreateDevice(physical_device, &device_info, NULL, device));
    }

    void get_surface(VkInstance instance, Linux_Platform xcb, VkSurfaceKHR* surface)
    {
        VkXcbSurfaceCreateInfoKHR surface_info = {};
        surface_info.sType      = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        surface_info.connection = xcb.connection;
        surface_info.window     = xcb.window;

        *surface = VK_NULL_HANDLE;
        VK_ASSERT(vkCreateXcbSurfaceKHR(instance, &surface_info, NULL, surface));
    }

    void create_command_pool(VkDevice device, uint32 queue_fam_index,
                             VkCommandPool* command_pool)
    {
        VkCommandPoolCreateInfo create_info = {};
        create_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        create_info.queueFamilyIndex = queue_fam_index;

        *command_pool = VK_NULL_HANDLE;
        VK_ASSERT(vkCreateCommandPool(device, &create_info, NULL, command_pool));
    }

    void allocate_commandbuffer(VkDevice device, VkCommandPool command_pool,
                                VkCommandBuffer* command_buffer)
    {

        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = command_pool;
        alloc_info.commandBufferCount = 1;

        VK_ASSERT(vkAllocateCommandBuffers(device, &alloc_info, command_buffer));
    }

    void record_commandbuffer(VkCommandBuffer command_buffer)
    {

        VkCommandBufferBeginInfo buffer_begin_info = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
        };

        VK_ASSERT(vkBeginCommandBuffer(command_buffer, &buffer_begin_info));

        VK_ASSERT(vkEndCommandBuffer(command_buffer));
    }

    void create_swapchain(Region_Alloc* region, VkPhysicalDevice physical_device,
                          VkDevice device, VkSurfaceKHR surface, uint32 width,
                          uint32 height, Queue_Family_Indices indices,
                          Swap_Chain_attrib* swap_chain)
    {

        VkSurfaceCapabilitiesKHR surface_cap;
        VK_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface,
                                                            &surface_cap));

        Temp_Alloc<VkPresentModeKHR> present_modes;
        uint32 present_mode_count = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
                                                  &present_mode_count, NULL);
        if (present_mode_count)
        {
            present_modes.init(region, present_mode_count);

            vkGetPhysicalDeviceSurfacePresentModesKHR(
                physical_device, surface, &present_mode_count, present_modes.data);
        }

        Temp_Alloc<VkSurfaceFormatKHR> surface_formats;
        uint32 surface_format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface,
                                             &surface_format_count, NULL);

        if (surface_format_count)
        {
            surface_formats.init(region, surface_format_count);

            vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface,
                                                 &surface_format_count,
                                                 surface_formats.data);
        }

        VkPresentModeKHR present_mode_to_use = VK_PRESENT_MODE_FIFO_KHR;
        for (uint32 i = 0; i < present_mode_count; i++)
        {
            if (present_modes.data[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                present_mode_to_use = present_modes.data[i];
                break;
            }
        }
        VkSurfaceFormatKHR surface_format_to_use = surface_formats.data[0];
        for (uint32 i = 0; i < surface_format_count; i++)
        {
            if (surface_formats.data[i].format == VK_FORMAT_R8G8B8A8_SRGB &&
                surface_formats.data[i].colorSpace ==
                    VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                surface_format_to_use = surface_formats.data[i];
                break;
            }
        }

        VkExtent2D extent_2D = surface_cap.currentExtent;
        if (surface_cap.currentExtent.width == 0xFFFFFFFF)
        {
            extent_2D.width = clamp_u32(width, surface_cap.minImageExtent.width,
                                        surface_cap.maxImageExtent.width);

            extent_2D.height = clamp_u32(height, surface_cap.minImageExtent.height,
                                         surface_cap.maxImageExtent.height);
        }

        uint32_t min_image_count = surface_cap.minImageCount + 1;
        if (surface_cap.minImageCount + 1 > surface_cap.maxImageCount &&
            surface_cap.maxImageCount > 0)
        {
            min_image_count = surface_cap.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swap_info = {};
        swap_info.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swap_info.surface          = surface;
        swap_info.minImageCount    = min_image_count;
        swap_info.imageFormat      = surface_format_to_use.format;
        swap_info.imageColorSpace  = surface_format_to_use.colorSpace;
        swap_info.imageExtent      = extent_2D;
        swap_info.imageArrayLayers = 1;
        swap_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swap_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swap_info.queueFamilyIndexCount = 0;
        swap_info.pQueueFamilyIndices   = NULL;
        swap_info.preTransform          = surface_cap.currentTransform;
        swap_info.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swap_info.presentMode           = present_mode_to_use;
        swap_info.clipped               = VK_FALSE;
        swap_info.oldSwapchain          = VK_NULL_HANDLE;
        if (indices.num_index_fam > 1)
        {
            swap_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
            swap_info.queueFamilyIndexCount = indices.num_index_fam;
            swap_info.pQueueFamilyIndices   = indices.indices;
        }

        swap_chain->swap_chain   = VK_NULL_HANDLE;
        swap_chain->color_format = surface_format_to_use.format;
        swap_chain->extent_2D    = extent_2D;

        VK_ASSERT(
            vkCreateSwapchainKHR(device, &swap_info, NULL, &swap_chain->swap_chain));
    }

    void create_fence_semaphore(VkDevice device, VkFence* fence,
                                VkSemaphore* semaphore)
    {
        VkFenceCreateInfo fence_info = {
            VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        };
        VkSemaphoreCreateInfo semaphore_info = {
            VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        VK_ASSERT(vkCreateFence(device, &fence_info, NULL, fence));
        VK_ASSERT(vkCreateSemaphore(device, &semaphore_info, NULL, semaphore));
    }

    void create_render_pass(VkDevice device, VkFormat color_format,
                            VkRenderPass* render_pass)
    {
        // If the attachment uses a color format, then loadOp and storeOp are used,
        // and stencilLoadOp and stencilStoreOp are ignored.
        // If the format has depth and/or stencil components, loadOp and storeOp
        // apply only to the depth data, while stencilLoadOp and stencilStoreOp
        // define how the stencil data is handled.
        // If a set of attachments alias each other, then all except the first to be
        // used in the render pass must use an initialLayout of
        // VK_IMAGE_LAYOUT_UNDEFINED. -Vulkan Specification
        //
        VkAttachmentDescription attachment_descs[1] = {};

        VkAttachmentDescription color_attach_desc = {};
        color_attach_desc.format                  = color_format;
        color_attach_desc.samples                 = VK_SAMPLE_COUNT_1_BIT;
        color_attach_desc.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attach_desc.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
        color_attach_desc.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attach_desc.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attach_desc.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attach_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachment_descs[0] = color_attach_desc;

        VkAttachmentReference color_attach_ref = {};
        color_attach_ref.attachment            = 0;
        color_attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass_desc = {};
        subpass_desc.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_desc.colorAttachmentCount = 1;
        subpass_desc.pColorAttachments    = &color_attach_ref;

        VkRenderPassCreateInfo render_pass_info = {};
        render_pass_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = sy_size(attachment_descs);
        render_pass_info.pAttachments    = attachment_descs;
        render_pass_info.subpassCount    = 1;
        render_pass_info.pSubpasses      = &subpass_desc;

        VK_ASSERT(vkCreateRenderPass(device, &render_pass_info, NULL, render_pass));
    }

    void get_swapchain_images(Region_Alloc* region, VkDevice device,
                              Swap_Chain_attrib* swap_chain)
    {

        vkGetSwapchainImagesKHR(device, swap_chain->swap_chain,
                                &swap_chain->num_images, NULL);

        if (!swap_chain->images)
            swap_chain->images =
                dyn_array(region, swap_chain->num_images, VkImage, synt::PERM_ARRAY);

        vkGetSwapchainImagesKHR(device, swap_chain->swap_chain,
                                &swap_chain->num_images, swap_chain->images);

        assert(synt::capacity_arr(swap_chain->images) == swap_chain->num_images);
    }

    void create_frame_buffer(VkDevice device, VkRenderPass render_pass,
                             VkExtent2D extent_2D, VkImageView img_view,
                             VkFramebuffer* framebuffer)
    {
        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass      = render_pass;
        framebuffer_info.renderPass      = render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments    = &img_view;
        framebuffer_info.width           = extent_2D.width;
        framebuffer_info.height          = extent_2D.height;
        framebuffer_info.layers          = 1;

        VK_ASSERT(vkCreateFramebuffer(device, &framebuffer_info, NULL, framebuffer));
    }

    void create_image_view(VkDevice device, VkImage image,
                           VkImageViewType image_view_type, VkFormat image_format,
                           VkImageAspectFlags aspect_mask, VkImageView* image_view)
    {
        VkImageViewCreateInfo view_create_info = {};
        view_create_info.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_create_info.image        = image;
        view_create_info.viewType     = image_view_type;
        view_create_info.format       = image_format;
        view_create_info.components.r = VK_COMPONENT_SWIZZLE_R;
        view_create_info.components.g = VK_COMPONENT_SWIZZLE_G;
        view_create_info.components.b = VK_COMPONENT_SWIZZLE_B;
        view_create_info.components.a = VK_COMPONENT_SWIZZLE_A;
        view_create_info.subresourceRange.aspectMask = aspect_mask;
        view_create_info.subresourceRange.levelCount = 1;
        view_create_info.subresourceRange.layerCount = 1;

        VK_ASSERT(vkCreateImageView(device, &view_create_info, NULL, image_view));
    }

    void create_graphics_pipeline(Region_Alloc* region, const char* vert_path,
                                  const char* frag_path)
    {
        File_Attrib vert_module = read_file(region, vert_path, "rb");
        File_Attrib frag_module = read_file(region, frag_path, "rb");
    }
} // namespace synt

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        synt::set_log(false);
        synt::set_log_alloc(false);
    }

    synt::Linux_Platform xcb = {};
    synt::Region_Alloc region;

    VkInstance instance = VK_NULL_HANDLE;

    VkPhysicalDevice physical_device     = VK_NULL_HANDLE;
    VkDevice device                      = VK_NULL_HANDLE;
    VkQueue graphic_queue                = VK_NULL_HANDLE;
    synt::Queue_Family_Indices q_indices = {};

    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkCommandPool command_pool     = VK_NULL_HANDLE;
    VkCommandBuffer command_buffer = VK_NULL_HANDLE;

    synt::Swap_Chain_attrib swap_chain;

    const uint32 num_semaphores = 2;

    VkRenderPass render_pass = VK_NULL_HANDLE;

    synt::init_region(&region, 1000000);
    synt::init_platform(&xcb, 800, 600);
    synt::init_events(&region, 1);

    VkFence* fences =
        region_malloc(&region, num_semaphores, VkFence, synt::PERM_MALLOC);
    VkSemaphore* semaphores =
        region_malloc(&region, num_semaphores, VkSemaphore, synt::PERM_MALLOC);

    synt::create_graphics_pipeline(&region, "Syntics/res/vert.spv",
                                   "Syntics/res/frag.spv");

    synt::create_instance(&region, &instance);

    synt::get_surface(instance, xcb, &surface);

    synt::enumerate_py_devices(&region, instance, surface, &physical_device,
                               &q_indices);

    synt::logical_device(physical_device, q_indices, &device);

    vkGetDeviceQueue(device, q_indices.indices[GRAPHICS_QUEUE_IDX], 0,
                     &graphic_queue);

    for (uint32 i = 0; i < num_semaphores; i++)
    {
        synt::create_fence_semaphore(device, &fences[i], &semaphores[i]);
    }

    synt::create_swapchain(&region, physical_device, device, surface, xcb.width,
                           xcb.height, q_indices, &swap_chain);

    synt::create_render_pass(device, swap_chain.color_format, &render_pass);

    synt::get_swapchain_images(&region, device, &swap_chain);

    swap_chain.img_views = region_malloc(&region, swap_chain.num_images, VkImageView,
                                         synt::PERM_MALLOC);

    swap_chain.framebuffers = region_malloc(&region, swap_chain.num_images,
                                            VkFramebuffer, synt::PERM_MALLOC);

    for (uint32 i = 0; i < swap_chain.num_images; i++)
    {
        synt::create_image_view(device, swap_chain.images[i], VK_IMAGE_VIEW_TYPE_2D,
                                swap_chain.color_format, VK_IMAGE_ASPECT_COLOR_BIT,
                                &swap_chain.img_views[i]);

        synt::create_frame_buffer(device, render_pass, swap_chain.extent_2D,
                                  swap_chain.img_views[i],
                                  &swap_chain.framebuffers[i]);
    }

    synt::create_command_pool(device, q_indices.indices[GRAPHICS_QUEUE_IDX],
                              &command_pool);

    synt::allocate_commandbuffer(device, command_pool, &command_buffer);

    synt::print_region(region);

    synt::Events* evt;
    synt::subscribe(&evt, synt::EVT_KEY);

    bool running = true;
    while (running)
    {

        uint32 image_index = 0;

        synt::poll_events();
        if (evt->activated)
        {
            if (evt->key_evt.key == SYNT_KEY_Q) running = false;
        }
    }

    for (uint32 i = 0; i < swap_chain.num_images; i++)
    {
        vkDestroyFramebuffer(device, swap_chain.framebuffers[i], NULL);
        vkDestroyImageView(device, swap_chain.img_views[i], NULL);
    }
    vkDestroySwapchainKHR(device, swap_chain.swap_chain, NULL);
    vkDestroyRenderPass(device, render_pass, NULL);

    for (uint32 i = 0; i < num_semaphores; i++)
    {
        vkDestroyFence(device, fences[i], NULL);
        vkDestroySemaphore(device, semaphores[i], NULL);
    }
    vkDestroyCommandPool(device, command_pool, NULL);

    vkDestroyDevice(device, NULL);

    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyInstance(instance, NULL);

    synt::free_region(&region);

    synt_LOG("\nComplete!\n");

    return 0;
}

#include "linux_platform.h"
#include "event_system.h"
#include "region_alloc.h"
#include "math/transforms.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xcb.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

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

    static uint32 clamp_u32(uint32 value, uint32 min, uint32 max)
    {
        if (value > max)
            return max;
        else if (value < min)
            return min;
        return value;
    }

    void create_instance(VkInstance* instance)
    {
        uint32 version_supported = 0;
        VK_ASSERT(vkEnumerateInstanceVersion(&version_supported));
        printf("\nVulkan Version: %u.%u.%u.%u\n",
               VK_API_VERSION_VARIANT(version_supported),
               VK_API_VERSION_MAJOR(version_supported),
               VK_API_VERSION_MINOR(version_supported),
               VK_API_VERSION_PATCH(version_supported));

        VkApplicationInfo app_info = {
            .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName   = "Sandy",
            .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
            .pEngineName        = "Syntics",
            .engineVersion      = VK_MAKE_API_VERSION(0, 1, 0, 0),
            .apiVersion         = VK_API_VERSION_1_3,
        };

        uint32 extension_count    = 2;
        const char* extensions[3] = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_XCB_SURFACE_EXTENSION_NAME,
        };

        VkInstanceCreateInfo info = {
            .sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &app_info,
        };

        if (VALIDATIONS_ENABLE)
        {
            const char* validations[]     = { "VK_LAYER_KHRONOS_validation" };
            info.enabledLayerCount        = 1;
            info.ppEnabledLayerNames      = validations;
            extensions[extension_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        }

        info.enabledExtensionCount   = extension_count;
        info.ppEnabledExtensionNames = extensions;

        printf("\nExtensions used: \n");
        for (uint32 i = 0; i < extension_count; i++)
            printf("\t%s\n", extensions[i]);

        *instance = VK_NULL_HANDLE;

        VK_ASSERT(vkCreateInstance(&info, NULL, instance));
    }

#define GRAPHICS_QUEUE_IDX 0
#define PRESENT_QUEUE_IDX 1

    typedef struct Queue_Family_Indices
    {
        uint32 indices[2];
        uint32 num_index_fam;
    } Queue_Family_Indices;

    Queue_Family_Indices get_queue_indices(Region_Alloc* region,
                                           VkPhysicalDevice physical_device,
                                           VkSurfaceKHR surface, bool* all_supported)
    {
        uint32 queue_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count,
                                                 NULL);

        VkQueueFamilyProperties* queue_props = region_malloc(
            region, queue_count, VkQueueFamilyProperties, synt::TEMP_MALLOC);

        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count,
                                                 queue_props);

        Queue_Family_Indices indices = {};
        uint32 count                 = 0;
        for (uint32 i = 0; i < queue_count; i++)
        {
            if ((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) ==
                VK_QUEUE_GRAPHICS_BIT)
            {
                indices.indices[GRAPHICS_QUEUE_IDX] = i;
                count++;
            }
            VkBool32 surface_support = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface,
                                                 &surface_support);
            if (surface_support)
            {
                indices.indices[PRESENT_QUEUE_IDX] = i;
                count++;
            }
        }
        *all_supported = 0;
        if (count == SIZE(indices.indices)) *all_supported = 1;

        bool dublicate        = false;
        indices.num_index_fam = 0;
        for (uint32 i = 0; i < count; i++)
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

        region_pop(region, queue_count, VkQueueFamilyProperties, synt::TEMP_MALLOC);
        return indices;
    }

    void enumerate_py_devices(Region_Alloc* region, VkInstance instance,
                              VkSurfaceKHR surface,
                              VkPhysicalDevice* physical_device,
                              Queue_Family_Indices* q_indices)
    {
        uint32 device_count = 0;
        VK_ASSERT(vkEnumeratePhysicalDevices(instance, &device_count, NULL));
        VkPhysicalDevice* physical_devices =
            region_malloc(region, device_count, VkPhysicalDevice, synt::TEMP_MALLOC);
        VK_ASSERT(
            vkEnumeratePhysicalDevices(instance, &device_count, physical_devices));

        *physical_device = VK_NULL_HANDLE;

        printf("\nAvailable Physical devices: \n");
        bool supported = false;
        for (uint32 i = 0; i < device_count; i++)
        {
            VkPhysicalDeviceProperties props = {};
            vkGetPhysicalDeviceProperties(physical_devices[i], &props);
            printf("\t%s\n", props.deviceName);

            if (!supported)
            {
                *q_indices = get_queue_indices(region, physical_devices[i], surface,
                                               &supported);
                if (supported) *physical_device = physical_devices[i];
            }
        }

        assert(physical_device);

        VkPhysicalDeviceProperties props = {};
        vkGetPhysicalDeviceProperties(*physical_device, &props);
        printf("\nDevice in use: \n");
        printf("\t%s\n", props.deviceName);

        region_pop(region, device_count, VkPhysicalDevice, synt::TEMP_MALLOC);
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

        printf("\nNumber of queue indices: %u\n", q_indices.num_index_fam);

        const char* extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        VkDeviceCreateInfo device_info = {
            .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount    = q_indices.num_index_fam,
            .pQueueCreateInfos       = queue_infos,
            .enabledExtensionCount   = SIZE(extensions),
            .ppEnabledExtensionNames = extensions,
        };

        VK_ASSERT(vkCreateDevice(physical_device, &device_info, NULL, device));
    }

    void get_surface(VkInstance instance, Linux_Platform xcb, VkSurfaceKHR* surface)
    {
        VkXcbSurfaceCreateInfoKHR surface_info = {
            .sType      = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
            .connection = xcb.connection,
            .window     = xcb.window,
        };

        *surface = VK_NULL_HANDLE;
        VK_ASSERT(vkCreateXcbSurfaceKHR(instance, &surface_info, NULL, surface));
    }

    void create_command_pool(VkDevice device, uint32 queue_fam_index,
                             VkCommandPool* command_pool)
    {
        VkCommandPoolCreateInfo create_info = {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = queue_fam_index,
        };

        *command_pool = VK_NULL_HANDLE;
        VK_ASSERT(vkCreateCommandPool(device, &create_info, NULL, command_pool));
    }

    void allocate_commandbuffer(VkDevice device, VkCommandPool command_pool,
                                VkCommandBuffer* command_buffer)
    {

        VkCommandBufferAllocateInfo alloc_info = {
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = command_pool,
            .commandBufferCount = 1,
        };

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
                          VkDevice device, VkSurfaceKHR surface,
                          VkSwapchainKHR* swap_chain, uint32 width, uint32 height,
                          Queue_Family_Indices indices)
    {

        VkSurfaceCapabilitiesKHR surface_cap;
        VK_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface,
                                                            &surface_cap));

        VkPresentModeKHR* present_modes = NULL;
        uint32 present_mode_count       = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
                                                  &present_mode_count, NULL);
        if (present_mode_count)
        {
            present_modes = region_malloc(region, present_mode_count,
                                          VkPresentModeKHR, synt::TEMP_MALLOC);
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                physical_device, surface, &present_mode_count, present_modes);
        }

        VkSurfaceFormatKHR* surface_formats = NULL;
        uint32 surface_format_count         = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface,
                                             &surface_format_count, NULL);

        if (surface_format_count)
        {
            surface_formats = region_malloc(region, surface_format_count,
                                            VkSurfaceFormatKHR, synt::TEMP_MALLOC);
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                physical_device, surface, &surface_format_count, surface_formats);
        }

        VkPresentModeKHR present_mode_to_use = VK_PRESENT_MODE_FIFO_KHR;
        for (uint32 i = 0; i < present_mode_count; i++)
        {
            if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                present_mode_to_use = present_modes[i];
                break;
            }
        }
        VkSurfaceFormatKHR surface_format_to_use = surface_formats[0];
        for (uint32 i = 0; i < surface_format_count; i++)
        {
            if (surface_formats[i].format == VK_FORMAT_R8G8B8A8_SRGB &&
                surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                surface_format_to_use = surface_formats[i];
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

        VkSwapchainCreateInfoKHR swap_info = {
            .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface               = surface,
            .minImageCount         = min_image_count,
            .imageFormat           = surface_format_to_use.format,
            .imageColorSpace       = surface_format_to_use.colorSpace,
            .imageExtent           = extent_2D,
            .imageArrayLayers      = 1,
            .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices   = NULL,
            .preTransform          = surface_cap.currentTransform,
            .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode           = present_mode_to_use,
            .clipped               = VK_FALSE,
            .oldSwapchain          = VK_NULL_HANDLE,
        };
        if (indices.num_index_fam > 1)
        {
            swap_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
            swap_info.queueFamilyIndexCount = indices.num_index_fam;
            swap_info.pQueueFamilyIndices   = indices.indices;
        }

        *swap_chain = VK_NULL_HANDLE;

        VK_ASSERT(vkCreateSwapchainKHR(device, &swap_info, NULL, swap_chain));

        region_pop(region, present_mode_count, VkPresentModeKHR, synt::TEMP_MALLOC);
        region_pop(region, surface_format_count, VkSurfaceFormatKHR,
                   synt::TEMP_MALLOC);
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

    void create_render_pass(VkDevice device, VkRenderPass* render_pass)
    {
        VkRenderPassCreateInfo render_pass_info = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        };

        vkCreateRenderPass(device, &render_pass_info, NULL, render_pass);
    }

    void create_graphics_pipeline() { vkCreateGraphicsPipelines() }
} // namespace synt

int main()
{
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

    VkSwapchainKHR swap_chain = VK_NULL_HANDLE;

    VkFence fence         = VK_NULL_HANDLE;
    VkSemaphore semaphore = VK_NULL_HANDLE;

    VkRenderPass render_pass = VK_NULL_HANDLE;

    synt::init_region(&region, 1000000);
    synt::init_platform(&xcb, 800, 600);
    synt::init_events(&region, 1);

    synt::create_instance(&instance);

    synt::get_surface(instance, xcb, &surface);

    synt::enumerate_py_devices(&region, instance, surface, &physical_device,
                               &q_indices);

    synt::logical_device(physical_device, q_indices, &device);

    vkGetDeviceQueue(device, q_indices.indices[GRAPHICS_QUEUE_IDX], 0,
                     &graphic_queue);

    synt::create_fence_semaphore(device, &fence, &semaphore);

    synt::create_swapchain(&region, physical_device, device, surface, &swap_chain,
                           xcb.width, xcb.height, q_indices);

    synt::create_command_pool(device, q_indices.indices[GRAPHICS_QUEUE_IDX],
                              &command_pool);

    synt::allocate_commandbuffer(device, command_pool, &command_buffer);

    synt::record_commandbuffer(command_buffer);

    bool running = true;
    while (running)
    {
        // vkWaitForFences(device, 1, &fence, VK_TRUE, 0);

        // vkResetFences(device, 1, &fence);

        // VkSubmitInfo submit_info       = {};
        // submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        // submit_info.commandBufferCount = 1;
        // submit_info.waitSemaphoreCount = 1;
        // submit_info.pWaitSemaphores    = &semaphore;
        // submit_info.pCommandBuffers    = &command_buffer;

        // vkQueueSubmit(graphic_queue, 1, &submit_info, fence);

        synt::poll_events();
        if (synt::is_key_pressed(SYNT_Q_PRESSED)) running = false;
    }

    vkDestroyFence(device, fence, NULL);
    vkDestroySemaphore(device, semaphore, NULL);
    vkDestroyCommandPool(device, command_pool, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyInstance(instance, NULL);

    printf("\nComplete!\n");

    return 0;
}

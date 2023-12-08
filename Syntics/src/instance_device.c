#ifndef SY_UNIT_BUILD
#include "instance_device.h"
#include "region_alloc.h"
#include "vulkan_types.h"
#include "logging.h"
#include "platform.h"
#ifdef LINUX
#include <vulkan/vulkan_xcb.h>
#else
#if 0
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include "win32/sy_windows.h"
#endif
#include <vulkan/vulkan_win32.h>
#endif
#include <string.h>

#ifdef DEBUG
global const b8 VALIDATIONS_ENABLE = true;
#else
global const b8 VALIDATIONS_ENABLE = false;
#endif

#endif

b8 validation_enable()
{
    return VALIDATIONS_ENABLE;
}

internal VKAPI_ATTR VkBool32 VKAPI_CALL msg_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{

    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        SY_ERROR(pCallbackData->pMessage);
    }

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        sy_print("VULKAN WARNING: %s\n", pCallbackData->pMessage);
    }

    return VK_TRUE;
}

internal VkDebugUtilsMessengerCreateInfoEXT config_debug_info(void)
{
    VkDebugUtilsMessengerCreateInfoEXT out = { 0 };
    out.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    out.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    out.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    out.pfnUserCallback = msg_callback;

    return out;
}

void debug_messenger_init(Instance_State* state)
{
    if (!VALIDATIONS_ENABLE) return;

    VkDebugUtilsMessengerCreateInfoEXT messenger_info = config_debug_info();

    PFN_vkCreateDebugUtilsMessengerEXT callback =
        (PFN_vkCreateDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(
            state->instance, "vkCreateDebugUtilsMessengerEXT"));

    if (callback)
    {
        if (callback(state->instance, &messenger_info, NULL,
                     &state->debug_messenger))
            SY_ERROR("Failed to initialize debug messenger");
    }
    else
        SY_ERROR("Error extension is not present");
}

void debug_messenger_destroy(VkInstance instance,
                             VkDebugUtilsMessengerEXT debugMessenger,
                             const VkAllocationCallbacks* pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT callback =
        (PFN_vkDestroyDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(
            instance, "vkDestroyDebugUtilsMessengerEXT"));

    if (callback) callback(instance, debugMessenger, pAllocator);
}

void instance_init(VkInstance* instance)
{
    u32 version_supported = 0;
    VK_ASSERT(vkEnumerateInstanceVersion(&version_supported));
#if 0
    synt_LOG("\nVulkan Version: %u.%u.%u.%u\n",
             VK_API_VERSION_VARIANT(version_supported),
             VK_API_VERSION_MAJOR(version_supported),
             VK_API_VERSION_MINOR(version_supported),
             VK_API_VERSION_PATCH(version_supported));
#endif

    VkApplicationInfo app_info = { 0 };
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Sandy";
    app_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    app_info.pEngineName = "Syntics";
    app_info.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_3;

    u32 extension_count = 2;
    const char* extensions[3] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef LINUX
        VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#else
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif
    };

    VkInstanceCreateInfo info = { 0 };
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pApplicationInfo = &app_info;

    if (VALIDATIONS_ENABLE)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = config_debug_info();
        const char* validations[] = { "VK_LAYER_KHRONOS_validation" };
        info.enabledLayerCount = 1;
        info.ppEnabledLayerNames = validations;
        info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

        extensions[extension_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

    info.enabledExtensionCount = extension_count;
    info.ppEnabledExtensionNames = extensions;

#if 0
    synt_LOG("\nExtensions used: \n");
    for (u32i = 0; i < extension_count; i++)
        synt_LOG("\t%s\n", extensions[i]);
    synt_LOG("\n");
#endif

    *instance = VK_NULL_HANDLE;

    VK_ASSERT(vkCreateInstance(&info, NULL, instance));
}

Queue_Family_Indices queue_indices_get(Region_Alloc* region,
                                       VkPhysicalDevice physical_device,
                                       VkSurfaceKHR surface, b8* all_supported)
{
    stack_begin_scope(indi_stack);

    u32 queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, NULL);

    VkQueueFamilyProperties* queue_props =
        stack_malloc(queue_count, VkQueueFamilyProperties);

    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count,
                                             queue_props);

    Queue_Family_Indices indices = { 0 };
    b8 graphic_supported = false;
    b8 presentation_supported = false;
    for (u32 i = 0; i < queue_count; i++)
    {
        if (queue_props[i].queueCount > 0 &&
            (queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) ==
                VK_QUEUE_GRAPHICS_BIT)
        {
            indices.indices[GRAPHICS_QUEUE_IDX] = i;

            graphic_supported = true;
        }
        VkBool32 surface_support = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface,
                                             &surface_support);
        // TODO: presentation could be different from graphic support
        if (surface_support && graphic_supported &&
            indices.indices[GRAPHICS_QUEUE_IDX] == i)
        {
            presentation_supported = true;
        }
    }
    *all_supported = graphic_supported && presentation_supported;

    b8 dublicate = false;
    indices.num_index_fam = 0;
    for (u32 i = 0; i < sy_SIZE(indices.indices); i++)
    {
        for (u32 j = 0; j < i; j++)
        {
            if (indices.indices[i] == indices.indices[j])
            {
                dublicate = true;
            }
        }
        if (!dublicate) indices.num_index_fam++;

        dublicate = false;
        dublicate = true;
    }

    stack_end_scope(indi_stack);
    return indices;
}

void physical_device_pick(Region_Alloc* region, VkInstance instance,
                          VkSurfaceKHR surface, VkPhysicalDevice* physical_device,
                          Queue_Family_Indices* q_indices)
{
    stack_begin_scope(phy_device_stack);

    u32 device_count = 0;
    VK_ASSERT(vkEnumeratePhysicalDevices(instance, &device_count, NULL));

    VkPhysicalDevice* physical_devices =
        stack_malloc(device_count, VkPhysicalDevice);

    VK_ASSERT(vkEnumeratePhysicalDevices(instance, &device_count, physical_devices));

    *physical_device = VK_NULL_HANDLE;

    VkPhysicalDeviceProperties* phy_device_props =
        stack_malloc(device_count, VkPhysicalDeviceProperties);

    b8 supported = false;
    for (u32 i = 0; i < device_count; i++)
    {
        vkGetPhysicalDeviceProperties(physical_devices[i], phy_device_props + i);
        *q_indices =
            queue_indices_get(region, physical_devices[i], surface, &supported);
        if (supported)
        {
            *physical_device = physical_devices[i];
            break;
        }
    }
    ASSERT(physical_device, "Physical_device null");

    stack_end_scope(phy_device_stack);
}

void logical_device_create(VkPhysicalDevice physical_device,
                           Queue_Family_Indices q_indices, VkDevice* device)
{
    *device = VK_NULL_HANDLE;

    f32 queue_prio = 1.0f;
    VkDeviceQueueCreateInfo queue_infos[sy_SIZE(q_indices.indices)] = { 0 };

    for (u32 i = 0; i < q_indices.num_index_fam; i++)
    {
        VkDeviceQueueCreateInfo queue_info = { 0 };
        queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info.queueCount = 1;
        queue_info.pQueuePriorities = &queue_prio;
        queue_info.queueFamilyIndex = q_indices.indices[i];

        queue_infos[i] = queue_info;
    }
    const char* extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo device_info = { 0 };
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.queueCreateInfoCount = q_indices.num_index_fam;
    device_info.pQueueCreateInfos = queue_infos;
    device_info.enabledExtensionCount = sy_SIZE(extensions);
    device_info.ppEnabledExtensionNames = extensions;

    VkPhysicalDeviceFeatures pdf = { 0 };
    vkGetPhysicalDeviceFeatures(physical_device, &pdf);

    VkBool32 wide_lines = pdf.wideLines;
    VkBool32 fill_mode_non_solid = pdf.fillModeNonSolid;

    memset(&pdf, 0, sizeof(pdf));
    pdf.wideLines = wide_lines;
    pdf.fillModeNonSolid = fill_mode_non_solid;
    if (wide_lines || fill_mode_non_solid)
    {
        device_info.pEnabledFeatures = &pdf;
    }
    VK_ASSERT(vkCreateDevice(physical_device, &device_info, NULL, device));
}

#ifdef LINUX
void surface_create(Platform* platform, VkInstance instance, VkSurfaceKHR* surface)
{
    VkXcbSurfaceCreateInfoKHR surface_info = { 0 };
    surface_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surface_info.connection = platform_connection_get(platform);
    surface_info.window = platform_window_get(platform);

    *surface = VK_NULL_HANDLE;
    VK_ASSERT(vkCreateXcbSurfaceKHR(instance, &surface_info, NULL, surface));
}
#else
void surface_create(Platform* platform, VkInstance instance, VkSurfaceKHR* surface)
{
    VkWin32SurfaceCreateInfoKHR surface_info = { 0 };
    surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_info.hwnd = platform_window_get(platform);
    surface_info.hinstance = GetModuleHandle(0);

    *surface = VK_NULL_HANDLE;
    VK_ASSERT(vkCreateWin32SurfaceKHR(instance, &surface_info, NULL, surface));
}
#endif

void instance_destroy(Instance_State* state)
{
    debug_messenger_destroy(state->instance, state->debug_messenger, NULL);
    vkDestroyInstance(state->instance, NULL);
}

